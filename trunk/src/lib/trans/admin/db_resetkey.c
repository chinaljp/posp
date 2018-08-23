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
#include "t_cjson.h"
#include "t_macro.h"
#include "trans_detail.h"

int GetChannelKey(char *pcKeyAsc, char *pcKeyName) {
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT KEY_VALUE FROM S_PARAM WHERE KEY='%s'", pcKeyName);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcKeyAsc);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int UpdChannelKey(char *pcKeyAsc, char *pcKeyName) {
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE S_PARAM SET KEY_VALUE = '%s'  WHERE KEY='%s'", pcKeyAsc, pcKeyName);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新交易%s_LMK[%s]失败.", pcKeyName, pcKeyAsc);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "更新交易%s_LMK[%s]成功.", pcKeyName, pcKeyAsc);
    return 0;
}
