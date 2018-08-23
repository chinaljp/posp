/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "auth.h"
#include "t_cjson.h"
#include "tKms.h"

int FindJqCardBin(CardBin *pstCardBin, char *pstCardNo) {
    char sTrack[105] = {0}, sSqlStr[512] = {0};
    CardBin stCardBin;
    char cTrackNo, sCardType[2] = {0}, sAcqFlag[2] = {0};
    char sCardlen[2 + 1] = {0};
    OCI_Resultset *pstRes = NULL;


    strcpy(sTrack, pstCardNo);
    cTrackNo = '2';
    sprintf(sCardlen, "%d", strlen(sTrack));
    tLog(DEBUG, "CARD_NO_LEN: %s, TrackNo: %c, Track: %s", sCardlen, cTrackNo, sTrack);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  CARD_TYPE, CARD_NAME, ISS_BANK_ID, CARD_BIN, CARD_BIN_LEN, CARD_NO_LEN \
    FROM S_CARD_BIN WHERE CARD_NO_LEN = '%s'  \
    AND INSTR( '%s', CARD_BIN, 1, 1 )= 1  \
    ORDER BY CARD_BIN_LEN DESC, CARD_NO_LEN DESC", sCardlen, sTrack);
    tLog(INFO, "%s", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }



    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, sCardType);
        stCardBin.cCardType = sCardType[0];
        STRV(pstRes, 2, stCardBin.sCardName);
        STRV(pstRes, 3, stCardBin.sIssBankId);
        STRV(pstRes, 4, stCardBin.sCardBin);
        INTV(pstRes, 5, stCardBin.lCardBinLen);
        INTV(pstRes, 6, stCardBin.lCardNoLen);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }

    tLog(INFO, "��������BIN��Ϣ.");

    memcpy(pstCardBin, &stCardBin, sizeof (CardBin));
    tReleaseRes(pstRes);
    return 0;
}

int FindCardName(char * pstMerchid, char *pstName, char *pcIdCard) {
    char sPrivateData[100], sName[100] = {0}, sIDCard[100] = {0};
    char sSqlStr[256] = {0}, sDName[100] = {0}, sIDCardD[100] = {0};
    OCI_Resultset *pstRes = NULL;
    char sKey[128 + 1] = {0};
    if (FindValueByKey(sKey, "kms.encdata.key") < 0) {
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    strcpy(sPrivateData, pstName);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT ACCOUNT_NAME_ENCRYPT,ID_CARD_ENCRYPT  FROM b_merch_account WHERE MERCH_ID=%s", pstMerchid);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sName);
        STRV(pstRes, 2, sIDCard);
    }

    tTrim(sName);
    tTrim(sPrivateData);

    if (tHsm_Dec_Asc_Data(sDName, sKey, sName) < 0) {
        tLog(ERROR, "���ܽ�������������ʧ��.");
        tReleaseRes(pstRes);
        return -1;
    }

    if (tHsm_Dec_Asc_Data(sIDCardD, sKey, sIDCard) < 0) {
        tLog(ERROR, "���ܽ��������֤������ʧ��.");
        tReleaseRes(pstRes);
        return -1;
    }

    tLog(ERROR, "��������[%s]", sPrivateData);
    tLog(ERROR, "��������[%s]", sDName);

    tLog(ERROR, "�������֤��[%s]", pcIdCard);
    tLog(ERROR, "�������֤��[%s]", sIDCardD);

    tReleaseRes(pstRes);
    if (strcmp(sDName, sPrivateData)) {
        tLog(ERROR, "��Ȩ�����������������");
        return 1;
    }
    if (strcmp(sIDCardD, pcIdCard)) {
        tLog(ERROR, "��Ȩ������������֤�Ų���");
        return 2;
    }

    return 0;
}

int FindCardNum(cJSON *pstNetTran) {

    char sMerchId[16] = {0}, sCardNo[20] = {0}, sUserId[50] = {0};
    char sSqlStr[512] = {0};
    int iNum;
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstNetTran, "merch_id", sMerchId);
    GET_STR_KEY(pstNetTran, "card_no", sCardNo);
    GET_STR_KEY(pstNetTran, "user_code", sUserId);

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_MERCH_AUTH_BASIC  \
    WHERE USER_CODE = %s AND ACCOUNT_NO = %s  AND STATE = '00'", sUserId, sCardNo);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iNum);
    }

    tLog(ERROR, "�̻�[%s]����[%s]����:%d", sMerchId, sCardNo, iNum);
    tReleaseRes(pstRes);
    if (iNum >= 1) {
        tLog(ERROR, "�̻�[%s]����[%s]��������Ȩ...", sMerchId, sCardNo);
        return -1;
    }
    return 0;

}

int FindSamCardNum(cJSON *pstNetTran) {

    char sMerchId[16] = {0}, sCardNo[20] = {0}, sUserId[50] = {0};
    char sSqlStr[512] = {0};
    char sECardNo[128 + 1] = {0}, sKey[128 + 1] = {0};
    int iNum;
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstNetTran, "merch_id", sMerchId);
    GET_STR_KEY(pstNetTran, "card_no", sCardNo);
    GET_STR_KEY(pstNetTran, "user_code", sUserId);


    if (FindValueByKey(sKey, "kms.encdata.key") < 0) {
        ErrHanding(pstNetTran, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0) {
        ErrHanding(pstNetTran, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }


    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_WHITE_SAMCARD_LIST  \
    WHERE USER_CODE = %s AND CARD_NO_ENCRYPT = '%s'  AND STATUS = '1'", sUserId, sECardNo);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iNum);
    }

    tReleaseRes(pstRes);
    if (iNum >= 1) {
        tLog(ERROR, "�̻�[%s]����[%s]��������Ȩ...", sMerchId, sCardNo);
        return 1;
    }
    return 0;

}


#if 0

int UpautoFlag(cJSON *pstNetTran) {

    char sReqSn[13] = {0}, sState[3] = {0}, sRespDesc[100] = {0};
    char sSqlStr[256] = {0};
    char sRrn[13] = {0}, sRespCode[3] = {0};
    OCI_Resultset *pstRes = NULL;


    GET_STR_KEY(pstNetTran, "rrn", sRrn);
    GET_STR_KEY(pstNetTran, "resp_code", sRespCode);
    GET_STR_KEY(pstNetTran, "resp_desc", sRespDesc);

    if (!strcmp(sRespCode, "00")) {
        strcpy(sState, "00");
    } else if (!strcmp(sRespCode, "W2") ||
            !strcmp(sRespCode, "W3") ||
            !strcmp(sRespCode, "W4") ||
            !strcmp(sRespCode, "W5") ||
            !strcmp(sRespCode, "W6")) {
        strcpy(sState, "02");
    } else {
        strcpy(sState, "02");
    }

    tLog(INFO, "sRespCode %s", sRespCode);
    tLog(INFO, "sRespDesc %s", sRespDesc);

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_AUTH_BASIC  SET STATE = '%s' ,REQ_MSG = '%s' WHERE REQ_SN ='%s'", sState, sRespDesc, sRrn);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}
#endif

int InstLimit(cJSON *pstNetTran) {

    char sUserCode[50 + 1], sDate[12], sId[13];
    double dTotalLimit, dUserdLimit, dUsableLimit;
    char sSqlStr[256] = {0};
    char sMerchId[16] = {0}, sRrn[13] = {0};
    int icnt = 0;
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstNetTran, "merch_id", sMerchId);
    GET_STR_KEY(pstNetTran, "user_code", sUserCode);
    GET_STR_KEY(pstNetTran, "rrn", sRrn);
    tGetDate(sDate, "-", -1);

    tLog(ERROR, "�����̻���Ȩ��Ϣ[%s]", sMerchId);

    /* ��ѯ�̻��Ƿ��ж����Ϣ��û�оͲ��� */
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1)  FROM B_MERCH_AUTH_LIMIT WHERE USER_CODE = '%s'", sUserCode);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, icnt);
    }

    if (icnt == 0) {
        dTotalLimit = 0.0;
        dUserdLimit = 0.0;
        dUsableLimit = 0.0;

        snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_MERCH_AUTH_LIMIT (USER_CODE,TOTAL_LIMIT,USED_LIMIT,USABLE_LIMIT ) \
			      VALUES ('%s',%f,%f,%f)", sUserCode, dTotalLimit, dUserdLimit, dUsableLimit);
        if (tExecute(&pstRes, sSqlStr) < 0) {
            tLog(ERROR, "sql[%s] err!", sSqlStr);
            return -1;
        }

        tCommit();
    }
    tReleaseRes(pstRes);
    return 0;

}

int FindCont(cJSON *pstNetTran) {

    char sUserCode[16] = {0};
    char sSqlStr[256] = {0};
    int icnt = 0;
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstNetTran, "user_code", sUserCode);

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_MERCH_AUTH_BASIC WHERE USER_CODE ='%s' and STATE = '00'", sUserCode);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, icnt);
    }
    if (InstLimit(pstNetTran) < 0) {
        tLog(ERROR, "�����Ȩ��Ϣʧ��!");
        return -1;
    }
    tLog(INFO, "�̻���Ȩ��Ϣ����[%d]", icnt);
    tReleaseRes(pstRes);
    return icnt;
}

int UpLimit(int iCnt, cJSON *pstNetTran) {

    char sKey[16] = {0}, sUserId[50 + 1] = {0}, sDate[12] = {0}, sUserCode[30] = {0};
    double dTotalLimit = 0, dDebitLimit = 0, dUsableLimit = 0, dEposLimit = 0;
    char sSqlStr[512] = {0};
    int i = iCnt;
    OCI_Resultset *pstRes = NULL;
    GET_STR_KEY(pstNetTran, "user_code", sUserCode);
    tGetDate(sDate, "-", -1);
    //    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT USABLE_LIMIT,USED_LIMIT   FROM B_MERCH_AUTH_LIMIT WHERE USER_CODE= '%s'", sUserCode);
    //    if (tExecute(&pstRes, sSqlStr) < 0) {
    //        tLog(ERROR, "sql[%s] err!", sSqlStr);
    //
    //        return -1;
    //    }
    //    while (OCI_FetchNext(pstRes)) {
    //        DOUV(pstRes, 1, dUsableLimit);
    //        DOUV(pstRes, 2, dUserdLimit);
    //    }
    /* �����ܶ�� */
    if (i >= 1) {
        if (i == 1) {
            /*�����״μ�Ȩ���*/
            if (FindValueByKey(sKey, "MERCH_AUTH_TOTAL_LIMIT_ONCE") < 0) {
                tLog(ERROR, "����key[MERCH_AUTH_TOTAL_LIMIT_ONCE]ֵ,ʧ��.");
                return -1;
            }
            dTotalLimit = atof(sKey);
            /*�����״ν�ǿ���Ȩ���*/
            if (FindValueByKey(sKey, "MERCH_AUTH_DEBIT_LIMIT_ONCE") < 0) {
                tLog(ERROR, "����key[MERCH_AUTH_DEBIT_LIMIT_ONCE]ֵ,ʧ��.");
                return -1;
            }
            dDebitLimit = atof(sKey);
            /*�����״�EPOS��Ȩ���*/
            if (FindValueByKey(sKey, "MERCH_AUTH_EPOS_TOTAL_LIMIT_ONCE") < 0) {
                tLog(ERROR, "����key[MERCH_AUTH_EPOS_TOTAL_LIMIT_ONCE]ֵ,ʧ��.");
                return -1;
            }
            dEposLimit = atof(sKey);

            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_AUTH_LIMIT SET TOTAL_LIMIT = %f,USABLE_LIMIT = %f-USED_LIMIT,"
                    "DEBIT_TOTAL_LIMIT = %f, DEBIT_AVL_AMT = %f - DEBIT_PAID_AMT, EPOS_TOTAL_LIMIT=%f WHERE USER_CODE='%s'", dTotalLimit, dTotalLimit, dDebitLimit, dDebitLimit, dEposLimit, sUserCode);
        } else if (i >= 2) {
            /*���Ҷ��μ�Ȩ���*/
            if (FindValueByKey(sKey, "MERCH_AUTH_TOTAL_LIMIT_TWICE") < 0) {
                tLog(ERROR, "����key[MERCH_AUTH_TOTAL_LIMIT_TWICE]ֵ,ʧ��.");
                return -1;
            }
            dTotalLimit = atof(sKey);
            /*���Ҷ���EPOS��Ȩ���*/
            if (FindValueByKey(sKey, "MERCH_AUTH_EPOS_TOTAL_LIMIT_TWICE") < 0) {
                tLog(ERROR, "����key[MERCH_AUTH_EPOS_TOTAL_LIMIT_TWICE]ֵ,ʧ��.");
                return -1;
            }
            dEposLimit = atof(sKey);
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_AUTH_LIMIT SET TOTAL_LIMIT = %f,USABLE_LIMIT = %f-USED_LIMIT, EPOS_TOTAL_LIMIT=%f "
                    "WHERE USER_CODE='%s'", dTotalLimit, dTotalLimit, dEposLimit, sUserCode);
        }
        //dUsableLimit = dTotalLimit - dUserdLimit;

        if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
            tLog(ERROR, "sql[%s]", sSqlStr);
            return -1;
        }
    }
    /*��ֵ�̻��ܶ��*/
    SET_DOU_KEY(pstNetTran, "total_limit", dTotalLimit);
    SET_DOU_KEY(pstNetTran, "epos_total_limit", dEposLimit);
    tReleaseRes(pstRes);
    return 0;
}

int FindLimit(cJSON *pstNetTran) {

    char sDate[12] = {0}, sUserCode[30] = {0};
    double dTotalLimit = 0, dUserdLimit = 0, dUsableLimit = 0;
    char sSqlStr[256] = {0}, sTotalLimit[13] = {0}, sUsableLimit[13] = {0};
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstNetTran, "user_code", sUserCode);
    /* ��ѯ���ö�Ȳ���ֵ */
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT USABLE_LIMIT,TOTAL_LIMIT  FROM B_MERCH_AUTH_LIMIT WHERE USER_CODE='%s'", sUserCode);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        DOUV(pstRes, 1, dUsableLimit);
        DOUV(pstRes, 2, dTotalLimit);
    }
    if (0 == OCI_GetRowCount(pstRes)) {

        tReleaseRes(pstRes);
        tLog(INFO, "δ��������Ȩ��Ϣ.");
        /* ��ֵ���ö��ֵ */
        SET_STR_KEY(pstNetTran, "acct_type", "10");
        SET_STR_KEY(pstNetTran, "amt_type", "02");
        SET_STR_KEY(pstNetTran, "ccy_code", "156");
        SET_STR_KEY(pstNetTran, "balance_sign", "C");
        sprintf(sUsableLimit, "%012d", 0);
        SET_STR_KEY(pstNetTran, "usable_limit", sUsableLimit);
        /* ��ֵ�ܶ��ֵ */
        SET_STR_KEY(pstNetTran, "amt_type2", "03");
        sprintf(sTotalLimit, "%012d", 0);
        SET_STR_KEY(pstNetTran, "total_limit", sTotalLimit);
        tReleaseRes(pstRes);
        return 0;
    }

    tLog(INFO, "��Ȳ�ѯ��Ȳ�ѯ");

    /* ��ֵ���ö��ֵ */
    SET_STR_KEY(pstNetTran, "acct_type", "10");
    SET_STR_KEY(pstNetTran, "amt_type", "02");
    SET_STR_KEY(pstNetTran, "ccy_code", "156");
    SET_STR_KEY(pstNetTran, "balance_sign", "C");

    sprintf(sUsableLimit, "%012d", (int) ((dUsableLimit)*100));
    SET_STR_KEY(pstNetTran, "usable_limit", sUsableLimit);

    /* ��ֵ�ܶ��ֵ */
    SET_STR_KEY(pstNetTran, "amt_type2", "03");
    sprintf(sTotalLimit, "%012d", (int) ((dTotalLimit)*100));
    SET_STR_KEY(pstNetTran, "total_limit", sTotalLimit);
    tLog(INFO, "�̻����ö��[%s]", sUsableLimit);
    tLog(INFO, "�̻��ܶ��[%s]", sTotalLimit);
    tReleaseRes(pstRes);
    return 0;
}

int InstTranFlow(cJSON *pstNetTran) {

    char sReqSn[13], sDate[13], cIdType, sCardType[2] = {0};
    char sMerchId[16] = {0}, sCardNo[20] = {0}, sMobile[12] = {0}, sCerNo[19] = {0}, sAccName[20] = {0}, sRrn[13] = {0}, sUserId[50] = {0};
    char sSqlStr[512] = {0}, sSqlStr2[512] = {0}, sTraceNo[7] = {0}, sTransmit_time[11] = {0}, sTransCode[7] = {0}, sTransDate[9] = {0}, sTransTime[17] = {0}, sSysTrace[7] = {0}, sLogicDate[9] = {0}, sRespCode[3] = {0}, sRespDesc[50 + 1] = {0};
    int iNum;
    char sCardNoEnc[255 + 1] = {0}, sKeyName[255 + 1] = {0};
    OCI_Resultset *pstRes = NULL, *pstRes2 = NULL;

    GET_STR_KEY(pstNetTran, "resp_code", sRespCode);
    GET_STR_KEY(pstNetTran, "resp_desc", sRespDesc);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(INFO, "���׼�Ȩʧ�ܣ�����¼��Ȩ��ˮ");
        return 0;
    }

    GET_STR_KEY(pstNetTran, "merch_id", sMerchId);
    GET_STR_KEY(pstNetTran, "card_no", sCardNo);
    GET_STR_KEY(pstNetTran, "mobile", sMobile);
    GET_STR_KEY(pstNetTran, "cer_no", sCerNo);
    GET_STR_KEY(pstNetTran, "account_name", sAccName);
    GET_STR_KEY(pstNetTran, "rrn", sRrn);
    GET_STR_KEY(pstNetTran, "trans_code", sTransCode);
    GET_STR_KEY(pstNetTran, "trans_date", sTransDate);
    GET_STR_KEY(pstNetTran, "trans_time", sTransTime);
    GET_STR_KEY(pstNetTran, "sys_trace", sSysTrace);
    GET_STR_KEY(pstNetTran, "logic_date", sLogicDate);
    GET_STR_KEY(pstNetTran, "card_type", sCardType);
    GET_STR_KEY(pstNetTran, "trace_no", sTraceNo);
    GET_STR_KEY(pstNetTran, "user_code", sUserId);
    GET_STR_KEY(pstNetTran, "resp_code", sRespCode);
    GET_STR_KEY(pstNetTran, "resp_desc", sRespDesc);
    strcpy(sTransmit_time, sTransDate + 4);
    strcat(sTransmit_time, sTransTime);

    strcpy(sReqSn, sRrn);
    tGetDate(sDate, "-", -1);
    cIdType = '0';
    /* �������Ϊ���׼�Ȩ����ˮ����B_TRANS_AUTH_BASIC����,�������B_MERCH_AUTH_BASIC�� */
    if (!memcmp(sTransCode, "0AA001", 6)) {
        snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_TRANS_AUTH_BASIC ( USER_CODE,ACCOUNT_TYPE,ACCOUNT_NO,ACCOUNT_NAME,ID_TYPE,STATE,ID_NUM,ID_TEL,REQ_SN,REQ_MSG,TRANS_DATE \
            ) VALUES( '%s','%c','%s','%s','%c','%s','%s','%s','%s','%s','%s')", sUserId, sCardType[0], sCardNo, sAccName, cIdType, sRespCode, sCerNo, sMobile, sRrn, sRespDesc, sTransDate);
        if (tExecute(&pstRes, sSqlStr) < 0) {
            tLog(ERROR, "sql[%s] err!", sSqlStr);
            return -1;
        }


        if (FindValueByKey(sKeyName, "kms.encdata.key") < 0) {
            tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        } else {
            if (sCardNo[0] != '\0') {
                if (tHsm_Enc_Asc_Data(sCardNoEnc, sKeyName, sCardNo) < 0)
                    tLog(ERROR, "���ż���ʧ��.");
                else
                    tLog(DEBUG, "���ż��ܳɹ�.");
            }
        }
        {/* �ж�ͬ����ˢ�����������Ƿ��Ѿ����� */
            snprintf(sSqlStr2, sizeof (sSqlStr2), "select count(1) from B_WHITE_SAMCARD_LIST where CARD_NO_ENCRYPT = '%s' and user_code='%s' and status = '1'", sCardNoEnc, sUserId);
            if (tExecute(&pstRes2, sSqlStr2) < 0) {
                tLog(ERROR, "sql[%s] err!", sSqlStr2);
                return -1;
            }

            while (OCI_FetchNext(pstRes2)) {
                DOUV(pstRes2, 1, iNum);
            }

            if (iNum >= 1) {
                tLog(INFO, "�������ڰ�����������룬����[%s]", sCardNo);
                tReleaseRes(pstRes);
                tReleaseRes(pstRes2);
                return 0;
            }
        }
        if (sCardNo[0] != '\0')
            tScreenStr(sCardNo, strlen(sCardNo), '*', 6, 4);


        snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_WHITE_SAMCARD_LIST ( USER_CODE,CARD_TYPE,CARD_NO,STATUS,TRANS_DATE,CARD_NO_ENCRYPT \
            ) VALUES( '%s','%c','%s','1','%s','%s')", sUserId, sCardType[0], sCardNo, sTransDate, sCardNoEnc);
        if (tExecute(&pstRes, sSqlStr) < 0) {
            tLog(ERROR, "sql[%s] err!", sSqlStr);
            return -1;
        }

    } else if (!memcmp(sTransCode, "0AA002", 6)) {

        if (FindValueByKey(sKeyName, "kms.encdata.key") < 0) {
            tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        } else {
            if (sCardNo[0] != '\0') {
                if (tHsm_Enc_Asc_Data(sCardNoEnc, sKeyName, sCardNo) < 0)
                    tLog(ERROR, "���ż���ʧ��.");
                else
                    tLog(DEBUG, "���ż��ܳɹ�.");
            }
        }
        {/* �ж�ͬ����ˢ�����������Ƿ��Ѿ����� */
            snprintf(sSqlStr2, sizeof (sSqlStr2), "select count(1) from B_WHITE_SAMCARD_LIST where CARD_NO_ENCRYPT = '%s' and user_code='%s' and status = '1'", sCardNoEnc, sUserId);
            if (tExecute(&pstRes2, sSqlStr2) < 0) {
                tLog(ERROR, "sql[%s] err!", sSqlStr2);
                return -1;
            }

            while (OCI_FetchNext(pstRes2)) {
                DOUV(pstRes2, 1, iNum);
            }

            if (iNum >= 1) {
                tLog(INFO, "�������ڰ�����������룬����[%s]", sCardNo);
                tReleaseRes(pstRes2);
                return 0;
            }

        }
        if (sCardNo[0] != '\0')
            tScreenStr(sCardNo, strlen(sCardNo), '*', 6, 4);


        snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_WHITE_SAMCARD_LIST ( USER_CODE,CARD_TYPE,CARD_NO,STATUS,TRANS_DATE,CARD_NO_ENCRYPT \
            ) VALUES( '%s','%c','%s','1','%s','%s')", sUserId, sCardType[0], sCardNo, sTransDate, sCardNoEnc);
        if (tExecute(&pstRes, sSqlStr) < 0) {
            tLog(ERROR, "sql[%s] err!", sSqlStr);
            return -1;
        }
    } else {
        snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_MERCH_AUTH_BASIC ( USER_CODE,ACCOUNT_TYPE,ACCOUNT_NO,ACCOUNT_NAME,ID_TYPE,STATE,ID_NUM,ID_TEL,REQ_SN,REQ_MSG,TRANS_DATE \
            ) VALUES( '%s','%c','%s','%s','%c','%s','%s','%s','%s','%s','%s')", sUserId, sCardType[0], sCardNo, sAccName, cIdType, sRespCode, sCerNo, sMobile, sRrn, sRespDesc, sTransDate);
        if (tExecute(&pstRes, sSqlStr) < 0) {
            tLog(ERROR, "sql[%s] err!", sSqlStr);
            return -1;
        }
    }
    tReleaseRes(pstRes);
    if (!memcmp(sTransCode, "0AA001", 6) || !memcmp(sTransCode, "0AA002", 6)) {
        tReleaseRes(pstRes2);
    }
    return 0;
}

int insertwhiteFlow(cJSON *pstNetTran) {
    char sReqSn[13], sDate[13], cIdType, sCardType[2] = {0};
    char sMerchId[16] = {0}, sCardNo[20] = {0}, sMobile[12] = {0}, sCerNo[19] = {0}, sAccName[20] = {0}, sRrn[13] = {0};
    char sSqlStr[512] = {0}, sTransmit_time[11] = {0}, sTransDate[9] = {0}, sTransTime[17] = {0};
    int iNum = 0;
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstNetTran, "card_no", sCardNo);
    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from b_trans_card_white_list where account_no = '%s'", sCardNo);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        DOUV(pstRes, 1, iNum);
    }

    if (iNum >= 1) {
        tLog(INFO, "�������ڰ�����������룬����[%s]", sCardNo);
        tReleaseRes(pstRes);
        return 0;
    }

    GET_STR_KEY(pstNetTran, "merch_id", sMerchId);
    GET_STR_KEY(pstNetTran, "mobile", sMobile);
    GET_STR_KEY(pstNetTran, "cer_no", sCerNo);
    GET_STR_KEY(pstNetTran, "account_name", sAccName);
    GET_STR_KEY(pstNetTran, "card_type", sCardType);

    strcpy(sTransmit_time, sTransDate + 4);
    strcat(sTransmit_time, sTransTime);

    strcpy(sReqSn, sRrn);
    tGetDate(sDate, "-", -1);
    cIdType = '0';
    /* ��¼���׿��Ű�������Ϣ */
    snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO b_trans_card_white_list ( ACCOUNT_TYPE,ACCOUNT_NO,ACCOUNT_NAME,STATE,ID_NUM,ID_TEL \
            ) VALUES( '%c','%s','%s','1','%s','%s')", sCardType[0], sCardNo, sAccName, sCerNo, sMobile);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tLog(INFO, "�������������������[%s]�Ѳ������ְ�����", sCardNo);
    tReleaseRes(pstRes);
    return 0;
}

int JudgewhiteFlow(cJSON *pstNetTran) {
#if 0  
    char sCardNo[20 + 1] = {0};
    char sSqlStr[256] = {0};
    int iNum = 0;
    OCI_Resultset *pstRes = NULL;
    GET_STR_KEY(pstNetTran, "card_no", sCardNo);

    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from (select trans_date from b_trans_auth_basic where  \
     account_no='%s' group by trans_date order by trans_date desc)", sCardNo);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        DOUV(pstRes, 1, iNum);
    }

    if (iNum >= 1) {
        tLog(INFO, "����[%s]�������������", sCardNo);
        if (insertwhiteFlow(pstNetTran) != 0) {
            return -1;
        }
    }
    tReleaseRes(pstRes);
#endif /* �����жϼ�Ȩ������ֱ�Ӳ���D0������ */
    if (insertwhiteFlow(pstNetTran) != 0) {
        return -1;
    }
    return 0;
}

int InstLimitFlow(cJSON *pstNetTran) {

    char sSqlStr[512] = {0}, sMerchId[15 + 1] = {0};
    int iNum = 0, iTotalLimit = 0, iEposTotalLimit = 0;
    double dTotalLimit = 0, dEposTotalLimit = 0;
    OCI_Resultset *pstRes = NULL;
    GET_STR_KEY(pstNetTran, "merch_id", sMerchId);
    GET_DOU_KEY(pstNetTran, "total_limit", dTotalLimit);
    GET_DOU_KEY(pstNetTran, "epos_total_limit", dEposTotalLimit);
    iTotalLimit = dTotalLimit;
    iEposTotalLimit = dEposTotalLimit;

    snprintf(sSqlStr, sizeof (sSqlStr), "insert into b_notice_wallet(id,trans_code,launch_content,content_code,notice_date,notice_time,notice_cnt,status,create_time,create_user_id) \
          values(sys_guid(),'00TA00','{\"withdrawAmountLimit\":%d,\"merchantNo\":\"%s\",\"walletType\":100}','%s',to_char(sysdate,'YYYYMMDD'), \
                                   to_char(sysdate,'HH24MMSS'),0,'0',sysdate,'admin')", iTotalLimit, sMerchId, sMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tLog(INFO, "�̻���ȱ����ˮ����ɹ�");
    tReleaseRes(pstRes);

    snprintf(sSqlStr, sizeof (sSqlStr), "insert into b_notice_wallet(id,trans_code,launch_content,content_code,notice_date,notice_time,notice_cnt,status,create_time,create_user_id) \
          values(sys_guid(),'00TA00','{\"withdrawAmountLimit\":%d,\"merchantNo\":\"%s\",\"walletType\":400}','%s',to_char(sysdate,'YYYYMMDD'), \
                                   to_char(sysdate,'HH24MMSS'),0,'0',sysdate,'admin')", iEposTotalLimit, sMerchId, sMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tLog(INFO, "�̻���ȱ����ˮ����ɹ�");
    tReleaseRes(pstRes);
    return 0;
}