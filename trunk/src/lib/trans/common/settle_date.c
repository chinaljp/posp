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
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sIsYes);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无日期[%s]记录.", pcDate);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (sIsYes[0] == '1') {
        tLog(DEBUG, "日期[%s]是结算日.", pcDate);
        return 0;
    }
    tLog(DEBUG, "日期[%s]是非结算日.", pcDate);
    return -1;
}

/* 是否结算日 出参 */
int FindCardSettleDate(char *pcDate) {
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    int i = 0;

    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    if (memcmp(sTime, "230000", 6) > 0) {
        tAddDay(sDate, 1);
        tLog(ERROR, "当前时间[%s]大于银联日切时间[230000],检查T+2日[%s]是否是结算日.", sTime, sDate);
    }
    tAddDay(sDate, 1);
    /* 节假日没有太长的，所以只检查15天的 */
    for (i = 0; i < 15; i++) {
        if (IsSettleDate(sDate) == 0) {
            tStrCpy(pcDate, sDate, 8);
            return 0;
        }
        tAddDay(sDate, 1);
    }
    tStrCpy(pcDate, sDate, 8);
    tLog(ERROR, "查找结算日失败,使用最后的日期作为结算日[%s]", pcDate);
    return -1;
}

int FindQrSettleDate(char *pcDate) {
    char sDate[8 + 1] = {0};
    int i = 0;

    tGetDate(sDate, "", -1);
    tAddDay(sDate, 1);
    /* 节假日没有太长的，所以只检查15天的 */
    for (i = 0; i < 15; i++) {
        if (IsSettleDate(sDate) == 0) {
            tStrCpy(pcDate, sDate, 8);
            return 0;
        }
        tAddDay(sDate, 1);
    }
    tStrCpy(pcDate, sDate, 8);
    tLog(ERROR, "查找结算日失败,使用最后的日期作为结算日[%s]", pcDate);
    return -1;
}