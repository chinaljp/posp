#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "postransdetail.h"
#include "tKms.h"
void PosTransDetailTrim(PosTransDetail *pstPosTransDetail);

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

int FindRevAcctJon(PosTransDetail *pstPosTransDetail, char *pcMerchId, char *pcTermId, char *pcTermTrace) {
    PosTransDetail stPosTransDetail;
    char sRespCodeDup[3] = "94";
    char sSqlStr[2048];
    char sTestName[128 + 1] = {0}, sCardNo[256] = {0};
    OCI_Resultset *pstRes = NULL;
    if (FindValueByKey(sTestName, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
ID,\
TRANS_CODE,\
card_no_encrypt,\
CARD_ID,\
CARD_TYPE,\
CARD_EXP_DATE,\
AMOUNT,\
TRANSMIT_TIME,\
TRANS_DATE, \
TRANS_TIME, \
SYS_TRACE, \
LOGIC_DATE, \
SETTLE_DATE, \
MERCH_ID, \
TERM_ID, \
TERM_SN, \
TRACE_NO, \
RRN, \
MCC, \
INPUT_MODE, \
COND_CODE, \
PIN_CODE, \
ACQ_ID, \
ISS_ID, \
AGENT_ID, \
CHANNEL_ID, \
CHANNEL_MERCH_ID, \
CHANNEL_TERM_ID, \
CHANNEL_TERM_SN, \
CHANNEL_MCC, \
TRANS_TYPE, \
VALID_FLAG, \
CHECK_FLAG, \
SETTLE_FLAG, \
AUTHO_FLAG, \
BATCH_NO, \
AUTH_CODE, \
REFUND_AMT, \
FEE, \
FEE_FLAG, \
FEE_DESC, \
RESP_CODE, \
ISTRESP_CODE, \
RESP_DESC, \
RESP_ID, \
SIGN_FLAG, \
MERCH_ORDER_NO, \
O_RRN, \
O_TRANS_DATE, \
SETTLE_ORDER_ID \
FROM B_POS_TRANS_DETAIL \
WHERE TERM_ID = '%s'\
AND TRACE_NO = '%s'\
AND MERCH_ID = '%s'\
AND TRANS_TYPE != '1' \
AND RESP_CODE != '%s'\
AND TRANS_CODE NOT IN ('020003','020023','T20003','024003','M20003','M20023','024103','024023','024123')", pcTermId, pcTermTrace, pcMerchId, sRespCodeDup);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, stPosTransDetail.sId);
        STRV(pstRes, 2, stPosTransDetail.sTransCode);
        STRV(pstRes, 3, sCardNo);
        STRV(pstRes, 4, stPosTransDetail.sCardId);
        STRV(pstRes, 5, stPosTransDetail.sCardType);
        STRV(pstRes, 6, stPosTransDetail.sCardExpDate);
        DOUV(pstRes, 7, stPosTransDetail.dAmount);
        STRV(pstRes, 8, stPosTransDetail.sTransmitTime);
        STRV(pstRes, 9, stPosTransDetail.sTransDate);
        STRV(pstRes, 10, stPosTransDetail.sTransTime);
        STRV(pstRes, 11, stPosTransDetail.sSysTrace);
        STRV(pstRes, 12, stPosTransDetail.sLogicDate);
        STRV(pstRes, 13, stPosTransDetail.sSettleDate);
        STRV(pstRes, 14, stPosTransDetail.sMerchId);
        STRV(pstRes, 15, stPosTransDetail.sTermId);
        STRV(pstRes, 16, stPosTransDetail.sTermSn);
        STRV(pstRes, 17, stPosTransDetail.sTraceNo);
        STRV(pstRes, 18, stPosTransDetail.sRrn);
        STRV(pstRes, 19, stPosTransDetail.sMcc);
        STRV(pstRes, 20, stPosTransDetail.sInputMode);
        STRV(pstRes, 21, stPosTransDetail.sCondCode);
        STRV(pstRes, 22, stPosTransDetail.sPinCode);
        STRV(pstRes, 23, stPosTransDetail.sAcqId);
        STRV(pstRes, 24, stPosTransDetail.sIssId);
        STRV(pstRes, 25, stPosTransDetail.sAgentId);
        STRV(pstRes, 26, stPosTransDetail.sChannelId);
        STRV(pstRes, 27, stPosTransDetail.sChannelMerchId);
        STRV(pstRes, 28, stPosTransDetail.sChannelTermId);
        STRV(pstRes, 29, stPosTransDetail.sChannelTermSn);
        STRV(pstRes, 30, stPosTransDetail.sChannelMcc);
        STRV(pstRes, 31, stPosTransDetail.sTransType);
        STRV(pstRes, 32, stPosTransDetail.sValidFlag);
        STRV(pstRes, 33, stPosTransDetail.sCheckFlag);
        STRV(pstRes, 34, stPosTransDetail.sSettleFlag);
        STRV(pstRes, 35, stPosTransDetail.sAuthoFlag);
        STRV(pstRes, 36, stPosTransDetail.sBatchNo);
        STRV(pstRes, 37, stPosTransDetail.sAuthCode);
        DOUV(pstRes, 38, stPosTransDetail.dRefundAmt);
        DOUV(pstRes, 39, stPosTransDetail.dFee);
        STRV(pstRes, 40, stPosTransDetail.sFeeFlag);
        STRV(pstRes, 41, stPosTransDetail.sFeeDesc);
        STRV(pstRes, 42, stPosTransDetail.sRespCode);
        STRV(pstRes, 43, stPosTransDetail.sIstRespCode);
        STRV(pstRes, 44, stPosTransDetail.sRespDesc);
        STRV(pstRes, 45, stPosTransDetail.sRespId);
        STRV(pstRes, 46, stPosTransDetail.sSignFlag);
        STRV(pstRes, 47, stPosTransDetail.sMerchOrderNo);
        STRV(pstRes, 48, stPosTransDetail.sORrn);
        STRV(pstRes, 49, stPosTransDetail.sOTransDate);
        STRV(pstRes, 50, stPosTransDetail.sSettleOrderId);

        PosTransDetailTrim(&stPosTransDetail);
        if (tHsm_Dec_Asc_Data(stPosTransDetail.sCardNo, sTestName, sCardNo) < 0) {
            tLog(ERROR, "解密卡号数据失败.");
            tReleaseRes(pstRes);
            return -1;
        }
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "检索到流水信息.");
    memcpy(pstPosTransDetail, &stPosTransDetail, sizeof (stPosTransDetail));
    return 0;
}

int FindPosTrans(PosTransDetail *pstPosTransDetail, char *pcMerchId, char *pcTermId, char *pcTermTrace, char *pcCardNo) {
    PosTransDetail stPosTransDetail;
    char sRespCodeDup[3] = "94";
    char sSqlStr[2048];
    char sTestName[128 + 1] = {0}, sCardNo[256] = {0}, sTmp[256] = {0};
    OCI_Resultset *pstRes = NULL;
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    if (FindValueByKey(sTestName, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
ID,\
TRANS_CODE,\
card_no_encrypt,\
CARD_ID,\
CARD_TYPE,\
CARD_EXP_DATE,\
AMOUNT,\
TRANSMIT_TIME,\
TRANS_DATE, \
TRANS_TIME, \
SYS_TRACE, \
LOGIC_DATE, \
SETTLE_DATE, \
MERCH_ID, \
TERM_ID, \
TERM_SN, \
TRACE_NO, \
RRN, \
MCC, \
INPUT_MODE, \
COND_CODE, \
PIN_CODE, \
ACQ_ID, \
ISS_ID, \
AGENT_ID, \
CHANNEL_ID, \
CHANNEL_MERCH_ID, \
CHANNEL_TERM_ID, \
CHANNEL_TERM_SN, \
CHANNEL_MCC, \
TRANS_TYPE, \
VALID_FLAG, \
CHECK_FLAG, \
SETTLE_FLAG, \
AUTHO_FLAG, \
BATCH_NO, \
AUTH_CODE, \
REFUND_AMT, \
FEE, \
FEE_FLAG, \
FEE_DESC, \
RESP_CODE, \
ISTRESP_CODE, \
RESP_DESC, \
RESP_ID, \
SIGN_FLAG, \
MERCH_ORDER_NO, \
O_RRN, \
CHANNEL_RRN, \
O_TRANS_DATE, \
SETTLE_ORDER_ID \
FROM B_POS_TRANS_DETAIL \
WHERE TERM_ID = '%s'\
AND TRACE_NO = '%s'\
AND MERCH_ID = '%s'\
AND RESP_CODE != '%s'\
AND TRANS_CODE IN (SELECT TRANS_CODE FROM S_TRANS_CODE WHERE SAF_FLAG='1') order by trans_date desc,trans_time desc", pcTermId, pcTermTrace, pcMerchId, sRespCodeDup);
    if (tExecute(&pstRes, sSqlStr) < 0) {

        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, stPosTransDetail.sId);
        STRV(pstRes, 2, stPosTransDetail.sTransCode);
        STRV(pstRes, 3, sCardNo);
        STRV(pstRes, 4, stPosTransDetail.sCardId);
        STRV(pstRes, 5, stPosTransDetail.sCardType);
        STRV(pstRes, 6, stPosTransDetail.sCardExpDate);
        DOUV(pstRes, 7, stPosTransDetail.dAmount);
        STRV(pstRes, 8, stPosTransDetail.sTransmitTime);
        STRV(pstRes, 9, stPosTransDetail.sTransDate);
        STRV(pstRes, 10, stPosTransDetail.sTransTime);
        STRV(pstRes, 11, stPosTransDetail.sSysTrace);
        STRV(pstRes, 12, stPosTransDetail.sLogicDate);
        STRV(pstRes, 13, stPosTransDetail.sSettleDate);
        STRV(pstRes, 14, stPosTransDetail.sMerchId);
        STRV(pstRes, 15, stPosTransDetail.sTermId);
        STRV(pstRes, 16, stPosTransDetail.sTermSn);
        STRV(pstRes, 17, stPosTransDetail.sTraceNo);
        STRV(pstRes, 18, stPosTransDetail.sRrn);
        STRV(pstRes, 19, stPosTransDetail.sMcc);
        STRV(pstRes, 20, stPosTransDetail.sInputMode);
        STRV(pstRes, 21, stPosTransDetail.sCondCode);
        STRV(pstRes, 22, stPosTransDetail.sPinCode);
        STRV(pstRes, 23, stPosTransDetail.sAcqId);
        STRV(pstRes, 24, stPosTransDetail.sIssId);
        STRV(pstRes, 25, stPosTransDetail.sAgentId);
        STRV(pstRes, 26, stPosTransDetail.sChannelId);
        STRV(pstRes, 27, stPosTransDetail.sChannelMerchId);
        STRV(pstRes, 28, stPosTransDetail.sChannelTermId);
        STRV(pstRes, 29, stPosTransDetail.sChannelTermSn);
        STRV(pstRes, 30, stPosTransDetail.sChannelMcc);
        STRV(pstRes, 31, stPosTransDetail.sTransType);
        STRV(pstRes, 32, stPosTransDetail.sValidFlag);
        STRV(pstRes, 33, stPosTransDetail.sCheckFlag);
        STRV(pstRes, 34, stPosTransDetail.sSettleFlag);
        STRV(pstRes, 35, stPosTransDetail.sAuthoFlag);
        STRV(pstRes, 36, stPosTransDetail.sBatchNo);
        STRV(pstRes, 37, stPosTransDetail.sAuthCode);
        DOUV(pstRes, 38, stPosTransDetail.dRefundAmt);
        DOUV(pstRes, 39, stPosTransDetail.dFee);
        STRV(pstRes, 40, stPosTransDetail.sFeeFlag);
        STRV(pstRes, 41, stPosTransDetail.sFeeDesc);
        STRV(pstRes, 42, stPosTransDetail.sRespCode);
        STRV(pstRes, 43, stPosTransDetail.sIstRespCode);
        STRV(pstRes, 44, stPosTransDetail.sRespDesc);
        STRV(pstRes, 45, stPosTransDetail.sRespId);
        STRV(pstRes, 46, stPosTransDetail.sSignFlag);
        STRV(pstRes, 47, stPosTransDetail.sMerchOrderNo);
        STRV(pstRes, 48, stPosTransDetail.sORrn);
        STRV(pstRes, 49, stPosTransDetail.sPufaRrn);
        STRV(pstRes, 50, stPosTransDetail.sOTransDate);
        STRV(pstRes, 51, stPosTransDetail.sSettleOrderId);
        tLog(DEBUG, "card_no[%s]", sCardNo);

        tLog(DEBUG, "key name[%s],data[%s].", sTestName, sCardNo);
        PosTransDetailTrim(&stPosTransDetail);
        if (tHsm_Dec_Asc_Data(stPosTransDetail.sCardNo, sTestName, sCardNo) < 0) {
            tLog(ERROR, "解密卡号数据失败.");
            tReleaseRes(pstRes);
            return -1;
        }
        tLog(DEBUG, "card_no[%s][%s]", stPosTransDetail.sCardNo, sCardNo);
        if (!strcmp(stPosTransDetail.sCardNo, pcCardNo)) {
            break;
        }
        tLog(DEBUG, "no card_no.");
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "检索到流水信息.");
    memcpy(pstPosTransDetail, &stPosTransDetail, sizeof (stPosTransDetail));
    return 0;
}

int FindAuthPosTrans(PosTransDetail *pstPosTransDetail, char *pcTable, char *pcDate, char *pcAuth, char *pcCardNo) {
    PosTransDetail stPosTransDetail;
    char sRespCodeDup[3] = "94";
    char sSqlStr[2048];
    char sTestName[128 + 1] = {0}, sCardNo[256] = {0}, sECardNo[256 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    if (FindValueByKey(sTestName, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }

    if (tHsm_Enc_Asc_Data(sECardNo, sTestName, pcCardNo) < 0) {
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(ERROR, "加密卡号数据[%s]", sECardNo);

    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
ID,\
TRANS_CODE,\
card_no_encrypt,\
CARD_ID,\
CARD_TYPE,\
CARD_EXP_DATE,\
AMOUNT,\
TRANSMIT_TIME,\
TRANS_DATE, \
TRANS_TIME, \
SYS_TRACE, \
LOGIC_DATE, \
SETTLE_DATE, \
MERCH_ID, \
TERM_ID, \
TERM_SN, \
TRACE_NO, \
RRN, \
MCC, \
INPUT_MODE, \
COND_CODE, \
PIN_CODE, \
ACQ_ID, \
ISS_ID, \
AGENT_ID, \
CHANNEL_ID, \
CHANNEL_MERCH_ID, \
CHANNEL_TERM_ID, \
CHANNEL_TERM_SN, \
CHANNEL_MCC, \
TRANS_TYPE, \
VALID_FLAG, \
CHECK_FLAG, \
SETTLE_FLAG, \
AUTHO_FLAG, \
BATCH_NO, \
AUTH_CODE, \
REFUND_AMT, \
FEE, \
FEE_FLAG, \
FEE_DESC, \
RESP_CODE, \
ISTRESP_CODE, \
RESP_DESC, \
RESP_ID, \
SIGN_FLAG, \
MERCH_ORDER_NO, \
O_RRN, \
O_TRANS_DATE, \
SETTLE_ORDER_ID \
FROM %s \
WHERE AUTH_CODE = '%s'\
AND TRANS_DATE = '%s' \
AND RESP_CODE != '%s'\
AND CARD_NO_ENCRYPT ='%s' \
AND TRANS_CODE='024000'"
            , pcTable, pcAuth, pcDate, sRespCodeDup,sECardNo);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tLog(DEBUG, "sql[%s]", sSqlStr);
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, stPosTransDetail.sId);
        STRV(pstRes, 2, stPosTransDetail.sTransCode);
        STRV(pstRes, 3, sCardNo);
        STRV(pstRes, 4, stPosTransDetail.sCardId);
        STRV(pstRes, 5, stPosTransDetail.sCardType);
        STRV(pstRes, 6, stPosTransDetail.sCardExpDate);
        DOUV(pstRes, 7, stPosTransDetail.dAmount);
        STRV(pstRes, 8, stPosTransDetail.sTransmitTime);
        STRV(pstRes, 9, stPosTransDetail.sTransDate);
        STRV(pstRes, 10, stPosTransDetail.sTransTime);
        STRV(pstRes, 11, stPosTransDetail.sSysTrace);
        STRV(pstRes, 12, stPosTransDetail.sLogicDate);
        STRV(pstRes, 13, stPosTransDetail.sSettleDate);
        STRV(pstRes, 14, stPosTransDetail.sMerchId);
        STRV(pstRes, 15, stPosTransDetail.sTermId);
        STRV(pstRes, 16, stPosTransDetail.sTermSn);
        STRV(pstRes, 17, stPosTransDetail.sTraceNo);
        STRV(pstRes, 18, stPosTransDetail.sRrn);
        STRV(pstRes, 19, stPosTransDetail.sMcc);
        STRV(pstRes, 20, stPosTransDetail.sInputMode);
        STRV(pstRes, 21, stPosTransDetail.sCondCode);
        STRV(pstRes, 22, stPosTransDetail.sPinCode);
        STRV(pstRes, 23, stPosTransDetail.sAcqId);
        STRV(pstRes, 24, stPosTransDetail.sIssId);
        STRV(pstRes, 25, stPosTransDetail.sAgentId);
        STRV(pstRes, 26, stPosTransDetail.sChannelId);
        STRV(pstRes, 27, stPosTransDetail.sChannelMerchId);
        STRV(pstRes, 28, stPosTransDetail.sChannelTermId);
        STRV(pstRes, 29, stPosTransDetail.sChannelTermSn);
        STRV(pstRes, 30, stPosTransDetail.sChannelMcc);
        STRV(pstRes, 31, stPosTransDetail.sTransType);
        STRV(pstRes, 32, stPosTransDetail.sValidFlag);
        STRV(pstRes, 33, stPosTransDetail.sCheckFlag);
        STRV(pstRes, 34, stPosTransDetail.sSettleFlag);
        STRV(pstRes, 35, stPosTransDetail.sAuthoFlag);
        STRV(pstRes, 36, stPosTransDetail.sBatchNo);
        STRV(pstRes, 37, stPosTransDetail.sAuthCode);
        DOUV(pstRes, 38, stPosTransDetail.dRefundAmt);
        DOUV(pstRes, 39, stPosTransDetail.dFee);
        STRV(pstRes, 40, stPosTransDetail.sFeeFlag);
        STRV(pstRes, 41, stPosTransDetail.sFeeDesc);
        STRV(pstRes, 42, stPosTransDetail.sRespCode);
        STRV(pstRes, 43, stPosTransDetail.sIstRespCode);
        STRV(pstRes, 44, stPosTransDetail.sRespDesc);
        STRV(pstRes, 45, stPosTransDetail.sRespId);
        STRV(pstRes, 46, stPosTransDetail.sSignFlag);
        STRV(pstRes, 47, stPosTransDetail.sMerchOrderNo);
        STRV(pstRes, 48, stPosTransDetail.sORrn);
        STRV(pstRes, 49, stPosTransDetail.sOTransDate);
        STRV(pstRes, 50, stPosTransDetail.sSettleOrderId);

        PosTransDetailTrim(&stPosTransDetail);
        if (tHsm_Dec_Asc_Data(stPosTransDetail.sCardNo, sTestName, sCardNo) < 0) {
            tLog(ERROR, "解密卡号数据失败.");
            tReleaseRes(pstRes);
            return -1;
        }
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "检索到流水信息.");
    memcpy(pstPosTransDetail, &stPosTransDetail, sizeof (stPosTransDetail));
    return 0;
}

int FindAcctJonHis(PosTransDetail *pstPosTransDetail, char *pcMerchId, char *pcRrn, char *pcTransDate) {
    PosTransDetail stPosTransDetail;
    char sRespCodeDup[3] = "94";
    char sSqlStr[2048];
    char sTestName[128 + 1] = {0}, sCardNo[256] = {0};
    OCI_Resultset *pstRes = NULL;
    if (FindValueByKey(sTestName, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
ID,\
TRANS_CODE,\
card_no_encrypt,\
CARD_ID,\
CARD_TYPE,\
CARD_EXP_DATE,\
AMOUNT,\
TRANSMIT_TIME,\
TRANS_DATE, \
TRANS_TIME, \
SYS_TRACE, \
LOGIC_DATE, \
SETTLE_DATE, \
MERCH_ID, \
TERM_ID, \
TERM_SN, \
TRACE_NO, \
RRN, \
MCC, \
INPUT_MODE, \
COND_CODE, \
PIN_CODE, \
ACQ_ID, \
ISS_ID, \
AGENT_ID, \
CHANNEL_ID, \
CHANNEL_MERCH_ID, \
CHANNEL_TERM_ID, \
CHANNEL_TERM_SN, \
CHANNEL_MCC, \
TRANS_TYPE, \
VALID_FLAG, \
CHECK_FLAG, \
SETTLE_FLAG, \
AUTHO_FLAG, \
BATCH_NO, \
AUTH_CODE, \
REFUND_AMT, \
FEE, \
FEE_FLAG, \
FEE_DESC, \
RESP_CODE, \
ISTRESP_CODE, \
RESP_DESC, \
RESP_ID, \
SIGN_FLAG, \
MERCH_ORDER_NO, \
O_RRN, \
O_TRANS_DATE, \
SETTLE_ORDER_ID, \
FEE_TYPE \
FROM B_POS_TRANS_DETAIL_HIS \
WHERE RRN = '%s' \
AND MERCH_ID = '%s' \
AND TRANS_DATE = '%s' \
AND RESP_CODE = '00' \
AND TRANS_CODE IN ('020000','024100')", pcRrn, pcMerchId, pcTransDate);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, stPosTransDetail.sId);
        STRV(pstRes, 2, stPosTransDetail.sTransCode);
        STRV(pstRes, 3, sCardNo);
        STRV(pstRes, 4, stPosTransDetail.sCardId);
        STRV(pstRes, 5, stPosTransDetail.sCardType);
        STRV(pstRes, 6, stPosTransDetail.sCardExpDate);
        DOUV(pstRes, 7, stPosTransDetail.dAmount);
        STRV(pstRes, 8, stPosTransDetail.sTransmitTime);
        STRV(pstRes, 9, stPosTransDetail.sTransDate);
        STRV(pstRes, 10, stPosTransDetail.sTransTime);
        STRV(pstRes, 11, stPosTransDetail.sSysTrace);
        STRV(pstRes, 12, stPosTransDetail.sLogicDate);
        STRV(pstRes, 13, stPosTransDetail.sSettleDate);
        STRV(pstRes, 14, stPosTransDetail.sMerchId);
        STRV(pstRes, 15, stPosTransDetail.sTermId);
        STRV(pstRes, 16, stPosTransDetail.sTermSn);
        STRV(pstRes, 17, stPosTransDetail.sTraceNo);
        STRV(pstRes, 18, stPosTransDetail.sRrn);
        STRV(pstRes, 19, stPosTransDetail.sMcc);
        STRV(pstRes, 20, stPosTransDetail.sInputMode);
        STRV(pstRes, 21, stPosTransDetail.sCondCode);
        STRV(pstRes, 22, stPosTransDetail.sPinCode);
        STRV(pstRes, 23, stPosTransDetail.sAcqId);
        STRV(pstRes, 24, stPosTransDetail.sIssId);
        STRV(pstRes, 25, stPosTransDetail.sAgentId);
        STRV(pstRes, 26, stPosTransDetail.sChannelId);
        STRV(pstRes, 27, stPosTransDetail.sChannelMerchId);
        STRV(pstRes, 28, stPosTransDetail.sChannelTermId);
        STRV(pstRes, 29, stPosTransDetail.sChannelTermSn);
        STRV(pstRes, 30, stPosTransDetail.sChannelMcc);
        STRV(pstRes, 31, stPosTransDetail.sTransType);
        STRV(pstRes, 32, stPosTransDetail.sValidFlag);
        STRV(pstRes, 33, stPosTransDetail.sCheckFlag);
        STRV(pstRes, 34, stPosTransDetail.sSettleFlag);
        STRV(pstRes, 35, stPosTransDetail.sAuthoFlag);
        STRV(pstRes, 36, stPosTransDetail.sBatchNo);
        STRV(pstRes, 37, stPosTransDetail.sAuthCode);
        DOUV(pstRes, 38, stPosTransDetail.dRefundAmt);
        DOUV(pstRes, 39, stPosTransDetail.dFee);
        STRV(pstRes, 40, stPosTransDetail.sFeeFlag);
        STRV(pstRes, 41, stPosTransDetail.sFeeDesc);
        STRV(pstRes, 42, stPosTransDetail.sRespCode);
        STRV(pstRes, 43, stPosTransDetail.sIstRespCode);
        STRV(pstRes, 44, stPosTransDetail.sRespDesc);
        STRV(pstRes, 45, stPosTransDetail.sRespId);
        STRV(pstRes, 46, stPosTransDetail.sSignFlag);
        STRV(pstRes, 47, stPosTransDetail.sMerchOrderNo);
        STRV(pstRes, 48, stPosTransDetail.sORrn);
        STRV(pstRes, 49, stPosTransDetail.sOTransDate);
        STRV(pstRes, 50, stPosTransDetail.sSettleOrderId);
        STRV(pstRes, 51, stPosTransDetail.sFeeType);
        PosTransDetailTrim(&stPosTransDetail);
        if (tHsm_Dec_Asc_Data(stPosTransDetail.sCardNo, sTestName, sCardNo) < 0) {
            tLog(ERROR, "解密卡号数据失败.");
            tReleaseRes(pstRes);
            return -1;
        }
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "检索到流水信息.");
    memcpy(pstPosTransDetail, &stPosTransDetail, sizeof (stPosTransDetail));
    return 0;
}

int UpdOrgJon(char *pcMerchId, char *pcTermId, char *pcTermTrace, char cValidFlag) {
    char sSqlStr[1024];
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL SET "
            " VALID_FLAG= '%c' "
            " WHERE TERM_ID = '%s'"
            " AND TRACE_NO = '%s' "
            "AND MERCH_ID = '%s' "
            "AND TRANS_CODE not in ('020003','020023','T20003','M20003','M20023','024003','024023','024103','024123')"
            , cValidFlag, pcTermId, pcTermTrace, pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新原交易 商户[%s]终端[%s]流水[%s]valid_flag失败.", pcMerchId, pcTermId, pcTermTrace);
        return -1;
    }

    tReleaseRes(pstRes);
    tLog(INFO, "更新原交易 商户[%s]终端[%s]流水[%s]valid_flag[%c]成功.", pcMerchId, pcTermId, pcTermTrace, cValidFlag);
    return 0;
}

int UpdOrgJonHis(char *pcTransDate, char *pcRrn, char cValidFlag, double dRefundAmt) {
    char sSqlStr[1024];
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL_HIS SET "
            " REFUND_AMT = '%lf', "
            " VALID_FLAG= '%c' "
            " WHERE  RRN = '%s' "
            "AND TRANS_DATE = '%s' "
            "AND TRANS_CODE in ('020000')"
            , dRefundAmt, cValidFlag, pcRrn, pcTransDate);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新原交易 参考号[%s]交易日期[%s]金额[%lf]valid_flag[%c]失败.", pcRrn, pcTransDate, dRefundAmt, cValidFlag);
        return -1;
    }

    tReleaseRes(pstRes);
    tLog(INFO, "更新原交易 参考号[%s]交易时间[%s]金额[%lf]valid_flag[%c]成功.", pcRrn, pcTransDate, dRefundAmt, cValidFlag);
    return 0;
}

int UpdOrgJonRecover(char *pcRrn) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL SET "
            " VALID_FLAG= '0' "
            " WHERE RRN = '%s' "
            "AND TRANS_CODE NOT IN ('020003','020023','T20003','024003','024123','M20003')"
            , pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新交易rrn[%s]结果失败.", pcRrn);
        return -1;
    }

    tReleaseRes(pstRes);
    tLog(INFO, "更新交易rrn[%s]结果成功.", pcRrn);
    return 0;
}

int GetOrgRrnAndTransDate(char *pcRrn, char *pcTransDate, char *pcORrn, char *pcOTransDate) {
    char sSqlStr[512] = {0}, sSqlStr1[512] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  O_RRN, O_TRANS_DATE "
            "FROM B_POS_TRANS_DETAIL "
            "WHERE RRN='%s' "
            "AND TRANS_DATE = '%s' "
            , pcRrn, pcTransDate);
    snprintf(sSqlStr1, sizeof (sSqlStr1), "SELECT  O_RRN, O_TRANS_DATE "
            "FROM B_POS_TRANS_DETAIL_HIS "
            "WHERE RRN='%s' "
            "AND TRANS_DATE = '%s' "
            , pcRrn, pcTransDate);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        if (tExecute(&pstRes, sSqlStr1) < 0) {
            tLog(ERROR, "sql[%s] err!", sSqlStr1);
            return -1;
        }
        if (NULL == pstRes) {
            tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr1);
            return -1;
        }

    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcORrn);
        STRV(pstRes, 2, pcOTransDate);

    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "检索到流水信息 原RRN[%s].原TRANS_DATE[%s]", pcORrn, pcOTransDate);
    return 0;
}

void PosTransDetailTrim(PosTransDetail *pstPosTransDetail) {
    tTrim(pstPosTransDetail->sId);
    tTrim(pstPosTransDetail->sTransCode);
    tTrim(pstPosTransDetail->sCardNo);
    tTrim(pstPosTransDetail->sCardId);
    tTrim(pstPosTransDetail->sCardExpDate);
    tTrim(pstPosTransDetail->sTransmitTime);
    tTrim(pstPosTransDetail->sTransDate);
    tTrim(pstPosTransDetail->sTransTime);
    tTrim(pstPosTransDetail->sSysTrace);
    tTrim(pstPosTransDetail->sLogicDate);
    tTrim(pstPosTransDetail->sSettleDate);
    tTrim(pstPosTransDetail->sMerchId);
    tTrim(pstPosTransDetail->sTermId);
    tTrim(pstPosTransDetail->sTermSn);
    tTrim(pstPosTransDetail->sTraceNo);
    tTrim(pstPosTransDetail->sRrn);
    tTrim(pstPosTransDetail->sMcc);
    tTrim(pstPosTransDetail->sInputMode);
    tTrim(pstPosTransDetail->sCondCode);
    tTrim(pstPosTransDetail->sPinCode);
    tTrim(pstPosTransDetail->sAcqId);
    tTrim(pstPosTransDetail->sIssId);
    tTrim(pstPosTransDetail->sAgentId);
    tTrim(pstPosTransDetail->sChannelId);
    tTrim(pstPosTransDetail->sChannelMerchId);
    tTrim(pstPosTransDetail->sChannelTermId);
    tTrim(pstPosTransDetail->sChannelTermSn);
    tTrim(pstPosTransDetail->sChannelMcc);
    tTrim(pstPosTransDetail->sBatchNo);
    tTrim(pstPosTransDetail->sAuthCode);
    tTrim(pstPosTransDetail->sFeeDesc);
    tTrim(pstPosTransDetail->sRespCode);
    tTrim(pstPosTransDetail->sIstRespCode);
    tTrim(pstPosTransDetail->sRespDesc);
    tTrim(pstPosTransDetail->sRespId);
    tTrim(pstPosTransDetail->sMerchOrderNo);
    tTrim(pstPosTransDetail->sORrn);
    tTrim(pstPosTransDetail->sOTransDate);
    tTrim(pstPosTransDetail->sSettleOrderId);
}

/* 更新valid_flag标志 */
int UpdTransDetail(char *pcTable, char *pcDate, char *pcRrn, char *pcValidFlag) {
    char sSqlStr[1024];
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE %s SET "
            " VALID_FLAG= '%s' "
            " WHERE trans_date = '%s'"
            " AND rrn = '%s' "
            , pcTable, pcValidFlag, pcDate, pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新原交易[%s:%s]valid_flag[%s]失败.", pcDate, pcRrn, pcValidFlag);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "更新原交易[%s:%s]valid_flag[%s]成功[%d].", pcDate, pcRrn, pcValidFlag, tGetAffectedRows(pstRes));
    return 0;
}

int FindD0Trans(PosTransDetail *pstPosTransDetail, char *psTransDate, char *pcRrn) {
    PosTransDetail stPosTransDetail;
    char sRespCodeDup[3] = "94";
    char sTestName[128 + 1] = {0}, sCardNo[256] = {0};
    char sSqlStr[2048];
    OCI_Resultset *pstRes = NULL;
    if (FindValueByKey(sTestName, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
ID,\
TRANS_CODE,\
card_no_encrypt,\
CARD_ID,\
CARD_TYPE,\
CARD_EXP_DATE,\
AMOUNT,\
TRANSMIT_TIME,\
TRANS_DATE, \
TRANS_TIME, \
SYS_TRACE, \
LOGIC_DATE, \
SETTLE_DATE, \
MERCH_ID, \
TERM_ID, \
TERM_SN, \
TRACE_NO, \
RRN, \
MCC, \
INPUT_MODE, \
COND_CODE, \
PIN_CODE, \
ACQ_ID, \
ISS_ID, \
AGENT_ID, \
CHANNEL_ID, \
CHANNEL_MERCH_ID, \
CHANNEL_TERM_ID, \
CHANNEL_TERM_SN, \
CHANNEL_MCC, \
TRANS_TYPE, \
VALID_FLAG, \
CHECK_FLAG, \
SETTLE_FLAG, \
AUTHO_FLAG, \
BATCH_NO, \
AUTH_CODE, \
REFUND_AMT, \
FEE, \
FEE_FLAG, \
FEE_DESC, \
RESP_CODE, \
ISTRESP_CODE, \
RESP_DESC, \
RESP_ID, \
SIGN_FLAG, \
MERCH_ORDER_NO, \
O_RRN, \
O_TRANS_DATE, \
SETTLE_ORDER_ID \
FROM B_POS_TRANS_DETAIL \
WHERE TRANS_DATE = '%s'\
AND RRN = '%s'\
AND ISTRESP_CODE = '00'\
AND TRANS_CODE IN (SELECT TRANS_CODE FROM S_TRANS_CODE WHERE SAF_FLAG='1')", psTransDate, pcRrn);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, stPosTransDetail.sId);
        STRV(pstRes, 2, stPosTransDetail.sTransCode);
        STRV(pstRes, 3, sCardNo);
        STRV(pstRes, 4, stPosTransDetail.sCardId);
        STRV(pstRes, 5, stPosTransDetail.sCardType);
        STRV(pstRes, 6, stPosTransDetail.sCardExpDate);
        DOUV(pstRes, 7, stPosTransDetail.dAmount);
        STRV(pstRes, 8, stPosTransDetail.sTransmitTime);
        STRV(pstRes, 9, stPosTransDetail.sTransDate);
        STRV(pstRes, 10, stPosTransDetail.sTransTime);
        STRV(pstRes, 11, stPosTransDetail.sSysTrace);
        STRV(pstRes, 12, stPosTransDetail.sLogicDate);
        STRV(pstRes, 13, stPosTransDetail.sSettleDate);
        STRV(pstRes, 14, stPosTransDetail.sMerchId);
        STRV(pstRes, 15, stPosTransDetail.sTermId);
        STRV(pstRes, 16, stPosTransDetail.sTermSn);
        STRV(pstRes, 17, stPosTransDetail.sTraceNo);
        STRV(pstRes, 18, stPosTransDetail.sRrn);
        STRV(pstRes, 19, stPosTransDetail.sMcc);
        STRV(pstRes, 20, stPosTransDetail.sInputMode);
        STRV(pstRes, 21, stPosTransDetail.sCondCode);
        STRV(pstRes, 22, stPosTransDetail.sPinCode);
        STRV(pstRes, 23, stPosTransDetail.sAcqId);
        STRV(pstRes, 24, stPosTransDetail.sIssId);
        STRV(pstRes, 25, stPosTransDetail.sAgentId);
        STRV(pstRes, 26, stPosTransDetail.sChannelId);
        STRV(pstRes, 27, stPosTransDetail.sChannelMerchId);
        STRV(pstRes, 28, stPosTransDetail.sChannelTermId);
        STRV(pstRes, 29, stPosTransDetail.sChannelTermSn);
        STRV(pstRes, 30, stPosTransDetail.sChannelMcc);
        STRV(pstRes, 31, stPosTransDetail.sTransType);
        STRV(pstRes, 32, stPosTransDetail.sValidFlag);
        STRV(pstRes, 33, stPosTransDetail.sCheckFlag);
        STRV(pstRes, 34, stPosTransDetail.sSettleFlag);
        STRV(pstRes, 35, stPosTransDetail.sAuthoFlag);
        STRV(pstRes, 36, stPosTransDetail.sBatchNo);
        STRV(pstRes, 37, stPosTransDetail.sAuthCode);
        DOUV(pstRes, 38, stPosTransDetail.dRefundAmt);
        DOUV(pstRes, 39, stPosTransDetail.dFee);
        STRV(pstRes, 40, stPosTransDetail.sFeeFlag);
        STRV(pstRes, 41, stPosTransDetail.sFeeDesc);
        STRV(pstRes, 42, stPosTransDetail.sRespCode);
        STRV(pstRes, 43, stPosTransDetail.sIstRespCode);
        STRV(pstRes, 44, stPosTransDetail.sRespDesc);
        STRV(pstRes, 45, stPosTransDetail.sRespId);
        STRV(pstRes, 46, stPosTransDetail.sSignFlag);
        STRV(pstRes, 47, stPosTransDetail.sMerchOrderNo);
        STRV(pstRes, 48, stPosTransDetail.sORrn);
        STRV(pstRes, 49, stPosTransDetail.sOTransDate);
        STRV(pstRes, 50, stPosTransDetail.sSettleOrderId);

        PosTransDetailTrim(&stPosTransDetail);
        if (tHsm_Dec_Asc_Data(stPosTransDetail.sCardNo, sTestName, sCardNo) < 0) {
            tLog(ERROR, "解密卡号数据失败.");
            tReleaseRes(pstRes);
            return -1;
        }
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "检索到流水信息.");
    memcpy(pstPosTransDetail, &stPosTransDetail, sizeof (stPosTransDetail));
    return 0;
}

/*冲正类交易、退货交易 检索原交易商户的级别  add by GJQ at 20180309 */
int FindMerchLevel(char *pcMerchId, char *pcMerchLevel)
{
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select grade_merch_level from b_merch where merch_id='%s'", pcMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找商户[%s]的级别,失败.", pcMerchId);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcMerchLevel);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无商户[%s]级别,请检查商户资料.", pcMerchId);
        tReleaseRes(pstRes);
        return -1;
    }
    tTrim(pcMerchLevel);
    
    tReleaseRes(pstRes);
    return ( 0 );
}

/*预授权完成撤销交易，限额检查时，检索原交易（预授权完成）的原交易（预授权）输入方式*/
int FindOrigOInputMode(char *pcInputMode, char *pcRrn, char *pcTransDate, char *pcTableName) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "select b.input_mode from  b_pos_trans_detail a, %s b"
                        " where b.rrn = a.o_rrn and b.trans_date = a.o_trans_date and a.rrn = '%s' and a.trans_date = '%s'",pcTableName,pcRrn,pcTransDate );
    
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tLog(DEBUG, "sql[%s]", sSqlStr);
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcInputMode);
        tTrim(pcInputMode);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}