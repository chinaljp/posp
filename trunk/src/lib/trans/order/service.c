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
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRespDesc[50] = {0};
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_STR_KEY(pstRepJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRepJson, "resp_desc", sRespDesc);
    SET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    SET_STR_KEY(pstDataJson, "resp_desc", sRespDesc);
    ErrHanding(pstDataJson, sRespCode);
    return 0;
}

/* ת������Ҫ����pcKey��������Ȳ���Ҫת��������pcKey=NULL 
 * ֻ�������л��
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    char sTraceNo[TRACE_NO_LEN + 1] = {0};
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "trace_no", sTraceNo);
    snprintf(pcKey, tKey, "%s_%s", GetSvrId(), sTraceNo);
    tLog(DEBUG, "Key[%s]", pcKey);
    return 1;
}

int GetSvrName(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson) {
    snprintf(pSvrId, 11, "JNORDER_Q");
    return 0;
}