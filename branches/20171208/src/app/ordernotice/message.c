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




int RequestMsg(char *pcRequestMsg);
int ResponseMsg(char *pcRespMsg);
void TimeoutProc();

void Net2JavaNotice(cJSON *pstTransJson, cJSON *pstNetJson) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = {0}, sMerchName[60 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDateTime[14 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sMerchOrderNo[30 + 1] = {0}, sAgentId[8 + 1] = {0}, sAmount[12 + 1] = {}, sSettleAmount[12 + 1] = {}, sTransCodeOrder[4 + 1] = {0};
    char sUrl[20 + 1] = {0}, sPort[10 + 1] = {0};
    char sPlatcode[20 + 1] = {0};
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "merch_name", sMerchName);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "sys_trace", sTrace);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "merch_order_no", sMerchOrderNo);

    GET_STR_KEY(pstTransJson, "order_url", sUrl);
    GET_STR_KEY(pstTransJson, "order_port", sPort);
    GET_STR_KEY(pstTransJson, "platcode", sPlatcode);

    //组请求报文
    strcpy(sTransCodeOrder, sTransCode + 2);
    SET_STR_KEY(pstNetJson, "tranCode", sTransCodeOrder);
    SET_STR_KEY(pstNetJson, "merchantId", sMerchId);
    SET_STR_KEY(pstNetJson, "merchantName", sMerchName);
    SET_STR_KEY(pstNetJson, "platCode", sPlatcode);
    SET_STR_KEY(pstNetJson, "traceNo", sTrace);
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_STR_KEY(pstNetJson, "url", sUrl);
    SET_STR_KEY(pstNetJson, "port", sPort);
    strcpy(sTransDateTime, sTransDate);
    strcat(sTransDateTime, sTransTime);
    SET_STR_KEY(pstNetJson, "tranTime", sTransDateTime);
    SET_STR_KEY(pstNetJson, "orderId", sMerchOrderNo);
    sprintf(sAmount, "%.f", dAmount);
    sprintf(sSettleAmount, "%.f", dAmount - (dFee * 100));
    SET_STR_KEY(pstNetJson, "amount", sAmount);
    SET_STR_KEY(pstNetJson, "settleAmount", sSettleAmount);
    SET_STR_KEY(pstNetJson, "rrn", sRrn);

}

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
    int iRet = -1, iLen = 0, iCnt = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrName[64] = {0};

    snprintf(sSvrName, sizeof (sSvrName), "%s_Q", GetSvrId());
    tLog(DEBUG, "[%s]进程,启动成功.", sSvrName);
    while (g_iQuitLoop) {
        memset(sMsg, 0, sizeof (sMsg));
        iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "获取消息失败.");
                break;
            case MSG_TIMEOUT:
                iCnt++;
                tLog(DEBUG, "[%s]等待消息超时[%d].", sSvrName, TIMEOUT);
                /* 设置定时执行,通过计数器控制 */
                if (iCnt == 10) {
                    iCnt = 0;
                    TimeoutProc();
                }
                break;
            case MSG_SUCC:
                //tLog(DEBUG, "request msg[%s]", sMsg);
                if (RequestMsg(sMsg) < 0) {
                    tLog(ERROR, "请求失败,放弃处理.");
                }
                break;
        }
    }
    tLog(INFO, "[%s]进程,退出成功.", sSvrName);
    return 0;
}

void TimeoutProc() {

    /* 扫描notice表,自动发送通知 */
    tLog(INFO,"超时检查......");
    ScanNotice();

    return 0;
}

int RequestMsg(char *pcRequestMsg) {
    char *pcTransCode = NULL, *psTran = NULL;
    cJSON *pstJson = NULL, *pstDataJson = NULL, *pstNetJson = NULL;
    char *pcReqSvrId = NULL, *pcReqKey = NULL;
    char sReqSvrId[64] = {0}, sKey[64] = {0}, sRrn[16 + 1] = {0};

    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return 0;
    }

    pstJson = cJSON_Parse(pcRequestMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "获取消息失败,放弃处理.");
        return -1;
    }
    DUMP_JSON(pstJson, psTran);
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "获取data数据失败,消息放弃处理.");
        return -1;
    }
    GET_STR_KEY(pstDataJson, "rrn", sRrn);

    /* 添加notice流水,组notice消息 */
    if (AddNoticeLs(pstDataJson) < 0) {
        cJSON_Delete(pstJson);
        tLog(ERROR, "NOTICE服务器添加流水表出错");
        return -1;
    }

    Net2JavaNotice(pstDataJson, pstNetJson);

    /* 发送订单通知 */
    snprintf(sKey, sizeof (sKey), "%s_%s", GetSvrId(), sRrn);
    SendMsg("JAVANOTICE_Q", sKey, pstNetJson);
    cJSON_Delete(pstNetJson);
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
    char sRespCode[2 + 1] = {0};

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
    GET_STR_KEY(pstDataJson, "responseCode", sRespCode);

    if (!memcmp(sRespCode, "00", 2)) {
        if (UpdNoticeLs(pstDataJson) < 0) {
            tLog(ERROR, "NOTICE服务器更新Notice成功标志失败.");
        }
             tLog(ERROR, "NOTICE服务器更新Notice成功.");
    } else {
        if (UpdNoticeCnt(pstDataJson) < 0) {
            tLog(ERROR, "NOTICE服务器更新notice记数失败.");
        }
        tLog(ERROR, "NOTICE服务器更新notice记数成功.");
    }

    cJSON_Delete(pstJson);
    return 0;
}

