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
#include "merch.h"
#include "term.h"

int FindTermInfoById(Terminal *pcTerminal, char *pcMerchId, char *pcTermId) {
    char sSqlStr[256] = {0}, sStatus[2] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "select a.STATUS,a.tmk from B_TERM a, B_MERCH b "
            " where a.USER_CODE = b.USER_CODE AND A.TERM_ID='%s' AND B.MERCH_ID='%s' ", pcTermId, pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找终端[%s:%s]的资料,失败.", pcMerchId, pcTermId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcTerminal->sStatus);
        STRV(pstRes, 2, pcTerminal->sTmk);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无终端[%s:%s]资料,请先申领终端.", pcMerchId, pcTermId);
        tReleaseRes(pstRes);
        return -1;
    }

    tReleaseRes(pstRes);
    return 0;
}

int FindTermIdBySn(char *pcMerchId, char *pcTermId, char *pcTermVender, char *pcTermSN) {
    char sMerchId[15 + 1];
    char sTermId[8 + 1];
    char cBindFlag;
    char cInitFlag;
    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select B_MERCH_MERCH_ID, B_DEV_TERM_TERM_ID from V_TERM \
     WHERE B_DEV_TERM_SN = '%s'and b_dev_term_status='1' and b_term_status='1'", pcTermSN);
    tLog(DEBUG, "sql[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sMerchId);
        STRV(pstRes, 2, sTermId);

        tTrim(sMerchId);
        tTrim(sTermId);

        strcpy(pcMerchId, sMerchId);
        strcpy(pcTermId, sTermId);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到序列号[%s]的终端.", pcTermSN);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int FindTermParam(char *pcMerchName, char *pcAdvertise, char *pcMerchId, char *pcTermId) {
    char sAdvertise[121];
    char sMerchName[41];
    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    MEMSET(sAdvertise);
    MEMSET(sMerchName);

    snprintf(sSqlStr, sizeof (sSqlStr), "select MERCH_NAME from  B_MERCH where MERCH_ID='%s' ", pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, sMerchName);
        //        STRV(pstRes, 2, sAdvertise);
        tTrim(sMerchName);
        //       tTrim(sAdvertise);
        strcpy(pcMerchName, sMerchName);
        //        strcpy(pcAdvertise, sAdvertise);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        //tLog(ERROR, "未找到序列号[%s]的终端.", pcTermSN);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;

}

int UpdTermKey(char *psTmkuLmk, char * psTpkuTmk, char *psTakuTmk, char *psTdkuTmk, char *pstMerchId, char *pstTermId) {

    char sTmk[32 + 1] = {0}, sTpk[32 + 1] = {0}, sTak[32 + 1] = {0}, sTdk[32 + 1] = {0}, sTermId[9] = {0};
    char sSqlStr[256];
    strcpy(sTmk, psTmkuLmk);
    strcpy(sTpk, psTpkuTmk);
    strcpy(sTak, psTakuTmk);
    strcpy(sTdk, psTdkuTmk);
    strcpy(sTermId, pstTermId);
    tLog(INFO, "更新终端[%s]密钥.", sTermId);
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "update b_term set TMK='%s',MAC_KEY='%s',PIN_KEY='%s',TRACK_KEY='%s' WHERE TERM_ID='%s'", sTmk, sTak, sTpk, sTdk, sTermId);
    tLog(INFO, "更新终端[%s]密钥.", sTermId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tLog(INFO, "更新终端[%s]密钥.", sTermId);
    tReleaseRes(pstRes);

    return 0;
}