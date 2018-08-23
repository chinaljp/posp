/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_extiso.h"
#include "trans_code.h"
#include "t_macro.h"

extern char *GetSvrId();

/* iso.c */
extern IsoTable g_staIsoTable[];
IsoData g_stIsoData;
unsigned char g_caBuf[MSG_MAX_LEN];

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {
    tLog(DEBUG, "AppServerInit");

    tInitIso(&g_stIsoData, g_caBuf, MSG_MAX_LEN, BCD_LLVAR, g_staIsoTable);

    if (LoadTransCode() < 0) {
        return -1;
    }
    return 0;
}

/* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    tFreeConfig();
    return 0;
}

/* posd  pstInJson��8583���ģ�pstOutJson�ǽṹ��*/

/* ����,Ӧ�𶼻ص� */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstInJson, cJSON *pstOutJson) {
#if 0
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0};
    char sTermId[TERM_ID_LEN + 1] = {0}, sTermSn[SN_LEN + 1] = {0};

    GET_STR_KEY(pstOutJson, "trans_code", sTransCode);

    /* pos��ʼ��������rrn */
    if (!memcmp(sTransCode, "029300", 6)) {
        GET_STR_KEY(pstOutJson, "term_sn", sTermSn);
        snprintf(pcKey, tKey, "%s_%s", GetSvrId(), sTermSn);
    } else {
        GET_STR_KEY(pstOutJson, "merch_id", sMerchId);
        GET_STR_KEY(pstOutJson, "term_id", sTermId);
        snprintf(pcKey, tKey, "%s_%s%s", GetSvrId(), sMerchId, sTermId);
    }
#endif
    char sRrn[RRN_LEN + 1] = {0};
    GET_STR_KEY(pstOutJson, "rrn", sRrn);
    snprintf(pcKey, tKey, "%s_%s", GetSvrId(), sRrn);
    return 0;
}
/* posd  pstInJson��8583���ģ�pstOutJson�ǽṹ��*/

/* ֻ������ʱ�ص� */
int GetSvrName(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson) {
    char sTransCode[TRANS_CODE_LEN + 1];
    GET_STR_KEY(pstOutJson, "trans_code", sTransCode);
    snprintf(pSvrId, tSvrId, "%s_Q", sTransCode);
    return 0;
}