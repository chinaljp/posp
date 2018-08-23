/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <zlog.h>
#include "t_redis.h"
#include "gloab.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_redis.h"
#include "dylib.h"
#include "t_macro.h"

extern LibInfo g_stLibInfo_Q, g_stLibInfo_P;

/* ������ */
int RequestProc() {
    int iRet = -1, iLen = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrId[SVRID_LEN] = {0};
    MsgData stMsgData;

    snprintf(sSvrId, sizeof (sSvrId), "%s_Q", GetSvrId());
    tLog(ERROR, "[%s]����,�����ɹ�.", sSvrId);
    while (g_iQuitLoop)
    {
        MEMSET_ST(stMsgData);
        //iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        iRet = tWaitMsg(sSvrId, &stMsgData, TIMEOUT);
        switch (iRet)
        {
            case MSG_ERR:
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                break;
            case MSG_TIMEOUT:
                //tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                /* ʧ����Ϣ�Ѿ��ں����ڲ���ӡ�ˣ��������ж� */
                zlog_put_mdc("key", stMsgData.sKey);
                RequestMsg(&stMsgData);
                zlog_remove_mdc("key");
                cJSON_Delete(stMsgData.pstDataJson);
                break;
        }
    }
    tLog(ERROR, "[%s]����,�˳��ɹ�.", sSvrId);
    return 0;
}

/* Ӧ���� */
int ResponseProc() {
    int iRet = -1, iLen = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrId[SVRID_LEN] = {0};
    MsgData stMsgData;

    snprintf(sSvrId, sizeof (sSvrId), "%s_P", GetSvrId());
    tLog(ERROR, "[%s]����,�����ɹ�.", sSvrId);
    while (g_iQuitLoop)
    {
        MEMSET_ST(stMsgData);
        //iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        iRet = tWaitMsg(sSvrId, &stMsgData, TIMEOUT);
        switch (iRet)
        {
            case MSG_ERR:
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                break;
            case MSG_TIMEOUT:
                //tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                /* ʧ����Ϣ�Ѿ��ں����ڲ���ӡ�ˣ��������ж� */
                zlog_put_mdc("key", stMsgData.sKey);
                ResponseMsg(&stMsgData);
                zlog_remove_mdc("key");
                cJSON_Delete(stMsgData.pstDataJson);
                break;
        }
    }
    tLog(ERROR, "[%s]����,�˳�.", sSvrId);
    return 0;
}

int SetKey(char *pcKey, MsgData *pstMsgData) {
    char *pcJson = NULL;
    cJSON *pstJson = NULL;
    int iRet = -1;

    if (NULL == pcKey)
    {
        tLog(ERROR, "�����������key����ΪNULL.");
        return -1;
    }
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson)
    {
        tLog(ERROR, "������Ϣjsonʧ��,��������.");
        return -1;
    }
    SET_STR_KEY(pstJson, "svrid", pstMsgData->sSvrId);
    SET_STR_KEY(pstJson, "key", pstMsgData->sKey);
    SET_JSON_KEY(pstJson, "data", cJSON_Duplicate(pstMsgData->pstDataJson, 1));
    pcJson = cJSON_PrintUnformatted(pstJson);
    /* ���潻�������� */
    iRet = tSetKey(pcKey, pcJson, strlen(pcJson));
    if (pcJson)
        free(pcJson);
    cJSON_Delete(pstJson);
    return iRet;
}

int GetKey(char *pcKey, MsgData *pstMsgData) {
    char sKeyJson[MSG_MAX_LEN] = {0}, sSvrId[SVRID_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0};
    cJSON *pstJson = NULL;
    int iKeyLen = 0;

    if (tGetKey(pcKey, sKeyJson, &iKeyLen) < 0)
    {
        tLog(ERROR, "��ȡKey[%s]�Ľ���������ʧ��.", pcKey);
        return -1;
    }
    pstJson = cJSON_Parse(sKeyJson);
    if (NULL == pstJson)
    {
        tLog(ERROR, "ת��Key[%s]Ϊjson�ṹʧ��.", pcKey);
        return -1;
    }
    GET_STR_KEY(pstJson, "svrid", sSvrId);
    GET_STR_KEY(pstJson, "key", sKey);

    tStrCpy(pstMsgData->sKey, sKey, strlen(sKey));
    tStrCpy(pstMsgData->sSvrId, sSvrId, strlen(sSvrId));
    pstMsgData->pstDataJson = cJSON_Duplicate(GET_JSON_KEY(pstJson, "data"), 1);
    cJSON_Delete(pstJson);
    return 0;
}

/* ������Ϣ,���ͷ�json*/
int ReplyRepMsg(MsgData *pstMsgData) {
    return tReply(pstMsgData->sSvrId, pstMsgData);
}

/* ִ�н��״������еĵ��������� */
int RequestMsg(MsgData *pstMsgData) {
    char sKey[MSG_KEY_LEN + 1] = {0}, sSvrId[SVRID_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sTransCode[6 + 1] = {0}; //, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0};
    char sTmp[256 + 1] = {0};
    int iRet = -1;

    /* ���ҽ������Ӧ�Ľ�����,����ģʽ��������Ҫ�н����� */
    GET_STR_KEY(pstMsgData->pstDataJson, "trans_code", sTransCode);
    if (sTransCode[0] == '\0')
    {
        tLog(ERROR, "��Ϣ���޽������ֶ�,������Ϣ��������.");
        return -1;
    }
    /* ���ӽ���ǰ���� */
    if (g_stLibInfo_Q.pfBeginProc(pstMsgData->pstDataJson, REQ_MSG) < 0)
    {
        tLog(ERROR, "������ϢԤ����ʧ��,��Ϣ��������.");
        return -1;
    } else
    {
        tLog(INFO, "������ϢԤ����ɹ�.");
    }

    /* ���������� */
    if (DoStep(&g_stLibInfo_Q, sTransCode, pstMsgData->pstDataJson, pstMsgData->sKey) < 0)
    {
        /* ʧ�ܣ�ԭ·����Ӧ�� */
        /* ���׺��� */
        if (g_stLibInfo_Q.pfEndProc(pstMsgData->pstDataJson, REQ_MSG) < 0)
        {
            tLog(ERROR, "������Ϣ����������ʧ��,��������ʧ��.");
        } else
        {
            tLog(INFO, "������Ϣ����������ʧ��,��������ɹ�.");
        }
        ReplyRepMsg(pstMsgData);
        return -1;
    }
    /* ���׺���� */
    if (g_stLibInfo_Q.pfEndProc(pstMsgData->pstDataJson, REQ_MSG) < 0)
    {
        tLog(ERROR, "������Ϣ����������ɹ�,��������ʧ��.");
    } else
    {
        tLog(INFO, "������Ϣ����������ɹ�,��������ɹ�.");
    }
    //DUMP_JSON(pstJson);
    /* ��ȡ���׵�Ψһ��ʶ */
    g_stLibInfo_Q.pfGetKey(sKey, sizeof (sKey), pstMsgData->pstDataJson);
    tLog(DEBUG, "Key[%s].", sKey);
    /* ������Ϣ */
    /* �ɹ�����һ�����񷢣�ʧ�ܷ�����һ������ */
    if (g_stLibInfo_Q.pfGetSvrName(sSvrId, sizeof (sSvrId), pstMsgData->pstDataJson) < 0)
    {
        ReplyRepMsg(pstMsgData);
        return -1;
    }
    SetKey(sKey, pstMsgData);
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());
    tSvcACallEx(sSvrId, sRepSvrId, sKey, pstMsgData->pstDataJson);
    return 0;
}

/* ִ�н��״������еĵ��������� */
int ResponseMsg(MsgData *pstMsgData) {
    char sReqSvrId[SVRID_LEN + 1] = {0}, sReqKey[MSG_KEY_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sKey[MSG_KEY_LEN + 1] = {0};
    char sTransCode[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0};
    char sTmp[256 + 1] = {0};
    char *pcTran = NULL;
    int iRet = -1;
    MsgData stMsgData;

    DUMP_JSON(pstMsgData->pstDataJson, pcTran);
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());

    /* ��ȡӦ����Ϣ��json���ݣ�ͨ����������ȷ��ԭ���׵�Keyֵ */
    /* ͨ���ص���������ȡӦ���׵�Ψһ��ʶ */
    iRet = g_stLibInfo_P.pfGetKey(sKey, sizeof (sKey), pstMsgData->pstDataJson);
    if (iRet < 0)
    {
        tLog(ERROR, "��ȡ����Keyֵʧ��,��Ϣ��������.");
        return -1;
    } else
    {
        tLog(INFO, "��ȡӦ����Key[%s]�ɹ�.", sKey);
    }
    /* ��ȡkey��ԭ���������� */
    if (GetKey(sKey, &stMsgData) < 0)
    {
        tLog(ERROR, "��ȡkey[%s]��ԭ����������ʧ��,Ӧ����Ϣ��������.", sKey);
        return -1;
    } else
    {
        tLog(INFO, "��ȡkey[%s]��ԭ����������key[%s]�ɹ�.", sKey, stMsgData.sKey);
    }
    tDelKey(sKey);
    /* ɾ������������ */
    tLog(INFO, "ɾ��key[%s]�Ľ���������ɹ�.", sKey);

    SET_JSON_KEY(stMsgData.pstDataJson, "response", cJSON_Duplicate(pstMsgData->pstDataJson, 1));
    DUMP_JSON(stMsgData.pstDataJson, pcTran);
    /* ���ҽ������Ӧ�Ľ����� */
    GET_STR_KEY(stMsgData.pstDataJson, "trans_code", sTransCode);
    if (sTransCode[0] == '\0')
    {
        tLog(ERROR, "��ϢKey[%s]���޽�������Ϣ,Ӧ����Ϣ��������.", sKey);
        cJSON_Delete(stMsgData.pstDataJson);
        return -1;
    }
    /* ִ�н�����, ʹ��key��json,���۳ɹ�����ʧ�ܣ������� */
    if (g_stLibInfo_P.pfBeginProc(stMsgData.pstDataJson, REP_MSG) < 0)
    {
        tLog(ERROR, "Ӧ����ϢԤ����ʧ��.");
    } else
    {
        tLog(INFO, "Ӧ����ϢԤ����ɹ�.");
    }
    DoStep(&g_stLibInfo_P, sTransCode, stMsgData.pstDataJson, stMsgData.sKey);
    if (g_stLibInfo_P.pfEndProc(stMsgData.pstDataJson, REP_MSG) < 0)
    {
        tLog(ERROR, "Ӧ����Ϣ�������������,��������ʧ��.");
    } else
    {
        tLog(ERROR, "Ӧ����Ϣ�������������,��������ɹ�.");
    }
    /* ����Ӧ����Ϣ */
    ReplyRepMsg(&stMsgData);
    cJSON_Delete(stMsgData.pstDataJson);
    return 0;
}