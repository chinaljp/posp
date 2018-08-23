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
#include "tKms.h"

/* ��ܵĺ��� */
char *GetSvrId();
TransCode g_stTransCode;

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0) {
        return -1;
    }
    if (LoadTransCodeByGroupId(GetSvrId()) < 0) {
        return -1;
    }
    return 0;
}

/* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    return 0;
}

/* ת������Ҫ����pcKey��������Ȳ���Ҫת��������pcKey=NULL 
 * ֻ�������л��
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstDataJson) {
    char sRrn[RRN_LEN + 1] = {0};

    GET_STR_KEY(pstDataJson, "rrn", sRrn);
    snprintf(pcKey, tKey, "%s_%s", GetSvrId(), sRrn);
    tLog(DEBUG, "Key[%s]", pcKey);
    return 1;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstDataJson) {
    char sTransCode[6 + 1] = {0};
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    tLog(DEBUG,"trans_code = [%s]",sTransCode);
    if(sTransCode[2] == 'Y') {
        snprintf(pcSvrId, tSvrId, "CUPSCANPAY_Q");
        tLog(DEBUG,"������ά�뽻�ף�pcSvrId = [%s]",pcSvrId);
    }
    else {
        //΢�š�֧���� ��ά�뽻��
        snprintf(pcSvrId, tSvrId, "SCANPAY_Q");
    }
    
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
    switch (iMsgType)
    {
            /* ������ */
        case REQ_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (AddInlineTransLs(pstTransJson) < 0)
                {
                    tLog(ERROR, "��ˮ��¼ʧ��,���׷�������.");
                    return -1;
                }
                tCommit();
                tLog(INFO, "����[%s]Ԥ��ˮ��ӳɹ�.", sRrn);
            } else
            {
                tLog(INFO, "����[%s]����¼������ˮ.", sRrn);
            }
            iRet = 0;
            break;
            /* Ӧ����*/
        case REP_MSG:
#if 0
            pstRespJson = GET_JSON_KEY(pstDataJson, "response");
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (UpdPosTransResultLs(pstRespJson) < 0)
                {
                    tLog(ERROR, "����POS������ˮ[%s]���ʧ��,���׷�������.", sRrn);
                    return -1;
                } else
                {
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
    switch (iMsgType)
    {
            /* ����������������ݵ� */
        case REQ_MSG:
#if 0
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (UpdInlineTransLs(pstTransJson) < 0)
                {
                    tLog(ERROR, "����POS������ˮ���ʧ��,���׷�������.");
                    return -1;
                } else
                {
                    tLog(INFO, "����[%s]������ˮ�ɹ�.", sRrn);
                }
                tCommit();
            }
#endif
            break;
            /* Ӧ����*/
        case REP_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (UpdInlineTransLs(pstTransJson) < 0)
                {
                    tLog(ERROR, "����POS������ˮ[%s]���ʧ��,���׷�������.", sRrn);
                    return -1;
                } else
                {
                    tLog(INFO, "����[%s]������ˮ�ɹ�.", sRrn);
                }
                tCommit();
            }
            iRet = 0;
            break;
    }
    return iRet;
}
