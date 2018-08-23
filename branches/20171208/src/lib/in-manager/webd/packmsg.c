/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_extiso.h"
#include "trans_type_tbl.h"
#include "trans_detail.h"
#include "trans_code.h"
#include "t_macro.h"
#include "trace.h"
#include "err.h"
#include "t_db.h"
#include "tKms.h"

char *g_pcMsg = NULL;
int g_iMsgLen = 0;

int FindValueByKey(char *pcValue, char *pcKey) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key='%s'", pcKey);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "����Key[%s]��ֵʧ��.", pcKey);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcValue);
        tTrim(pcValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "��Key[%s]��ֵ.", pcKey);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

/* �������󣬲�� */
int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {
    char sTransCode[TRANS_CODE_LEN + 1] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0}, sTrace[TRACE_NO_LEN + 1] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0};
    char sIdCardNo[128] = {0}, sBankAccountNo[128] = {0}, sBankAccountName[128] = {0};
    char sIdCardNoDec[128] = {0}, sBankAccountNoDec[128] = {0}, sBankAccountNameDec[128] = {0};
    char sTelDec[128] = {0}, sTel[128] = {0};
    char sKeyuKey[128] = {0};
    char sTmp[128] = {0};
    TransCode stTransCode;
    cJSON *pstJson = NULL;

    GET_STR_KEY(pstDataJson, "msg", sMsg);
    int iMsgLen = strlen((const char *) sMsg);

    //tAsc2Bcd(sMsg, (UCHAR*) sTmpMsg, iMsgLen, LEFT_ALIGN);
    pstJson = cJSON_Parse((const char *) sMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "ת����Ϣʧ��,��������.");
        return -1;
    }

    g_pcMsg = (char*) sMsg;
    g_iMsgLen = iMsgLen;

    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    if (sTransCode[0] == '\0') {
        GET_STR_KEY(pstJson, "transCode", sTransCode);
    }
    if (FindTransCode(&stTransCode, sTransCode) < 0) {
        tLog(ERROR, "������δ����,���׷�������.");
        cJSON_Delete(pstJson);
        return -1;
    }

    SET_STR_KEY(pstReqJson, "trans_code", sTransCode);
    if (FindValueByKey(sKeyuKey, "kms.encdata.key") < 0) {
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    GET_STR_KEY(pstJson, "idCardNo", sIdCardNoDec);
    GET_STR_KEY(pstJson, "bankAccountNo", sBankAccountNoDec);
    GET_STR_KEY(pstJson, "bankAccountName", sBankAccountNameDec);
    GET_STR_KEY(pstJson, "mobile", sTelDec);
    if (sIdCardNoDec[0] != '\0') {
        tLog(DEBUG, "sIdCardNoDec[%s]", sIdCardNoDec);
        if (tHsm_Dec_Asc_Data(sIdCardNo, sKeyuKey, sIdCardNoDec) < 0) {
            tLog(ERROR, "tHsm_Dec_Asc_Data idCardNo");
            return -1;
        }
        tLog(DEBUG, "sIdCardNo[%s]", sIdCardNo);
        SET_STR_KEY(pstJson, "idCardNo", sIdCardNo);
    }
    if (sTelDec[0] != '\0') {
        tLog(DEBUG, "sTelDec[%s]", sTelDec);
        if (tHsm_Dec_Asc_Data(sTel, sKeyuKey, sTelDec) < 0) {
            tLog(ERROR, "tHsm_Dec_Asc_Data mobile");
            return -1;
        }
        tLog(DEBUG, "sTel[%s]", sTel);
        SET_STR_KEY(pstJson, "mobile", sTel);
    }
    if (sBankAccountNoDec[0] != '\0') {
        tLog(DEBUG, "sBankAccountNoDec[%s]", sBankAccountNoDec);
        if (tHsm_Dec_Asc_Data(sBankAccountNo, sKeyuKey, sBankAccountNoDec) < 0) {
            tLog(ERROR, "tHsm_Dec_Asc_Data sBankAccountNo");
            return -1;
        }
        tLog(DEBUG, "sBankAccountNo[%s]", sBankAccountNo);
        SET_STR_KEY(pstJson, "bankAccountNo", sBankAccountNo);
    }
    if (sBankAccountNameDec[0] != '\0') {
        tLog(DEBUG, "sBankAccountNameDec[%s]", sBankAccountNameDec);
        if (tHsm_Dec_Asc_Data(sBankAccountName, sKeyuKey, sBankAccountNameDec) < 0) {
            tLog(ERROR, "tHsm_Dec_Asc_Data sBankAccountNo");
            return -1;
        }
        tLog(DEBUG, "sBankAccountName[%s]", sBankAccountName);
        SET_STR_KEY(pstJson, "bankAccountName", sBankAccountName);
    }

    /* ��ȡ�������ڣ�ʱ�䣬rrn��ÿһ�����׶��Ǵ�manager��ͳ��� */
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    snprintf(sTmp, sizeof (sTmp), "%s%s", sDate + 5, sTime);
    SET_STR_KEY(pstReqJson, "trans_date", sDate);
    SET_STR_KEY(pstReqJson, "trans_time", sTime);
    sprintf(sTmp, "%s%s", sDate + 4, sTime);
    SET_STR_KEY(pstReqJson, "transmit_time", sTmp);
    if (GetSysTrace(sTrace) < 0) {
        tLog(ERROR, "��ȡϵͳ��ˮ��ʧ��,���׷���.");
        cJSON_Delete(pstJson);
        return -1;
    }
    sprintf(sTmp, "%s%s", sTime, sTrace);
    SET_STR_KEY(pstReqJson, "rrn", sTmp);
    SET_STR_KEY(pstReqJson, "sys_trace", sTrace);
    SET_STR_KEY(pstReqJson, "logic_date", sDate);

    COPY_JSON(pstReqJson, pstJson);

    //DUMP_JSON(pstReqJson);
    cJSON_Delete(pstJson);
    return 0;
}

//int ModulePack(char *pcMsg, int *piMsgLen, void *pvJson) {

int ResponseMsg(cJSON *pstRepJson, cJSON *pstDataJson) {
    char sTransCode[TRANS_CODE_LEN + 1] = {0};
    TransCode stTransCode;
    char sRespCode[2 + 1] = {0}, sRespDesc[2048] = {0}, *pcMsg = NULL, sTmp[256] = {0}, sBuf[2014] = {0};
    double dFAmount = 0, dTAmount = 0, dDeAmount = 0;
    double dQrFAmount = 0, dQrTAmount = 0, dQrDeAmount = 0;
    //char sMsg[MSG_KEY_LEN] = {0};
    cJSON *pstJson = NULL, *pstSubDataJson = NULL;

    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    /* �����ˮ ���������λ����ĸ��ʹ��INLINE_TRANS_DETAIL�����ֵ�ʹ��POS_TRANS_DETAIL*/
    if (FindTransCode(&stTransCode, sTransCode) < 0) {
        tLog(ERROR, "������δ����,���׷�������.");
        return -1;
    }
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }

    SET_STR_KEY(pstJson, "trans_code", sTransCode);
    SET_STR_KEY(pstJson, "resp_code", sRespCode);

    GET_STR_KEY(pstDataJson, "resp_desc", sRespDesc);
    SET_STR_KEY(pstJson, "resp_desc", sRespDesc);
    if (!memcmp("00TF00", sTransCode, 6)) {
        GET_STR_KEY(pstDataJson, "data", sBuf);
        pstSubDataJson = cJSON_Parse(sBuf);
        if (pstSubDataJson != NULL) {
            /* �Ѷ��� */
            GET_DOU_KEY(pstSubDataJson, "posFrozenTotalAmount", dFAmount);
            GET_DOU_KEY(pstSubDataJson, "qrCodeFrozenTotalAmount", dQrFAmount);
            SET_DOU_KEY(pstJson, "frozenTotalAmount", dFAmount + dQrFAmount);
            /* �ѽⶳ */
            GET_DOU_KEY(pstSubDataJson, "posThawAmount", dTAmount);
            GET_DOU_KEY(pstSubDataJson, "qrCodeThawAmount", dQrTAmount);
            SET_DOU_KEY(pstJson, "thawAmount", dTAmount + dQrTAmount);
            /* �ѿ۳� */
            GET_DOU_KEY(pstSubDataJson, "posDeductAmount", dDeAmount);
            GET_DOU_KEY(pstSubDataJson, "qrCodeDeductAmount", dQrDeAmount);
            SET_DOU_KEY(pstJson, "deductAmount", dDeAmount + dQrDeAmount);
            cJSON_Delete(pstSubDataJson);
        }
    } else if (!memcmp("00T000", sTransCode, 6)) {
        GET_STR_KEY(pstDataJson, "data", sBuf);
        pstSubDataJson = cJSON_Parse(sBuf);
        if (pstSubDataJson != NULL) {
            GET_STR_KEY(pstSubDataJson, "accountUuid", sTmp);
            SET_STR_KEY(pstJson, "accountUuid", sTmp);
            cJSON_Delete(pstSubDataJson);
        }
    }
    pcMsg = cJSON_PrintUnformatted(pstJson);
    SET_STR_KEY(pstRepJson, "msg", (const char*) pcMsg);
    if (pcMsg)
        free(pcMsg);
    cJSON_Delete(pstJson);
    return 0;
}


