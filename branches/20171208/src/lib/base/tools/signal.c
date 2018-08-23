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

int g_iQuitLoop = APP_RUN; /* �����˳���־ */
extern int GetAppNum();
extern char *GetSvrId();

int tIsQuit() {
    return g_iQuitLoop;
}

/* �����˳��ź� */
void tSigMsgQuit(int iSigNo) {
    int i;
    char sMsg[128] = {0};
    
    tLog(DEBUG, "�յ�[%d]�˳��ź�!", iSigNo);
    tStrCpy(sMsg, "{STOP}", 4);
    g_iQuitLoop = APP_EXIT;
    for (i = 0; i < GetAppNum(); i++) {
        tSendMsg(GetSvrId(), sMsg, 4);
    }
}

void tSigQuit(int iSigNo) {
    int i;
    tLog(DEBUG, "�յ�[%d]�˳��ź�!", iSigNo);
    g_iQuitLoop = APP_EXIT;
}

/* �����źţ�ֻ�����Ƴ��ź� */
void tSigProc() {
    int i;
    for (i = 0; i < NSIG; i++) {
        if (i != SIGSEGV)
            sigset(i, SIG_IGN);
    }
}