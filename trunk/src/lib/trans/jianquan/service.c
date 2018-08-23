/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_extiso.h"
#include "t_macro.h"
#include "resp_code.h"

/* ��ܵĺ��� */
extern char *GetSvrId();

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0) {
        return -1;
    }
    return 0;
}

/* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    return 0;
}

/* ���ش�����Ӧ�����Ϣcopy��data�� */
int RespPro(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRespDesc[163] = {0}, sSeqNo[50] = {0};
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_STR_KEY(pstRepJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRepJson, "resp_desc", sRespDesc);
    GET_STR_KEY(pstRepJson, "qr_order_no", sSeqNo);
    SET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    SET_STR_KEY(pstDataJson, "resp_desc", sRespDesc);
    SET_STR_KEY(pstDataJson, "qr_order_no", sSeqNo);
    
    //��ֹ��Ȩ������Ϣresp_desc������
    if(strlen(sRespDesc) == 0)
        ErrHanding(pstDataJson, sRespCode);
    else{
        SET_STR_KEY(pstDataJson, "resp_log", sRespDesc);
    }
    return 0;
}

/* ת������Ҫ����pcKey��������Ȳ���Ҫת��������pcKey=NULL 
 * ֻ�������л��
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    cJSON *pstDataJson = NULL;
    char sRrn[RRN_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN + 1] = {0}, sTransCode[6 + 1] = {0};
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "rrn", sRrn);
    snprintf(pcKey, tKey, "%s_%s%s", GetSvrId(), sTransDate, sRrn);
    tLog(DEBUG, "Key[%s]", pcKey);

    if (!memcmp(sTransCode, "02A100", 6) ||
            !memcmp(sTransCode, "0AA100", 6)) {
        return 0;
    }
    return 1;
}

int RespSucc(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    ErrHanding(pstDataJson, "00", "���׳ɹ�");
    return 0;
}

int GetSvrName(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson) {
    char sTransCode[TRANS_CODE_LEN + 1];
    char sSvrId[SVRID_LEN + 1] = {0};
    cJSON *pstDataJson = NULL;

    pstDataJson = GET_JSON_KEY(pstInJson, "data");
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    if (!memcmp(sTransCode, "02A100", 6) ||
            !memcmp(sTransCode, "0AA100", 6)) {
        GET_STR_KEY(pstInJson, "svrid", sSvrId);
        strcpy(pSvrId, sSvrId);
    } else {
        snprintf(pSvrId, tSvrId, "AUTH_Q");
    }
    return 0;
}