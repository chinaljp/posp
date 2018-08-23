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


extern char *GetSvrId();

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {

    return 0;
}

/* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    return 0;
}

int AppGetKey(char *pcKey, size_t tKey, cJSON *pstInJson, cJSON *pstOutJson) {
    char sRrn[RRN_LEN+1]={0};

    GET_STR_KEY(pstInJson, "rrn", sRrn);
    snprintf(pcKey, tKey, "%s_%s", GetSvrId(),sRrn);
    tLog(DEBUG, "Key[%s]", pcKey);
    return 0;
}

int GetSvrName(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson) {
    tStrCpy(pSvrId, "SCANPAY", 11);
    return 0;
}