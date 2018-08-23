/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Administrator
 *
 * Created on 2017��3��28��, ����2:16
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
#include "t_db.h"

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
}

int CheckTrans() {

    /* epos������Ľ��� */
    if (EposNoSettleProc() < 0)
        return -1;
    /* epos����ƽ */
    if (EposEqualProc() < 0){
        tLog(INFO, "epos����ƽ,����ʧ��.");
        return -1;
    }
    /* epos���� */
    if (EposLongProc() < 0){
        tLog(INFO, "epos����,����ʧ��.");
        return -1;
    }
    
    return 0;
}

/*
 * 
 */
int main(int iArgc, char *pcArgv[]) {

    int iRet = -1, i = 0;
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
    sigset(SIGUSR1, tSigQuit);

    if (InitApp() < 0) {
        DoneApp();
        return (EXIT_FAILURE);
    }
    tLog(INFO, "epos���˳���ʼ���ɹ�.");

    iRet = CheckTrans();
    if (iRet < 0) {
        tLog(ERROR, "epos����ʧ��");
        return -1;
    }
    tLog(INFO, "epos���˴���ɹ�.");
    DoneApp();
    return (EXIT_SUCCESS);
}

