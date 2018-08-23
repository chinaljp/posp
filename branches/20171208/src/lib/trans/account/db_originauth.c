#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "postransdetail.h"

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
extern void PosTransDetailTrim(PosTransDetail *pstPosTransDetail);

int FindRevAuthJon(PosTransDetail *pstPosTransDetail, char *pcCardNo, char *pcTransDate, char *pcAuthNo) {
    PosTransDetail stPosTransDetail;
    char sTransCode[6 + 1];
    char sRespCodeDup[3] = "94";
    char sSqlStr[2048];
    char sSqlStr1[2048];
    OCI_Resultset *pstRes = NULL;
    
    /* 1笔预授权交易可对应多笔预授权完成交易, 因此预授权完成撤销交易 **
     ** 可能需要2次查找, 以锁定一支未撤销的预授权完成交易.            */

    /* 第1次查找: 查找未撤销的预授权完成交易 */
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    snprintf(sSqlStr, sizeof (sSqlStr), "\
        SELECT  \
            ID,      \
            TRANS_CODE, \
            CARD_NO, \
            CARD_ID, \
            CARD_TYPE, \
            CARD_EXP_DATE, \
            AMOUNT, \
            TRANSMIT_TIME, \
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
            FROM B_POS_TRANS_DETAIL\
            WHERE TRANS_CODE = '%s'\
            AND CARD_NO = '%s'\
            AND INSTR( TRANS_DATE, %s, '4' ) <> 0 \
            AND AUTH_CODE = '%s'\
            AND VALID_FLAG='0' \
            AND RESP_CODE != '%s'", pstPosTransDetail->sTransCode, pcCardNo, pcTransDate, pcAuthNo, sRespCodeDup);
    /* 第2次查找: 查找任意匹配预授权号的交易 */
    snprintf(sSqlStr1, sizeof (sSqlStr1), "\
        SELECT  \
            ID,      \
            TRANS_CODE, \
            CARD_NO, \
            CARD_ID, \
            CARD_TYPE, \
            CARD_EXP_DATE, \
            AMOUNT, \
            TRANSMIT_TIME, \
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
            FROM B_POS_TRANS_DETAIL\
            WHERE TRANS_CODE = '%s'\
            AND CARD_NO = '%s'\
            AND INSTR( TRANS_DATE, %s, '4' ) <> 0 \
            AND AUTH_CODE = '%s'\
            AND RESP_CODE != '%s'", pstPosTransDetail->sTransCode, pcCardNo, pcTransDate, pcAuthNo, sRespCodeDup);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        if (tExecute(&pstRes, sSqlStr1) < 0) {
            tLog(ERROR, "sql[%s] err!", sSqlStr1);
            return -1;
        }
        if (NULL == pstRes) {
            tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr1);
            return -1;
        }
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL或未找到记录.", sSqlStr);
        if (tExecute(&pstRes, sSqlStr1) < 0) {
            tLog(ERROR, "sql[%s] err!", sSqlStr1);
            return -1;
        }
        if (NULL == pstRes) {
            tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr1);
            return -1;
        }

        //return 0;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, stPosTransDetail.sId);
        STRV(pstRes, 2, stPosTransDetail.sTransCode);
        STRV(pstRes, 3, stPosTransDetail.sCardNo);
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

int FindRevAuthJonHis(PosTransDetail *pstPosTransDetail, char *pcCardNo, char *pcTransDate, char *pcAuthNo) {
    char sRespCodeDup[3] = "94";
    PosTransDetail stPosTransDetail;
    char sSqlStr[2048] = {0}, sSqlStr1[2048] = {0};
    OCI_Resultset *pstRes = NULL;

    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));


    snprintf(sSqlStr, sizeof (sSqlStr), "\
        SELECT  \
            ID,      \
            TRANS_CODE, \
            CARD_NO, \
            CARD_ID, \
            CARD_TYPE, \
            CARD_EXP_DATE, \
            AMOUNT, \
            TRANSMIT_TIME, \
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
            SIGN_DATA, \
            MERCH_ORDER_NO, \
            O_RRN, \
            O_TRANS_DATE, \
            SETTLE_ORDER_ID \
            FROM B_POS_TRANS_DETAIL_HIS\
            WHERE TRANS_CODE = '%s'\
            AND CARD_NO = '%s'\
            AND INSTR( TRANS_DATE, %s, '4' ) <> 0 \
            AND AUTH_CODE = '%s'\
            AND RESP_CODE != '%s'", pstPosTransDetail->sTransCode, pcCardNo, pcTransDate, pcAuthNo, sRespCodeDup);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr1);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, stPosTransDetail.sId);
        STRV(pstRes, 2, stPosTransDetail.sTransCode);
        STRV(pstRes, 3, stPosTransDetail.sCardNo);
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
        STRV(pstRes, 18, stPosTransDetail.sMcc);
        STRV(pstRes, 19, stPosTransDetail.sInputMode);
        STRV(pstRes, 20, stPosTransDetail.sCondCode);
        STRV(pstRes, 21, stPosTransDetail.sPinCode);
        STRV(pstRes, 22, stPosTransDetail.sAcqId);
        STRV(pstRes, 23, stPosTransDetail.sIssId);
        STRV(pstRes, 24, stPosTransDetail.sAgentId);
        STRV(pstRes, 25, stPosTransDetail.sChannelId);
        STRV(pstRes, 26, stPosTransDetail.sChannelMerchId);
        STRV(pstRes, 27, stPosTransDetail.sChannelTermId);
        STRV(pstRes, 28, stPosTransDetail.sChannelTermSn);
        STRV(pstRes, 29, stPosTransDetail.sChannelMcc);
        STRV(pstRes, 30, stPosTransDetail.sTransType);
        STRV(pstRes, 31, stPosTransDetail.sValidFlag);
        STRV(pstRes, 32, stPosTransDetail.sCheckFlag);
        STRV(pstRes, 33, stPosTransDetail.sSettleFlag);
        STRV(pstRes, 34, stPosTransDetail.sAuthoFlag);
        STRV(pstRes, 35, stPosTransDetail.sBatchNo);
        STRV(pstRes, 36, stPosTransDetail.sAuthCode);
        DOUV(pstRes, 37, stPosTransDetail.dRefundAmt);
        DOUV(pstRes, 38, stPosTransDetail.dFee);
        STRV(pstRes, 39, stPosTransDetail.sFeeFlag);
        STRV(pstRes, 40, stPosTransDetail.sFeeDesc);
        STRV(pstRes, 41, stPosTransDetail.sRespCode);
        STRV(pstRes, 42, stPosTransDetail.sIstRespCode);
        STRV(pstRes, 43, stPosTransDetail.sRespDesc);
        STRV(pstRes, 44, stPosTransDetail.sRespId);
        STRV(pstRes, 45, stPosTransDetail.sSignFlag);
        STRV(pstRes, 46, stPosTransDetail.sMerchOrderNo);
        STRV(pstRes, 47, stPosTransDetail.sORrn);
        STRV(pstRes, 48, stPosTransDetail.sOTransDate);
        STRV(pstRes, 49, stPosTransDetail.sSettleOrderId);

        PosTransDetailTrim(&stPosTransDetail);
    }
    tLog(INFO, "检索到流水信息.");
    memcpy(pstPosTransDetail, &stPosTransDetail, sizeof (stPosTransDetail));
    return 0;
}

int UpdOrgJonAuth(char *pcRrn, char *pcTransdate, char cFlag) {
    char sSqlStr[1024] = {0}, sSqlStr1[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL SET "
            " AUTHO_FLAG= '%c' "
            " WHERE RRN = '%s' "
            " AND TRANS_DATE = '%s'"
            "AND TRANS_CODE NOT IN ('020003','020023')"
            , cFlag, pcRrn, pcTransdate);
    snprintf(sSqlStr1, sizeof (sSqlStr1), "UPDATE B_POS_TRANS_DETAIL_HIS SET "
            " AUTHO_FLAG= '%c' "
            " WHERE RRN = '%s' "
            " AND TRANS_DATE = '%s'"
            "AND TRANS_CODE NOT IN ('020003','020023')"
            , cFlag, pcRrn, pcTransdate);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新交易rrn[%s]结果失败.", pcRrn);
        if (tExecute(&pstRes, sSqlStr1) < 0 || tGetAffectedRows() <= 0) {
            tLog(ERROR, "更新交易rrn[%s]结果失败.", pcRrn);
            return -1;
        }
    }
    tReleaseRes(pstRes);
    tLog(INFO, "更新交易rrn[%s]结果成功.", pcRrn);
    return 0;
}

int UpdOrgJonAuthUndo(char *pcRrn, char *pcTransdate, char cFlag) {
    char sSqlStr[1024] = {0}, sSqlStr1[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL SET "
            " VALID_FLAG= '%c' "
            " WHERE RRN = '%s' "
            " AND TRANS_DATE = '%s'"
            "AND TRANS_CODE NOT IN ('020003','020023')"
            , cFlag, pcRrn, pcTransdate);
    snprintf(sSqlStr1, sizeof (sSqlStr1), "UPDATE B_POS_TRANS_DETAIL_HIS SET "
            " VALID_FLAG= '%c' "
            " WHERE RRN = '%s' "
            " AND TRANS_DATE = '%s'"
            "AND TRANS_CODE NOT IN ('020003','020023')"
            , cFlag, pcRrn, pcTransdate);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新交易rrn[%s]结果失败.", pcRrn);
        if (tExecute(&pstRes, sSqlStr1) < 0 || tGetAffectedRows() <= 0) {
            tLog(ERROR, "更新交易rrn[%s]结果失败.", pcRrn);
            return -1;
        }
    }
    tReleaseRes(pstRes);
    tLog(INFO, "更新交易rrn[%s]结果成功.", pcRrn);
    return 0;
}

int UpdOrgJonAuthRecover(char *pcRrn, char *pcDate, char cFlag) {
    char sSqlStr[1024] = {0}, sSqlStr1[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL SET "
            " AUTHO_FLAG= '%c' "
            " WHERE RRN = '%s' AND TRANS_DATE='%s' "
            "AND TRANS_CODE = '024000'"
            , cFlag, pcRrn, pcDate);

    snprintf(sSqlStr1, sizeof (sSqlStr1), "UPDATE B_POS_TRANS_DETAIL_HIS SET "
            " AUTHO_FLAG= '%c' "
            " WHERE RRN = '%s' AND TRANS_DATE='%s' "
            "AND TRANS_CODE = '024000'"
            , cFlag, pcRrn, pcDate);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        if (tExecute(&pstRes, sSqlStr1) < 0 || tGetAffectedRows() <= 0) {
            tLog(ERROR, "更新交易rrn[%s]结果失败.", pcRrn);
            return -1;
        }
    }
    tReleaseRes(pstRes);
    tLog(INFO, "更新交易rrn[%s]结果成功.", pcRrn);
    return 0;
}

/*
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
}*/