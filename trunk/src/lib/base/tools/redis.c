/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <sys/types.h>
#include <unistd.h>
#include "t_log.h"
#include "t_tools.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "t_macro.h"

static redisContext *g_pstRedisCxt;

int tInitSvr(const char *pcIp, const int iPort) {
    //redis默认监听端口为6387 可以再配置文件中修改  
    g_pstRedisCxt = redisConnect(pcIp, iPort);
    if (g_pstRedisCxt->err) {
        redisFree(g_pstRedisCxt);
        tLog(ERROR, "Connect to redisServer faile");
        return MSG_ERR;
    }
    tLog(DEBUG, "Connect to redisServer[%s:%d] Success[%p]", pcIp, iPort, g_pstRedisCxt);
    return MSG_SUCC;
}

int tCall(char *pcQSvrId, MsgData *pstMsgData) {
    cJSON *pstJson = NULL, *pcstSubJson = NULL;
    char *pcMsg = NULL, sRepSvrId[MSG_KEY_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0};

    /* 判断接收的svrid和应答的svrid是否为空 */
    if (NULL == pcQSvrId) {
        tLog(ERROR, "无目标队列,消息key[%s]放弃发送.", pstMsgData->sKey);
        return -1;
    }
    if ('\0' == pstMsgData->sKey[0]) {
        tLog(ERROR, "发送消息[%s]From[%s]的key为空,放弃发送.", pcQSvrId, pstMsgData->sSvrId);
        return -1;
    }
    if (NULL == pstMsgData->pstDataJson) {
        tLog(ERROR, "发送消息[%s]From[%s]的key[%s],数据为空,,放弃发送."
                , pcQSvrId, pstMsgData->sSvrId, pstMsgData->sKey);
        return -1;
    }
    /* 发送的交易对象 */
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    SET_STR_KEY(pstJson, "svrid", pstMsgData->sSvrId);

    if ('\0' == pstMsgData->sKey[0])
        tGetUniqueKey(sKey);
    else {
        strcpy(sKey, pstMsgData->sKey);
    }
    SET_STR_KEY(pstJson, "key", sKey);

    pcstSubJson = cJSON_Duplicate(pstMsgData->pstDataJson, 1);
    if (NULL == pcstSubJson) {
        tLog(ERROR, "数据json拷贝失败.");
        cJSON_Delete(pstJson);
        return -1;
    }

    SET_JSON_KEY(pstJson, "data", pcstSubJson);
    DUMP_JSON(pstJson, pcMsg);
    pcMsg = cJSON_PrintUnformatted(pstJson);
    if (tSendMsg(pcQSvrId, pcMsg, strlen(pcMsg)) < 0) {
        tLog(ERROR, "发送消息[%s]From[%s]失败,data[%s].", pcQSvrId, pstMsgData->sSvrId, pcMsg);
        cJSON_Delete(pstJson);
        if (pcMsg) {
            free(pcMsg);
            pcMsg = NULL;
        }
        return -1;
    }
    if (pcMsg) {
        free(pcMsg);
        pcMsg = NULL;
    }
    tLog(ERROR, "[%s]Send[%s] To[%s] Key[%s] Succ!"
            , pstMsgData->cMsgType == '1' ? "REQ" : "REP"
            , pstMsgData->sSvrId, pcQSvrId
            , pstMsgData->sKey);
    cJSON_Delete(pstJson);
    return 0;
}

int tSvcACall(char *pcQSvrId, MsgData *pstMsgData) {
    pstMsgData->cMsgType = REQ_MSG;
    return tCall(pcQSvrId, pstMsgData);
}

int tReply(char *pcQSvrId, MsgData *pstMsgData) {
    pstMsgData->cMsgType = REP_MSG;
    return tCall(pcQSvrId, pstMsgData);
}

/* 获取唯一值，gettimeofday */
void tGetUniqueKey(char *uniKey) {
    struct timeval time;
    gettimeofday(&time, NULL);
    sprintf(uniKey, "%ld", (long) (time.tv_sec) * 1000000 + time.tv_usec);
}

int tCheckStatus( ) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    snprintf(sCommand, sizeof (sCommand), "ping");

    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply || REDIS_REPLY_STATUS != pcRedisReply->type || strcasecmp(pcRedisReply->str,"PONG") != 0) {
        tLog(ERROR, "Execut command[%s] failure[%s].", sCommand, pcRedisReply->str);
        freeReplyObject(pcRedisReply);
        return MSG_ERR;
    }
    tLog(DEBUG, "Cmd[PING]");
    freeReplyObject(pcRedisReply);
    return MSG_SUCC;
}


int tWaitMsg(char *pcSvrId, MsgData *pstMsgData, int iTimeout) {
    cJSON *pstJson = NULL, *pcDataJson = NULL, *pcRecvJson = NULL, *pcRecvDataJson = NULL;
    char sKey[MSG_KEY_LEN + 1] = {0}, sPSvrId[SVRID_LEN + 1] = {0};
    char sRecvData[MSG_MAX_LEN + 1] = {0}, *pcMsg = NULL;
    int iLen = 0, iRet = -1;

    iRet = tRecvMsg(pcSvrId, sRecvData, &iLen, iTimeout);
    if (iRet < 0) {
        return iRet;
    }
    pcRecvJson = cJSON_Parse(sRecvData);
    GET_STR_KEY(pcRecvJson, "key", sKey);
    GET_STR_KEY(pcRecvJson, "svrid", sPSvrId);

    strcpy(pstMsgData->sKey, sKey);
    strcpy(pstMsgData->sSvrId, sPSvrId);

    tLog(ERROR, "[%s]Recv[%s] From[%s] Key[%s] Succ!"
            , pstMsgData->cMsgType == '1' ? "REQ" : "REP"
            , pcSvrId, pstMsgData->sSvrId
            , pstMsgData->sKey);
    DUMP_JSON(pcRecvJson, pcMsg);
    pcDataJson = GET_JSON_KEY(pcRecvJson, "data");
    if (pcDataJson == NULL) {
        tLog(ERROR, "接收的消息key[%s]无dataJson.", sKey);
        cJSON_Delete(pcRecvJson);
        return -1;
    }
    pcRecvDataJson = cJSON_Duplicate(pcDataJson, 1);
    pstMsgData->pstDataJson = pcRecvDataJson;
    cJSON_Delete(pcRecvJson);
    return 0;
}

int tSvcCall(char *pcQSvrId, MsgData *pstQMsgData, char *pcPSvrId, MsgData *pstPMsgData, int iTimeout) {
    if (tSvcACall(pcQSvrId, pstQMsgData) < 0) {
        tLog(ERROR, "发送消息失败.");
        return -1;
    }
    return tWaitMsg(pcPSvrId, pstPMsgData, iTimeout);
#if 0
    cJSON *pstJson = NULL, *pcDataJson = NULL, *pcRecvJson = NULL, *pcRecvDataJson = NULL;
    char sKey[MSG_KEY_LEN + 1] = {0}, sKey2[32 + 1] = {0};
    char *pcJsonString = NULL;
    char sRecvData[MSG_MAX_LEN + 1] = {0};
    int iLen = 0;

    /* 发送的交易对象 */
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    //snprintf(sPid, sizeof (sPid), "%d", getpid());
    if (pcKey == NULL || pcKey[0] == '\0')
        tGetUniqueKey(sKey);
    else {
        strcpy(sKey, pcKey);
    }
    /* 进程号接收消息 */
    //SET_STR_KEY(pstJson, "svrid", sPid);
    SET_STR_KEY(pstJson, "svrid", pcRepSvrId);
    SET_STR_KEY(pstJson, "key", sKey);
    pcDataJson = cJSON_Duplicate(pcSendJson, 1);
    if (NULL == pcDataJson) {
        tLog(ERROR, "数据json拷贝失败.");
        return -1;
    }
    SET_JSON_KEY(pstJson, "data", pcDataJson);

    pcJsonString = cJSON_PrintUnformatted(pstJson);
    /* 创建json */
    DUMP_JSON(pcRecvJson);
    if (tSendMsg(pcReqSvrId, pcJsonString, strlen(pcJsonString)) < 0) {
        cJSON_Delete(pstJson);
        return -1;
    }
    tLog(ERROR, "Send To[%s] Key[%s] Recv[%s] Succ!", pcReqSvrId, pcKey, pcRepSvrId);
    //if (tRecvMsg(sPid, sRecvData, &iLen, iTimeout) < 0) {
    if (tRecvMsg(pcRepSvrId, sRecvData, &iLen, iTimeout) < 0) {
        cJSON_Delete(pstJson);
        return -1;
    }
    pcRecvJson = cJSON_Parse(sRecvData);
    DUMP_JSON(pcRecvJson);
    GET_STR_KEY(pcRecvJson, "key", sKey2);
    tLog(ERROR, "Recv Msg From[%s] Key[%s] Succ!", pcRepSvrId, sKey2);
    pcRecvDataJson = cJSON_Duplicate(GET_JSON_KEY(pcRecvJson, "data"), 1);
    //*ppcRecvJson = cJSON_Duplicate(pcRecvJson, 1);
    *ppcRecvJson = pcRecvDataJson;
    cJSON_Delete(pstJson);
    cJSON_Delete(pcRecvJson);


    return 0;
#endif
}

int tSendMsg(char *pcQSvrId, char *pcData, int iDataLen) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    char sData[MSG_MAX_LEN * 2] = {0};

    if (MSG_MAX_LEN < iDataLen) {
        tLog(ERROR, "send data len[%d] > Max Len[%d] failure.", iDataLen, MSG_MAX_LEN);
        return MSG_ERR;
    }
    tBcd2Asc(sData, pcData, iDataLen * 2, LEFT_ALIGN);
    snprintf(sCommand, sizeof (sCommand), "LPUSH %s %s", pcQSvrId, sData);

    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply || REDIS_REPLY_INTEGER != pcRedisReply->type) {
        tLog(ERROR, "Execut command[%s] failure[%s].", sCommand, pcRedisReply->str);
        freeReplyObject(pcRedisReply);
        return MSG_ERR;
    }
    tLog(DEBUG, "Cmd[LPUSH] Key[%s] Len[%d] Msg[%s] RTN[%d].", pcQSvrId, iDataLen, pcData, pcRedisReply->integer);
    tLog(ERROR, "Send Msg To[%s] Succ[%d]!", pcQSvrId, pcRedisReply->integer);
    freeReplyObject(pcRedisReply);

    return MSG_SUCC;
}

int tRecvMsg(char *pcSvrId, char *pcData, int *piDataLen, int iTimeout) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    char sData[MSG_MAX_LEN * 2] = {0};
    int i, iLen = 0;

    snprintf(sCommand, sizeof (sCommand), "BLPOP %s %d", pcSvrId, iTimeout);

    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply || REDIS_REPLY_ERROR == pcRedisReply->type) {
        tLog(ERROR, "Execut command[%s] failure[%s].", sCommand, pcRedisReply->str);
        freeReplyObject(pcRedisReply);
        return MSG_ERR;
    }
    if (REDIS_REPLY_ARRAY == pcRedisReply->type) {
        for (i = 0; i < pcRedisReply->elements; ++i) {
            redisReply* pchildReply = pcRedisReply->element[i];
            if (pchildReply->type == REDIS_REPLY_STRING) {
                tStrCpy(sData, pchildReply->str, pchildReply->len);
                iLen = pchildReply->len;
            }
        }
    }
    if (sData[0] == '\0') {
        tLog(WARN, "Cmd[BLPOP] Key[%s] Timeout[%d].", pcSvrId, iTimeout);
        freeReplyObject(pcRedisReply);
        return MSG_TIMEOUT;
    }
    tAsc2Bcd(pcData, sData, iLen, LEFT_ALIGN);
    *piDataLen = (iLen + 1) / 2;
    tLog(DEBUG, "Cmd[BLPOP] Key[%s] Len[%d] Msg[%s].", pcSvrId, *piDataLen, pcData);
    tLog(ERROR, "Recv Msg From[%s] Succ!", pcSvrId);
    freeReplyObject(pcRedisReply);

    return MSG_SUCC;
}

/*add by GuoJiaQing at 20170718 */
int tRecvMsgEX(char *pcSvrId, char *pcData, int *piDataLen, int iTimeout) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    char sData[MSG_MAX_LEN * 2] = {0};
    int i, iLen = 0;

    snprintf(sCommand, sizeof (sCommand), "BLPOP %s %d", pcSvrId, iTimeout);

    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply || REDIS_REPLY_ERROR == pcRedisReply->type) {
        tLog(ERROR, "Execut command[%s] failure[%s].", sCommand, pcRedisReply->str);
        freeReplyObject(pcRedisReply);
        return MSG_ERR;
    }
    if (REDIS_REPLY_ARRAY == pcRedisReply->type) {
        for (i = 0; i < pcRedisReply->elements; ++i) {
            redisReply* pchildReply = pcRedisReply->element[i];
            if (pchildReply->type == REDIS_REPLY_STRING) {
                tStrCpy(sData, pchildReply->str, pchildReply->len);
                iLen = pchildReply->len;
            }
        }
    }
    if (sData[0] == '\0') {
        tLog(WARN, "Cmd[BLPOP] Key[%s] Timeout[%d].", pcSvrId, iTimeout);
        freeReplyObject(pcRedisReply);
        return MSG_TIMEOUT;
    }
    
    tStrCpy(pcData, sData, iLen);
    *piDataLen = iLen;
    tLog(DEBUG, "Cmd[BLPOP] Key[%s] Len[%d] Msg[%s].", pcSvrId, *piDataLen, pcData);
    tLog(ERROR, "Recv Msg From[%s] Succ!", pcSvrId);
    freeReplyObject(pcRedisReply);

    return MSG_SUCC;
}

int tSetKey(char *pcKey, char *pcData, int iDataLen) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    char sData[MSG_MAX_LEN * 2] = {0};

    if (MSG_MAX_LEN < iDataLen) {
        tLog(ERROR, "send data len[%d] > Max Len[%d] failure", iDataLen, MSG_MAX_LEN);
        return MSG_ERR;
    }
    tBcd2Asc(sData, pcData, iDataLen * 2, LEFT_ALIGN);
    snprintf(sCommand, sizeof (sCommand), "SET %s %s", pcKey, sData);

    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply || REDIS_REPLY_NIL == pcRedisReply->type) {
        tLog(ERROR, "Execut command[%s] failure[%s].", sCommand, pcRedisReply->str);
        freeReplyObject(pcRedisReply);
        return MSG_ERR;
    }
    if (!(REDIS_REPLY_STATUS == pcRedisReply->type && strcasecmp(pcRedisReply->str, "OK") == 0)) {
        tLog(ERROR, "Execut command[%s] failure[%s].", sCommand, pcRedisReply->str);
        freeReplyObject(pcRedisReply);
        return MSG_ERR;
    }
    //tLog(DEBUG, "Cmd[SET] Key[%s] Len[%d] Msg[%s] RTN[%s].", pcKey, iDataLen, pcData, pcRedisReply->str);
    freeReplyObject(pcRedisReply);

    return MSG_SUCC;
}

int tGetKey(char *pcKey, char *pcData, int *piDataLen) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    char sData[MSG_MAX_LEN * 2] = {0};
    int iLen = 0;

    snprintf(sCommand, sizeof (sCommand), "GET %s", pcKey);

    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply
            || REDIS_REPLY_NIL == pcRedisReply->type
            || REDIS_REPLY_STRING != pcRedisReply->type) {
        tLog(ERROR, "Execut command[%s] failure[%s].", sCommand, pcRedisReply->str);
        freeReplyObject(pcRedisReply);
        return MSG_ERR;
    }
    iLen = pcRedisReply->len;
    tStrCpy(sData, pcRedisReply->str, iLen);
    tAsc2Bcd(pcData, sData, iLen, LEFT_ALIGN);
    *piDataLen = (iLen + 1) / 2;
    //tLog(DEBUG, "Cmd[GET] Key[%s] Len[%d] Msg[%s].", pcKey, *piDataLen, pcData);
    freeReplyObject(pcRedisReply);

    return MSG_SUCC;
}

int tDelKey(char *pcKey) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    char sData[MSG_MAX_LEN * 2] = {0};
    int iLen = 0;

    snprintf(sCommand, sizeof (sCommand), "DEL %s", pcKey);

    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply
            || REDIS_REPLY_NIL == pcRedisReply->type
            || REDIS_REPLY_INTEGER != pcRedisReply->type) {
        tLog(ERROR, "Execut command[%s] failure[%s].", sCommand, pcRedisReply->str);
        freeReplyObject(pcRedisReply);
        return MSG_ERR;
    }
    tLog(INFO, "Cmd[DEL] Key[%s] RTN[%d].", pcKey, pcRedisReply->integer);
    freeReplyObject(pcRedisReply);

    return MSG_SUCC;
}
#if 0

int tSendCmd(char *pcCmd, char *pcKey, char *pcMsg, int iMsgLen) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    char sData[MSG_MAX_LEN * 2] = {0};

    if (MSG_MAX_LEN < iMsgLen) {
        tLog(ERROR, "send data len[%d] > Max Len[%d] failure", iMsgLen, MSG_MAX_LEN);
        return MSG_ERR;
    }
    if (iMsgLen > 0) {
        tBcd2Asc(sData, pcMsg, iMsgLen * 2, LEFT_ALIGN);
        snprintf(sCommand, sizeof (sCommand), "%s %s %s", pcCmd, pcKey, sData);
    } else
        snprintf(sCommand, sizeof (sCommand), "%s %s", pcCmd, pcKey);
    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply || REDIS_REPLY_NIL == pcRedisReply->type) {
        tLog(ERROR, "Execut command[%s] failure", sCommand);
        return MSG_ERR;
    }
    freeReplyObject(pcRedisReply);
    tLog(DEBUG, "Send Cmd[%s] Key[%s] Len[%d] Msg[%s].", pcCmd, pcKey, iMsgLen, pcMsg);

    return MSG_SUCC;
}

int tRecvCmd(char *pcCmd, char *pcKey, char *pcMsg, int *piMsgLen, int iTimeout) {
    char sCommand[MSG_MAX_LEN * 2] = {0};
    char sData[MSG_MAX_LEN * 2] = {0};
    int i, iLen = 0;

    /* 命令没有超时, 传iTimeout=-1*/
    if (iTimeout < 0) {
        snprintf(sCommand, sizeof (sCommand), "%s %s", pcCmd, pcKey);
    } else
        snprintf(sCommand, sizeof (sCommand), "%s %s %d", pcCmd, pcKey, iTimeout);

    redisReply* pcRedisReply = (redisReply*) redisCommand(g_pstRedisCxt, sCommand);
    if (NULL == pcRedisReply || REDIS_REPLY_NIL == pcRedisReply->type) {
        tLog(ERROR, "Execut command[%s] failure", sCommand);
        return MSG_ERR;
    }
    /* 数组类型 */
    if (REDIS_REPLY_ARRAY == pcRedisReply->type) {
        for (i = 0; i < pcRedisReply->elements; ++i) {
            redisReply* childReply = pcRedisReply->element[i];
            if (childReply->type == REDIS_REPLY_STRING) {
                tStrCpy(sData, childReply->str, strlen(childReply->str));
            }
        }

    }
    /* 字符串 */
    if (REDIS_REPLY_STRING == pcRedisReply->type) {
        tStrCpy(sData, pcRedisReply->str, strlen(pcRedisReply->str));
    }
    /* 整数 */
    if (REDIS_REPLY_INTEGER == pcRedisReply->type) {
        snprintf(sData, sizeof (sData), "%d", pcRedisReply->integer);
    }
    iLen = strlen(sData);
    tAsc2Bcd(pcMsg, sData, iLen, LEFT_ALIGN);
    tLog(DEBUG, "Cmd[%s] Key[%s] Len[%d] Msg[%s].", pcCmd, pcKey, (iLen + 1) / 2, pcMsg);
    if (pcMsg[0] == '\0') {
        freeReplyObject(pcRedisReply);
        return MSG_TIMEOUT;
    }
    *piMsgLen = (iLen + 1) / 2;
    freeReplyObject(pcRedisReply);

    return MSG_SUCC;
}
#endif

void tDoneSvr() {
    redisFree(g_pstRedisCxt);
}