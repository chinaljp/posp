/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on  2017��2��25��, ����6:50
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

/* ��ʼ��Ӧ�ã���־�� */
int InitApp() {
    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis����ʧ��.");
        return -1;
    }
    return 0;
}

/* �ͷż����ڴ�͹ر����ӵ� */
void DoneApp() {
    tCloseRedis();
    tDoneLog();
}

int main(int argc, char** argv) {
    int iRet = -1;
    pid_t tSonPid;

    tDaemon();
    /* �����źţ�ֻ�����Ƴ��ź� */
    tSigProc();

    sigset(SIGUSR1, tSigQuit);
    /*  ������������ */
    if ((iRet = InitParam(argc, argv) < 0)) {
        fprintf(stderr, "Ӧ��[%s]���ز���ʧ��.\n", argv[0]);
        return (EXIT_FAILURE);
    }


    if (tInitLog(GetLogName()) < 0) {
        fprintf(stderr, "[%s]��ʼ����־ʧ��.\n", argv[0]);
        return (EXIT_FAILURE);
    }
    /*  �������ʼ��redis��������
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
    tLog(INFO, "����ģ�������ɹ�.");

    RecvProc();
    kill(tSonPid, SIGUSR1);
    //mSvrDown(g_stModuleInfo.sSvrId);
    DoneApp();
    //tFreeConfig();

    tLog(INFO, "����ģ���˳��ɹ�.");
    return (EXIT_SUCCESS);
}

