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
char *GetSvrId();

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0)
    {
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

/* ���ý���key */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstTransJson) {
    return 0;
}

/* ֻ���������е��ã�����1ָ����һ��ģ�飬��Ҫ������һ��ģ���svrid������-1������Ҫ����svrid����ܻ��Զ�������Ķ��з�����Ϣ */
int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstTransJson) {
    return -1;
}

/* ��������Ƿ��سɹ��� */
int RespSucc(cJSON *pstTransJson, int *piFlag) {
    ErrHanding(pstTransJson, "00", "���׳ɹ�");
    return 0;
}

int BeginProc(cJSON *pstTransJson, int iMsgType) {
    char sTmp[256 + 1] = {0};
    char sTransCode[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0}, sTermSn[100 + 1] = {0};

    /* Ϊ����־��ѯ����ӡһЩҵ������ */
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    if (!memcmp(sTransCode, "029300", 6))
    {
        GET_STR_KEY(pstTransJson, "term_sn", sTermSn);
        GET_STR_KEY(pstTransJson, "rrn", sRrn);
        snprintf(sTmp, sizeof (sTmp), "%s_%s_%s", sTransCode, sTermSn, sRrn);
    } else
    {
        GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
        GET_STR_KEY(pstTransJson, "rrn", sRrn);
        snprintf(sTmp, sizeof (sTmp), "%s_%s_%s", sTransCode, sMerchId, sRrn);
    }
    zlog_put_mdc("key", sTmp);
    return 0;
}

int EndProc(cJSON *pstTransJson, int iMsgType) {
    return 0;
}
