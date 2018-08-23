/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"

int GetSysTrace(char *pcTrace) {
    char sSqlStr[512] = {0}, sTmp[6 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "Select SEQ_TRACE.nextval  from dual");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "获取系统流水号失败.");
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sTmp);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录[%s].", sSqlStr);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    sprintf(pcTrace, "%06ld", atol(sTmp));
    tLog(DEBUG, "当前流水号[%s].", pcTrace);
    return 0;
}