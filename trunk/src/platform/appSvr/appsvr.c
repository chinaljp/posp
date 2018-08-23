/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2016年11月30日, 下午9:01
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_tools.h"
#include "t_signal.h"
#include "t_config.h"
#include "t_redis.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "t_log.h"
#include "gloab.h"
#include "param.h"
#include "dylib.h"


/*
 * 业务流程处理
 */

/* 初始化应用，日志等 */
int InitApp() {
    int iRet = -1;

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
    tFreeConfig();
}

int main(int iArgc, char* pcArgv[]) {
    int iRet = -1, i = 0;
    pid_t tSonPid;

    tDaemon();
    /* 屏蔽信号，只保留推出信号 */
    tSigProc();

    /* 解析启动参数 */
    if ((iRet = InitParam(iArgc, pcArgv) < 0)) {
        fprintf(stderr, "应用[%s]加载参数失败.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    if (tInitLog(GetLogName()) < 0) {
        fprintf(stderr, "初始化日志失败.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    /* 请求进程 */
    for (i = 0; i < GetAppNum(); i++) {
        tSonPid = fork();
        if (tSonPid < 0) {
            tLog(ERROR, "Init son pid Failed! %d:%s", errno, strerror(errno));
            DoneApp();
            return (EXIT_FAILURE);
        }
        if (0 == tSonPid) {
            sigset(SIGUSR1, tSigQuit);
            if (InitApp() < 0) {
                DoneApp();
                return (EXIT_FAILURE);
            }
            if (InitQDyLib(GetSvrId(), GetLibName()) < 0) {
                DoneApp();
                return (EXIT_FAILURE);
            }
            if (LoadQTrans(GetConfig(), GetSvrId()) < 0) {
                DoneQDyLib();
                DoneApp();
                return -1;
            }
            if (g_stLibInfo_Q.pfAppServerInit(iArgc, pcArgv) < 0) {
                DoneApp();
                return (EXIT_FAILURE);
            }
            RequestProc();
            g_stLibInfo_Q.pfAppServerDone();
            DoneQDyLib();
            DoneApp();
            return (EXIT_SUCCESS);
        }
    }
    /* 应答进程 */
    for (i = 0; i < GetAppNum(); i++) {
        tSonPid = fork();
        if (tSonPid < 0) {
            tLog(ERROR, "Init son pid Failed! %d:%s", errno, strerror(errno));
            DoneApp();
            return (EXIT_FAILURE);
        }
        if (0 == tSonPid) {
            sigset(SIGUSR1, tSigQuit);
            if (InitApp() < 0) {
                DoneApp();
                return (EXIT_FAILURE);
            }
            if (InitPDyLib(GetSvrId(), GetLibName()) < 0) {
                DoneApp();
                return (EXIT_FAILURE);
            }
            if (LoadPTrans(GetConfig(), GetSvrId()) < 0) {
                DonePDyLib();
                DoneApp();
                return -1;
            }
            if (g_stLibInfo_P.pfAppServerInit(iArgc, pcArgv) < 0) {
                DoneApp();
                return (EXIT_FAILURE);
            }
            ResponseProc();
            g_stLibInfo_P.pfAppServerDone();
            DonePDyLib();
            DoneApp();
            return (EXIT_SUCCESS);
        }
    }
    tDoneLog();
    return (EXIT_SUCCESS);
}
