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
#include "t_cjson.h"
#include "trans_code.h"

static TransCode staTransCode[MAX_TRANS_CODE];
static int g_iTransCodeNum = 0;

int FindTransCode(TransCode *pstTransCode, char *pcTransCode) {
    int i;

    for (i = 0; i < g_iTransCodeNum; i++) {
        if (!strcmp(staTransCode[i].sTransCode, pcTransCode)) {
            *pstTransCode = staTransCode[i];
            return 0;
        }
    }
    tLog(ERROR, "未找到交易码[%s][%d]的定义.", pcTransCode, g_iTransCodeNum);
    return -1;
}

int LoadTransCode() {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT GROUP_CODE,TRANS_CODE,TRANS_NAME,LOG_FLAG,SETTLE_FLAG,ACCT_FLAG"
            " ,SAF_FLAG,STATUS FROM S_TRANS_CODE");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "加载交易码失败.");
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, staTransCode[g_iTransCodeNum].sGroupCode);
        STRV(pstRes, 2, staTransCode[g_iTransCodeNum].sTransCode);
        STRV(pstRes, 3, staTransCode[g_iTransCodeNum].sTransName);
        STRV(pstRes, 4, staTransCode[g_iTransCodeNum].sLogFlag);
        STRV(pstRes, 5, staTransCode[g_iTransCodeNum].sSettleFlag);
        STRV(pstRes, 6, staTransCode[g_iTransCodeNum].sAccFlag);
        STRV(pstRes, 7, staTransCode[g_iTransCodeNum].sSafFlag);
        STRV(pstRes, 8, staTransCode[g_iTransCodeNum].sStatus);
        tTrim(staTransCode[g_iTransCodeNum].sGroupCode);
        tTrim(staTransCode[g_iTransCodeNum].sTransName);
        g_iTransCodeNum++;
        if (MAX_TRANS_CODE == g_iTransCodeNum) {
            tLog(ERROR, "交易码达到最大值[%d],不再加载.", MAX_TRANS_CODE);
            break;
        }
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无交易码配置.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "成功加载交易码数量[%d].", g_iTransCodeNum);
    return 0;
}

int LoadTransCodeByCode(TransCode *pstTransCode, char *pcTransCode) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT GROUP_CODE,TRANS_CODE,TRANS_NAME,LOG_FLAG,SETTLE_FLAG,ACCT_FLAG"
            " ,SAF_FLAG,STATUS FROM S_TRANS_CODE where trans_code='%s'", pcTransCode);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "加载交易码失败.");
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstTransCode->sGroupCode);
        STRV(pstRes, 2, pstTransCode->sTransCode);
        STRV(pstRes, 3, pstTransCode->sTransName);
        STRV(pstRes, 4, pstTransCode->sLogFlag);
        STRV(pstRes, 5, pstTransCode->sSettleFlag);
        STRV(pstRes, 6, pstTransCode->sAccFlag);
        STRV(pstRes, 7, pstTransCode->sSafFlag);
        STRV(pstRes, 8, pstTransCode->sStatus);
        tTrim(pstTransCode->sGroupCode);
        tTrim(pstTransCode->sTransName);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无交易码[%s]配置.", pcTransCode);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "成功加载交易码[%s].", pcTransCode);
    return 0;
}
