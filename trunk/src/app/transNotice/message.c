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

int PosRequestMsg();
int InlineRequestMsg();
int FailInlineResquestMsg(int iDayFlag) ;
//int ResponseMsg(char *pcRespMsg);
#if 0
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
#endif
/* 请求处理 */
int RequestProc() {
    int iRet = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrName[64] = {0};
    char sTransGroup[6 + 1] = {0};
    char sCurTime[6+1] = {0}; 
    
    snprintf(sSvrName, sizeof (sSvrName), "%s_Q", GetSvrId());
    tLog(INFO, "[%s]进程,启动成功.", sSvrName);
    memcpy( sTransGroup, GetTransGroup(), strlen(GetTransGroup()) );
    tTrim(sTransGroup);
    
    tLog(INFO,"本进程指定扫描的交易流水组[%s]",sTransGroup);
    
    while (g_iQuitLoop) {
        /*指定需要扫的交易流水表*/
        if( !memcmp(sTransGroup,"POS",3) ) {
            PosRequestMsg();
        } else if( !memcmp(sTransGroup,"INLINE",6) ) {
            InlineRequestMsg();
        } else if( !memcmp(sTransGroup,"FAIL",4) ) {
            /*获取当前时间*/
            tGetTime(sCurTime, "", -1);
            /*0点整至0点10分整， 前一日的待支付状态或交易失败的的二维码交易切到历史流水表中，此时段 去扫描历史表 进行推送 二维码失败的交易;
            二维码失败交易推送通知暂时不再进行扫描流水表推送*/
            if(memcmp(sCurTime,"000000",6) >= 0 && memcmp(sCurTime,"001000",6) <= 0 ) {
                tLog(INFO,"当前时间sCurTime:[%s]，扫描历史表推送前一日的二维码交易失败的通知",sCurTime);
                FailInlineResquestMsg(1);
                /*175 + 5 秒扫描一次历史流水表*/
                sleep(175);
            } else {
                FailInlineResquestMsg(0);
            }   
        }
        sleep(5);
    }
    tLog(INFO, "[%s]进程,退出成功.", sSvrName);
    return 0;
}

int PosRequestMsg() {
    int iRet = 0;
    /* 扫表查找 */
    iRet = ScanPosTrans();
    if (iRet < 0) {
        /* -2代表未查找到交易流水 */
        if (iRet != -2)
            tLog(ERROR, "查询待推送的pos交易流水出错！");
        return -1;
    }
    return 0;
}

int InlineRequestMsg() {
    int iRet = 0;
    /* 扫表查找 */
    iRet = ScanInlineTrans();
    if (iRet < 0) {
        /* -2代表未查找交易流水 */
        if (iRet != -2)
            tLog(ERROR, "查询待推送的二维码交易流水出错！");
        return -1;
    }
    return 0;
}

int FailInlineResquestMsg(int iDayFlag) {
    int iRet = 0;
    /* 扫表查找 */
    if ( 0 == iDayFlag ) {
        iRet = ScanFailInlineTrans();
        if (iRet < 0) {
            /* -2代表未查找交易流水 */
            if (iRet != -2)
                tLog(ERROR, "查询待推送的二维码失败的交易流水出错！");
            return -1;
        }
    } else if ( 1 == iDayFlag ){
        iRet = ScanDayFailInlineTrans();
        if (iRet < 0) {
            /* -2代表未查找交易流水 */
            if (iRet != -2)
                tLog(ERROR, "查询待推送的二维码失败的交易流水出错！");
            return -1;
        }
    }
    
    return 0;
}

/* 应答处理 暂时用不到*/
#if 0
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
    char sMerchOrderNo[64] = {0};
    char sRespCode[2 + 1] = {0};

    pstJson = cJSON_Parse(pcRespMsg);

    /* 处理业务 */
    GET_STR_KEY(pstJson, "respCode ", sRespCode);
    GET_STR_KEY(pstJson, "orderNo", sMerchOrderNo);

    if (!memcmp(sRespCode, "00", 2)) {
        //更新valid_flag为0
        if (UpdQRNoticeFlag(pstJson) < 0) {
            tLog(ERROR, "NOTICE服务器更新Notice成功标志失败.");
        }
        tLog(ERROR, "二维码交易回调成功.");
    }

    cJSON_Delete(pstJson);
    return 0;
}
#endif