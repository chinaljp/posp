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
#include "t_cjson.h"
#include "t_macro.h"
#include "trans_code.h"

/* ��ܵĺ��� */
char *GetSvrId();
TransCode g_stTransCode;

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0) {
        return -1;
    }
    if (LoadTransCodeByCode(&g_stTransCode, GetSvrId()) < 0) {
        return -1;
    }
    return 0;
}

/* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    return 0;
}

/* �����Ǹ�ͨ�õĽ����������õ��������һ��
 * ʧ�ܵ���Ϣ��ܴ���
 * �����ڽ�������Ҫ����resp_codeӦ����
 */
#if 0

int Transfer(cJSON *pstJson, int *piFlag) {
    char sKey[MSG_KEY_LEN + 1] = {0};
    char sRepSvrId[SVRID_LEN + 1] = {0};
    cJSON *pstDataJson = NULL;

    DUMP_JSON(pstJson);

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "data json is null.");
        return -1;
    }
    AppGetKey(sKey, sizeof (sKey), pstJson);
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());
    tLog(DEBUG, "Rep[%s]", sRepSvrId);
    /* ����Ӧ����Ϣ */
    return tSvcACall("CUPS_Q", sRepSvrId, sKey, pstDataJson);
}
#endif

/* ���ش�����Ӧ�����Ϣcopy��data�� */
int RespPro(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0};
    char sBanlance[AMOUNT_LEN + 1] = {0}, sRrn[12 + 1] = {0};
    char sAcctType[2 + 1] = {0};
    char sAmtType[2 + 1] = {0};
    char sBalCcyCode[3 + 1] = {0};
    char sBalanceSign[1 + 1] = {0};
    char sAuthCode[6 + 1] = {0};
    char sIcData[1024 + 1] = {0};
    char sChannelSettleDate[4 + 1] = {0};
    char sRespId[8 + 1] = {0}, sRespDesc[256 + 1] = {0};

    //DUMP_JSON(pstJson);
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_STR_KEY(pstRepJson, "istresp_code", sRespCode);
    SET_STR_KEY(pstDataJson, "istresp_code", sRespCode);
    /*38�� ��Ȩ��*/
    GET_STR_KEY(pstRepJson, "auth_code", sAuthCode);
    SET_STR_KEY(pstDataJson, "auth_code", sAuthCode);
    /*54��*/
    GET_STR_KEY(pstRepJson, "balance", sBanlance);
    GET_STR_KEY(pstRepJson, "acct_type", sAcctType);
    GET_STR_KEY(pstRepJson, "amt_type", sAmtType);
    GET_STR_KEY(pstRepJson, "balance_sign", sBalanceSign);
    SET_STR_KEY(pstDataJson, "amt_type", sAmtType);
    SET_STR_KEY(pstDataJson, "acct_type", sAcctType);
    SET_STR_KEY(pstDataJson, "balance_sign", sBalanceSign);
    SET_STR_KEY(pstDataJson, "balance1", sBanlance);
    /*55��*/
    GET_STR_KEY(pstRepJson, "ic_data", sIcData);
    DEL_KEY(pstDataJson, "ic_data");
    SET_STR_KEY(pstDataJson, "ic_data", sIcData);

    /*100��*/
    GET_STR_KEY(pstRepJson, "resp_id", sRespId);
    SET_STR_KEY(pstDataJson, "resp_id", sRespId);

    GET_STR_KEY(pstRepJson, "channel_rrn", sRrn);
    SET_STR_KEY(pstDataJson, "channel_rrn", sRrn);
    
    GET_STR_KEY(pstRepJson, "channel_settle_date", sChannelSettleDate);
    SET_STR_KEY(pstDataJson, "channel_settle_date", sChannelSettleDate);

    GetRespCode(sRespDesc, sRespCode);
    ErrHanding(pstDataJson, sRespCode, "����[%s]����[%s]:%s", sRrn, sRespCode, sRespDesc);
    return 0;
}

int RespSucc(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL;

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    SET_STR_KEY(pstDataJson, "resp_id", "99999999");
    ErrHanding(pstDataJson, "00", "���׳ɹ�");
    return 0;
}

/* ת������Ҫ����pcKey��������Ȳ���Ҫת��������pcKey=NULL 
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    cJSON *pstDataJson = NULL;
    //  char sRrn[RRN_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0}, sTransCode[6 + 1] = {0};

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "��ȡdata����ʧ��,��Ϣ��������.");
        return -1;
    }
    GET_STR_KEY(pstDataJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    //  GET_STR_KEY(pstDataJson, "trans_date", sTransDate);

    //snprintf(pcKey, tKey, "ACCT_%s", sSysTrace);
    snprintf(pcKey, tKey, "ACCT_%s%s", sTransCode+4,sSysTrace);
    tLog(DEBUG, "Key[%s]", pcKey);

    /* �ս�ȷ�Ϻ��ֻ��ս�֪ͨ */
    if (!memcmp(sTransCode, "TA0010", 6) ||
            !memcmp(sTransCode, "T20010", 6)) {
        return 0;
    }
    return 1;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstJson) {
    cJSON *pstDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0};
    char sSvrId[SVRID_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0}, sChannelId[8 + 1] = {0};

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "channel_id", sChannelId);
    /* �ַ�����*/
    if (!memcmp(sChannelId, "48560000", 8)) {
        if (sRespCode[0] == '\0') {
            strcpy(pcSvrId, "PUFA_Q");
            tLog(INFO, "ʹ���ַ�����");
        } else {
            GET_STR_KEY(pstJson, "svrid", sSvrId);
            strcpy(pcSvrId, sSvrId);
        }
        return 0;
    }
    /*��������*/
    if (sRespCode[0] == '\0' || !memcmp(sTransCode, "020400", TRANS_CODE_LEN)) {
        strcpy(pcSvrId, "CUPS_Q");
    } else {
        GET_STR_KEY(pstJson, "svrid", sSvrId);
        strcpy(pcSvrId, sSvrId);
    }
    return 0;
}