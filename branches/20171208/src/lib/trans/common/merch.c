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

int FindMerchInfoById(Merch *pstMerch, char *pcMerchId) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select merch_id,merch_name,merch_pname,status,open_date,expire_date,user_code,agent_id,merch_type,mcc "
            " from b_merch where merch_id='%s'", pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找商户[%s]的资料,失败.", pcMerchId);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstMerch->sMerchId);
        STRV(pstRes, 2, pstMerch->sCName);
        STRV(pstRes, 3, pstMerch->sPName);
        STRV(pstRes, 4, pstMerch->sStatus);
        STRV(pstRes, 5, pstMerch->sOpenDate);
        STRV(pstRes, 6, pstMerch->sExpireDate);
        STRV(pstRes, 7, pstMerch->sUserCode);
        STRV(pstRes, 8, pstMerch->sAgentId);
        STRV(pstRes, 9, pstMerch->sMerchType);
        STRV(pstRes, 10, pstMerch->sMcc);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无商户[%s]资料,请先做商户入网.", pcMerchId);
        tReleaseRes(pstRes);
        return -1;
    }

    tTrim(pstMerch->sCName);
    tTrim(pstMerch->sPName);

    tReleaseRes(pstRes);
    return 0;
}

int FindMerchAcctUuid(char *pcAccountUuid, char *pcMerchId) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select account_uuid "
            " from b_merch_account where merch_id='%s'", pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找商户[%s]的资料,失败.", pcMerchId);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcAccountUuid);

    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无商户[%s]account_uuid,请添加.", pcMerchId);
        tReleaseRes(pstRes);
        return -1;
    }
    tTrim(pcAccountUuid);
    tReleaseRes(pstRes);
    return 0;
}

int FindMerchFee(MerchFee *pstMerch, char *pcMerchId) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select USER_CODE,MERCH_ID,C_FEE_RATE,C_FEE_MAX"
            ",D_FEE_RATE,D_FEE_MAX,WX_FEE_RATE,BB_FEE_RATE"
            ",YC_FREE_FEE_RATE,YD_FREE_FEE_RATE"
            ",YC_QRCODE_FEE_RATE,YD_QRCODE_FEE_RATE"
            ",YC_NFC_FEE_RATE,YD_NFC_FEE_RATE"
            " from B_MERCH_FEE where merch_id='%s'", pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找商户[%s]的费率失败.", pcMerchId);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstMerch->sUserCode);
        STRV(pstRes, 2, pstMerch->sMerchId);
        DOUV(pstRes, 3, pstMerch->dCRate);
        DOUV(pstRes, 4, pstMerch->dCMax);
        DOUV(pstRes, 5, pstMerch->dDRate);
        DOUV(pstRes, 6, pstMerch->dDMax);
        DOUV(pstRes, 7, pstMerch->dWxRate);
        DOUV(pstRes, 8, pstMerch->dBbRate);
        DOUV(pstRes, 9, pstMerch->dYc2FeeCRate);
        DOUV(pstRes, 10, pstMerch->dYc2FeeDRate);
        DOUV(pstRes, 11, pstMerch->dYcQrCRate);
        DOUV(pstRes, 12, pstMerch->dYcQrDRate);
        DOUV(pstRes, 13, pstMerch->dYcNfcCRate);
        DOUV(pstRes, 14, pstMerch->dYcNfcDRate);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无商户[%s]费率,请先做商户入网.", pcMerchId);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}