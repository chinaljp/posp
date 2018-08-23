/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   qrpursebook.c
 * Author: Administrator
 *
 * Created on 2017年8月11日, 上午10:26
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_db.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "param.h"
#include "qrpursebook.h"
/*
 * 
 */

void SigQuit(int iSigNo) {
    int i;
    tLog(DEBUG, "收到[%d]退出信号!", iSigNo);
    g_iQutLoop = 0;
}

/* 屏蔽信号，只保留推出信号 */
void SigProc() {
    int i;
    for (i = 0; i < NSIG; i++) {
       if (i != SIGSEGV)
       sigset(i, SIG_IGN);
    }
}

/* 初始化应用，日志等 */
int InitApp() {

    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis连接失败.");
        return -1;
    }
    if (tOpenDb() < 0) {
        tLog(ERROR, "数据库连接失败.");
        return -1;
    }
    return 0;
}

/* 释放加载内存和关闭连接等 */
void DoneApp() {
    tCloseRedis();
    tCloseDb();
    tDoneLog();
}


int main(int iArgc, char* pcArgv[]) {
    int iLen,iRet;
    char sReqSvrId[32] = {0};
    char sMsg[MSG_MAX_LEN];
    
    g_iQutLoop = 1;
    tDaemon();
    
    /* 屏蔽信号，只保留推出信号 */
    SigProc();
    sigset(SIGUSR1, SigQuit);
    
    /* 解析启动参数 */
    if ((iRet = InitParam(iArgc, pcArgv) < 0)) {
        fprintf(stderr, "应用[%s]加载参数失败.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    if (tInitLog(GetLogName()) < 0) {
        fprintf(stderr, "初始化[%s]日志失败.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }
    if (InitApp() < 0) {
        tLog(ERROR, "[%s]启动失败.", pcArgv[0]);
        DoneApp();
        return (EXIT_FAILURE);
    }
    
    snprintf(sReqSvrId, sizeof (sReqSvrId), "%s_REQ", GetSvrId());
    tLog(INFO,"SvrId : [%s]",sReqSvrId);
    while(g_iQutLoop) {
        iLen = 0;
        MEMSET(sMsg);
        iRet = tRecvMsgEX(sReqSvrId, sMsg, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "获取消息失败.");
                break;
            case MSG_TIMEOUT:
                tLog(DEBUG, "[%s]等待消息超时[%d].", sReqSvrId, TIMEOUT);
                break;
            case MSG_SUCC:
                //tLog(DEBUG, "response msg[%s]", sMsg);
                /*返回处理*/
                if (DealRespMsg(sMsg) < 0) {
                    tLog(INFO, "处理失败返回fail");//待定
                    GoBackMsg("fail");
                }
                else {
                    tLog(INFO, "处理成功返回success");
                    GoBackMsg("success");
                }
                break;
        }
    }
    
    DoneApp();
    return (EXIT_SUCCESS);
}
