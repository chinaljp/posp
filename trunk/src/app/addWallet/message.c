/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"
#include "t_macro.h"

typedef struct {
    char sTransCode[6 + 1];
    char sPostType[4 + 1];
} PostType;
/* 入账类型和交易码转换 */
PostType g_staPostType[] = {
    {"020000", "1000"},
    {"020003", "2001"},
    {"020002", "2000"},
    {"020023", "1001"},
    {"020001", "2002"},
    /* 预授权 */
    {"024100", "1010"},
    {"024102", "2010"},
    {"024103", "2011"},
    {"024123", "1011"},
    {"T20000", "1000"},
    {"T20003", "2001"},
    {"U20000", "1000"},
    {"M20000", "1000"},
    {"M20003", "2001"},
    {"M20002", "2000"},
    {"M20023", "1001"},
    {"M20001", "2002"}
};
int GetPostType(char *pcPostType, char *pcTransCode) {
    int i;
    for (i = 0; i < 128; i++) {
        if (!strcmp(g_staPostType[i].sTransCode, pcTransCode)) {
            strcpy(pcPostType, g_staPostType[i].sPostType);
            return 0;
        }
    }
    return -1;
}
/* 充值 */
void PosTransProc() {
    /* 扫描账户通知表,自动发送交易 */
    char sSqlStr[1024] = {0}, sSql[256] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sData[2048] = {0};
    char sReqSvrId[SVRID_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sAcctUuid[64 + 1] = {0}, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0}, sTrace[6 + 1] = {0}, *pcMsg = NULL;
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sSettleDate[8 + 1] = {0}, sTransType[2] = {0};
    char sPostType[16] = {0};
    double dAmount = 0.0, dFee = 0.0;
    OCI_Resultset *pstRes = NULL, *pstUpdRes = NULL;
    int iRet = -1;
    cJSON *pstNetJson = NULL, *pcRecvJson = NULL;
    MsgData stQMsgData, stPMsgData;

    tLog(WARN, "开始处理pos入账......");
    //tLog(DEBUG, "p svrid[%s]", sRepSvrId);
    snprintf(sSqlStr, sizeof (sSqlStr), "select MERCH_ID,trans_code,amount,fee,t.rrn, t.sys_trace,trans_date,trans_time"
            ",settle_date,trans_type from B_POS_TRANS_DETAIL t"
            " where t.SETTLE_FLAG='N' and istresp_code='00' and t.fee_flag in ('M','Y') and resp_code='00' and valid_flag='0' "
            " and sysdate-create_time>=10/60/24 "
            " and t.trans_code in (select trans_code from s_trans_code where settle_flag='1' and group_code='POS' ) order by t.trans_date,t.trans_time");
    //and sysdate-create_time>=10/60/24 十分钟前
    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sMerchId);
        STRV(pstRes, 2, sTransCode);
        DOUV(pstRes, 3, dAmount);
        DOUV(pstRes, 4, dFee);
        //STRV(pstRes, 5, sAcctUuid);
        STRV(pstRes, 5, sRrn);
        STRV(pstRes, 6, sTrace);
        STRV(pstRes, 7, sTransDate);
        STRV(pstRes, 8, sTransTime);
        STRV(pstRes, 9, sSettleDate);
        STRV(pstRes, 10, sTransType);

        pstNetJson = cJSON_CreateObject();
        if (NULL == pstNetJson) {
            tLog(ERROR, "创建netJson失败.");
            continue;
        }

        SET_STR_KEY(pstNetJson, "trans_code", "00T600");
        SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
        SET_STR_KEY(pstNetJson, "trans_type", "POS"); //用于区分银行卡还是二维码的交易
        SET_STR_KEY(pstNetJson, "rrn", sRrn);
        //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
        SET_DOU_KEY(pstNetJson, "transAmount", dAmount);
        SET_DOU_KEY(pstNetJson, "settleAmount", dAmount - dFee);
        SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
        SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
        SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
        SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);
        //SET_STR_KEY(pstNetJson, "walletType", "101"); //钱包类型
        if (GetPostType(sPostType, sTransCode) < 0) {
            tLog(ERROR, "交易码[%s]不是需要处理的交易.", sTransCode);
            cJSON_Delete(pstNetJson);
            continue;
        }
        SET_STR_KEY(pstNetJson, "postType", sPostType);
#if 0
        if (sTransType[0] == '0') {
            SET_STR_KEY(pstNetJson, "walletType", "100"); //钱包类型           
        } else {
            SET_STR_KEY(pstNetJson, "walletType", "101"); //钱包类型
        }
#endif
        if (sTransType[0] == '0' || sTransType[0] == 'S' || sTransType[0] == '1') {
        /* 包含:脱机消费,消费,订单消费,T0消费 */
            SET_STR_KEY(pstNetJson, "walletType", "100"); //钱包类型
        } else if (sTransType[0] == '2' || sTransType[0] == '3') {
            SET_STR_KEY(pstNetJson, "walletType", "101"); //钱包类型
            /* POS撤销入账 */
            /* POS冲正入账 */
            /* POS预授权完成撤销入账 */
        }

        /* 发送新的消息 */
        //DUMP_JSON(pstNetJson, pcMsg);
        tGetUniqueKey(stQMsgData.sKey);
        stQMsgData.pstDataJson = pstNetJson;
        strcpy(stQMsgData.sSvrId, stQMsgData.sKey);
        iRet = tSvcCall("00T600_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
        //iRet = tSvcCall("00T600_Q", pstNetJson, &pcRecvJson, 30);
        DUMP_JSON(pcRecvJson, pcMsg);
        cJSON_Delete(pstNetJson);
    }
    tLog(DEBUG, "共处理[%d]条记录.", OCI_GetRowCount(pstRes));
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "无POS充值交易通知.");
        tReleaseRes(pstRes);
        return 0;
    }
    tReleaseRes(pstRes);
    return 0;
}
#if 0

/* 充值 */
void UnPosTransProc() {
    /* 扫描账户通知表,自动发送交易 */
    char sSqlStr[1024] = {0}, sSql[256] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sData[2048] = {0};
    char sReqSvrId[SVRID_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sAcctUuid[64 + 1] = {0}, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0}, sTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sSettleDate[8 + 1] = {0}, sTransType[2] = {0};
    double dAmount = 0.0, dFee = 0.0;
    OCI_Resultset *pstRes = NULL, *pstUpdRes = NULL;
    int iRet = -1;
    cJSON *pstNetJson = NULL, *pcRecvJson = NULL;
    tLog(WARN, "开始处理pos充值......");
    //tLog(DEBUG, "p svrid[%s]", sRepSvrId);
    snprintf(sSqlStr, sizeof (sSqlStr), "select MERCH_ID,trans_code,amount,fee,t.rrn, t.sys_trace,trans_date,trans_time"
            ",settle_date,trans_type from B_POS_TRANS_DETAIL t"
            " where t.SETTLE_FLAG='N' and t.fee_flag in ('M','Y') and resp_code='00' and trans_type='2' "
            " and t.trans_code in (select trans_code from s_trans_code where settle_flag='1' and group_code='POS' and trans_code !='T20000' ) order by t.trans_date,t.trans_time");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sMerchId);
        STRV(pstRes, 2, sTransCode);
        DOUV(pstRes, 3, dAmount);
        DOUV(pstRes, 4, dFee);
        //STRV(pstRes, 5, sAcctUuid);
        STRV(pstRes, 5, sRrn);
        STRV(pstRes, 6, sTrace);
        STRV(pstRes, 7, sTransDate);
        STRV(pstRes, 8, sTransTime);
        STRV(pstRes, 9, sSettleDate);
        STRV(pstRes, 10, sTransType);

        pstNetJson = cJSON_CreateObject();
        if (NULL == pstNetJson) {
            tLog(ERROR, "创建netJson失败.");
            continue;
        }

        SET_STR_KEY(pstNetJson, "trans_code", "00T600");
        SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
        SET_STR_KEY(pstNetJson, "trans_type", "POS"); //用于区分银行卡还是二维码的交易
        SET_STR_KEY(pstNetJson, "rrn", sRrn);
        //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
        SET_DOU_KEY(pstNetJson, "transAmount", dAmount);
        SET_DOU_KEY(pstNetJson, "settleAmount", dAmount - dFee);
        SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
        SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
        SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
        SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);
        SET_STR_KEY(pstNetJson, "walletType", "100"); //钱包类型  
#if 0
        if (sTransType[0] == '0') {
            SET_STR_KEY(pstNetJson, "walletType", "100"); //钱包类型           
        } else {
            SET_STR_KEY(pstNetJson, "walletType", "101"); //钱包类型
        }
#endif

        /* 包含:脱机消费,消费,订单消费,T0消费 */
        if (!memcmp(sTransCode + 2, "0000", 4)) {
            SET_STR_KEY(pstNetJson, "postType", "1000");
            /*  POS预授权完成入账*/
        } else if (!memcmp(sTransCode, "024100", 6)) {
            SET_STR_KEY(pstNetJson, "postType", "1010");
            /* POS撤销入账 */
        } else if (!memcmp(sTransCode + 2, "0002", 2)) {
            SET_STR_KEY(pstNetJson, "postType", "2000");
            /* POS冲正入账 */
        } else if (!memcmp(sTransCode + 2, "0003", 2) ||
                !memcmp(sTransCode + 2, "0023", 2)) {
            SET_STR_KEY(pstNetJson, "postType", "2001");
            /* POS预授权完成撤销入账 */
        } else if (!memcmp(sTransCode + 2, "4102", 2)) {
            SET_STR_KEY(pstNetJson, "postType", "2010");
        } else if (!memcmp(sTransCode + 2, "4103", 2)) {
            SET_STR_KEY(pstNetJson, "postType", "2011");
        } else {
            tLog(ERROR, "交易码[%s]不是需要处理的交易.", sTransCode);
            cJSON_Delete(pstNetJson);
            continue;
        }

        /* 发送新的消息 */
        tGetUniqueKey(sKey);
        snprintf(sReqSvrId, sizeof (sReqSvrId), "%s_Q", "00T600");
        DUMP_JSON(pstNetJson);
        iRet = tSvcCall("00T600_Q", pstNetJson, &pcRecvJson, 30);
        DUMP_JSON(pcRecvJson);
        cJSON_Delete(pstNetJson);
    }
    tLog(DEBUG, "共处理[%d]条记录.", OCI_GetRowCount(pstRes));
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "无POS充值交易通知.");
        tReleaseRes(pstRes);
        return 0;
    }
    tReleaseRes(pstRes);
    return 0;
}
#endif