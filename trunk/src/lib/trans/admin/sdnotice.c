/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "term.h"
#include "t_macro.h"
#include "t_redis.h"

void Net2Notice(cJSON *pstTransJson, cJSON *pstNetJson) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = {0}, sMerchName[60 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDateTime[14 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sMerchOrderNo[30 + 1] = {0}, sAgentId[8 + 1] = {0}, sUrl[20 + 1] = {0}, sPort[10 + 1] = {0}, sPlatcode[20 + 1] = {0};
    cJSON * pstExtDate=NULL;

    pstExtDate = cJSON_CreateObject();
    if (NULL == pstExtDate) {
        ErrHanding(pstTransJson, "96", "创建Json结构失败.", sMerchId);
    }
    
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "merch_name", sMerchName);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "sys_trace", sTrace);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "agent_id", sAgentId);
    GET_STR_KEY(pstTransJson, "merch_order_no", sMerchOrderNo);
    GET_STR_KEY(pstTransJson, "order_url", sUrl);
    GET_STR_KEY(pstTransJson, "order_port", sPort);
    GET_STR_KEY(pstTransJson, "platcode", sPlatcode);

    //组请求报文
    SET_STR_KEY(pstNetJson, "merchantId", sMerchId);
    SET_STR_KEY(pstNetJson, "merchantName", sMerchName);
    SET_STR_KEY(pstNetJson, "platCode", sPlatcode);
    SET_STR_KEY(pstNetJson, "traceNo", sTrace);
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    strcpy(sTransDateTime, sTransDate);
    strcat(sTransDateTime, sTransTime);
    SET_STR_KEY(pstNetJson, "tranTime", sTransDateTime);
    SET_STR_KEY(pstNetJson, "orderId", sMerchOrderNo);
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_STR_KEY(pstNetJson, "url", sUrl);
    SET_STR_KEY(pstNetJson, "port", sPort);
    SET_JSON_KEY(pstNetJson, "extdate", pstExtDate);
    
}

int SdNotice(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL, *pstRecvDataJson = NULL, *pstNetJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0}, sAmount[12 + 1] = {0};
    char *pcMsg = NULL;
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;


    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "rrn", sRrn);

    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return 0;
    }
    /* 获取订单路由信息 */
    if (FindNoticeTransfer(pstTransJson) < 0) {
        ErrHanding(pstTransJson, "96", "获取商户订单路由信息失败.");
        cJSON_Delete(pstNetJson);
        return 0;
    }

    Net2Notice(pstTransJson, pstNetJson);
    SET_STR_KEY(pstNetJson, "tranCode", "1000");

 //   DUMP_JSON(pstNetJson, pcMsg);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "1000%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("ORQUERY_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    //iRet = tSvcCall("NOTICE_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            ErrHanding(pstTransJson, "68", "订单查询交易[%s]超时.", sRrn);
        } else
            ErrHanding(pstTransJson, "96", "订单查询交易[%s]失败.", sRrn);
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "responseCode", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "responseDesc", sResvDesc);
        GET_STR_KEY(pstRecvDataJson, "amount", sAmount);

        if (memcmp(sRespCode, "00", 2)) {
            tLog(ERROR, "订单查询失败[%s:%s].", sRespCode, sResvDesc);
            ErrHanding(pstTransJson, "96", "订单查询交易[%s]失败.", sRrn);
        } else {
            tLog(INFO, "交易[%s]订单查询成功.", sRrn);

            SET_STR_KEY(pstTransJson, "amount", sAmount);
            SET_STR_KEY(pstTransJson, "balance1", sAmount);
            SET_STR_KEY(pstTransJson, "amt_type", "02");
            SET_STR_KEY(pstTransJson, "acct_type", "10");
            SET_STR_KEY(pstTransJson, "balance_sign", "C");
            SET_STR_KEY(pstTransJson, "ccy_code", "156");
            SET_STR_KEY(pstTransJson, "resp_code", sRespCode);
            SET_STR_KEY(pstTransJson, "resp_desc", sResvDesc);
            ErrHanding(pstTransJson, "00", "交易成功");

        }
    }
    cJSON_Delete(pstNetJson);
    cJSON_Delete(pstRecvDataJson);
    return 0;
}


