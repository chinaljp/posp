/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   wallet.c
 * Author: pangpang
 *
 * Created on 2017年4月28日, 上午11:31
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "t_redis.h"
#include "settle_date.h"
#include "t_macro.h"
#include "status.h"

typedef struct {
    char sTransCode[6 + 1];
    char sPostType[4 + 1];
} PostType;
/* 入账类型和交易码转换 */
PostType g_staPostType[] = {
    /* pos */
    {"02W100", "1200"},
    {"02W200", "1200"},
    {"02W300", "1200"},
    {"02B100", "1200"},
    {"02B200", "1200"},
    {"02B300", "1200"},
    {"02W600", "2200"},
    {"02B600", "2200"},
    {"02Y600", "2200"},
    /* 手机app */
    {"0AW100", "1200"},
    {"0AW200", "1200"},
    {"0AW300", "1200"}, 
    {"0AB100", "1200"},
    {"0AB200", "1200"},
    {"0AB300", "1200"},
    {"0AB600", "2200"},
    {"0AW600", "2200"}
};

void GetPostType(char *pcPostType, char *pcTransCode) {
    int i;
    for (i = 0; i < 64; i++) {
        if (!strcmp(g_staPostType[i].sTransCode, pcTransCode)) {
            strcpy(pcPostType, g_staPostType[i].sPostType);
        }                 
    }
}

void Net2Wallet(cJSON *pstTransJson, cJSON *pstNetJson) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = {0}, sSettleDate[8 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sOrrn[RRN_LEN + 1]  = {0}, sTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransType[1 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sCardType[1 + 1] = {0}, sInputMode[3 + 1] = {0}, sPostType[4 + 1] = {0};

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "o_rrn", sOrrn);//微信、支付宝 二维码查询交易 支付成功时 入账处理 需要使用原交易rrn
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "sys_trace", sTrace);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "settle_date", sSettleDate);
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    //组请求报文
    SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
    SET_STR_KEY(pstNetJson, "trans_type", "INLINE"); //用于区分银行卡还是二维码的交易
    
    if( sTransCode[3] == '3' ) {
        //微信、支付宝 二维码查询交易 支付成功时 入账处理 rrn使用原交易的sOrrn
        SET_STR_KEY(pstNetJson, "rrn", sOrrn);
    }
    else {
        SET_STR_KEY(pstNetJson, "rrn", sRrn);
    }
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY(pstNetJson, "transAmount", dAmount / 100);
    SET_DOU_KEY(pstNetJson, "settleAmount", dAmount / 100 - dFee);
    SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
    SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
    SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
    SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);
    /* 
     * 210、二维码D0可提现业务冻结钱包
     * 211、二维码D0待入账业务冻结钱包(保留)
     *  */
    SET_STR_KEY(pstNetJson, "walletType", "200"); //钱包类型    
    GetPostType(sPostType, sTransCode);
    SET_STR_KEY(pstNetJson, "postType", sPostType);
}

/* 二维码主扫入账 ,失败不处理*/
int AddQrWallet(cJSON *pstTransJson, int *piFlag) {
    cJSON  *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "交易[%s]失败[%s],无钱包入账.", sRrn, sRespCode);
        return 0;
    }
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return 0;
    }
    Net2Wallet(pstTransJson, pstNetJson);
    SET_STR_KEY(pstNetJson, "trans_code", "00T600");
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T600%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    //iRet = tSvcCall("00T600_Q", pstNetJson, &pstRecvJson, 30);
   /*iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    
    if (iRet < 0) {
        tLog(DEBUG, "二维码交易[%s]钱包入账失败.", sRrn);
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
        if (memcmp(sRespCode, "00", 2)) {
            tLog(ERROR, "钱包入账失败[%s:%s].", sRespCode, sResvDesc);
        } else {
            tLog(DEBUG, "交易[%s]钱包入账成功.", sRrn);
        }
    }
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);
    */
    iRet = tSvcACall("WALLET_Q", &stQMsgData);
    if (iRet < 0)
    {
        if (MSG_TIMEOUT == iRet)
        {
            ErrHanding(pstTransJson, sRespCode, "钱包入账超时,忽略.");
        } else
            ErrHanding(pstTransJson, sRespCode, "钱包入账失败,忽略.");
    } 
    else
    {
        tLog(INFO, "钱包入账请求发送成功.");
    }
    cJSON_Delete(pstNetJson);
    
    return 0;
}

void Net2FrozenWallet(cJSON *pstTransJson, cJSON *pstNetJson) {
    char sMerchId[15 + 1] = {0}, sUserCode[15 + 1] = {0}, sTransType[1 + 1] = {0};
    char sRrn[12 + 1] = {0}, sTrace[7] = {0};
    double dAmount = 0.0, dFee = 0.0;
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "sys_trace", sTrace);
    SET_STR_KEY(pstNetJson, "walletType", "200");
    SET_STR_KEY(pstNetJson, "frozenType", "31");
    SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
    SET_STR_KEY(pstNetJson, "user_code", sUserCode);
    SET_DOU_KEY(pstNetJson, "amount", dAmount / 100 - dFee);
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
}

int FrozenWallet(cJSON *pstTransJson, int *piFlag) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char  sRrn[RRN_LEN + 1] = {0}, sRespCode[2 + 1] = {0}, sResvDesc[4096 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    Net2FrozenWallet(pstTransJson, pstNetJson);
    SET_STR_KEY(pstNetJson, "trans_code", "00T400");

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T400%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            ErrHanding(pstTransJson, "96", "交易[%s]钱包冻结通知超时.", sRrn);
        } else
            ErrHanding(pstTransJson, "96", "交易[%s]钱包冻结通知失败.", sRrn);
        cJSON_Delete(pstNetJson);
        return -1;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
    if (memcmp(sRespCode, "00", 2)) {
        ErrHanding(pstTransJson, "96", "钱包冻结失败[%s:%s].", sRespCode, sResvDesc);
    } else
        tLog(INFO, "交易[%s]钱包冻结成功.", sRrn);
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);

    return 0;
}

void Net2UnFrozenWallet(cJSON *pstTransJson, cJSON *pstNetJson) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = {0}, sSettleDate[8 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransType[1 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sCardType[1 + 1] = {0}, sInputMode[3 + 1] = {0}, sPostType[4 + 1] = {0};

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "sys_trace", sTrace);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "settle_date", sSettleDate);
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    //组请求报文
    SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
    SET_STR_KEY(pstNetJson, "trans_type", "INLINE"); //用于区分银行卡还是二维码的交易
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY(pstNetJson, "transAmount", dAmount / 100);
    SET_DOU_KEY(pstNetJson, "settleAmount", dAmount / 100 - dFee);
    SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
    SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
    SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
    SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);
    SET_STR_KEY(pstNetJson, "walletType", "210");

    GetPostType(sPostType, sTransCode);
    SET_STR_KEY(pstNetJson, "postType", sPostType);
}

int UnFrozenWallet(cJSON *pstTransJson, int *piFlag) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    if (!memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "交易[%s]成功,恢复钱包.", sRrn);
        Net2UnFrozenWallet(pstTransJson, pstNetJson);
        SET_STR_KEY(pstNetJson, "trans_code", "00T600");
        tGetUniqueKey(stQMsgData.sSvrId);
        snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T600%s", sRrn);
        stQMsgData.pstDataJson = pstNetJson;
        iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
        //iRet = tSvcCall("00T600_Q", pstNetJson, &pstRecvJson, 30);
        if (iRet < 0) {
            if (MSG_TIMEOUT == iRet) {
                ErrHanding(pstTransJson, "96", "交易[%s]钱包解冻超时.", sRrn);
            } else
                tLog(ERROR, "交易[%s]钱包解冻失败.", sRrn);
        } else {
            pstRecvDataJson = stPMsgData.pstDataJson;
            GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
            GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
            if (memcmp(sRespCode, "00", 2)) {
                tLog(ERROR, "钱包解冻入账失败[%s:%s].", sRespCode, sResvDesc);
            } else {
                tLog(INFO, "交易[%s]钱包解冻入账成功.", sRrn);
            }
        }
        cJSON_Delete(pstNetJson);
        if (NULL != pstRecvDataJson)
            cJSON_Delete(pstRecvDataJson);
        return 0;
    }
    tLog(INFO, "交易[%s]失败,解冻钱包......", sRrn);
    Net2FrozenWallet(pstTransJson, pstNetJson);
    SET_STR_KEY(pstNetJson, "trans_code", "00T500");
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T500%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    //iRet = tSvcCall("00T500_Q", pstNetJson, &pstRecvJson, 30);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            ErrHanding(pstTransJson, "96", "交易[%s]钱包解冻超时.", sRrn);
        } else
            tLog(ERROR, "交易[%s]钱包解冻失败.", sRrn);
        cJSON_Delete(pstNetJson);
        return 0;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "钱包解冻失败[%s:%s].", sRespCode, sResvDesc);
    } else {
        tLog(INFO, "交易[%s]钱包解冻成功.", sRrn);
    }
    cJSON_Delete(pstNetJson);
    cJSON_Delete(pstRecvDataJson);
    return 0;
}

/*新增查询交易 返回交易流  查询交易返回 支付成功的结果 则判断原交易流水 是否在异步通知中 进行了钱包充值， 
 * 若没有进行钱包充值则本交易流中进行钱包充值 
 * 钱包充值时 对原交易流水进行加锁（行级索  避免此处进行钱包充值时 异步通知也同时进行钱包充值 导致重复入账）
 */
int AddQrWalletEx(cJSON *pstTransJson, int *piFlag) {
    char sSqlStr[512 + 1] = {0};
    char sRespCode[2 + 1] = {0},sMerchOrderNo[32 + 1] = {0},sRrn[RRN_LEN + 1] = {0};
    int iCnt = 0;
    
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson,"rrn",sRrn);
    GET_STR_KEY(pstTransJson,"qr_order_no",sMerchOrderNo);
    
    if (memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "交易[%s]失败[%s],无钱包入账.", sRrn, sRespCode);
        return 0;
    }
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select count(1) from b_inline_tarns_detail where merch_order_no = '%s' and settle_flag = 'N' "
            "and trans_code in (select trans_code from s_trans_code where settle_flag = '1') ",sMerchOrderNo);
    iCnt = tQueryCount(sSqlStr);
    if (iCnt < 0) {
        tLog(ERROR, "sSqlStr[%s]", sSqlStr);
        return ( -1);
    }
    if (1 == iCnt) {
        tLog(INFO,"订单[%s]在异步通知处理中钱包未入账,进行钱包入账处理... ###iCnt = [%d]",sMerchOrderNo,iCnt );
        AddQrWallet( pstTransJson, piFlag );
        tLog(INFO,"进行钱包入账处理完毕。",sMerchOrderNo);
    }else if (0 == iCnt) {
        tLog(WARN, "此订单[%s]已在异步通知处理中进行了钱包充值,此处不做钱包入账处理！###iCnt = [%d]",sMerchOrderNo,iCnt );
    }
    
    return ( 0 );
}