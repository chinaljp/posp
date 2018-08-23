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
                RequestMsg(&stMsgData);
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
                ResponseMsg(&stMsgData);
                cJSON_Delete(stMsgData.pstDataJson);
                break;
        }
    }
    tLog(ERROR, "[%s]进程,退出.", sSvrId);
    return 0;
}

int SetKey(char *pcKey, cJSON *pstJson) {
    char *pcJson = NULL;
    int iRet = -1;
    if (NULL == pcKey) {
        tLog(ERROR, "交易链表项的key不能为NULL.");
        return -1;
    }
    pcJson = cJSON_PrintUnformatted(pstJson);
    /* 保存交易链表项 */
    iRet =tSetKey(pcKey, pcJson, strlen(pcJson));
    free(pcJson);
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

/* 返回消息,不释放json*/
int SendRepMsg(cJSON *pstJson) {
    char sReqSvrId[SVRID_LEN + 1] = {0}, sReqKey[MSG_KEY_LEN + 1] = {0}; //, sRepSvrId[64] = {0}; 返回消息不填svrid了
    cJSON *pstDataJson = NULL;
    char sKey[128] = {0};
    MsgData stMsgData;

    GET_STR_KEY(pstJson, "svrid", sReqSvrId);
    GET_STR_KEY(pstJson, "key", sReqKey);

    //snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_Q", GetSvrId());
    snprintf(sKey, sizeof (sKey), "%s_%s", GetSvrId(), sReqKey);
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "获取data数据失败,消息放弃处理.");
        return -1;
    }
    MEMSET_ST(stMsgData);
    strcpy(stMsgData.sKey, sReqKey);
    stMsgData.pstDataJson = pstDataJson;
    return tSvcACall(sReqSvrId, &stMsgData);
    //return tSvcACall(sReqSvrId, NULL, sReqKey, pstDataJson);
}

/* 执行交易处理序列的单个处理步骤 */
int RequestMsg(MsgData *pstMsgData) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char sKey[MSG_KEY_LEN + 1] = {0}, sSvrId[SVRID_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    int iRet = -1;
    MsgData stMsgData;

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建消息json失败,放弃处理.");
        return -1;
    }
    SET_STR_KEY(pstJson, "svrid", pstMsgData->sSvrId);
    SET_STR_KEY(pstJson, "key", pstMsgData->sKey);
    pstDataJson = cJSON_Duplicate(pstMsgData->pstDataJson, 1);
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    /* 执行交易流,成功了流内处理是否需要转发，但是失败了由框架返回应答 */
    if (DoStep(&g_stLibInfo_Q, pstJson) < 0) {
        /* 失败，原路返回应答 */
        SendRepMsg(pstJson);
        cJSON_Delete(pstJson);
        return -1;
    }
    //DUMP_JSON(pstJson);
    /* 获取交易的唯一标识 */
    iRet = g_stLibInfo_Q.pfGetKey(sKey, sizeof (sKey), pstJson);
    if (iRet < 0) {
        tLog(ERROR, "获取交易Key值失败,消息放弃处理.");
        cJSON_Delete(pstJson);
        return -1;
    }
    if (iRet > 0 && sKey[0] != '\0') {
        if (SetKey(sKey, pstJson) < 0) {
            tLog(ERROR, "添加key[%s]的交易交易链表项失败,消息放弃处理.", sKey);
            cJSON_Delete(pstJson);
            return -1;
        }
        tLog(DEBUG, "添加key[%s]的交易交易链表项成功.", sKey);
    }
    /* 发送消息 */
    /* 成功向下一个服务发，失败返回上一个服务 */
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

/* 执行交易处理序列的单个处理步骤 */
int ResponseMsg(MsgData *pstMsgData) {
    cJSON *pstJson = NULL, *pstDataJson = NULL, *pstKeyJson = NULL;
    char sReqSvrId[SVRID_LEN + 1] = {0}, sReqKey[MSG_KEY_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sKey[MSG_KEY_LEN + 1] = {0};
    int iRet = -1;
    MsgData stMsgData;
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建消息json失败,放弃处理.");
        return -1;
    }
    SET_STR_KEY(pstJson, "svrid", pstMsgData->sSvrId);
    SET_STR_KEY(pstJson, "key", pstMsgData->sKey);
    pstDataJson = cJSON_Duplicate(pstMsgData->pstDataJson, 1);
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    /* 获取交易的唯一标识 */
    iRet = g_stLibInfo_P.pfGetKey(sKey, sizeof (sKey), pstJson);
    if (iRet < 0) {
        tLog(ERROR, "获取交易Key值失败,消息放弃处理.");
        cJSON_Delete(pstJson);
        return -1;
    }
    /* 获取key的交易链表项 */
    /* 将返回的数据放入到交易链表项的json */
    pstKeyJson = GetKey(sKey);
    if (NULL == pstKeyJson) {
        tLog(ERROR, "获取key[%s]的交易链表项失败,消息放弃处理.", sKey);
        cJSON_Delete(pstJson);
        return -1;
    }
    tLog(WARN, "获取key[%s]的交易链表项成功.", sKey);
    /* 删除交易链表项 */
    tDelKey(sKey);
    tLog(WARN, "删除key[%s]的交易交易链表项成功.", sKey);
    //DUMP_JSON(pstDataJson);
    SET_JSON_KEY(pstKeyJson, "response", cJSON_Duplicate(pstDataJson, 1));
    //DUMP_JSON(pstKeyJson);
    /* 执行交易流, 使用key的json,无论成功还是失败，都返回 */
    DoStep(&g_stLibInfo_P, pstKeyJson);

    /* 返回应答消息 */
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