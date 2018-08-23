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
#include "t_macro.h"
#include "trans_detail.h"

int FindRevAcctJon(PosTransDetail *pstTransDetail, char *pcMerchId, char *pcTermId, char *pcTermTrace) {

    char  sSqlStr[256] = {0}, sCardNo[20] = {0}, sRrn[13] = {0}, sLogicDate[9] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT CARD_NO,AMOUNT,LOGIC_DATE,RRN FROM B_POS_TRANS_DETAIL "
            " WHERE TERM_ID = '%s' AND TRACE_NO = '%s' AND MERCH_ID = '%s' \
            ", pcTermId, pcTermTrace, pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, pstTransDetail->sCardNo);
        DOUV(pstRes, 2, pstTransDetail->dAmount);
        STRV(pstRes, 3, pstTransDetail->sLogicDate);
        STRV(pstRes, 4, pstTransDetail->sRrn);

        tTrim(pstTransDetail->sCardNo);
        tTrim(pstTransDetail->sLogicDate);
        tTrim(pstTransDetail->sRrn);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int JudgeSignFlag(char *pcMerchId, char *pcTermId, char *pcTermTrace) {
    char sSqlStr[256] = {0}, sSignFlag[2] = {0};
    OCI_Resultset *pstRes = NULL;

    sprintf(sSqlStr, "select SIGN_FLAG from B_POS_TRANS_DETAIL  WHERE MERCH_ID='%s' AND TERM_ID='%s' AND TRACE_NO='%s' ", \
		pcMerchId, pcTermId, pcTermTrace);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sSignFlag);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "未找到商户[%s]终端[%s]凭证号[%s]的原交易流水.", pcMerchId, pcTermId, pcTermTrace);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (sSignFlag[0] == '1') {
        return 1;
    }
    return 0;
}

int UpdAcctJonForSign(int iLen, char *psData1, char *pcMerchId, char *pcTermId, char *pcTermTrace) {
    char sSqlStr[256] = {0}, sSignFlag[2] = {0};
    int *iLen1 = 0;
    unsigned long len = 0;
    unsigned int cb = 0;
    unsigned int bb = 0;
    OCI_Resultset *pstRes = NULL;
    OCI_Lob *lob;
    sprintf(sSqlStr, "select SIGN_DATA from B_POS_TRANS_DETAIL  WHERE MERCH_ID='%s' AND TERM_ID='%s' AND TRACE_NO='%s' for update", \
		pcMerchId, pcTermId, pcTermTrace);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    bb = strlen(psData1);
    //  OCI_LobTruncate(lob,0);
    while (OCI_FetchNext(pstRes)) {
        lob = OCI_GetLob(pstRes, 1);
        OCI_LobWrite2(lob, psData1, &cb, &iLen);
        OCI_LobFree(lob);
    }
    tCommit();
    tReleaseRes(pstRes);
    return 0;
}
/******************************************************************************/
/*      函数名:     SignatureProc()                  	                      */
/*      功能说明:   电子签名处理											  */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */
/******************************************************************************/
int SignatureProc(cJSON *pstTransJson, int *piFlag) {
    char sTraceNo[TRACE_NO_LEN + 1] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    char sCardNo[CARD_NO_LEN + 1] = {0}, sData1[2048 + 1] = {0}, sSignData[2048 + 1] = {0};
    PosTransDetail stPosTransDetail;
    double dAmout = 0.0;
    int iLen = 0, iRet = -1;
    
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);

    tLog(INFO, "trace_no[%s],merch_id[%s],term_id[%s]", sTraceNo, sMerchId, sTermId);

    /*  判断是否已经上送过 */
    iRet = JudgeSignFlag(sMerchId, sTermId, sTraceNo);
    if (iRet > 0) {
        ErrHanding(pstTransJson, "94", "原流水号[%ld] 原商户号[%s] 原终端号[%s]已经上送过电子签名." \
                , sTraceNo, sMerchId, sTermId);
        return -1;
    } else if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "原流水号[%ld] 原商户号[%s] 原终端号[%s]检查电子签名标志失败." \
                , sTraceNo, sMerchId, sTermId);
        return -1;
    }

    /* 查找原交易 */
    MEMSET_ST(stPosTransDetail);
    if (FindRevAcctJon(&stPosTransDetail, sMerchId, sTermId, sTraceNo) != 0) {
        ErrHanding(pstTransJson, "25", "原交易检查失败,无原交易流水,原流水号[%ld] 原商户号[%s] 原终端号[%s]." \
                , sTraceNo, sMerchId, sTermId);
        return -1;
    }

    /* 检查账号 */
#if 0 
    GET_STR_KEY(pstTransJson, "card_no", sCardNo)
    if (strcmp(stPosTransDetail.sCardNo, sCardNo)) {
        ErrHanding(pstTransJson, "25", "原交易检查失败,卡号不一致.原卡号[%s],上送卡号[%s]."
                , stPosTransDetail.sCardNo, sCardNo);
        return -1;
    }
#endif
    /* 金额 */
    GET_DOU_KEY(pstTransJson, "amount", dAmout);
    if (!DBL_EQ(dAmout, stPosTransDetail.dAmount * 100)) {
        ErrHanding(pstTransJson, "25", "原交易检查失败,交易金额不一致.原金额[%.02f],上送金额[%.02f]."
                , stPosTransDetail.dAmount, dAmout / 100);
        return -1;
    }

    GET_STR_KEY(pstTransJson, "62_req_data", sData1);
    GET_INT_KEY(pstTransJson, "62_req_len", iLen);
    tLog(INFO, "data len [%d]", iLen);
    tAsc2Bcd((UCHAR*) sSignData, (UCHAR*) sData1, iLen << 1, LEFT_ALIGN);

    if (UpdAcctJonForSign(iLen, sSignData, sMerchId, sTermId, sTraceNo) < 0) {
        ErrHanding(pstTransJson, "96", "更新商户号[%s]终端号[%s]流水号[%ld]的电子签名失败.", \
	  sMerchId, sTermId, sTraceNo);
        return -1;
    }
    tLog(INFO, "更新商户号[%s]终端号[%s]流水号[%s]的电子签名成功.", sMerchId, sTermId, sTraceNo);
    return 0;
}

