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
                RequestMsg(&stMsgData);
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
                ResponseMsg(&stMsgData);
                cJSON_Delete(stMsgData.pstDataJson);
                break;
        }
    }
    tLog(ERROR, "[%s]����,�˳�.", sSvrId);
    return 0;
}

int SetKey(char *pcKey, cJSON *pstJson) {
    char *pcJson = NULL;
    int iRet = -1;
    if (NULL == pcKey) {
        tLog(ERROR, "�����������key����ΪNULL.");
        return -1;
    }
    pcJson = cJSON_PrintUnformatted(pstJson);
    /* ���潻�������� */
    iRet =tSetKey(pcKey, pcJson, strlen(pcJson));
    free(pcJson);
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

/* ������Ϣ,���ͷ�json*/
int SendRepMsg(cJSON *pstJson) {
    char sReqSvrId[SVRID_LEN + 1] = {0}, sReqKey[MSG_KEY_LEN + 1] = {0}; //, sRepSvrId[64] = {0}; ������Ϣ����svrid��
    cJSON *pstDataJson = NULL;
    char sKey[128] = {0};
    MsgData stMsgData;

    GET_STR_KEY(pstJson, "svrid", sReqSvrId);
    GET_STR_KEY(pstJson, "key", sReqKey);

    //snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_Q", GetSvrId());
    snprintf(sKey, sizeof (sKey), "%s_%s", GetSvrId(), sReqKey);
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "��ȡdata����ʧ��,��Ϣ��������.");
        return -1;
    }
    MEMSET_ST(stMsgData);
    strcpy(stMsgData.sKey, sReqKey);
    stMsgData.pstDataJson = pstDataJson;
    return tSvcACall(sReqSvrId, &stMsgData);
    //return tSvcACall(sReqSvrId, NULL, sReqKey, pstDataJson);
}

/* ִ�н��״������еĵ��������� */
int RequestMsg(MsgData *pstMsgData) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char sKey[MSG_KEY_LEN + 1] = {0}, sSvrId[SVRID_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    int iRet = -1;
    MsgData stMsgData;

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "������Ϣjsonʧ��,��������.");
        return -1;
    }
    SET_STR_KEY(pstJson, "svrid", pstMsgData->sSvrId);
    SET_STR_KEY(pstJson, "key", pstMsgData->sKey);
    pstDataJson = cJSON_Duplicate(pstMsgData->pstDataJson, 1);
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    /* ִ�н�����,�ɹ������ڴ����Ƿ���Ҫת��������ʧ�����ɿ�ܷ���Ӧ�� */
    if (DoStep(&g_stLibInfo_Q, pstJson) < 0) {
        /* ʧ�ܣ�ԭ·����Ӧ�� */
        SendRepMsg(pstJson);
        cJSON_Delete(pstJson);
        return -1;
    }
    //DUMP_JSON(pstJson);
    /* ��ȡ���׵�Ψһ��ʶ */
    iRet = g_stLibInfo_Q.pfGetKey(sKey, sizeof (sKey), pstJson);
    if (iRet < 0) {
        tLog(ERROR, "��ȡ����Keyֵʧ��,��Ϣ��������.");
        cJSON_Delete(pstJson);
        return -1;
    }
    if (iRet > 0 && sKey[0] != '\0') {
        if (SetKey(sKey, pstJson) < 0) {
            tLog(ERROR, "���key[%s]�Ľ��׽���������ʧ��,��Ϣ��������.", sKey);
            cJSON_Delete(pstJson);
            return -1;
        }
        tLog(DEBUG, "���key[%s]�Ľ��׽���������ɹ�.", sKey);
    }
    /* ������Ϣ */
    /* �ɹ�����һ�����񷢣�ʧ�ܷ�����һ������ */
    g_stLibInfo_Q.pfGetSvrName(sSvrId, sizeof (sSvrId), pstJson);
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());
    MEMSET_ST(stMsgData);
    strcpy(stMsgData.sSvrId, sRepSvrId);
    strcpy(stMsgData.sKey, sKey);
    //stMsgData.pstDataJson = cJSON_Duplicate(pstDataJson, 1);
    stMsgData.pstDataJson = pstDataJson;
    //tSvcACall(sSvrId, sRepSvrId, sKey, pstDataJson);
    tSvcACall(sSvrId, &stMsgData);
    cJSON_Delete(pstJson);
    return 0;
}

/* ִ�н��״������еĵ��������� */
int ResponseMsg(MsgData *pstMsgData) {
    cJSON *pstJson = NULL, *pstDataJson = NULL, *pstKeyJson = NULL;
    char sReqSvrId[SVRID_LEN + 1] = {0}, sReqKey[MSG_KEY_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sKey[MSG_KEY_LEN + 1] = {0};
    int iRet = -1;
    MsgData stMsgData;
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "������Ϣjsonʧ��,��������.");
        return -1;
    }
    SET_STR_KEY(pstJson, "svrid", pstMsgData->sSvrId);
    SET_STR_KEY(pstJson, "key", pstMsgData->sKey);
    pstDataJson = cJSON_Duplicate(pstMsgData->pstDataJson, 1);
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    /* ��ȡ���׵�Ψһ��ʶ */
    iRet = g_stLibInfo_P.pfGetKey(sKey, sizeof (sKey), pstJson);
    if (iRet < 0) {
        tLog(ERROR, "��ȡ����Keyֵʧ��,��Ϣ��������.");
        cJSON_Delete(pstJson);
        return -1;
    }
    /* ��ȡkey�Ľ��������� */
    /* �����ص����ݷ��뵽�����������json */
    pstKeyJson = GetKey(sKey);
    if (NULL == pstKeyJson) {
        tLog(ERROR, "��ȡkey[%s]�Ľ���������ʧ��,��Ϣ��������.", sKey);
        cJSON_Delete(pstJson);
        return -1;
    }
    tLog(WARN, "��ȡkey[%s]�Ľ���������ɹ�.", sKey);
    /* ɾ������������ */
    tDelKey(sKey);
    tLog(WARN, "ɾ��key[%s]�Ľ��׽���������ɹ�.", sKey);
    //DUMP_JSON(pstDataJson);
    SET_JSON_KEY(pstKeyJson, "response", cJSON_Duplicate(pstDataJson, 1));
    //DUMP_JSON(pstKeyJson);
    /* ִ�н�����, ʹ��key��json,���۳ɹ�����ʧ�ܣ������� */
    DoStep(&g_stLibInfo_P, pstKeyJson);

    /* ����Ӧ����Ϣ */
    GET_STR_KEY(pstKeyJson, "svrid", sReqSvrId);
    GET_STR_KEY(pstKeyJson, "key", sReqKey);
    //tSvcACall(sReqSvrId, NULL, sReqKey, GET_JSON_KEY(pstKeyJson, "data"));
    MEMSET_ST(stMsgData);
    strcpy(stMsgData.sSvrId, sRepSvrId);
    strcpy(stMsgData.sKey, sReqKey);
    stMsgData.pstDataJson = GET_JSON_KEY(pstKeyJson, "data");
    tReply(sReqSvrId, &stMsgData);

    cJSON_Delete(pstJson);
    cJSON_Delete(pstKeyJson);
    return 0;
}