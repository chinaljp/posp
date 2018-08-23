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

static RespCode staRespCode[MAX_RESP_CODE];
static int g_iRespCodNum = 0;

void ErrHanding(cJSON *pstJson, char *pcRespCode, ...) {
    char sTransCode[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0};
    char *pcFmt = NULL;
    int i;
    char sError[ERR_MAX_LEN] = {0};

    if (NULL == pstJson)
    {
        return;
    }
    va_list sArgv;

    va_start(sArgv, pcRespCode);
    pcFmt = va_arg(sArgv, char*);
    if (NULL != pcFmt)
    {
        vsnprintf(sError, sizeof (sError), pcFmt, sArgv);
        /*
                if (!memcmp(sError, "00", 2)) {
                    strcpy(sError, "交易成功");
                }
         */
        SET_STR_KEY(pstJson, "resp_log", sError);
        GET_STR_KEY(pstJson, "trans_code", sTransCode);
        GET_STR_KEY(pstJson, "merch_id", sMerchId);
        GET_STR_KEY(pstJson, "rrn", sRrn);
        tLog(WARN, "%s:%s", pcRespCode, sError);
        va_end(sArgv);
        SetRespCode(pstJson, pcRespCode);
    }
}

int GetRespCode(char *pcRespDesc, char *pcRespCode) {
    int i;
    for (i = 0; i < g_iRespCodNum; i++)
    {
        if (!strcmp(staRespCode[i].sRespCode, pcRespCode))
        {
            strcpy(pcRespDesc, staRespCode[i].sRespDesc);
            return 0;
        }
    }
    strcpy(pcRespDesc, DAF_RESP_DESC);
    return 0;
}

int SetRespCode(cJSON *pstJson, char *pcRespCode) {
    int i;
    DEL_KEY(pstJson, "resp_code");
    DEL_KEY(pstJson, "resp_desc");
    for (i = 0; i < g_iRespCodNum; i++)
    {
        if (!strcmp(staRespCode[i].sRespCode, pcRespCode))
        {
            DEL_KEY(pstJson, "resp_code");
            DEL_KEY(pstJson, "resp_desc");
            SET_STR_KEY(pstJson, "resp_code", staRespCode[i].sRespCode);
            SET_STR_KEY(pstJson, "resp_desc", staRespCode[i].sRespDesc);
            return 0;
        }
    }
    /* 没有匹配的应答码,设置初始值 */
    SET_STR_KEY(pstJson, "resp_code", pcRespCode);
    SET_STR_KEY(pstJson, "resp_desc", DAF_RESP_DESC);
    return 0;
}

int LoadRespCode() {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT RESP_CODE,RESP_DESC FROM S_RESP_CODE WHERE STATUS='1'");

    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tLog(ERROR, "加载应答码失败.");
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, staRespCode[g_iRespCodNum].sRespCode);
        STRV(pstRes, 2, staRespCode[g_iRespCodNum].sRespDesc);
        tTrim(staRespCode[g_iRespCodNum].sRespCode);
        tTrim(staRespCode[g_iRespCodNum].sRespDesc);

        tLog(DEBUG, "[%d]:resp_code[%s]desc[%s].", g_iRespCodNum, staRespCode[g_iRespCodNum].sRespCode, staRespCode[g_iRespCodNum].sRespDesc);

        g_iRespCodNum++;
        if (MAX_RESP_CODE == g_iRespCodNum)
        {
            tLog(ERROR, "应答码达到最大值[%d],不再加载.", MAX_RESP_CODE);
            break;
        }
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "成功加载应答码数量[%d].", g_iRespCodNum);
    return 0;
}