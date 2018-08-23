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

/* 二维码 */
void QrTransProc() {
    /* 扫描账户通知表,自动发送交易 */
    char sSqlStr[1024] = {0}, sSql[256] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sData[2048] = {0};
    char sReqSvrId[SVRID_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sAcctUuid[64 + 1] = {0}, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0}, sTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sSettleDate[8 + 1] = {0};
    double dAmount = 0.0, dFee = 0.0;
    OCI_Resultset *pstRes = NULL, *pstUpdRes = NULL;
    int iRet = -1;
    cJSON *pstNetJson = NULL;
    MsgData stMsgData;

    tLog(WARN, "开始处理二维码充值......");
    snprintf(sRepSvrId, sizeof (sReqSvrId), "%s_P", GetSvrId());
    //tLog(DEBUG, "p svrid[%s]", sRepSvrId);
    snprintf(sSqlStr, sizeof (sSqlStr), "select MERCH_ID,trans_code,amount,fee,t.rrn, t.sys_trace,trans_date,trans_time,settle_date"
            " from B_INLINE_TARNS_DETAIL t"
            " where t.SETTLE_FLAG='N' and t.fee_flag in ('M','Y') and resp_code='00' and valid_flag='0' "
            //" and sysdate-create_time>=10/60/24" //motify by gjq at 20180403
            " and t.trans_code in (select trans_code from s_trans_code where settle_flag='1' and group_code='INLINE' ) order by t.trans_date,t.trans_time");

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

        pstNetJson = cJSON_CreateObject();
        if (NULL == pstNetJson) {
            tLog(ERROR, "创建netJson失败.");
            continue;
        }

        SET_STR_KEY(pstNetJson, "trans_code", "00T600");
        SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
        SET_STR_KEY(pstNetJson, "rrn", sRrn);
        //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
        SET_DOU_KEY(pstNetJson, "transAmount", dAmount);
        SET_DOU_KEY(pstNetJson, "settleAmount", dAmount - dFee);
        SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
        SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
        SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
        SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);

        /* 包含:脱机消费,消费,订单消费,T0消费 */
        if (!memcmp(sTransCode + 2, "W1", 2)
                || !memcmp(sTransCode + 2, "W2", 2)
                || !memcmp(sTransCode + 2, "B1", 2)
                || !memcmp(sTransCode + 2, "B2", 2)
                || !memcmp(sTransCode + 2, "Y1", 2)
                || !memcmp(sTransCode + 2, "Y2", 2)) {
            SET_STR_KEY(pstNetJson, "trans_type", "INLINE"); //交易类型：1：pos 2：二维码
            SET_STR_KEY(pstNetJson, "walletType", "200");
            SET_STR_KEY(pstNetJson, "postType", "1200");
        }
        else if ( !memcmp( sTransCode + 2, "Q0", 2 ) ) {
            SET_STR_KEY(pstNetJson, "trans_type", "QUICKPAY"); // QUICKPAY  Epos快捷无卡交易
            SET_STR_KEY(pstNetJson, "walletType", "400");
            SET_STR_KEY(pstNetJson, "postType", "1500");
        }
        /* 二维码撤销 */
        else if (!memcmp(sTransCode + 2, "W6", 2)
                || !memcmp(sTransCode + 2, "B6", 2)
                || !memcmp(sTransCode + 2, "Y6", 2)) {
            SET_STR_KEY(pstNetJson, "trans_type", "INLINE"); //交易类型：1：pos 2：二维码
            SET_STR_KEY(pstNetJson, "walletType", "200");
            SET_STR_KEY(pstNetJson, "postType", "2200");
        } else {
            tLog(ERROR, "交易码[%s]不是需要处理的交易.", sTransCode);
            cJSON_Delete(pstNetJson);
            continue;
        }
        /* 发送新的消息 */
        tGetUniqueKey(sKey);
        snprintf(sReqSvrId, sizeof (sReqSvrId), "%s_Q", "00T600");
        strcpy(stMsgData.sKey, sKey);
        stMsgData.pstDataJson = pstNetJson;
        iRet = tSvcACall(sReqSvrId, &stMsgData);
        //iRet = tSvcACall(sReqSvrId, sRepSvrId, sKey, pstNetJson);
        cJSON_Delete(pstNetJson);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "无二维码充值交易通知.");
        tReleaseRes(pstRes);
        return 0;
    }
    tReleaseRes(pstRes);
    return 0;
}