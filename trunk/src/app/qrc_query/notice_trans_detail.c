/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"

//extern char* GetNoticeCnt();
#define QUERY_CNT  6

int FindQRQueryInterval(long lWeight[QUERY_CNT]) {

    char sSqlStr[1024] = {0}, sInterval[50] = {0}, sTmp[50] = {0};
    char *pInterval;
    int i;
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;
    memset(sSqlStr, 0, sizeof (sSqlStr));

    snprintf(sSqlStr, sizeof (sSqlStr), "select  key_value FROM \
    S_PARAM where KEY = 'QR_AUTO_QUERY_INTERVAL'");
    
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "FindQRQueryInterval失败.");
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sInterval);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
 
    tLog(INFO, "二维码自动查询时间间隔设置[%s]", sInterval);
    pInterval = sInterval;
    for (i = 0; i < QUERY_CNT; i++) {
        tGetOneField(&pInterval, ',', sTmp);
        lWeight[i] = atol(sTmp);
        tLog(DEBUG, "lWeight[%d]=[%ld].", i, lWeight[i]);
    }
    return 0;
}

int JudgeTime(char *pcTransmitTime, int iQueryCount) {
    time_t tCTime, tCntTime;
    time_t tTransTime;
    struct tm stm;
    long lWeight[QUERY_CNT] = {0};
    int i, iTimeIntv;

    //获得当前时间
    time(&tCTime);
    strptime(pcTransmitTime, "%Y%m%d%H%M%S", &stm);
    tTransTime = mktime(&stm);
    tCntTime = tCTime - tTransTime;
    tLog(ERROR, "下单时间[%s]", pcTransmitTime);
    tLog(ERROR, "当前查询次数[%d],当前时间戳[%ld],下单时间戳[%ld],间隔时间[%ld]", iQueryCount, tCTime, tTransTime, tCntTime);
    if (FindQRQueryInterval(&lWeight) < 0) {
        tLog(ERROR, "未设定二维码自动查询间隔参数");
    }
    iTimeIntv = GetTimeInterval();
    tLog(ERROR, "二维码扫表间隔[%d]", iTimeIntv);
    for (i = 0; i < QUERY_CNT; i++) {
        tLog(ERROR, "设定查询间隔时间[%ld]", lWeight[iQueryCount]);
        if (abs(tCntTime - lWeight[iQueryCount]) <= iTimeIntv) {
            return 0;
        }
        else {
            return -1;
        }
    }
    return -1;
}

int UpdQRCnt(cJSON *pstJson) {
    char sSqlStr[512] = {0};
    char sDate[9] = {0}, sTime[7] = {0};
    char sRrn[12 + 1] = {0};
    int iCnt = 0, iResCnt = 0;
    char sTranTime[14 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDate[8 + 1] = {0};
    char sMerchOrderNo[26 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "tranTime", sTranTime);
    GET_INT_KEY(pstJson, "query_count", iCnt);
    GET_STR_KEY(pstJson, "qr_order_no", sMerchOrderNo);

    strncpy(sTransDate, sTranTime, 8);
    strcpy(sTransTime, sTranTime + 8);
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);


    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET "
            " QUERY_COUNT=QUERY_COUNT+1 WHERE MERCH_ORDER_NO='%s' and amount>0 "
            , sMerchOrderNo);
    tLog(INFO, "sql[%s]", sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tCommit();
    tLog(ERROR, "交易订单[%s]查询次数+1", sMerchOrderNo);
    tLog(DEBUG, "影响记录数[%d]", iResCnt);
    return 0;
}

int ScanQrTrans() {

    char sSqlStr[2048];
    char sRrn[RRN_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0};
    char sTransDate[TRANS_DATE_LEN] = {0}, sTransTime[TRANS_TIME_LEN + 1] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTransmitTime[14 + 1] = {0};
    char sTraceNo[TRACE_NO_LEN + 1] = {0}, sReqSvrId[64] = {0};
    char sAgentId[INST_ID_LEN + 1] = {0}, sTransDateTime[14 + 1] = {0};
    double dAmount = 0;
    char sAmount[12 + 1] = {0}, sOrderNo[30 + 1] = {0};
    char sUrl[100 + 1] = {0}, sTermId[8 + 1] = {0};
    char sPlatcode[20 + 1] = {0};
    int iQrCount;

    cJSON *pstJson = NULL;
    char sKey[64] = {0};


    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
    TRANS_CODE,\
    AMOUNT,\
    TRANS_DATE, \
    TRANS_TIME, \
    MERCH_ID, \
    TERM_ID, \
    TRACE_NO, \
    RRN, \
    AGENT_ID, \
    MERCH_ORDER_NO, \
    ORDER_URL, \
    QUERY_COUNT \
    FROM B_INLINE_TARNS_DETAIL \
    WHERE query_count < %d and substr(trans_code,3,2) in('W2','B2','W1','B1','Y2','Y1') and valid_flag='4' and resp_code='00'and sysdate-create_time<=6/60/24 ", QUERY_CNT);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        pstJson = cJSON_CreateObject();
        if (NULL == pstJson) {
            tLog(ERROR, "创建发送Json失败.");
            return 0;
        }

        STRV(pstRes, 1, sTransCode);
        DOUV(pstRes, 2, dAmount);
        STRV(pstRes, 3, sTransDate);
        STRV(pstRes, 4, sTransTime);
        STRV(pstRes, 5, sMerchId);
        STRV(pstRes, 6, sTermId);
        STRV(pstRes, 7, sTraceNo);
        STRV(pstRes, 8, sRrn);
        STRV(pstRes, 9, sAgentId);
        STRV(pstRes, 10, sOrderNo);
        STRV(pstRes, 11, sUrl);
        INTV(pstRes, 12, iQrCount);
        //STRV(pstRes, 12, sPlatcode);
        if (sTransCode[3] == '2') {
            sTransCode[3] = sTransCode[3] + 1;
        } else if (sTransCode[3] == '1') {
            sTransCode[3] = sTransCode[3] + 2;
        }

        SET_STR_KEY(pstJson,"head", "612100311000");
        SET_STR_KEY(pstJson, "trans_code", sTransCode);
        tLog(INFO, "当前订单号[%s].", sOrderNo);
        SET_STR_KEY(pstJson, "merch_id", sMerchId);
        SET_STR_KEY(pstJson, "traceNo", sTraceNo);
        SET_STR_KEY(pstJson, "rrn", sRrn);
        SET_STR_KEY(pstJson, "trans_date", sTransDate);
        SET_STR_KEY(pstJson, "trans_time", sTransTime);
        SET_STR_KEY(pstJson, "term_id", sTermId);
        strcpy(sTransDateTime, sTransDate);
        strcat(sTransDateTime, sTransTime);
        SET_STR_KEY(pstJson, "tranTime", sTransDateTime);
        SET_STR_KEY(pstJson, "qr_order_no", sOrderNo);
        SET_DOU_KEY(pstJson, "amount", dAmount);
        SET_STR_KEY(pstJson, "rrn", sRrn);
        //SET_STR_KEY(pstJson, "platCode", sPlatcode);
        //SET_STR_KEY(pstJson, "url", sUrl);
        SET_INT_KEY(pstJson, "query_count", iQrCount);

        tLog(INFO, "检索到二维码流水信息.");
        /* 发送二维码查询交易 */
        snprintf(sKey, sizeof (sKey), "%s_%s", GetSvrId(), sRrn);

        sprintf(sReqSvrId, "%s_Q", sTransCode);
        sprintf(sTransmitTime, "%s%s", sTransDate, sTransTime);
        if (!JudgeTime(sTransmitTime, iQrCount)) {
            SendMsg(sReqSvrId, sKey, pstJson);
            tLog(ERROR, "二维码[%s]请求发送成功！", sOrderNo);

            if (UpdQRCnt(pstJson) < 0) {
                tLog(ERROR, "更新二维码[%s]查询次数[%d]失败！", sOrderNo, iQrCount);
                return -1;
            }
        }
        cJSON_Delete(pstJson);

    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未检索到待发送的二维码流水.....");
        tReleaseRes(pstRes);
        return -2;
    }
    tReleaseRes(pstRes);
    return 0;
}