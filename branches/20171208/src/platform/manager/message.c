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
#include "gloab.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "dylib.h"
#include "t_cjson.h"
#include "err.h"
#include "t_macro.h"

/* ������ */
int RequestProc() {
    int iRet = -1, iLen = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrId[SVRID_LEN] = {0};
    MsgData stMsgData;

    snprintf(sSvrId, sizeof (sSvrId), "%s_Q", GetSvrId());
    tLog(ERROR, "[%s]����,�����ɹ�.", sSvrId);
    while (g_iQuitLoop) {
        MEMSET_ST(stMsgData);
        //iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        iRet = tWaitMsg(sSvrId, &stMsgData, TIMEOUT);
        switch (iRet) {
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
    while (g_iQuitLoop) {
        MEMSET_ST(stMsgData);
        //iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        iRet = tWaitMsg(sSvrId, &stMsgData, TIMEOUT);
        switch (iRet) {
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
    cJSON * pstJson = NULL;
    char *pcJson = NULL;
    int iRet = -1;

    if (NULL == pcKey) {
        tLog(ERROR, "�����������key����ΪNULL.");
        return -1;
    }
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "��������Jsonʧ��.");
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

cJSON *GetKey(char *pcKey) {
    char sKeyJson[MSG_MAX_LEN] = {0};
    int iKeyLen = 0;

    if (tGetKey(pcKey, sKeyJson, &iKeyLen) < 0) {
        tLog(ERROR, "��ȡkey[%s]ʧ��,��Ϣ��������.", pcKey);
        return NULL;
    }
    return cJSON_Parse(sKeyJson);
}

int RequestMsg(MsgData *pstMsgData) {
    cJSON *pstNetJson = NULL, *pstKeyJson = NULL;
    char sKey[MSG_KEY_LEN + 1] = {0}, sQKey[MSG_KEY_LEN + 1] = {0}, sSvrId[SVRID_LEN + 1] = {0};
    char sPSvrId[SVRID_LEN + 1] = {0}, sQSvrId[SVRID_LEN + 1] = {0};
    int iRet = -1;
    char sDirecType[1 + 1] = {0}, *pcMsg = NULL; // 0:�˿ڷ���,1:���ڷ���
    MsgData stMsgData;

    snprintf(sPSvrId, sizeof (sPSvrId), "%s_P", GetSvrId());
    snprintf(sQSvrId, sizeof (sPSvrId), "%s_Q", GetSvrId());
    /*���ʱʹ�õĽ���json*/
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    /* posd  pstDataJson��8583���ģ�pstNetJson�ǽṹ��*/
    /* cupsd  pstDataJson�ǽṹ��,pstNetJson8583����*/
    iRet = g_stLibInfo.pfRequestMsg(pstNetJson, pstMsgData->pstDataJson);
    if (iRet < 0) {
        tLog(ERROR, "key[%s]������ʧ��,��������.", pstMsgData->sKey);
#if 0
        /* ϸ�ִ���,�����macУ��ʧ�ܵȣ����Է��͸�Ӧ����У����Ҽ�¼��־ */
        GET_STR_KEY(pstMsgData->pstDataJson, "direct_type", sDirecType);
        if (MAC_GEN_ERR == iRet && sDirecType[0] != '0') {
            SET_STR_KEY(pstMsgData->pstDataJson, "istresp_code", "A7");
            strcpy(sSvrId, pstMsgData->sSvrId);
            strcpy(pstMsgData->sSvrId, sQSvrId);
            tReply(sSvrId, pstMsgData);
        }
#endif
        cJSON_Delete(pstNetJson);
        return -1;
    }
    tCommit();
    //DUMP_JSON(pstNetJson);
    /* ��ȡ���׵�Ψһ��ʶ */
    g_stLibInfo.pfGetKey(sKey, sizeof (sKey), pstMsgData->pstDataJson, pstNetJson);
    tLog(INFO, "key[%s]�����Ĵ������,ԭkey[%s].", sKey, pstMsgData->sKey);
    /*��ڷ�����*/
    GET_STR_KEY(pstMsgData->pstDataJson, "direct_type", sDirecType);
    if (sDirecType[0] == '0') {
        pstKeyJson = GetKey(sKey);
        if (NULL == pstKeyJson) {
            tLog(ERROR, "��ȡkey[%s]�Ľ��׽���������ʧ��.", sKey);
            cJSON_Delete(pstNetJson);
            return -1;
        }
        tLog(DEBUG, "��ȡkey[%s]�Ľ��׽���������ɹ�.", sKey);
        /* ɾ������������ */
        tDelKey(sKey);
        g_stLibInfo.pfGetSvrName(sSvrId, sizeof (sSvrId), pstMsgData->pstDataJson, pstNetJson);
        DUMP_JSON(pstNetJson, pcMsg);
        DUMP_JSON(pstKeyJson, pcMsg);
        MEMSET_ST(stMsgData);
        //GET_STR_KEY(pstKeyJson, "svrid", sSvrId);
        GET_STR_KEY(pstKeyJson, "key", sQKey);
        strcpy(stMsgData.sKey, sQKey);
        stMsgData.pstDataJson = pstNetJson;
        tReply(sSvrId, &stMsgData);
        cJSON_Delete(pstKeyJson);
        cJSON_Delete(pstNetJson);
        return 0;
    }

    if (SetKey(sKey, pstMsgData) < 0) {
        tLog(ERROR, "���key[%s]�Ľ���������ʧ��,ԭkey[%s].", sKey, pstMsgData->sKey);
        cJSON_Delete(pstNetJson);
        return -1;
    } else {
        tLog(INFO, "���key[%s]�Ľ���������ɹ�,ԭkey[%s].", sKey), pstMsgData->sKey;
    }
    /* ������Ϣ */
    g_stLibInfo.pfGetSvrName(sSvrId, sizeof (sSvrId), pstMsgData->pstDataJson, pstNetJson);

    strcpy(stMsgData.sSvrId, sPSvrId);
    strcpy(stMsgData.sKey, sKey);
    stMsgData.pstDataJson = pstNetJson;
    tSvcACall(sSvrId, &stMsgData);
    cJSON_Delete(pstNetJson);
    return 0;
}

int ResponseMsg(MsgData * pstMsgData) {
    cJSON *pstDataJson = NULL, *pstKeyJson = NULL, *pstNetJson = NULL;
    char sKey[MSG_KEY_LEN + 1] = {0}, sSvrId[SVRID_LEN + 1] = {0}, sQKey[MSG_KEY_LEN + 1] = {0};
    char sPSvrId[SVRID_LEN + 1] = {0}, sQSvrId[SVRID_LEN + 1] = {0};
    int iRet = -1;
    char sDirecType[1 + 1] = {0}; // 0:�˿ڷ���,1:���ڷ���
    MsgData stMsgData;

    snprintf(sPSvrId, sizeof (sPSvrId), "%s_P", GetSvrId());
    snprintf(sQSvrId, sizeof (sQSvrId), "%s_Q", GetSvrId());
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    /* posd  pstDataJson��8583���ģ�pstNetJson�ǽṹ��*/
    /* cupsd  pstDataJson�ǽṹ��,pstNetJson8583����*/
    iRet = g_stLibInfo.pfResponseMsg(pstNetJson, pstMsgData->pstDataJson);
    if (iRet < 0) {
        tLog(ERROR, "key[%s]Ӧ����ʧ��,��������.", pstMsgData->sKey);
        cJSON_Delete(pstNetJson);
        return -1;
    }
    tCommit();
    /* ��ȡ���׵�Ψһ��ʶ */
    /*  posd pstNetJson �ǽṹ�壬pstDataJson��8583���� */
    /* cupsd  pstDataJson�ǽṹ��,pstNetJson8583���� */
    g_stLibInfo.pfGetKey(sKey, sizeof (sKey), pstNetJson, pstMsgData->pstDataJson);
    tLog(INFO, "key[%s]Ӧ���Ĵ������.", sKey);

    /*��ڷ�����*/
    GET_STR_KEY(pstNetJson, "direct_type", sDirecType);
    if (sDirecType[0] == '0') {
        if (SetKey(sKey, pstMsgData) < 0) {
            tLog(ERROR, "���key[%s]�Ľ��׽���������ʧ��.", sKey);
            cJSON_Delete(pstNetJson);
            return -1;
        }
        tLog(DEBUG, "���key[%s]�Ľ���������ɹ�.", sKey);
        /* ������Ϣ */
        GET_STR_KEY(pstNetJson, "trans_code", sSvrId);
        strcat(sSvrId, "_Q");
        snprintf(sQSvrId, sizeof (sQSvrId), "%s_Q", GetSvrId());
        MEMSET_ST(stMsgData);
        strcpy(stMsgData.sSvrId, sQSvrId);
        strcpy(stMsgData.sKey, sKey);
        stMsgData.pstDataJson = pstNetJson;
        tSvcACall(sSvrId, &stMsgData);
        cJSON_Delete(pstNetJson);
        return 0;
    }

    pstKeyJson = GetKey(sKey);
    if (NULL == pstKeyJson) {
        tLog(ERROR, "��ȡkey[%s]�Ľ���������ʧ��.", sKey);
        cJSON_Delete(pstNetJson);
        return -1;
    }
    /* ɾ������������ */
    GET_STR_KEY(pstKeyJson, "svrid", sSvrId);
    GET_STR_KEY(pstKeyJson, "key", sQKey);
    tLog(INFO, "��ȡkey[%s]�Ľ���������ɹ�,ԭkey[%s].", sKey, sQKey);
    tDelKey(sKey);
    tLog(INFO, "ɾ��key[%s]�Ľ���������ɹ�.", sKey);
    MEMSET_ST(stMsgData);
    strcpy(stMsgData.sKey, sQKey);
    strcpy(stMsgData.sSvrId, sPSvrId);
    stMsgData.pstDataJson = pstNetJson;
    tReply(sSvrId, &stMsgData);
    cJSON_Delete(pstKeyJson);
    cJSON_Delete(pstNetJson);
    return 0;
}