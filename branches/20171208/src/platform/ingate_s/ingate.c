/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on  2017年2月25日, 下午6:50
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "t_redis.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "t_log.h"
#include "t_signal.h"
#include "param.h"
#include "ingate.h"

/*
 * 
 */
int g_iaPipeFd[2];

/* 初始化应用，日志等 */
int InitApp() {
    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis连接失败.");
        return -1;
    }
    return 0;
}

/* 释放加载内存和关闭连接等 */
void DoneApp() {
    tCloseRedis();
    tDoneLog();
}

int main(int argc, char** argv) {
    int iRet = -1;
    pid_t tSonPid;

    tDaemon();
    /* 屏蔽信号，只保留推出信号 */
    tSigProc();

    sigset(SIGUSR1, tSigQuit);
    /*  解析启动参数 */
    if ((iRet = InitParam(argc, argv) < 0)) {
        fprintf(stderr, "应用[%s]加载参数失败.\n", argv[0]);
        return (EXIT_FAILURE);
    }


    if (tInitLog(GetLogName()) < 0) {
        fprintf(stderr, "[%s]初始化日志失败.\n", argv[0]);
        return (EXIT_FAILURE);
    }
    /*  在这里初始化redis会有问题
        if (InitApp() < 0) {
            DoneApp();
            return (EXIT_FAILURE);
        }
     */
    if (pipe(g_iaPipeFd) < 0) {
        tLog(ERROR, "unable open pipe! %d:%s", errno, strerror(errno));
        //tFreeConfig();
        tDoneLog();
        return (EXIT_FAILURE);
    }

    tSonPid = fork();
    if (tSonPid < 0) {
        tLog(ERROR, "unable fork! %d:%s", errno, strerror(errno));
        //tFreeConfig();
        tDoneLog();
        return (EXIT_FAILURE);
    } else if (tSonPid == 0) {
        if (InitApp() < 0) {
            DoneApp();
            return (EXIT_FAILURE);
        }
        SendProc();
        kill(getppid(), SIGUSR1);
        DoneApp();
        exit(0);
    }
    if (InitApp() < 0) {
        DoneApp();
        return (EXIT_FAILURE);
    }
    tLog(INFO, "接入模块启动成功.");

    RecvProc();
    kill(tSonPid, SIGUSR1);
    //mSvrDown(g_stModuleInfo.sSvrId);
    DoneApp();
    //tFreeConfig();

    tLog(INFO, "接入模块退出成功.");
    return (EXIT_SUCCESS);
}

