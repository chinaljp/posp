/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"

int IsSettleDate(char *pcDate) {
    char sSqlStr[512] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT ISWORKINGDAY FROM S_HOLIDAY WHERE DAYS='%s'", pcDate);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sIsYes);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "������[%s]��¼.", pcDate);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (sIsYes[0] == '1') {
        tLog(DEBUG, "����[%s]�ǽ�����.", pcDate);
        return 0;
    }
    tLog(DEBUG, "����[%s]�Ƿǽ�����.", pcDate);
    return -1;
}

/* �Ƿ������ ���� */
int FindCardSettleDate(char *pcDate) {
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    int i = 0;
    
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
        if (memcmp(sTime, "230000", 6) > 0) {
            tAddDay(sDate, 1);
            tLog(ERROR, "��ǰʱ��[%s]������������ʱ��[230000],���T+2��[%s]�Ƿ��ǽ�����.", sTime, sDate);
        }
    tAddDay(sDate, 1);
    /* �ڼ���û��̫���ģ�����ֻ���15��� */
    for (i = 0; i < 15; i++) {
        if (IsSettleDate(sDate) == 0) {
            tStrCpy(pcDate, sDate, 8);
            return 0;
        }
        tAddDay(sDate, 1);
    }
    tStrCpy(pcDate, sDate, 8);
    tLog(ERROR, "���ҽ�����ʧ��,ʹ������������Ϊ������[%s]", pcDate);
    return -1;
}

int FindQrSettleDate(char *pcDate) {
    char sDate[8 + 1] = {0};
    int i = 0;

    tGetDate(sDate, "", -1);
    tAddDay(sDate, 1);
    /* �ڼ���û��̫���ģ�����ֻ���15��� */
    for (i = 0; i < 15; i++) {
        if (IsSettleDate(sDate) == 0) {
            tStrCpy(pcDate, sDate, 8);
            return 0;
        }
        tAddDay(sDate, 1);
    }
    tStrCpy(pcDate, sDate, 8);
    tLog(ERROR, "���ҽ�����ʧ��,ʹ������������Ϊ������[%s]", pcDate);
    return -1;
}



/* �Ƿ������ ���� */
int GetSysSettleDate(char *pcDate, char * pcChannelSettleDate) {
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    int i = 0;
    
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    
    if (strlen(pcChannelSettleDate) == 0) {
        tLog(ERROR,"��ȡ����������Ϊ��");
        if (memcmp(sTime, "230000", 6) > 0) {
            tAddDay(sDate, 1);
            tLog(ERROR, "��ǰʱ��[%s]������������ʱ��[230000],���T+2��[%s]�Ƿ��ǽ�����.", sTime, sDate);
        }
    } else {

        if ((memcmp(sDate+4, "1231", 4) == 0)&&(memcmp(pcChannelSettleDate, "0101", 4) == 0)) {
            tAddDay(sDate, 1); //��ĩ���һ�죬��һ������һ���1��1��
        } else {
            //��ȫ�������
            strncpy(sDate + 4, pcChannelSettleDate, 4);
        }
    }
    tLog(INFO, "�õ�������������[%s]", sDate);
    tAddDay(sDate, 1);
    tLog(INFO,"������Ϊ[%s]",sDate);
    /* �ڼ���û��̫���ģ�����ֻ���15��� */
    for (i = 0; i < 15; i++) {
        if (IsSettleDate(sDate) == 0) {
            tStrCpy(pcDate, sDate, 8);
            return 0;
        }
        tAddDay(sDate, 1);
    }
    tStrCpy(pcDate, sDate, 8);
    tLog(ERROR, "���ҽ�����ʧ��,ʹ������������Ϊ������[%s]", pcDate);
    return -1;
}