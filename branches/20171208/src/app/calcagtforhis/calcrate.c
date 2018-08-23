/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2016��12��27��, ����5:39
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_redis.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "t_log.h"
#include "param.h"

extern int RequestProc();
extern int ResponseProc();

/* ��ʼ��Ӧ�ã���־�� */
int InitApp() {
    /*
        if (tOpenRedis() < 0) {
            tLog(ERROR, "redis����ʧ��.");
            return -1;
        }
     */
    if (tOpenDb() < 0) {
        tLog(ERROR, "���ݿ�����ʧ��.");
        return -1;
    }
    return 0;
}

/* �ͷż����ڴ�͹ر����ӵ� */
void DoneApp() {
   //tCloseRedis();
    tCloseDb();
    tDoneLog();
    tFreeConfig();
}

int main(int iArgc, char* pcArgv[]) {
    int iRet = -1, i = 0, piDataLen;
    pid_t tSonPid;
    char sData[5000] = {0};
    tDaemon();
    /* �����źţ�ֻ�����Ƴ��ź� */
    tSigProc();
    sigset(SIGUSR1, tSigQuit);
    /* ������������ */
    if ((iRet = InitParam(iArgc, pcArgv) < 0)) {
        fprintf(stderr, "Ӧ��[%s]���ز���ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    if (tInitLog(GetLogName()) < 0) {
        fprintf(stderr, "��ʼ��[%s]��־ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }
    if (InitApp() < 0) {
        tLog(ERROR, "[%s]����ʧ��.", pcArgv[0]);
        DoneApp();
        return (EXIT_FAILURE);
    }
    while (g_iQuitLoop) {
        iRet = FeeAdjust(pcArgv[1], pcArgv[2]);
        if( iRet == -2 ){
            break;
        }
    }
    DoneApp();
    return (EXIT_SUCCESS);
}
