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

/* 请求处理 */
int RequestProc() {
    int iRet = -1, iLen = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrId[SVRID_LEN] = {0};
    MsgData stMsgData;

    snprintf(sSvrId, sizeof (sSvrId), "%s_Q", GetSvrId());
    tLog(ERROR, "[%s]进程,启动成功.", sSvrId);
    while (g_iQuitLoop) {
        MEMSET_ST(stMsgData);
        //iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        iRet = tWaitMsg(sSvrId, &stMsgData, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "获取消息失败.");
                break;
            case MSG_TIMEOUT:
                //tLog(DEBUG, "[%s]等待消息超时[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                /* 失败消息已经在函数内部打印了，无需再判断 */
                zlog_put_mdc("key", stMsgData.sKey);
                RequestMsg(&stMsgData);
                zlog_remove_mdc("key");
                cJSON_Delete(stMsgData.pstDataJson);
                break;
        }
    }
    tLog(ERROR, "[%s]进程,退出成功.", sSvrId);
    return 0;
}

/* 应答处理 */
int ResponseProc() {
    int iRet = -1, iLen = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrId[SVRID_LEN] = {0};
    MsgData stMsgData;

    snprintf(sSvrId, sizeof (sSvrId), "%s_P", GetSvrId());
    tLog(ERROR, "[%s]进程,启动成功.", sSvrId);
    while (g_iQuitLoop) {
        MEMSET_ST(stMsgData);
        //iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        iRet = tWaitMsg(sSvrId, &stMsgData, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "获取消息失败.");
                break;
            case MSG_TIMEOUT:
                //tLog(DEBUG, "[%s]等待消息超时[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                /* 失败消息已经在函数内部打印了，无需再判断 */
                zlog_put_mdc("key", stMsgData.sKey);
                ResponseMsg(&stMsgData);
                zlog_remove_mdc("key");
                cJSON_Delete(stMsgData.pstDataJson);
                break;
        }
    }
    tLog(ERROR, "[%s]进程,退出.", sSvrId);
    return 0;
}

int SetKey(char *pcKey, MsgData *pstMsgData) {
    cJSON * pstJson = NULL;
    char *pcJson = NULL;
    int iRet = -1;

    if (NULL == pcKey) {
        tLog(ERROR, "交易链表项的key不能为NULL.");
        return -1;
    }
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    SET_STR_KEY(pstJson, "svrid", pstMsgData->sSvrId);
    SET_STR_KEY(pstJson, "key", pstMsgData->sKey);
    SET_JSON_KEY(pstJson, "data", cJSON_Duplicate(pstMsgData->pstDataJson, 1));
    pcJson = cJSON_PrintUnformatted(pstJson);
    /* 保存交易链表项 */
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
        tLog(ERROR, "获取key[%s]失败,消息放弃处理.", pcKey);
        return NULL;
    }
    return cJSON_Parse(sKeyJson);
}

int RequestMsg(MsgData *pstMsgData) {
    cJSON *pstNetJson = NULL, *pstKeyJson = NULL;
    char sKey[MSG_KEY_LEN + 1] = {0}, sQKey[MSG_KEY_LEN + 1] = {0}, sSvrId[SVRID_LEN + 1] = {0};
    char sPSvrId[SVRID_LEN + 1] = {0}, sQSvrId[SVRID_LEN + 1] = {0};
    int iRet = -1;
    char sDirecType[1 + 1] = {0}, *pcMsg = NULL; // 0:人口方向,1:出口方向
    MsgData stMsgData;

    snprintf(sPSvrId, sizeof (sPSvrId), "%s_P", GetSvrId());
    snprintf(sQSvrId, sizeof (sPSvrId), "%s_Q", GetSvrId());
    /*拆包时使用的交易json*/
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    /* posd  pstDataJson是8583报文，pstNetJson是结构体*/
    /* cupsd  pstDataJson是结构体,pstNetJson8583报文*/
    iRet = g_stLibInfo.pfRequestMsg(pstNetJson, pstMsgData->pstDataJson);
    if (iRet < 0) {
        tLog(ERROR, "key[%s]请求处理失败,放弃处理.", pstMsgData->sKey);
#if 0
        /* 细分处理,如果是mac校验失败等，可以发送给应答队列，并且记录日志 */
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
    /* 获取交易的唯一标识 */
    g_stLibInfo.pfGetKey(sKey, sizeof (sKey), pstMsgData->pstDataJson, pstNetJson);
    tLog(INFO, "key[%s]请求报文处理完成,原key[%s].", sKey, pstMsgData->sKey);
    /*入口方向处理*/
    GET_STR_KEY(pstMsgData->pstDataJson, "direct_type", sDirecType);
    if (sDirecType[0] == '0') {
        pstKeyJson = GetKey(sKey);
        if (NULL == pstKeyJson) {
            tLog(ERROR, "获取key[%s]的交易交易链表项失败.", sKey);
            cJSON_Delete(pstNetJson);
            return -1;
        }
        tLog(DEBUG, "获取key[%s]的交易交易链表项成功.", sKey);
        /* 删除交易链表项 */
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
        tLog(ERROR, "添加key[%s]的交易链表项失败,原key[%s].", sKey, pstMsgData->sKey);
        cJSON_Delete(pstNetJson);
        return -1;
    } else {
        tLog(INFO, "添加key[%s]的交易链表项成功,原key[%s].", sKey), pstMsgData->sKey;
    }
    /* 发送消息 */
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
    char sDirecType[1 + 1] = {0}; // 0:人口方向,1:出口方向
    MsgData stMsgData;

    snprintf(sPSvrId, sizeof (sPSvrId), "%s_P", GetSvrId());
    snprintf(sQSvrId, sizeof (sQSvrId), "%s_Q", GetSvrId());
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    /* posd  pstDataJson是8583报文，pstNetJson是结构体*/
    /* cupsd  pstDataJson是结构体,pstNetJson8583报文*/
    iRet = g_stLibInfo.pfResponseMsg(pstNetJson, pstMsgData->pstDataJson);
    if (iRet < 0) {
        tLog(ERROR, "key[%s]应答处理失败,放弃处理.", pstMsgData->sKey);
        cJSON_Delete(pstNetJson);
        return -1;
    }
    tCommit();
    /* 获取交易的唯一标识 */
    /*  posd pstNetJson 是结构体，pstDataJson是8583报文 */
    /* cupsd  pstDataJson是结构体,pstNetJson8583报文 */
    g_stLibInfo.pfGetKey(sKey, sizeof (sKey), pstNetJson, pstMsgData->pstDataJson);
    tLog(INFO, "key[%s]应答报文处理完成.", sKey);

    /*入口方向处理*/
    GET_STR_KEY(pstNetJson, "direct_type", sDirecType);
    if (sDirecType[0] == '0') {
        if (SetKey(sKey, pstMsgData) < 0) {
            tLog(ERROR, "添加key[%s]的交易交易链表项失败.", sKey);
            cJSON_Delete(pstNetJson);
            return -1;
        }
        tLog(DEBUG, "添加key[%s]的交易链表项成功.", sKey);
        /* 发送消息 */
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
        tLog(ERROR, "获取key[%s]的交易链表项失败.", sKey);
        cJSON_Delete(pstNetJson);
        return -1;
    }
    /* 删除交易链表项 */
    GET_STR_KEY(pstKeyJson, "svrid", sSvrId);
    GET_STR_KEY(pstKeyJson, "key", sQKey);
    tLog(INFO, "获取key[%s]的交易链表项成功,原key[%s].", sKey, sQKey);
    tDelKey(sKey);
    tLog(INFO, "删除key[%s]的交易链表项成功.", sKey);
    MEMSET_ST(stMsgData);
    strcpy(stMsgData.sKey, sQKey);
    strcpy(stMsgData.sSvrId, sPSvrId);
    stMsgData.pstDataJson = pstNetJson;
    tReply(sSvrId, &stMsgData);
    cJSON_Delete(pstKeyJson);
    cJSON_Delete(pstNetJson);
    return 0;
}