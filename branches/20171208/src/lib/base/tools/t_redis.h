/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   redis.h
 * Author: feng.gaoo
 *
 * Created on 2016年12月5日, 下午9:21
 */

#ifndef REDIS_H
#define REDIS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "t_cjson.h"
#include "t_macro.h"
    /* 错误码 */
#define MSG_SUCC    0
#define MSG_ERR -1
#define MSG_TIMEOUT -2
    /* 消息的最大长度，只在此处设置 */
#define MSG_MAX_LEN 4096
    /* 接收消息超时时间 */
#define TIMEOUT 3
#define REQ_MSG '1'
#define REP_MSG '2'

    typedef struct {
        char sSvrId[SVRID_LEN + 1]; //应答队列，也可当成请求的原地址
        char sKey[MSG_KEY_LEN + 1];
        char cMsgType;
        cJSON *pstDataJson;
    } MsgData;

    int tInitSvr(const char *pcIp, const int iPort);
    int tSvcACallEx(char *pcQSvrId, char *pcPSvrId, char *pcKey, cJSON *pstDataJson);
    int tReplyEx(char *pcQSvrId, char *pcPSvrId, char *pcKey, cJSON *pstDataJson);
    
    //int tSvcCall(char *pcSvrId, cJSON *pcSendJson, cJSON **ppcRecvJson, int iTimeout);
    int tSvcACall(char *pcQSvrId, MsgData *pstMsgData);
    /* 应答 */
    int tReply(char *pcQSvrId, MsgData *pstMsgData);
    int tSvcCall(char *pcQSvrId, MsgData *pstQMsgData, char *pcPSvrId, MsgData *pstPMsgData, int iTimeout);
    int tWaitMsg(char *pcPSvrId, MsgData *pstMsgData, int iTimeout);
    int tRecvMsgEX(char *pcSvrId, char *pcData, int *piDataLen, int iTimeout);/*add by GuoJiaQing */
    //int tSendMsg(char *pcQSvrId, char *pcData, int iDataLen);
    //int tRecvMsg(char *pcSvrName, char *pcData, int *piDataLen, int iTimeout);
    int tSetKey(char *pcKey, char *pcData, int iDataLen);
    int tGetKey(char *pcKey, char *pcData, int *piDataLen);
    int tDelKey(char *pcKey);
    void tDoneSvr();
    void tGetUniqueKey(char *uniKey);
#ifdef __cplusplus
}
#endif

#endif /* REDIS_H */

