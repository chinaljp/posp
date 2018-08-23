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

extern IsoTable g_staIsoTable[];
IsoData g_stIsoData;
unsigned char g_caBuf[MSG_MAX_LEN];

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {
    tInitIso(&g_stIsoData, g_caBuf, MSG_MAX_LEN, BCD_LLVAR, g_staIsoTable);

    return 0;
}

/* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    return 0;
}
/* cups  pstInJson�ǽṹ�壬pstOutJson��8583����*/
/* ����,Ӧ�𶼻ص� */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstInJson, cJSON *pstOutJson) {
    char sTransDate[TRANS_DATE_LEN + 1] = {0}, sSysTrace[TRACE_NO_LEN + 1] = {0};
    char sTransCode[6 + 1] = {0}; /*add by gjq at 20171208*/
    
    //char *sMsg = NULL;
    //DUMP_JSON(pstInJson,sMsg);
    
    GET_STR_KEY(pstInJson, "trans_date", sTransDate);
    GET_STR_KEY(pstInJson, "sys_trace", sSysTrace);
    //snprintf(pcKey, tKey, "PUFA_%s", sSysTrace);
   
    /*BEGIN  motify by gjq at 20171208*/
    GET_STR_KEY(pstInJson, "trans_code", sTransCode);
    snprintf(pcKey, tKey, "PUFA_%s%s", sTransCode+4,sSysTrace);
    /*END   */
    
    tLog(DEBUG, "Key[%s]", pcKey);
    return 0;
}
/* ֻ������ʱ�ص� */
/* cups  pstInJson�ǽṹ�壬pstOutJson��8583����*/
int GetSvrName(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson) {
    tStrCpy(pSvrId, "PUFA", 7);
    return 0;
}