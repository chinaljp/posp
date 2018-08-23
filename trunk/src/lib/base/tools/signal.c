/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_redis.h"
#include "t_signal.h"
#include "t_log.h"

int g_iQuitLoop = APP_RUN; /* 程序退出标志 */
extern int GetAppNum();
extern char *GetSvrId();

int tIsQuit() {
    return g_iQuitLoop;
}

/* 接收退出信号 */
void tSigMsgQuit(int iSigNo) {
    int i;
    char sMsg[128] = {0};
    
    tLog(DEBUG, "收到[%d]退出信号!", iSigNo);
    tStrCpy(sMsg, "{STOP}", 4);
    g_iQuitLoop = APP_EXIT;
    for (i = 0; i < GetAppNum(); i++) {
        tSendMsg(GetSvrId(), sMsg, 4);
    }
}

void tSigQuit(int iSigNo) {
    int i;
    tLog(DEBUG, "收到[%d]退出信号!", iSigNo);
    g_iQuitLoop = APP_EXIT;
}

/* 屏蔽信号，只保留推出信号 */
void tSigProc() {
    int i;
    for (i = 0; i < NSIG; i++) {
        if (i != SIGSEGV)
            sigset(i, SIG_IGN);
    }
}