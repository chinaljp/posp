/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "postransdetail.h"
void PosTransDetailTrim(PosTransDetail *pstPosTransDetail);

#define POS_TRANS_DETAIL_COLUMN "ID,TRANS_CODE,card_no,CARD_ID,CARD_TYPE,CARD_EXP_DATE,AMOUNT,TRANSMIT_TIME,TRANS_DATE,TRANS_TIME,SYS_TRACE,LOGIC_DATE,SETTLE_DATE,MERCH_ID,TERM_ID,TERM_SN,TRACE_NO,RRN,MCC,INPUT_MODE,COND_CODE,PIN_CODE,ACQ_ID,ISS_ID,\
                AGENT_ID,CHANNEL_ID,CHANNEL_MERCH_ID,CHANNEL_TERM_ID,CHANNEL_TERM_SN,CHANNEL_MCC,TRANS_TYPE,VALID_FLAG,CHECK_FLAG,SETTLE_FLAG,AUTHO_FLAG,BATCH_NO,AUTH_CODE,REFUND_AMT,FEE,FEE_FLAG,FEE_DESC,RESP_CODE,ISTRESP_CODE,\
                RESP_DESC,RESP_ID,SIGN_FLAG,MERCH_ORDER_NO,O_RRN,CHANNEL_RRN,O_TRANS_DATE,SETTLE_ORDER_ID,MERCH_PNAME,CHANNEL_MERCH_NAME,BUSI_TYPE,USER_CODE,agent_organization,CARD_NO_ENCRYPT"

void GetOMsgType(char *pcOMsgType, char *pcTransCode) {
    if (strstr("020000,020002,024102,T20000,024100,M20000,M20002,021000", pcTransCode) != NULL)
    {
        strcat(pcOMsgType, "0200");
    } else if (strstr("024000,024002", pcTransCode) != NULL)
    {
        strcat(pcOMsgType, "0100");
    }
}

void PrtAcctJournal(PosTransDetail *pstPosTransDetail) {
    const char sCardType[][10] = {"��ǿ�", "���ÿ�"};

    tLog(DEBUG, "\n������[%s] ����[%s] ������[%s][%c] ���׽��[%.2f]\n"
            "��������[%8s]     ����ʱ��[%6s]     ��������[%8s]     ϵͳ��ˮ��[%s]\n"
            "������Ч��־[%s] ���˱�־[%s] Ԥ��Ȩ��ɱ�־[%s]\n"
            "ϵͳ����[%s] �̻���ʶ[%s] �ն˱�ʶ[%s]\n"
            "����������[%.02f] ԭ����rrn[%s] "
            "�ն���ˮ��[%s] �յ�����[%s] ��������[%s]\n",
            pstPosTransDetail->sTransCode, pstPosTransDetail->sCardNo,
            pstPosTransDetail->sCardId, sCardType[pstPosTransDetail->sCardType[0] - '0'], pstPosTransDetail->dAmount,\
                    pstPosTransDetail->sTransDate, pstPosTransDetail->sTransTime, \
                    pstPosTransDetail->sSettleDate, pstPosTransDetail->sSysTrace, \
                    pstPosTransDetail->sValidFlag, pstPosTransDetail->sCheckFlag, pstPosTransDetail->sAuthoFlag, \
                    pstPosTransDetail->sLogicDate, pstPosTransDetail->sMerchId, pstPosTransDetail->sTermId,\
                    pstPosTransDetail->dFee, pstPosTransDetail->sORrn, \
                    pstPosTransDetail->sTraceNo, pstPosTransDetail->sAcqId, pstPosTransDetail->sIssId);
}

int GetPosTransDetail(PosTransDetail *pstPosTransDetail, char *pcSql) {
    char sTestName[128 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    PosTransDetail stPosTransDetail;

    if (tExecute(&pstRes, pcSql) < 0)
    {
        tLog(ERROR, "sql[%s] err!", pcSql);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]�����ΪNULL.", pcSql);
        return -1;
    }

    while (OCI_FetchNext(pstRes))
    {
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
        STRV(pstRes, 49, stPosTransDetail.sPufaRrn);
        STRV(pstRes, 50, stPosTransDetail.sOTransDate);
        STRV(pstRes, 51, stPosTransDetail.sSettleOrderId);
        STRV(pstRes, 52, stPosTransDetail.sMerchPName);
        STRV(pstRes, 53, stPosTransDetail.sChannelMerchName);
        STRV(pstRes, 54, stPosTransDetail.sBusiType);
        STRV(pstRes, 55, stPosTransDetail.sUserCode);
        STRV(pstRes, 56, stPosTransDetail.sAgentOrganization);
        STRV(pstRes, 57, stPosTransDetail.sCardNoEnc);
        PosTransDetailTrim(&stPosTransDetail);
    }
    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "δ��������ʷ��ˮ��¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "��������ˮ��Ϣ.");
    PrtAcctJournal(&stPosTransDetail);
    if (FindValueByKey(sTestName, "kms.encdata.key") < 0)
    {
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Dec_Asc_Data(stPosTransDetail.sCardNo, sTestName, stPosTransDetail.sCardNoEnc) < 0)
    {
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    memcpy(pstPosTransDetail, &stPosTransDetail, sizeof (stPosTransDetail));
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
    tTrim(pstPosTransDetail->sMerchPName);
    tTrim(pstPosTransDetail->sChannelMerchName);
    tTrim(pstPosTransDetail->sBusiType);
}

int FindPosTransByTraceCardNo(PosTransDetail *pstPosTransDetail, char *pcMerchId, char *pcTermId, char *pcTermTrace, char *pcCardNo) {
    char sRespCodeDup[3] = "94";
    char sSqlStr[2048];
    char sTestName[128 + 1] = {0}, sCardNo[256] = {0}, sTmp[256] = {0};

    if (FindValueByKey(sTestName, "kms.encdata.key") < 0)
    {
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sCardNo, sTestName, pcCardNo) < 0)
    {
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT %s FROM B_POS_TRANS_DETAIL "
            " WHERE TERM_ID = '%s' AND TRACE_NO = '%s'AND MERCH_ID = '%s' AND RESP_CODE != '%s'"
            " and CARD_NO_ENCRYPT='%s'"
            " AND TRANS_CODE IN (SELECT TRANS_CODE FROM S_TRANS_CODE WHERE SAF_FLAG='1')"
            " order by trans_date desc,trans_time desc"
            , POS_TRANS_DETAIL_COLUMN, pcTermId, pcTermTrace, pcMerchId, sRespCodeDup, sCardNo);

    return GetPosTransDetail(pstPosTransDetail, sSqlStr);
}

void SetOldValue(cJSON * pstTransJson, PosTransDetail *pstPosTransDetail) {
    char sMerchId[16] = {0}, sCardNo[30] = {0};

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    tTrim(pstPosTransDetail->sTransCode);
    tTrim(pstPosTransDetail->sTransmitTime);
    if (strlen(sCardNo) == 0)
    {
        SET_STR_KEY(pstTransJson, "card_no", pstPosTransDetail->sCardNo);
    }
    SET_STR_KEY(pstTransJson, "auth_code", pstPosTransDetail->sAuthCode);

    SET_STR_KEY(pstTransJson, "o_rrn", pstPosTransDetail->sRrn);

    /* �ַ�ר��key =======*/
    SET_STR_KEY(pstTransJson, "o_pufa_rrn", pstPosTransDetail->sPufaRrn);
    SET_STR_KEY(pstTransJson, "o_sys_trace", pstPosTransDetail->sSysTrace);
    SET_STR_KEY(pstTransJson, "o_batch_no", pstPosTransDetail->sBatchNo);
    /* ==================*/

    SET_STR_KEY(pstTransJson, "o_trans_date", pstPosTransDetail->sTransDate);
    SET_STR_KEY(pstTransJson, "oo_rrn", pstPosTransDetail->sORrn);
    SET_STR_KEY(pstTransJson, "oo_trans_date", pstPosTransDetail->sOTransDate);

    SET_STR_KEY(pstTransJson, "channel_id", pstPosTransDetail->sChannelId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", pstPosTransDetail->sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", pstPosTransDetail->sChannelTermId);
    SET_STR_KEY(pstTransJson, "trans_type", pstPosTransDetail->sTransType);
    SET_STR_KEY(pstTransJson, "card_type", pstPosTransDetail->sCardType);
    SET_STR_KEY(pstTransJson, "card_bin", pstPosTransDetail->sCardId);
    SET_STR_KEY(pstTransJson, "agent_id", pstPosTransDetail->sAgentId);
    SET_DOU_KEY(pstTransJson, "fee", pstPosTransDetail->dFee);
    SET_STR_KEY(pstTransJson, "settle_date", pstPosTransDetail->sSettleDate);
    SET_STR_KEY(pstTransJson, "term_sn", pstPosTransDetail->sTermSn);
    SET_STR_KEY(pstTransJson, "channel_merch_name", pstPosTransDetail->sChannelMerchName);
    SET_STR_KEY(pstTransJson, "busi_type", pstPosTransDetail->sBusiType);
    SET_STR_KEY(pstTransJson, "fee_desc", pstPosTransDetail->sFeeDesc);
    SET_STR_KEY(pstTransJson, "fee_flag", pstPosTransDetail->sFeeFlag);
    SET_STR_KEY(pstTransJson, "iss_id", pstPosTransDetail->sIssId);
    SET_STR_KEY(pstTransJson, "user_code", pstPosTransDetail->sUserCode);
    SET_STR_KEY(pstTransJson, "merch_p_name", pstPosTransDetail->sMerchPName);
    SET_STR_KEY(pstTransJson, "mcc", pstPosTransDetail->sMcc);
    SET_STR_KEY(pstTransJson, "agent_organization", pstPosTransDetail->sAgentOrganization);
}
/******************************************************************************/
/*      ������:     ChkOriginInfo()                                           */
/*      ����˵��:   �������ԭ��ˮ                                                */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int Chk0002TransDetail(cJSON *pstTransJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};
    char sTransCode[9] = {0}, sCardNo[30] = {0}, sAddData[100] = {0};
    char sOMsgType[4 + 1] = {0}, sAcqInstId[11 + 1] = {0};

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    if (FindPosTransByTraceCardNo(&stPosTransDetail, sMerchId, sTermId, sOTraceNo, sCardNo) != 0)
    {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ�ܣ���ԭ������ˮ��ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sOTraceNo, sMerchId, sTermId);
        return -1;
    }

    SetOldValue(pstTransJson, &stPosTransDetail);
    if (!memcmp(stPosTransDetail.sValidFlag, "1", 1))
    {
        ErrHanding(pstTransJson, "C9", "ԭ�����ѳ�����ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s] ԭ����[%s].", \
                sTraceNo, sMerchId, sTermId, sCardNo);
        return -1;
    }

    GetOMsgType(sOMsgType, stPosTransDetail.sTransCode);
    strcat(sAcqInstId, "0004900");
    tStrCpy(sAcqInstId + 7, stPosTransDetail.sChannelMerchId + 3, 4);
    sprintf(sAddData, "%s%06s%10s%11s%11s"
            , sOMsgType
            , stPosTransDetail.sSysTrace
            , stPosTransDetail.sTransmitTime
            , sAcqInstId
            , "00049000000");
    SET_STR_KEY(pstTransJson, "90_data", sAddData);

    tLog(INFO, "ԭ�����̻���[%s],�ն˺�[%s],������ˮ��[%s],��������[%s],����ʱ��[%s]", \
            sMerchId, sTermId, sOTraceNo, stPosTransDetail.sSettleDate, stPosTransDetail.sTransmitTime);
    return 0;
}

/* ���ѳ���,��������,Ԥ��Ȩ�������ף����ԭ���� */
int Chk0003TransDetail(cJSON *pstTransJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sAddData[100] = {0};
    char sOMsgType[4 + 1] = {0}, sAcqInstId[11 + 1] = {0};
    char sTransCode[9] = {0}, sCardNo[30] = {0};
    double dAmount = 0;

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    dAmount = dAmount / 100;

    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));

    if (FindPosTransByTraceCardNo(&stPosTransDetail, sMerchId, sTermId, sTraceNo, sCardNo) != 0)
    {
        ErrHanding(pstTransJson, "00", "��ԭ������ˮ��ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sTraceNo, sMerchId, sTermId);
        return -1;
    }

    SetOldValue(pstTransJson, &stPosTransDetail);
    if (!memcmp(stPosTransDetail.sValidFlag, "2", 1))
    {
        ErrHanding(pstTransJson, "00", "ԭ�����ѳ�����ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sTraceNo, sMerchId, sTermId);
        return -1;
    }

    if (memcmp(stPosTransDetail.sIstRespCode, "00", 2) && memcmp(stPosTransDetail.sRespCode, "XX", 2))
    {
        ErrHanding(pstTransJson, "00", "ԭ����ʧ��[%s]��ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                stPosTransDetail.sIstRespCode, sTraceNo, sMerchId, sTermId);
        return -1;
    }

    if (!DBL_EQ(stPosTransDetail.dAmount, dAmount))
    {
        ErrHanding(pstTransJson, "25", "����[%f:%f]��ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                stPosTransDetail.dAmount, dAmount, sTraceNo, sMerchId, sTermId);
        return -1;
    }

    SET_STR_KEY(pstTransJson, "sys_trace", stPosTransDetail.sSysTrace);
    SET_STR_KEY(pstTransJson, "trans_time", stPosTransDetail.sTransTime);
    SET_STR_KEY(pstTransJson, "trans_date", stPosTransDetail.sTransDate);

    GetOMsgType(sOMsgType, stPosTransDetail.sTransCode);
    strcat(sAcqInstId, "0004900");
    tStrCpy(sAcqInstId + 7, stPosTransDetail.sChannelMerchId + 3, 4);
    sprintf(sAddData, "%s%06s%10s%11s%11s"
            , sOMsgType
            , stPosTransDetail.sSysTrace
            , stPosTransDetail.sTransmitTime
            , sAcqInstId
            , "00049000000");
    SET_STR_KEY(pstTransJson, "90_data", sAddData);
    tLog(INFO, "ԭ�����̻���[%s],�ն˺�[%s],������ˮ��[%s],��������[%s],����ʱ��[%s]", \
            sMerchId, sTermId, sTraceNo, stPosTransDetail.sSettleDate, stPosTransDetail.sTransmitTime);
    return 0;
}

/* ����valid_flag��־ */
int UpdTransDetail(char *pcTable, char *pcDate, char *pcRrn, char *pcValidFlag) {
    char sSqlStr[1024];
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE %s SET "
            " VALID_FLAG= '%s' "
            " WHERE trans_date = '%s'"
            " AND rrn = '%s' "
            , pcTable, pcValidFlag, pcDate, pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0)
    {
        tLog(ERROR, "����ԭ����[%s:%s]valid_flag[%s]ʧ��.", pcDate, pcRrn, pcValidFlag);
        tLog(ERROR, "sql[%s]", sSqlStr);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "����ԭ����[%s:%s]valid_flag[%s]�ɹ�[%d].", pcDate, pcRrn, pcValidFlag, tGetAffectedRows(pstRes));
    return 0;
}

/* �������ѳ���ԭ���� */
int UpdPosTransDetail(cJSON *pstTransJson, int *piFlag) {
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "����ʧ��[%s],���ָ�ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "1") < 0)
    {
        ErrHanding(pstTransJson, sRespCode, "����ԭ���ѽ���[%s:%s]��Ч��־[1]ʧ��.", sOTransDate, sORrn);
        return 0;
    }
    tLog(INFO, "����ԭ���ѽ���[%s:%s]��Ч��־[1]�ɹ�.", sOTransDate, sORrn);
    return 0;
}

/* �������ѳ���,Ԥ��Ȩ����ԭ���� */
int Upd0003PosTransDetail(cJSON *pstTransJson, int *piFlag) {
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "����ʧ��[%s],���ָ�ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "2") < 0)
    {
        ErrHanding(pstTransJson, sRespCode, "����ԭ���ѽ���[%s:%s]��Ч��־[2]ʧ��.", sOTransDate, sORrn);
        return 0;
    }
    tLog(DEBUG, "����ԭ���ѽ���[%s:%s]��Ч��־[2]�ɹ�.", sOTransDate, sORrn);
    return 0;
}

/* �������ѳ�������ԭ���� */
int Upd0023PosTransDetail(cJSON *pstTransJson, int *piFlag) {
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    char sOORrn[12 + 1] = {0}, sOOTransDate[8 + 1] = {0};
    int iRet = -1;

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "����ʧ��[%s],���ָ�ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "2") < 0)
    {
        ErrHanding(pstTransJson, "96", "����ԭ��������[%s:%s]��Ч��־[2]ʧ��.", sOTransDate, sORrn);
        return -1;
    }
    tLog(INFO, "����ԭ��������[%s:%s]��Ч��־[2]�ɹ�.", sOTransDate, sORrn);
    GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
    GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOOTransDate, sOORrn, "0") < 0)
    {
        ErrHanding(pstTransJson, "96", "����ԭ���ѽ���[%s:%s]��Ч��־[0]ʧ��.", sOOTransDate, sOORrn);
        return -1;
    }
    tLog(INFO, "����ԭ���ѽ���[%s:%s]��Ч��־[0]�ɹ�.", sOOTransDate, sOORrn);
    return 0;
}

/* �������ѳ���ԭ���� */
int UpdAuthTransDetail(cJSON *pstTransJson, int *piFlag) {
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "����ʧ��[%s],���ָ�ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "1") != 0)
    {
        if (UpdTransDetail("B_POS_TRANS_DETAIL_HIS", sOTransDate, sORrn, "1") != 0)
        {
            ErrHanding(pstTransJson, sRespCode, "����ԭ���ѽ���[%s:%s]��Ч��־[1]ʧ��.", sOTransDate, sORrn);
            return 0;
        }
    }
    tLog(DEBUG, "����ԭ���ѽ���[%s:%s]��Ч��־[1]�ɹ�.", sOTransDate, sORrn);
    return 0;
}

int FindPosTransByAutho(PosTransDetail *pstPosTransDetail, char *pcTable, char *pcDate, char *pcAuth, char *pcCardNo) {
    char sRespCodeDup[3] = "94";
    char sSqlStr[2048];
    char sTestName[128 + 1] = {0}, sECardNo[256] = {0};

    if (FindValueByKey(sTestName, "kms.encdata.key") < 0)
    {
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sTestName, pcCardNo) < 0)
    {
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT %s FROM %s "
            " WHERE AUTH_CODE = '%s' AND TRANS_DATE = '%s' "
            " AND RESP_CODE != '%s' AND CARD_NO_ENCRYPT ='%s' AND TRANS_CODE='024000'"
            , POS_TRANS_DETAIL_COLUMN, pcTable, pcAuth, pcDate, sRespCodeDup, sECardNo);

    return GetPosTransDetail(pstPosTransDetail, sSqlStr);
}

/* Ԥ��Ȩ������Ԥ��Ȩ��� */
int ChkAuthTransDetail(cJSON *pstTransJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sTransDate[8 + 1] = {0};
	char sYear[4 + 1] = {0},sOTmpDate[4 + 1] = {0};
    char sTransCode[9] = {0}, sCardNo[30] = {0}, sAuthCode[6 + 1] = {0}, sOTransDate[8 + 1] = {0}, sAddData[100] = {0};

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_STR_KEY(pstTransJson, "auth_code", sAuthCode);
    //GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
	GET_STR_KEY(pstTransJson, "o_trans_date", sOTmpDate);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    /* ��Ȩ�밴λ���� */
    tTrim(sAuthCode);
    tStrPad(sAuthCode, 6, 0x00 | RIGHT_ALIGN);
	
	/**new version  add by gjq at 20180102*/
    tLog(DEBUG,"Ԥ��Ȩ������Ԥ��Ȩ������͵�ԭ��������sOTmpDate=[%s],���ν�������sTransDate = [%s],sTransDate+4 = [%s] ",
            sOTmpDate,sTransDate,sTransDate+4);
    if( memcmp(sOTmpDate,sTransDate+4,4) <= 0 ) {
        memcpy(sOTransDate,sTransDate,4);
        memcpy(sOTransDate+4,sOTmpDate,4);
        sOTransDate[9] = '\0';
    }
    else {
        memcpy(sYear,sTransDate,4);
        sprintf(sOTransDate,"%d%s",atoi(sYear)-1,sOTmpDate);
    }
    tLog(INFO,"ԭԤ��Ȩ��������sOTransDate = [%s]",sOTransDate);
    /**new version*/
	
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    if (FindPosTransByAutho(&stPosTransDetail, "B_POS_TRANS_DETAIL", sOTransDate, sAuthCode, sCardNo) != 0)
    {
        if (FindPosTransByAutho(&stPosTransDetail, "B_POS_TRANS_DETAIL_HIS", sOTransDate, sAuthCode, sCardNo) != 0)
        {
            ErrHanding(pstTransJson, "25", "ԭԤ��Ȩ���׼��ʧ�ܣ���ԭ������ˮ��ԭ��������[%s] ԭ��Ȩ��[%s].", \
                sOTransDate, sAuthCode);
            return -1;
        }
    }

    SetOldValue(pstTransJson, &stPosTransDetail);
    if (!memcmp(stPosTransDetail.sValidFlag, "1", 1))
    {
        ErrHanding(pstTransJson, "C9", "ԭ�����ѳ�����ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sTraceNo, sMerchId, sTermId);
        return -1;
    }

    sprintf(sAddData, "%s%s%4s"
            , "000000", stPosTransDetail.sSysTrace, stPosTransDetail.sTransDate + 4);
    SET_STR_KEY(pstTransJson, "add_data", sAddData);
    tLog(INFO, "ԭ�����̻���[%s],�ն˺�[%s],������ˮ��[%s],����ʱ��[%s]", \
            sMerchId, sTermId, stPosTransDetail.sTraceNo, stPosTransDetail.sTransmitTime);
    return 0;
}

int FindTransDetailByTermTrace(PosTransDetail *pstPosTransDetail, char *pcMerchId, char *pcTermId, char *pcTermTrace) {
    char sRespCodeDup[3] = "94";
    char sSqlStr[2048];

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT %s FROM B_POS_TRANS_DETAIL "
            " WHERE TERM_ID = '%s' AND TRACE_NO = '%s' AND MERCH_ID = '%s' "
            "AND TRANS_TYPE != '1' AND RESP_CODE != '%s' "
            "AND TRANS_CODE IN ('020000','021000','024000','M20000','U20000','T20000')"
            , POS_TRANS_DETAIL_COLUMN, pcTermId, pcTermTrace, pcMerchId, sRespCodeDup);

    return GetPosTransDetail(pstPosTransDetail, sSqlStr);
}

/* �ű�֪ͨ */
int Chk0400TransDetail(cJSON *pstTransJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sOMsgType[4 + 1] = {0}, sAcqInstId[11 + 1] = {0};
    char sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sAddData[100] = {0};
    int iRet = 0;
    char sTransCode[9] = {0}, sCardNo[30] = {0};

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    MEMSET_ST(stPosTransDetail);
    /* �ǳ�������ʹ��Ԫ��ˮ�� */
    if ('3' != sTransCode[5])
        tStrCpy(sTraceNo, sOTraceNo, 6);

    if (FindTransDetailByTermTrace(&stPosTransDetail, sMerchId, sTermId, sTraceNo) != 0)
    {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ�ܣ���ԭ������ˮ��ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sTraceNo, sMerchId, sTermId);
        return -1;
    }

    SetOldValue(pstTransJson, &stPosTransDetail);
    SET_STR_KEY(pstTransJson, "o_trans_code", stPosTransDetail.sTransCode);
    SET_STR_KEY(pstTransJson, "resp_code", stPosTransDetail.sRespCode);
    SET_STR_KEY(pstTransJson, "trans_time", stPosTransDetail.sTransTime);
    SET_STR_KEY(pstTransJson, "trans_date", stPosTransDetail.sTransDate);


    GetOMsgType(sOMsgType, stPosTransDetail.sTransCode);
    strcat(sAcqInstId, "0004900");
    tStrCpy(sAcqInstId + 7, stPosTransDetail.sChannelMerchId + 3, 4);
    sprintf(sAddData, "%s%06s%10s%11s%11s"
            , sOMsgType
            , stPosTransDetail.sSysTrace
            , stPosTransDetail.sTransmitTime
            , sAcqInstId
            , "00049000000");
    SET_STR_KEY(pstTransJson, "90_data", sAddData);

    tLog(INFO, "ԭ�����̻���[%s],�ն˺�[%s],������ˮ��[%s],��������[%s],����ʱ��[%s]", \
            sMerchId, sTermId, sOTraceNo, stPosTransDetail.sSettleDate, stPosTransDetail.sTransmitTime);
    return 0;
}

int FindPosTransByRrn(PosTransDetail *pstPosTransDetail, char *psTransDate, char *pcRrn) {
    char sSqlStr[2048];

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT %s FROM B_POS_TRANS_DETAIL "
            " WHERE TRANS_DATE = '%s' AND RRN = '%s' AND ISTRESP_CODE = '00' "
            " AND TRANS_CODE IN (SELECT TRANS_CODE FROM S_TRANS_CODE WHERE SAF_FLAG='1')"
            , POS_TRANS_DETAIL_COLUMN, psTransDate, pcRrn);

    return GetPosTransDetail(pstPosTransDetail, sSqlStr);
}

/* �ս�ȷ�Ͻ��ף����ԭ���ף���Ҫ�Ǵ�ԭ������ȡֵ */
int ChkD0TransDetail(cJSON *pstTransJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sMerchId[16] = {0}, sTermId[9] = {0}, sOldRrn[13] = {0}, sTransDate[8 + 1] = {0};

    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    /* ����ԭ���� */
    MEMSET_ST(stPosTransDetail);
    if (FindPosTransByRrn(&stPosTransDetail, sTransDate, sOldRrn) != 0)
    {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ�ܣ���ԭ������ˮ��ԭrrn[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sOldRrn, sMerchId, sTermId);
        return -1;
    }
    tLog(DEBUG, "�̻�[%s]�ն�[%s]ԭ����rrn[%s]���ҳɹ�.", sMerchId, sTermId, sOldRrn);
    if (memcmp(stPosTransDetail.sRespCode, "00", 2))
    {
        ErrHanding(pstTransJson, "25", "�̻�[%s]�ն�[%s]ԭ����rrn[%s]ʧ��, ��ֹ����.", sMerchId, sTermId, sOldRrn);
        return -1;
    }
    SetOldValue(pstTransJson, &stPosTransDetail);

    SET_DOU_KEY(pstTransJson, "amount", stPosTransDetail.dAmount * 100);
    /*add by GJQ at 20171124 begin */
    SET_STR_KEY(pstTransJson, "trans_type", stPosTransDetail.sTransType);
    SET_STR_KEY(pstTransJson, "settle_flag", stPosTransDetail.sSettleFlag);
    /*add by GJQ at 20171124 end   */
    tLog(DEBUG, "amount=%.02f", stPosTransDetail.dAmount);

#if 0
    SET_STR_KEY(pstTransJson, "settle_date", stPosTransDetail.sSettleDate);
    SET_DOU_KEY(pstTransJson, "fee", stPosTransDetail.dFee);
    SET_STR_KEY(pstTransJson, "card_type", stPosTransDetail.sCardType);
    SET_STR_KEY(pstTransJson, "o_trans_date", stPosTransDetail.sTransDate);
    SET_STR_KEY(pstTransJson, "o_resp_code", stPosTransDetail.sRespCode);

#endif
    return 0;
}

int FindPosTransHisByRrn(PosTransDetail *pstPosTransDetail, char *pcMerchId, char *psTransDate, char *pcRrn) {
    char sSqlStr[2048];

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT %s FROM B_POS_TRANS_DETAIL_HIS "
            " WHERE TRANS_DATE = '%s' AND RRN = '%s' AND ISTRESP_CODE = '00' AND MERCH_ID='%s'"
            " AND TRANS_CODE IN ('020000','024100')"
            , POS_TRANS_DETAIL_COLUMN, psTransDate, pcRrn, pcMerchId);
    return GetPosTransDetail(pstPosTransDetail, sSqlStr);
}

/* �˻����� */
int Chk0001TransDetail(cJSON *pstTransJson, int *piFlag) {
    char sOrgTraceNo[6 + 1] = {0}, sAddData[100] = {0};
    char sRrn[12 + 1] = {0}, sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    char sTraceNo[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0};
    char sOMsgType[4 + 1] = {0}, sAcqInstId[8 + 1] = {0}, sCardNo[19 + 1] = {0}, sTransDate[8 + 1] = {0}, sOTmpDate[4 + 1] = {0};
    char sYear[4 + 1] = {0};
    double dTranAmt = 0L, dUnrefundAmt = 0L, dFee = 0;
    PosTransDetail stPosTransDetail;

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTmpDate); //motify by gjq at 20171220
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate); //�������� add by gjq at 20171220
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    dTranAmt /= 100;
    tLog(DEBUG,"�˻����͵�ԭ��������sOTmpDate=[%s],���ν�������sTransDate = [%s],sTransDate+4 = [%s] ",sOTmpDate,sTransDate,sTransDate+4);
    if (memcmp(sOTmpDate, sTransDate + 4, 4) <= 0)
    {
        memcpy(sOTransDate, sTransDate, 4);
        memcpy(sOTransDate + 4, sOTmpDate, 4);
        sOTransDate[9] = '\0';
    } else
    {
        memcpy(sYear, sTransDate, 4);
        sprintf(sOTransDate, "%d%s", atoi(sYear) - 1, sOTmpDate);
    }
    /* ����ԭ���� */
    MEMSET_ST(stPosTransDetail);
    if (FindPosTransHisByRrn(&stPosTransDetail, sMerchId, sOTransDate, sORrn) != 0)
    {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ��,��ԭ������ˮ,ԭ����[%s],ԭrrn[%s],ԭ�̻���[%s],ԭ�ն˺�[%s].", \
                sOTransDate, sORrn, sMerchId, sTermId);
        return -1;
    }

    if (stPosTransDetail.sValidFlag[0] != '0' && stPosTransDetail.sValidFlag[0] != '3')
    {
        ErrHanding(pstTransJson, "12", "ԭ������Ч��־λ[%c].", stPosTransDetail.sValidFlag[0]);
        return -1;
    }

    /* ���ԭ�����Ƿ�ɹ� */
    if (memcmp(stPosTransDetail.sRespCode, "00", 2))
    {
        ErrHanding(pstTransJson, "12", "ԭ����ʧ��,Ӧ����[%s].", stPosTransDetail.sRespCode);
        return -1;
    }
    /* ��鿨�� */
    if (strcmp(stPosTransDetail.sCardNo, sCardNo))
    {
        ErrHanding(pstTransJson, "63", "���Ų�һ�� ԭ����[%s] ���Ϳ���[%s].", stPosTransDetail.sCardNo, sCardNo);
        return -1;
    }
    /* ����� */
    dUnrefundAmt = stPosTransDetail.dAmount - stPosTransDetail.dRefundAmt;
    tLog(DEBUG, "�˿���[%.10f], δ�˽��[%.10f].", dTranAmt, dUnrefundAmt);

    if (dTranAmt - dUnrefundAmt > 0.00001)
    {
        ErrHanding(pstTransJson, "13", "�˿���[%.02f]����δ�˽��[%.02f].", dTranAmt, dUnrefundAmt);
        return -1;
    }

    dUnrefundAmt = dTranAmt + stPosTransDetail.dRefundAmt;
    SET_DOU_KEY(pstTransJson, "refund_amt", dUnrefundAmt);
    DEL_KEY(pstTransJson, "o_trans_date");
    SET_STR_KEY(pstTransJson, "o_trans_date", stPosTransDetail.sTransDate);
    GetOMsgType(sOMsgType, stPosTransDetail.sTransCode);
    strcat(sAcqInstId, "0004900");
    tStrCpy(sAcqInstId + 7, sMerchId + 3, 4);
    sprintf(sAddData, "%s%06s%10s%11s%11s"
            , sOMsgType
            , stPosTransDetail.sSysTrace
            //  , stPosTransDetail.sTransDate + 4
            , stPosTransDetail.sTransmitTime
            , sAcqInstId
            , "00049000000");

    SET_STR_KEY(pstTransJson, "90_data", sAddData);
    SET_STR_KEY(pstTransJson, "auth_code", stPosTransDetail.sAuthCode);
    SET_STR_KEY(pstTransJson, "channel_id", stPosTransDetail.sChannelId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", stPosTransDetail.sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", stPosTransDetail.sChannelTermId);
    SET_STR_KEY(pstTransJson, "channel_merch_name", stPosTransDetail.sChannelMerchName);
    SET_STR_KEY(pstTransJson, "trans_type", stPosTransDetail.sTransType);
    SET_STR_KEY(pstTransJson, "fee_type", stPosTransDetail.sFeeType);
    SET_STR_KEY(pstTransJson, "card_type", stPosTransDetail.sCardType);
    dFee = stPosTransDetail.dFee * dTranAmt / stPosTransDetail.dAmount;
    tMake(&dFee, 2, 'u');
    SET_DOU_KEY(pstTransJson, "fee", dFee);
    /*�˻�����o_amount*/
    SET_DOU_KEY(pstTransJson, "o_amount", stPosTransDetail.dAmount);

    tLog(INFO, "�˻�ԭ���׼��ɹ� ԭ�̻���[%s] ԭ�����ο���[%s] ԭ��������[%s]", \
            sMerchId, sORrn, sOTransDate);
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
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0)
    {
        tLog(ERROR, "����ԭ���� �ο���[%s]��������[%s]���[%lf]valid_flag[%c]ʧ��.", pcRrn, pcTransDate, dRefundAmt, cValidFlag);
        return -1;
    }

    tReleaseRes(pstRes);
    tLog(INFO, "����ԭ���� �ο���[%s]����ʱ��[%s]���[%lf]valid_flag[%c]�ɹ�.", pcRrn, pcTransDate, dRefundAmt, cValidFlag);
    return 0;
}

int Upd0001TransDetail(cJSON *pstTransJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sOTransDate[8 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0};
    double dUnrefundAmt = 0L;

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_DOU_KEY(pstTransJson, "refund_amt", dUnrefundAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2))
    {
        tLog(INFO, "����ʧ��,����Ҫ����ԭ����.");
        return 0;
    }
    /* ����ԭ������ˮ����Ч��־valid_flag */
    if (UpdOrgJonHis(sOTransDate, sORrn, '3', dUnrefundAmt) != 0)
    {
        ErrHanding(pstTransJson, sRespCode, "ԭ���׸��±�־ʧ�ܣ���ԭ���ײο���[%s]��ԭ��������[%s] ���˿���[%lf].", \
                sORrn, sOTransDate, dUnrefundAmt);
        return 0;
    }

    tLog(INFO, "ԭ���׸��±�־�ɹ���ԭ���ײο���[%s]��ԭ��������[%s] ���˿���[%lf].", \
                sORrn, sOTransDate, dUnrefundAmt);
    return 0;
}
