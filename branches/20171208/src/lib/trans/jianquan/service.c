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
#include "trans_code.h"

/* ��ܵĺ��� */
extern char *GetSvrId();

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadTransCode() < 0) {
        return -1;
    }
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
    cJSON *pstRepJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRespDesc[50] = {0};
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_STR_KEY(pstRepJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRepJson, "resp_desc", sRespDesc);
    SET_STR_KEY(pstJson, "resp_code", sRespCode);
    SET_STR_KEY(pstJson, "resp_desc", sRespDesc);
    ErrHanding(pstJson, sRespCode);
    return 0;
}

/* ������ִ��ǰ������Ҫ�ж���Ϣ������������Ӧ�� */
int BeginProc(cJSON *pstTransJson, int iMsgType) {
    int iRet = 0;
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sRrn[12 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sTmp[256 + 1] = {0};
    TransCode stTransCode;

    //pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    FindTransCode(&stTransCode, sTransCode);

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    snprintf(sTmp, sizeof (sTmp), "%s_%s_%s", sTransCode, sMerchId, sRrn);
    zlog_put_mdc("key", sTmp);
    switch (iMsgType) {
            /* ������ */
        case REQ_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
                if (AddInlineTransLs(pstTransJson) < 0) {
                    tLog(ERROR, "��ˮ��¼ʧ��,���׷�������.");
                    return -1;
                }
                tCommit();
                tLog(INFO, "����[%s]Ԥ��ˮ��ӳɹ�.", sRrn);
            } else {
                tLog(INFO, "����[%s]����¼������ˮ.", sRrn);
            }
            iRet = 0;
            break;
            /* Ӧ����*/
        case REP_MSG:
#if 0
            pstRespJson = GET_JSON_KEY(pstDataJson, "response");
            if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
                if (UpdPosTransResultLs(pstRespJson) < 0) {
                    tLog(ERROR, "����POS������ˮ[%s]���ʧ��,���׷�������.", sRrn);
                    return -1;
                } else {
                    tLog(INFO, "����[%s]������ˮ�ɹ�.", sRrn);
                }
                tCommit();
            }
#endif
            iRet = 0;
            break;
    }
    return iRet;
}

/* ������֮���� */
int EndProc(cJSON *pstTransJson, int iMsgType) {
    int iRet = 0;
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sRrn[12 + 1] = {0};
    TransCode stTransCode;

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    FindTransCode(&stTransCode, sTransCode);

    tLog(DEBUG, "msg:%d", iMsgType);
    switch (iMsgType) {

            /* ����������������ݵ� */
        case REQ_MSG:
#if 0
            if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
                if (UpdInlineTransLs(pstTransJson) < 0) {
                    tLog(ERROR, "����POS������ˮ���ʧ��,���׷�������.");
                    return -1;
                } else {
                    tLog(INFO, "����[%s]������ˮ�ɹ�.", sRrn);
                }
                tCommit();
            }
#endif
            iRet = 0;
            break;

            /* Ӧ����*/
        case REP_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
                if (UpdInlineTransLs(pstTransJson) < 0) {
                    tLog(ERROR, "����POS������ˮ[%s]���ʧ��,���׷�������.", sRrn);
                    return -1;
                } else {
                    tLog(INFO, "����[%s]������ˮ�ɹ�.", sRrn);
                }
                tCommit();
            }
            iRet = 0;
            break;
    }
    return iRet;
}

/* ת������Ҫ����pcKey��������Ȳ���Ҫת��������pcKey=NULL 
 * ֻ�������л��
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    char sRrn[RRN_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN + 1] = {0}, sTransCode[6 + 1] = {0};
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "rrn", sRrn);
    snprintf(pcKey, tKey, "%s_%s%s", GetSvrId(), sTransDate, sRrn);
    tLog(DEBUG, "Key[%s]", pcKey);

    if (!memcmp(sTransCode, "02A100", 6) ||
            !memcmp(sTransCode, "0AA100", 6)) {
        return 0;
    }
    return 1;
}

int RespSucc(cJSON *pstJson, int *piFlag) {
    ErrHanding(pstJson, "00", "���׳ɹ�");
    return 0;
}

int GetSvrName(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson) {
    char sTransCode[TRANS_CODE_LEN + 1];
    char sSvrId[SVRID_LEN + 1] = {0};
    GET_STR_KEY(pstInJson, "trans_code", sTransCode);
    if (!memcmp(sTransCode, "02A100", 6) ||
            !memcmp(sTransCode, "0AA100", 6)) {
        return -1;
    } else {
        snprintf(pSvrId, tSvrId, "AUTH_Q");
    }
    return 0;
}