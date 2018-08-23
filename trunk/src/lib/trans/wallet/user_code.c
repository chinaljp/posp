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

int FindAcctIdByUserCode(char *pcAcctId, char *pcUserCode) {
    char sSqlStr[1024] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    snprintf(sSqlStr, sizeof (sSqlStr), "select  account_uuid from B_MERCH_ACCOUNT "
            "where user_code='%s'", pcUserCode);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcAcctId);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "无商户[%s]的账户id.", pcUserCode);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int FindAcctIdByMerchId(char *pcAcctId, char *pcMerchId) {
    char sSqlStr[1024] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    /*snprintf(sSqlStr, sizeof (sSqlStr), "select account_uuid from B_MERCH_ACCOUNT "
            " where merch_id='%s'", pcMerchId);
    */
    snprintf(sSqlStr, sizeof (sSqlStr), "select account_uuid "
            " from b_merch_account where merch_id= :merch_id");
    
    if (tExecutePre(&pstRes, sSqlStr, 0, 0, 1, pcMerchId, ":merch_id") < 0) {
        tLog(ERROR, "查找商户[%s]的资料,失败.", pcMerchId);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcAcctId);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "无商户[%s]的账户id.", pcMerchId);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tTrim(pcAcctId);
    if (pcAcctId[0] == '\0') {
        tLog(WARN, "商户[%s]的账户id是null.", pcMerchId);
        return -1;
    }
    return 0;
}

int FindAcctIdByAgentId(char *pcAcctId, char *pcAgentId) {
    char sSqlStr[1024] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    snprintf(sSqlStr, sizeof (sSqlStr), "select account_uuid from B_AGENT "
            " where agent_id='%s'", pcAgentId);
    tLog(DEBUG, "[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcAcctId);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "无代理商[%s]的账户id.", pcAgentId);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tTrim(pcAcctId);
    if (pcAcctId[0] == '\0') {
        tLog(WARN, "代理商[%s]的账户id是null.", pcAgentId);
        return -1;
    }
    return 0;
}
int UserCode2Acctuuid(cJSON *pstJson, int *piFlag) {
    char sUuid[64] = {0}, sUserCode[15 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    if (sUserCode[0] == '\0') {
        return 0;
    }
    tLog(DEBUG, "user_code=%s", sUserCode);
    if (FindAcctIdByUserCode(sUuid, sUserCode) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的账户uuid.", sUserCode);
        return -1;
    }
    SET_STR_KEY(pstTransJson, "accountUuid", sUuid);
    tLog(INFO, "商户[%s]uuid[%s].", sUserCode, sUuid);
    return 0;
}

int MerchId2Acctuuid(cJSON *pstJson, int *piFlag) {
    char sUuid[64] = {0}, sMerchId[15 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merchantNo", sMerchId);
    GET_STR_KEY(pstTransJson, "accountUuid", sUuid);
    if (sUuid[0] != '\0') {
        return 0;
    }
    if (FindAcctIdByMerchId(sUuid, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的账户uuid.", sMerchId);
        return -1;
    }
    //strcpy(sUuid, "fdb446bd-baba-43b8-8e87-00dee0d76831");
    SET_STR_KEY(pstTransJson, "accountUuid", sUuid);
    tLog(INFO, "商户[%s]uuid[%s].", sMerchId, sUuid);
    return 0;
}
int AgentId2Acctuuid(cJSON *pstJson, int *piFlag) {
    char sUuid[64] = {0}, sAgentId[8 + 1] = {0};
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "agentNo", sAgentId);
    GET_STR_KEY(pstTransJson, "accountUuid", sUuid);
    if (sUuid[0] != '\0') {
        return 0;
    }
    if (FindAcctIdByAgentId(sUuid, sAgentId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到代理商[%s]的账户uuid.", sAgentId);
        return -1;
    }
    SET_STR_KEY(pstTransJson, "accountUuid", sUuid);
    tLog(INFO, "代理商[%s]uuid[%s].", sAgentId, sUuid);
    return 0;
}
