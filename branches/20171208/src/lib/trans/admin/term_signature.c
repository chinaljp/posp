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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
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
        tLog(ERROR, "δ�ҵ���¼.");
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sSignFlag);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "δ�ҵ��̻�[%s]�ն�[%s]ƾ֤��[%s]��ԭ������ˮ.", pcMerchId, pcTermId, pcTermTrace);
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
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
/*      ������:     SignatureProc()                  	                      */
/*      ����˵��:   ����ǩ������											  */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */
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

    /*  �ж��Ƿ��Ѿ����͹� */
    iRet = JudgeSignFlag(sMerchId, sTermId, sTraceNo);
    if (iRet > 0) {
        ErrHanding(pstTransJson, "94", "ԭ��ˮ��[%ld] ԭ�̻���[%s] ԭ�ն˺�[%s]�Ѿ����͹�����ǩ��." \
                , sTraceNo, sMerchId, sTermId);
        return -1;
    } else if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "ԭ��ˮ��[%ld] ԭ�̻���[%s] ԭ�ն˺�[%s]������ǩ����־ʧ��." \
                , sTraceNo, sMerchId, sTermId);
        return -1;
    }

    /* ����ԭ���� */
    MEMSET_ST(stPosTransDetail);
    if (FindRevAcctJon(&stPosTransDetail, sMerchId, sTermId, sTraceNo) != 0) {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ��,��ԭ������ˮ,ԭ��ˮ��[%ld] ԭ�̻���[%s] ԭ�ն˺�[%s]." \
                , sTraceNo, sMerchId, sTermId);
        return -1;
    }

    /* ����˺� */
#if 0 
    GET_STR_KEY(pstTransJson, "card_no", sCardNo)
    if (strcmp(stPosTransDetail.sCardNo, sCardNo)) {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ��,���Ų�һ��.ԭ����[%s],���Ϳ���[%s]."
                , stPosTransDetail.sCardNo, sCardNo);
        return -1;
    }
#endif
    /* ��� */
    GET_DOU_KEY(pstTransJson, "amount", dAmout);
    if (!DBL_EQ(dAmout, stPosTransDetail.dAmount * 100)) {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ��,���׽�һ��.ԭ���[%.02f],���ͽ��[%.02f]."
                , stPosTransDetail.dAmount, dAmout / 100);
        return -1;
    }

    GET_STR_KEY(pstTransJson, "62_req_data", sData1);
    GET_INT_KEY(pstTransJson, "62_req_len", iLen);
    tLog(INFO, "data len [%d]", iLen);
    tAsc2Bcd((UCHAR*) sSignData, (UCHAR*) sData1, iLen << 1, LEFT_ALIGN);

    if (UpdAcctJonForSign(iLen, sSignData, sMerchId, sTermId, sTraceNo) < 0) {
        ErrHanding(pstTransJson, "96", "�����̻���[%s]�ն˺�[%s]��ˮ��[%ld]�ĵ���ǩ��ʧ��.", \
	  sMerchId, sTermId, sTraceNo);
        return -1;
    }
    tLog(INFO, "�����̻���[%s]�ն˺�[%s]��ˮ��[%s]�ĵ���ǩ���ɹ�.", sMerchId, sTermId, sTraceNo);
    return 0;
}

