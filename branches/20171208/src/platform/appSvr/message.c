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
    while (g_iQuitLoop)
    {
        MEMSET_ST(stMsgData);
        //iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        iRet = tWaitMsg(sSvrId, &stMsgData, TIMEOUT);
        switch (iRet)
        {
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
    while (g_iQuitLoop)
    {
        MEMSET_ST(stMsgData);
        //iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        iRet = tWaitMsg(sSvrId, &stMsgData, TIMEOUT);
        switch (iRet)
        {
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
    char *pcJson = NULL;
    cJSON *pstJson = NULL;
    int iRet = -1;

    if (NULL == pcKey)
    {
        tLog(ERROR, "交易链表项的key不能为NULL.");
        return -1;
    }
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson)
    {
        tLog(ERROR, "创建消息json失败,放弃处理.");
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

int GetKey(char *pcKey, MsgData *pstMsgData) {
    char sKeyJson[MSG_MAX_LEN] = {0}, sSvrId[SVRID_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0};
    cJSON *pstJson = NULL;
    int iKeyLen = 0;

    if (tGetKey(pcKey, sKeyJson, &iKeyLen) < 0)
    {
        tLog(ERROR, "获取Key[%s]的交易链表项失败.", pcKey);
        return -1;
    }
    pstJson = cJSON_Parse(sKeyJson);
    if (NULL == pstJson)
    {
        tLog(ERROR, "转换Key[%s]为json结构失败.", pcKey);
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

/* 返回消息,不释放json*/
int ReplyRepMsg(MsgData *pstMsgData) {
    return tReply(pstMsgData->sSvrId, pstMsgData);
}

/* 执行交易处理序列的单个处理步骤 */
int RequestMsg(MsgData *pstMsgData) {
    char sKey[MSG_KEY_LEN + 1] = {0}, sSvrId[SVRID_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sTransCode[6 + 1] = {0}; //, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0};
    char sTmp[256 + 1] = {0};
    int iRet = -1;

    /* 查找交易码对应的交易流,已组模式启动必须要有交易码 */
    GET_STR_KEY(pstMsgData->pstDataJson, "trans_code", sTransCode);
    if (sTransCode[0] == '\0')
    {
        tLog(ERROR, "消息中无交易码字段,请求消息放弃处理.");
        return -1;
    }
    /* 增加交易前处理 */
    if (g_stLibInfo_Q.pfBeginProc(pstMsgData->pstDataJson, REQ_MSG) < 0)
    {
        tLog(ERROR, "请求消息预处理失败,消息放弃处理.");
        return -1;
    } else
    {
        tLog(INFO, "请求消息预处理成功.");
    }

    /* 交易流处理 */
    if (DoStep(&g_stLibInfo_Q, sTransCode, pstMsgData->pstDataJson, pstMsgData->sKey) < 0)
    {
        /* 失败，原路返回应答 */
        /* 交易后处理 */
        if (g_stLibInfo_Q.pfEndProc(pstMsgData->pstDataJson, REQ_MSG) < 0)
        {
            tLog(ERROR, "请求消息交易流处理失败,结束处理失败.");
        } else
        {
            tLog(INFO, "请求消息交易流处理失败,结束处理成功.");
        }
        ReplyRepMsg(pstMsgData);
        return -1;
    }
    /* 交易后完成 */
    if (g_stLibInfo_Q.pfEndProc(pstMsgData->pstDataJson, REQ_MSG) < 0)
    {
        tLog(ERROR, "请求消息交易流处理成功,结束处理失败.");
    } else
    {
        tLog(INFO, "请求消息交易流处理成功,结束处理成功.");
    }
    //DUMP_JSON(pstJson);
    /* 获取交易的唯一标识 */
    g_stLibInfo_Q.pfGetKey(sKey, sizeof (sKey), pstMsgData->pstDataJson);
    tLog(DEBUG, "Key[%s].", sKey);
    /* 发送消息 */
    /* 成功向下一个服务发，失败返回上一个服务 */
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

/* 执行交易处理序列的单个处理步骤 */
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

    /* 获取应答消息的json数据，通过返回数据确定原交易的Key值 */
    /* 通过回调函数，获取应答交易的唯一标识 */
    iRet = g_stLibInfo_P.pfGetKey(sKey, sizeof (sKey), pstMsgData->pstDataJson);
    if (iRet < 0)
    {
        tLog(ERROR, "获取交易Key值失败,消息放弃处理.");
        return -1;
    } else
    {
        tLog(INFO, "获取应答交易Key[%s]成功.", sKey);
    }
    /* 获取key的原交易链表项 */
    if (GetKey(sKey, &stMsgData) < 0)
    {
        tLog(ERROR, "获取key[%s]的原交易链表项失败,应答消息放弃处理.", sKey);
        return -1;
    } else
    {
        tLog(INFO, "获取key[%s]的原交易链表项key[%s]成功.", sKey, stMsgData.sKey);
    }
    tDelKey(sKey);
    /* 删除交易链表项 */
    tLog(INFO, "删除key[%s]的交易链表项成功.", sKey);

    SET_JSON_KEY(stMsgData.pstDataJson, "response", cJSON_Duplicate(pstMsgData->pstDataJson, 1));
    DUMP_JSON(stMsgData.pstDataJson, pcTran);
    /* 查找交易码对应的交易流 */
    GET_STR_KEY(stMsgData.pstDataJson, "trans_code", sTransCode);
    if (sTransCode[0] == '\0')
    {
        tLog(ERROR, "消息Key[%s]中无交易码信息,应答消息放弃处理.", sKey);
        cJSON_Delete(stMsgData.pstDataJson);
        return -1;
    }
    /* 执行交易流, 使用key的json,无论成功还是失败，都返回 */
    if (g_stLibInfo_P.pfBeginProc(stMsgData.pstDataJson, REP_MSG) < 0)
    {
        tLog(ERROR, "应答消息预处理失败.");
    } else
    {
        tLog(INFO, "应答消息预处理成功.");
    }
    DoStep(&g_stLibInfo_P, sTransCode, stMsgData.pstDataJson, stMsgData.sKey);
    if (g_stLibInfo_P.pfEndProc(stMsgData.pstDataJson, REP_MSG) < 0)
    {
        tLog(ERROR, "应答消息交易流处理完成,结束处理失败.");
    } else
    {
        tLog(ERROR, "应答消息交易流处理完成,结束处理成功.");
    }
    /* 返回应答消息 */
    ReplyRepMsg(&stMsgData);
    cJSON_Delete(stMsgData.pstDataJson);
    return 0;
}