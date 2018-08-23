/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   qrpursebook.c
 * Author: Administrator
 *
 * Created on 2017��8��11��, ����10:26
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_db.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "param.h"
#include "qrpursebook.h"
/*
 * 
 */

void SigQuit(int iSigNo) {
    int i;
    tLog(DEBUG, "�յ�[%d]�˳��ź�!", iSigNo);
    g_iQutLoop = 0;
}

/* �����źţ�ֻ�����Ƴ��ź� */
void SigProc() {
    int i;
    for (i = 0; i < NSIG; i++) {
       if (i != SIGSEGV)
       sigset(i, SIG_IGN);
    }
}

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
}


int main(int iArgc, char* pcArgv[]) {
    int iLen,iRet;
    char sReqSvrId[32] = {0};
    char sMsg[MSG_MAX_LEN];
    
    g_iQutLoop = 1;
    tDaemon();
    
    /* �����źţ�ֻ�����Ƴ��ź� */
    SigProc();
    sigset(SIGUSR1, SigQuit);
    
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
    
    snprintf(sReqSvrId, sizeof (sReqSvrId), "%s_REQ", GetSvrId());
    tLog(INFO,"SvrId : [%s]",sReqSvrId);
    while(g_iQutLoop) {
        iLen = 0;
        MEMSET(sMsg);
        iRet = tRecvMsgEX(sReqSvrId, sMsg, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                break;
            case MSG_TIMEOUT:
                tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", sReqSvrId, TIMEOUT);
                break;
            case MSG_SUCC:
                //tLog(DEBUG, "response msg[%s]", sMsg);
                /*���ش���*/
                if (DealRespMsg(sMsg) < 0) {
                    tLog(INFO, "����ʧ�ܷ���fail");//����
                    GoBackMsg("fail");
                }
                else {
                    tLog(INFO, "����ɹ�����success");
                    GoBackMsg("success");
                }
                break;
        }
    }
    
    DoneApp();
    return (EXIT_SUCCESS);
}
