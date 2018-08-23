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
#include "t_app_conf.h"
#include "t_log.h"
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

int getNextLinePos(FILE *p) {
    int ch;
    int curpos = 0;

    ch = getc(p);
    curpos = ftell(p);
    while (ch != EOF) {
        //    putchar(ch);
        ch = getc(p);
        if (ch == '\n') {
            break;
        }
    }

    return (ftell(p) - curpos + 1);
}

int ImpAcom(char *pcDate) {
    char sCurDate[9] = {0}, sPath[256] = {0}, sTmp[1024] = {0}, sData[128];
    FILE * pstFp = NULL;
    ZxSettleTransDetail stZxTransDetail;
    int i = 0, j = 0, iRet = 0;
    char * pstr = NULL, *pcT[33];

    //    tGetDate( sCurDate, "", -1 );
    //   tAddDay(sCurDate, -1 );
    strcpy(sCurDate, pcDate);
    sprintf(sPath, "%s/saomafuZXBill%s.csv", getenv("ZX_SETTLE_DIR"), sCurDate);
    if ((pstFp = fopen(sPath, "r")) == NULL) {
        tLog(ERROR, "���ļ�[%s]ʧ��,errno[%d:%s]", sPath, errno, strerror(errno));
        return -1;
    }
    /* �����ļ����б��⣬������һ�� */
    fseek(pstFp, getNextLinePos(pstFp), SEEK_SET);
    while (!feof(pstFp)) {
        MEMSET(sTmp);
        memset(&stZxTransDetail, 0x00, sizeof (ZxSettleTransDetail));
        fgets(sTmp, 1024, pstFp);
        if (!memcmp(sTmp, "�ܽ��׵���", strlen("�ܽ��׵���"))) {
            tLog(DEBUG, "��������ļ���β");
            break;
        }

        tLog(DEBUG, "��[%d]��:[%s]", ++j, sTmp);

        if (sTmp[0] == '\0') {
            tLog(ERROR, "�ļ�[%s]�޼�¼,����.", sPath);
            break;
        }

        i = tSeperate(sTmp, pcT, 34, ',');

        tStrCpy(stZxTransDetail.sTansTime, *pcT + 1, strlen(*pcT + 1));
        tStrCpy(stZxTransDetail.sAccountId, *(pcT + 1) + 1, strlen(*(pcT + 1) + 1));
        tStrCpy(stZxTransDetail.sThMerchNo, *(pcT + 2) + 1, strlen(*(pcT + 2) + 1));
        tStrCpy(stZxTransDetail.sMerchNo, *(pcT + 3) + 1, strlen(*(pcT + 3) + 1));
        tStrCpy(stZxTransDetail.sSubMerchNo, *(pcT + 4) + 1, strlen(*(pcT + 4) + 1));
        tStrCpy(stZxTransDetail.sTermId, *(pcT + 5) + 1, strlen(*(pcT + 5) + 1));
        tStrCpy(stZxTransDetail.sWftOrderNo, *(pcT + 6) + 1, strlen(*(pcT + 6) + 1));
        tStrCpy(stZxTransDetail.sThOrderNo, *(pcT + 7) + 1, strlen(*(pcT + 7) + 1));
        tStrCpy(stZxTransDetail.sMerchOrderNo, *(pcT + 8) + 1, strlen(*(pcT + 8) + 1));
        tStrCpy(stZxTransDetail.sUserId, *(pcT + 9) + 1, strlen(*(pcT + 9) + 1));
        tStrCpy(stZxTransDetail.sTransType, *(pcT + 10) + 1, strlen(*(pcT + 10) + 1));
        tStrCpy(stZxTransDetail.sTranStatus, *(pcT + 11) + 1, strlen(*(pcT + 11) + 1));
        tStrCpy(stZxTransDetail.sPayBank, *(pcT + 12) + 1, strlen(*(pcT + 12) + 1));
        tStrCpy(stZxTransDetail.sAmtType, *(pcT + 13) + 1, strlen(*(pcT + 13) + 1));
        stZxTransDetail.dDmount = atof(*(pcT + 14) + 1);
        stZxTransDetail.dRepakAmt = atof(*(pcT + 15) + 1);
        tStrCpy(stZxTransDetail.sRefundNo, *(pcT + 16) + 1, strlen(*(pcT + 16) + 1));
        tStrCpy(stZxTransDetail.sMerchRefundNo, *(pcT + 17) + 1, strlen(*(pcT + 17) + 1));
        stZxTransDetail.dRefundAmt = atof(*(pcT + 18) + 1);
        stZxTransDetail.dRepakRefundAmt = atof(*(pcT + 19) + 1);
        tStrCpy(stZxTransDetail.sRefundType, *(pcT + 20) + 1, strlen(*(pcT + 20) + 1));
        tStrCpy(stZxTransDetail.sRefundStatus, *(pcT + 21) + 1, strlen(*(pcT + 21) + 1));
        tStrCpy(stZxTransDetail.sCommName, *(pcT + 22) + 1, strlen(*(pcT + 22) + 1));
        tStrCpy(stZxTransDetail.sMerchData, *(pcT + 23) + 1, strlen(*(pcT + 23) + 1));
        stZxTransDetail.dFee = atof(*(pcT + 24) + 1);
        stZxTransDetail.dRate = atof(*(pcT + 25) + 1) / 100;
        tStrCpy(stZxTransDetail.sTermType, *(pcT + 26) + 1, strlen(*(pcT + 26) + 1));
        tStrCpy(stZxTransDetail.sChkMark, *(pcT + 27) + 1, strlen(*(pcT + 27) + 1));
        tStrCpy(stZxTransDetail.sNumber, *(pcT + 28) + 1, strlen(*(pcT + 28) + 1));
        tStrCpy(stZxTransDetail.sMerchName, *(pcT + 29) + 1, strlen(*(pcT + 29) + 1));
        tStrCpy(stZxTransDetail.sMerchId, *(pcT + 30) + 1, strlen(*(pcT + 30) + 1));
        tStrCpy(stZxTransDetail.sSubMerchId, *(pcT + 31) + 1, strlen(*(pcT + 31) + 1));
        tStrCpy(stZxTransDetail.sExtend1, *(pcT + 32) + 1, strlen(*(pcT + 32) + 1));
        stZxTransDetail.dActualAmount = atof(*(pcT + 33) + 1);

        iRet = AddZxTrans(&stZxTransDetail);
        if (iRet < 0) {
            tLog(ERROR, "�����ά�������ˮ����,������[%s]", stZxTransDetail.sThOrderNo);
        }
    }
    fclose(pstFp);
    return 0;
}

/*
 * 
 */
int main(int iArgc, char *pcArgv[]) {

    int iRet = -1;
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
    tLog(INFO, "��ά������ļ��������ʼ���ɹ�.");
    iRet = ImpAcom(GetMyDate());
    if (iRet < 0) {
        tLog(ERROR, "��ά������ļ��������ݿ�ʧ��");
        return -1;
    }
    DoneApp();
    return (EXIT_SUCCESS);
}