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
#include "resp_code.h"
#include "t_cjson.h"
#include "merch.h"

/* ��ȡ */
int FindValueByKey(char *pcValue, char *pcKey) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key='%s'", pcKey);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "����Key[%s]��ֵʧ��.", pcKey);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcValue);
        tTrim(pcValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "��Key[%s]��ֵ.", pcKey);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}