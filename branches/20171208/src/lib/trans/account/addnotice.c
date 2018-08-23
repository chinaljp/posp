/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_redis.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"

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


int FindNoticeTransfer(cJSON * pstTransJson) {

    char sSqlStr[256] = {0}, sStatus[2] = {0};
    char sUrl[20 + 1] = {0}, sPort[10 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sPlatcode[20 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select url,port,platcode from b_platform_access where platcode = \
                 (select platcode from b_merch_platform_middle where merch_id='%s' and status = '1') and status = '1'", sMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找商户[%s]的订单路由资料,失败.", sMerchId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sUrl);
        STRV(pstRes, 2, sPort);
        STRV(pstRes, 3, sPlatcode);
    }

    SET_STR_KEY(pstTransJson, "order_url", sUrl);
    SET_STR_KEY(pstTransJson, "order_port", sPort);
    SET_STR_KEY(pstTransJson, "platcode", sPlatcode);

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无商户[%s]路由资料,请先配置路由", sMerchId);
        tReleaseRes(pstRes);
        return -1;
    }
    tLog(INFO, "商户[%s]订单平台码[%s],URL[%s],PORT[%s]", sMerchId, sUrl, sPort, sPlatcode);
    tReleaseRes(pstRes);
    return 0;
}