/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2016��11��30��, ����9:00
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

    if (tOpenDb() < 0) {
        tLog(ERROR, "���ݿ�����ʧ��.");
        return -1;
    }
    return 0;
}

/* �ͷż����ڴ�͹ر����ӵ� */
void DoneApp() {
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
    InitApp();
    SndProc(pcArgv);

    DoneApp();
    return (EXIT_SUCCESS);
}




