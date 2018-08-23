/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <zlog.h>
#include "t_redis.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_macro.h"

/* ��ܵĺ��� */
int SendRepMsg(cJSON *pstJson);
char *GetSvrId();

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0) {
        return -1;
    }
    return 0;
}

/* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
int AppServerDone(void) {
    HSM_UNLINK();
    return 0;
}

/* �����Ǹ�ͨ�õĽ����������õ��������һ��
 * ʧ�ܵ���Ϣ��ܴ���
 * �����ڽ�������Ҫ����resp_codeӦ����
 */
#if 0
int Transfer(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    /* ���������һ���� */
    ErrHanding(pstTransJson, "00", "���׳ɹ�");
    DUMP_JSON(pstJson);
    /* ����Ӧ����Ϣ */
    return SendRepMsg(pstJson);
}
#endif
/* ת������Ҫ����pcKey��������Ȳ���Ҫת��������pcKey=NULL 
 * ֻ�������л��
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    char sKey[MSG_KEY_LEN + 1] = {0};

    GET_STR_KEY(pstJson, "key", sKey);
    strcpy(pcKey, sKey);
    return 0;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstJson) {
    char sSvrId[SVRID_LEN + 1] = {0};

    GET_STR_KEY(pstJson, "svrid", sSvrId);
    strcpy(pcSvrId, sSvrId);
    return 0;
}

/* ��������Ƿ��سɹ��� */
int RespSucc(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL;

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    ErrHanding(pstDataJson, "00", "���׳ɹ�");
    return 0;
}