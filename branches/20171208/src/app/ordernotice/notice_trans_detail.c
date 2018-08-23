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

#define SAF_FIELDS "TRANS_CODE,CARD_NO,CARD_ID,CARD_TYPE,CARD_EXP_DATE,AMOUNT" \
",TRANSMIT_TIME,TRANS_DATE,TRANS_TIME,SYS_TRACE,LOGIC_DATE,MERCH_ID,TERM_ID,TERM_SN" \
",TRACE_NO,RRN,MCC,INPUT_MODE,COND_CODE,PIN_CODE,ACQ_ID,ISS_ID,AGENT_ID,CHANNEL_ID" \
",CHANNEL_MERCH_ID,CHANNEL_TERM_ID,CHANNEL_TERM_SN,CHANNEL_MCC,AUTHO_FLAG,BATCH_NO" \
",AUTH_CODE,IC_DATA,RESP_CODE,ISTRESP_CODE,RESP_DESC,RESP_ID,MERCH_ORDER_NO,O_RRN" \
",SAF_DATE,SAF_TIME,SAF_CNT,SAF_STATUS,CREATE_TIME,LAST_MOD_TIME,ORDER_URL,ORDER_PORT,PLATCODE"

int AddNoticeLs(cJSON *pstJson) {
    char sSqlStr[1024] = {0};
    char sRrn[RRN_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0}, sCardNo[CARD_NO_LEN + 1] = {0};
    char sCardId[CARD_ID_LEN + 1] = {0}, sCardType[CARD_TYPE_LEN + 1] = {0}, sCardExpDate[EXP_DATE_LEN + 1] = {0};
    char sTransmitTime[TRANSMIT_TIME_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN] = {0}, sTransTime[TRANS_TIME_LEN + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0}, sLogicDate[LOGIC_DATE_LEN + 1] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0};
    char sTermId[TERM_ID_LEN + 1] = {0}, sTermSn[SN_LEN + 1] = {0}, sTraceNo[TRACE_NO_LEN + 1] = {0}, sMcc[MCC_LEN + 1] = {0};
    char sInputMode[INPUT_MODE_LEN + 1] = {0}, sCondCode[COND_CODE_LEN + 1] = {0}, sPinCode[PIN_CODE_LEN + 1] = {0};
    char sAcqId[INST_ID_LEN + 1] = {0}, sIssId[INST_ID_LEN + 1] = {0}, sAgentId[INST_ID_LEN + 1] = {0}, sChannelId[INST_ID_LEN + 1] = {0};
    char sChannelMerchId[MERCH_ID_LEN + 1] = {0}, sChannelTermId[TERM_ID_LEN + 1] = {0};
    char sChannelTermSn[SN_LEN + 1] = {0}, sChannelMcc[MCC_LEN + 1] = {0}, sAuthoFlag[AUTHO_FLAG_LEN + 1] = {0};
    char sBatchNo[BATCH_NO_LEN + 1] = {0}, sAuthCode[AUTH_CODE_LEN + 1] = {0}, sIcData[IC_DATA_LEN + 1] = {0};
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sIstRespCode[RESP_CODE_LEN + 1] = {0}, sRespLog[RESP_LOG_LEN + 1] = {0};
    char sRespId[INST_ID_LEN + 1] = {0}, sMerchOrderNo[ORDER_NO_LEN] = {0};
    char sOrrn[RRN_LEN + 1] = {0}, sOTransDate[TRANS_DATE_LEN + 1] = {0}, sRespDesc[100 + 1] = {0};
    double dAmount = 0.0;
    char sDate[9] = {0}, sTime[7] = {0};
    char sUrl[20 + 1] = {0}, sPort[10 + 1] = {0};
    char sPlatcode[20 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    GET_STR_KEY(pstJson, "resp_desc", sRespDesc);
    GET_STR_KEY(pstJson, "resp_id", sRespId);
    GET_STR_KEY(pstJson, "merch_order_no", sMerchOrderNo);
    GET_STR_KEY(pstJson, "o_rrn", sOrrn);
    GET_STR_KEY(pstJson, "o_trans_date", sOTransDate);

    GET_STR_KEY(pstJson, "mcc", sMcc);
    GET_STR_KEY(pstJson, "input_mode", sInputMode);
    GET_STR_KEY(pstJson, "cond_code", sCondCode);
    GET_STR_KEY(pstJson, "pin_code", sPinCode);
    GET_STR_KEY(pstJson, "acq_id", sAcqId);
    GET_STR_KEY(pstJson, "iss_id", sIssId);
    GET_STR_KEY(pstJson, "agent_id", sAgentId);

    GET_STR_KEY(pstJson, "channel_id", sChannelId);
    GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
    GET_STR_KEY(pstJson, "channel_term_id", sChannelTermId);
    GET_STR_KEY(pstJson, "channel_term_sn", sChannelTermSn);
    GET_STR_KEY(pstJson, "channel_mcc", sChannelMcc);
    GET_STR_KEY(pstJson, "autho_flag", sAuthoFlag);
    GET_STR_KEY(pstJson, "batch_no", sBatchNo);
    GET_STR_KEY(pstJson, "auth_code", sAuthCode);
    //   GET_STR_KEY(pstJson, "ic_data", sIcData);
    GET_STR_KEY(pstJson, "resp_code", sRespCode);
    GET_STR_KEY(pstJson, "istresp_code", sIstRespCode);

    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "card_id", sCardId);
    GET_STR_KEY(pstJson, "card_type", sCardType);
    GET_STR_KEY(pstJson, "card_exp_date", sCardExpDate);
    GET_DOU_KEY(pstJson, "amount", dAmount);

    GET_STR_KEY(pstJson, "transmit_time", sTransmitTime);
    GET_STR_KEY(pstJson, "trans_date", sTransDate);
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    GET_STR_KEY(pstJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstJson, "logic_date", sLogicDate);
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "term_id", sTermId);
    GET_STR_KEY(pstJson, "term_sn", sTermSn);
    GET_STR_KEY(pstJson, "trace_no", sTraceNo);

    GET_STR_KEY(pstJson, "order_url", sUrl);
    GET_STR_KEY(pstJson, "order_port", sPort);
    GET_STR_KEY(pstJson, "platcode", sPlatcode);

    {
        /* 处理交易码转换 */
    }
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_NOTICE_TRANS_DETAIL(%s) "
            " VALUES('%s','%s','%s','%s','%s',%f"
            ",'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s'"
            ",'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s'"
            ",'%s','%s','%s','%s','%s','%s','%s'"
            ",'%s','%s','%s','%s','%s',0,'0',sysdate,sysdate,'%s','%s','%s')", SAF_FIELDS
            , sTransCode, sCardNo, sCardId, sCardType, sCardExpDate, dAmount/100
            , sTransmitTime, sTransDate, sTransTime, sSysTrace, sLogicDate, sMerchId, sTermId, sTermSn, sTraceNo, sRrn
            , sMcc, sInputMode, sCondCode, sPinCode, sAcqId, sIssId, sAgentId, sChannelId, sChannelMerchId, sChannelTermId, sChannelTermSn
            , sChannelMcc, sAuthoFlag, sBatchNo, sAuthCode, sIcData, sRespCode, sIstRespCode, sRespDesc
            , sRespId, sMerchOrderNo, sOrrn, sDate, sTime, sUrl, sPort, sPlatcode);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "添加交易rrn[%s]Notice记录失败.", sRrn);
        tRollback();
        return -1;
    }
    tReleaseRes(pstRes);
    tCommit();
    tLog(INFO, "交易rrn[%s]记录Notice成功.", sRrn);
    return 0;
}

int UpdNoticeCnt(cJSON *pstJson) {
    char sSqlStr[512] = {0};
    char sDate[9] = {0}, sTime[7] = {0};
    char sRrn[12 + 1] = {0};
    int iCnt = 0;
    char sTranTime[14 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDate[8 + 1] = {0};
    OCI_Resultset *pstRes = NULL, *pstRes1 = NULL, *pstRes2 = NULL;

    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "tranTime", sTranTime);
    strncpy(sTransDate, sTranTime, 8);
    strcpy(sTransTime, sTranTime + 8);
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT SAF_CNT FROM B_NOTICE_TRANS_DETAIL WHERE TRANS_DATE='%s' \
            AND TRANS_TIME='%s' AND RRN='%s'", sTransDate,sTransTime,sRrn);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "交易rrn[%s]获取通知次数失败.", sRrn);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iCnt);
    }

    tLog(INFO, "notice cnt [%d]", GetNoticeCnt());
    //  tLog(INFO, "notice cnt [%s]", atoi(GetNoticeCnt()));
    if (iCnt < GetNoticeCnt() - 1) {

        snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_NOTICE_TRANS_DETAIL SET "
                " SAF_DATE=%s, SAF_TIME=%s,SAF_CNT=SAF_CNT+1 WHERE TRANS_DATE='%s' \
            AND TRANS_TIME='%s' AND RRN='%s'"
                , sDate, sTime, sTransDate,sTransTime, sRrn);
        tLog(INFO, "sql[%s]", sSqlStr);
        if (tExecute(&pstRes1, sSqlStr) < 0) {
            tLog(ERROR, "交易rrn[%s]订单通知次数+1失败.", sRrn);
            tRollback();
            return -1;
        }
        tCommit();
        tLog(ERROR, "交易rrn[%s]订单通知次数+1", sRrn);

    } else {

        snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_NOTICE_TRANS_DETAIL SET "
                " SAF_DATE=%s, SAF_TIME=%s,SAF_CNT=SAF_CNT+1,SAF_STATUS='1' WHERE TRANS_DATE='%s' \
            AND TRANS_TIME='%s'  AND RRN='%s'"
                , sDate, sTime, sTransDate,sTransTime,sRrn);
        tLog(INFO, "sql[%s]", sSqlStr);
        if (tExecute(&pstRes2, sSqlStr) < 0) {
            tLog(ERROR, "交易rrn[%s]订单通知强制失败.", sRrn);
            tRollback();
            return -1;
        }
        tCommit();
        tLog(ERROR, "交易rrn[%s]订单通知强制成功.", sRrn);

    }
    tReleaseRes(pstRes);
    tReleaseRes(pstRes1);
    tReleaseRes(pstRes2);
    return 0;
}

int UpdNoticeLs(cJSON * pstJson) {
    char sSqlStr[512] = {0};
    char sDate[9] = {0}, sTime[7] = {0};
    char sRrn[12 + 1] = {0};
    char sTranTime[14 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDate[8 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "tranTime", sTranTime);
    strncpy(sTransDate, sTranTime, 8);
    strcpy(sTransTime, sTranTime + 8);
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_NOTICE_TRANS_DETAIL SET "
            " SAF_DATE=%s, SAF_TIME=%s,SAF_CNT=SAF_CNT+1, SAF_STATUS='2' WHERE TRANS_DATE='%s' AND TRANS_TIME='%s' AND RRN='%s'"
            , sDate, sTime, sTransDate, sTransTime, sRrn);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "更新交易rrn[%s]状态[成功]失败.", sRrn);
        tRollback();
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "交易rrn[%s]通知成功.", sRrn);
    return 0;
}

int ScanNotice() {

    char sSqlStr[2048];
    char sRrn[RRN_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0};
    char sTransDate[TRANS_DATE_LEN] = {0}, sTransTime[TRANS_TIME_LEN + 1] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0};
    char sTraceNo[TRACE_NO_LEN + 1] = {0};
    char sAgentId[INST_ID_LEN + 1] = {0}, sTransCodeNotice[4 + 1] = {0}, sTransDateTime[14 + 1] = {0};
    double dAmount = 0;
    char sAmount[12 + 1] = {0}, sOrderNo[30 + 1] = {0};
    char sUrl[20 + 1] = {0}, sPort[10 + 1] = {0};
    char sPlatcode[20 + 1] = {0};

    cJSON *pstJson = NULL;
    char sKey[64] = {0};


    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
    TRANS_CODE,\
    AMOUNT,\
    TRANS_DATE, \
    TRANS_TIME, \
    MERCH_ID, \
    TRACE_NO, \
    RRN, \
    AGENT_ID, \
    MERCH_ORDER_NO, \
    ORDER_URL, \
    ORDER_PORT, \
    PLATCODE \
    FROM B_NOTICE_TRANS_DETAIL \
    WHERE SAF_STATUS ='0' ");
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
        STRV(pstRes, 6, sTraceNo);
        STRV(pstRes, 7, sRrn);
        STRV(pstRes, 8, sAgentId);
        STRV(pstRes, 9, sOrderNo);

        STRV(pstRes, 10, sUrl);
        STRV(pstRes, 11, sPort);
        STRV(pstRes, 12, sPlatcode);

        strcpy(sTransCodeNotice, sTransCode + 2);
        SET_STR_KEY(pstJson, "tranCode", sTransCodeNotice);
        SET_STR_KEY(pstJson, "merchantId", sMerchId);
        //        SET_STR_KEY(pstJson, "merchantName", sMerchName);
        SET_STR_KEY(pstJson, "traceNo", sTraceNo);
        SET_STR_KEY(pstJson, "rrn", sRrn);
        strcpy(sTransDateTime, sTransDate);
        strcat(sTransDateTime, sTransTime);
        SET_STR_KEY(pstJson, "tranTime", sTransDateTime);
        SET_STR_KEY(pstJson, "orderId", sOrderNo);
        sprintf(sAmount, "%d", (int)dAmount*100);
        SET_STR_KEY(pstJson, "amount", sAmount);
        SET_STR_KEY(pstJson, "rrn", sRrn);
        SET_STR_KEY(pstJson, "platCode", sPlatcode);
        SET_STR_KEY(pstJson, "url", sUrl);
        SET_STR_KEY(pstJson, "port", sPort);

        tLog(INFO, "检索到NOTICE流水信息.");
        /* 发送订单通知 */
        snprintf(sKey, sizeof (sKey), "%s_%s", GetSvrId(), sRrn);
        SendMsg("JAVANOTICE_Q", sKey, pstJson);
        cJSON_Delete(pstJson);

    }



    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未检索到待发送的NOTICE流水.....");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}