/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * 
 */

/* 
 * File:   main.c
 * Author: Administrator
 *
 * Created on 2017��3��24��, ����4:02
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
#include "t_app_conf.h"
#include "t_log.h"
#include "dayend_trans_detail.h"
#include "param.h"

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

int DayEnd() {

    char sLogicDate[8 + 1] = {0};
    if (GetSysParam(&sLogicDate) != 0)
        return -1;
    tLog(INFO, "ϵͳ״̬[����],���д�����...");
    if (CutOff(&sLogicDate) < 0) {
        tLog(ERROR, "ϵͳ�߼�����ʧ��.");
        return ( -S_CUTOFF);
    }

    tLog(INFO, "ϵͳ�߼����гɹ�,���߼�����Ϊ[%s].", sLogicDate);
    /* �̻��޶��� */
    if (ChkLimitAmt() < 0) {
        tLog(ERROR, "������ˮʧ��.");
        return ( -S_LIMIT);
    }
    tLog(ERROR, "ϵͳ״̬[������ˮ],������ˮת��ʷ��ˮ������..."); 
    if (ClearProc(&sLogicDate) < 0) {
        tLog(ERROR, "������ˮʧ��.");
        return ( -S_CLEAR);
    }
    tLog(INFO, "������ˮ�ɹ�,�߼�����С��[%s]�Ľ�����ˮת����ʷ��ˮ.", sLogicDate);
    if (RecoveryLimit() < 0) {
        tLog(ERROR, "�ָ��̻�D0���ʧ��.");
        return ( -S_RECOVLIMIT);
    }
    tLog(INFO, "�̻�D0��Ȼָ��ɹ�.");

    if (RePufaLimit() < 0) {
        tLog(ERROR, "�ָ��ַ��������ʧ��.");
        return ( -S_RECOVLIMIT);
    }
    tLog(ERROR, "�ָ��ַ�������ȳɹ�.");
    if (RePufaLocalMerchLimit() < 0) {
        tLog(ERROR, "�ָ��ַ������̻����ʧ��.");
        return ( -S_RECOVLIMIT);
    }
    tLog(ERROR, "�ָ��ַ������̻���ȳɹ�.");

    return (0);
}

/*
 * 
 */
int main(int iArgc, char *pcArgv[]) {

    int iRet = 0;

    /* �����źţ�ֻ�����Ƴ��ź� */
    tSigProc();

    /* ������������ */
    if ((iRet = InitParam(iArgc, pcArgv) < 0)) {
        fprintf(stderr, "Ӧ��[%s]���ز���ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    if (tInitLog("dayend") < 0) {
        fprintf(stderr, "��ʼ��[%s]��־ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    sigset(SIGUSR1, tSigQuit);

    if (InitApp() < 0) {
        DoneApp();
        return (EXIT_FAILURE);
    }
    tLog(INFO, "���մ������ʼ���ɹ�.");

    iRet = DayEnd();

    if (iRet < 0)
        tLog(ERROR, "���ش���!!!���մ����������ʧ��!");
    else
        tLog(INFO, "���մ���������гɹ�!");

    DoneApp();
    return (EXIT_SUCCESS);
}

