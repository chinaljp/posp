/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Gjq
 *
 * Created on 2018��4��12��, ����10:12
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

    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis����ʧ��.");
        return -1;
    }
    if (tOpenDb() < 0) {
        tLog(ERROR, "���ݿ�����ʧ��.");
        return -1;
    }
    return 0;
}

/* �ͷż����ڴ�͹ر����ӵ� */
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
    /* �����źţ�ֻ�����Ƴ��ź� */
    tSigProc();

    /* ������������ */
    if ((iRet = InitParam(iArgc, pcArgv) < 0)) {
        fprintf(stderr, "Ӧ��[%s]���ز���ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    if (tInitLog(GetLogName()) < 0) {
        fprintf(stderr, "��ʼ��[%s]��־ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    /* �������� */
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
            RequestProc();
            DoneApp();
            return (EXIT_SUCCESS);
        }
    }
    DoneApp();
    return (EXIT_SUCCESS);
}                                                                                                                      

