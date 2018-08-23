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

    /* ΢�ڶ�ά�벻����Ľ��� */
    if (WbNoSettleProc() < 0)
        return -1;
    /* ΢�ڶ�ά�����ƽ */
    if (WbEqualProc() < 0){
        tLog(INFO, "΢�ڶ�ά�����ƽ,����ʧ��.");
        return -1;
    }
    /* ΢�ڶ�ά�볤�� */
    if (WbLongProc() < 0){
        tLog(INFO, "΢�ڶ�ά�볤��,����ʧ��.");
        return -1;
    }
    
       /* ���Ŷ�ά�벻����Ľ��� */
    /*
    if (ZxNoSettleProc() < 0)
        return -1;*/
    /* ���Ŷ�ά�����ƽ */
    if (ZxEqualProc() < 0){
        tLog(INFO, "���Ŷ�ά�����ƽ,����ʧ��.");
        return -1;
    }
    /* ���Ŷ�ά�볤�� */
    if (ZxLongProc() < 0){
        tLog(INFO, "���Ŷ�ά�볤��,����ʧ��.");
        return -1;
    }
    
     /* ���̶�ά�����ƽ */
    if (BsEqualProc() < 0){
        tLog(INFO, "���̶�ά�����ƽ,����ʧ��.");
        return -1;
    }
    /* ���̶�ά�볤�� */
    if (BsLongProc() < 0){
        tLog(INFO, "���̶�ά�볤��,����ʧ��.");
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
    tLog(INFO, "��ά����˳���ʼ���ɹ�.");

    iRet = CheckTrans();
    if (iRet < 0) {
        tLog(ERROR, "��ά�����ʧ��");
        return -1;
    }
    tLog(INFO, "��ά����˴���ɹ�.");
    DoneApp();
    return (EXIT_SUCCESS);
}

