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
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"




int RequestMsg();
int ResponseMsg(char *pcRespMsg);

int SendMsg(char *pcReqSvrId, char *pcKey, cJSON *pstDataJson) {
    cJSON *pstJson = NULL, *pcstSubJson = NULL;
    char sRepSvrId[64] = {0};
    char *pcMsg = NULL;

    /* 判断接收的svrid和应答的svrid是否为空 */
    if (NULL == pcReqSvrId || pcReqSvrId[0] == '\0') {
        tLog(ERROR, "请求队列[%s]有空值,无法发送.", pcReqSvrId);
        return -1;
    }
    /* 发送的交易对象 */
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());
    SET_STR_KEY(pstJson, "svrid", sRepSvrId);
    SET_STR_KEY(pstJson, "key", pcKey);

    pcstSubJson = cJSON_Duplicate(pstDataJson, 1);

    SET_JSON_KEY(pstJson, "data", pcstSubJson);


    pcMsg = cJSON_PrintUnformatted(pstJson);
    if (tSendMsg(pcReqSvrId, pcMsg, strlen(pcMsg)) < 0) {
        tLog(ERROR, "发送消息到SvrId[%s]失败,data[%s].", pcReqSvrId, pcMsg);
    }
    cJSON_Delete(pstJson);
    return 0;
}

/* 请求处理 */
int RequestProc() {
    char sSvrName[64] = {0};

    snprintf(sSvrName, sizeof (sSvrName), "%s_Q", GetSvrId());
    tLog(DEBUG, "[%s]进程,启动成功.", sSvrName);
    while (g_iQuitLoop) {
        RequestMsg();
        /* 间隔5秒发起一次查询 */
        sleep(GetTimeInterval());
    }
    tLog(INFO, "[%s]进程,退出成功.", sSvrName);
    return 0;
}

int RequestMsg() {
    cJSON *pstJson = NULL;  
    int iRet = 0;
    pstJson = cJSON_CreateObject();
    /* 扫表查找 */
    iRet = ScanQrTrans(pstJson);
    if (iRet < 0) {
        /* -2代表未查找到二维码流水 */
        if (iRet != -2)
            tLog(ERROR, "查找未支付二维码流水表出错！");
        return -1;
    }
    cJSON_Delete(pstJson);
    return 0;
}

/* 应答处理 */
int ResponseProc() {
    int iRet = -1, iLen = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrName[64] = {0};

    snprintf(sSvrName, sizeof (sSvrName), "%s_P", GetSvrId());
    tLog(DEBUG, "[%s]进程,启动成功.", sSvrName);
    while (g_iQuitLoop) {
        memset(sMsg, 0, sizeof (sMsg));
        iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "获取消息失败.");
                break;
            case MSG_TIMEOUT:
                tLog(DEBUG, "[%s]等待消息超时[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                //tLog(DEBUG, "response msg[%s]", sMsg);
                if (ResponseMsg(sMsg) < 0) {
                    tLog(ERROR, "应答处理失败,放弃处理.");
                }
                break;
        }
    }
    tLog(INFO, "[%s]进程,退出成功.", sSvrName);
    return 0;
}

int ResponseMsg(char *pcRespMsg) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char sRepSvrId[64] = {0}, sRepKey[64] = {0};
    char sRespCode[2 + 1] = {0}, sMerchOrderNo[27 + 1] = {0}, sTransCode[6 + 1] = {0};

    pstJson = cJSON_Parse(pcRespMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "获取消息失败,放弃处理.");
        return -1;
    }
    GET_STR_KEY(pstJson, "svrid", sRepSvrId);
    GET_STR_KEY(pstJson, "key", sRepKey);
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "获取data数据失败,消息放弃处理.");
        cJSON_Delete(pstJson);
        return -1;
    }
    /* 处理业务 */
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "qr_order_no", sMerchOrderNo);
    
    
    tLog(ERROR, "交易码[%s]订单号[%s]的查询交易返回码为[%s]", sTransCode, sMerchOrderNo, sRespCode);
    if (!memcmp(sRespCode, "00", 2)) {
        //更新valid_flag为0
        //if (UpdQRTrans(pstDataJson) < 0) {
        //    tLog(ERROR, "NOTICE服务器更新Notice成功标志失败.");
        //}
        tLog(ERROR, "交易码[%s]订单号[%s]交易支付成功.", sTransCode, sMerchOrderNo);
    }

    cJSON_Delete(pstJson);
    return 0;
}

