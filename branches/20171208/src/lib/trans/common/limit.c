#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"
#include "limitrule.h"
#include "merchlimit.h"

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

int FindLimitRule(LimitRule *pstLimitRule, char *pcMerchId) {
    char sTrack[105] = {0};
    char sSqlStr[1024];
    int iCount = 0;
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT ID, \
USER_CODE, \
D_P_LIMIT, \
WX_LIMIT, \
BB_LIMIT, \
D_D_LIMIT, \
D_WX_LIMIT, \
D_BB_LIMIT, \
D_M_LIMIT, \
M_WX_LIMIT, \
M_BB_LIMIT, \
C_P_LIMIT, \
C_D_LIMIT, \
C_M_LIMIT, \
TRANS_CODE FROM　B_LIMIT_RULE where USER_CODE in((SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s'),'AAAAAAAAAAAAAAA') order by user_code desc ", pcMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }


    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, pstLimitRule->sId);
        STRV(pstRes, 2, pstLimitRule->sUserCode);
        DOUV(pstRes, 3, pstLimitRule->dDPLimit);
        DOUV(pstRes, 4, pstLimitRule->dWXLimit);
        DOUV(pstRes, 5, pstLimitRule->dBBLimit);
        DOUV(pstRes, 6, pstLimitRule->dDDLimit);
        DOUV(pstRes, 7, pstLimitRule->dDWXLimit);
        DOUV(pstRes, 8, pstLimitRule->dDBBLimit);
        DOUV(pstRes, 9, pstLimitRule->dDMLimit);
        DOUV(pstRes, 10, pstLimitRule->dMWXLimit);
        DOUV(pstRes, 11, pstLimitRule->dMBBLimit);
        DOUV(pstRes, 12, pstLimitRule->dCPLimit);
        DOUV(pstRes, 13, pstLimitRule->dCDLimit);
        DOUV(pstRes, 14, pstLimitRule->dCMLimit);
        STRV(pstRes, 15, pstLimitRule->sATransCode);


        tTrim(pstLimitRule->sId);
        tTrim(pstLimitRule->sUserCode);
        tTrim(pstLimitRule->sATransCode);

    }

    iCount = OCI_GetRowCount(pstRes);
    if (0 == iCount) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (iCount == 1) {
        tLog(DEBUG, "检索到默认商户限额规则信息.限制的交易码为[%s]", pstLimitRule->sATransCode);
    } else {
        tLog(DEBUG, "检索到商户[%s]限额规则信息.限制的交易码为[%s]", pcMerchId, pstLimitRule->sATransCode);
    }

    return 0;
}

int FindMerchLimit(MerchLimit *pstMerchLimit, char *pcMerchId) {
    char sTrack[105] = {0};
    char sSqlStr[1024];
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT ID, \
USER_CODE, \
D_AMT, \
D_WX_AMT, \
D_BB_AMT, \
M_AMT, \
M_WX_AMT, \
M_BB_AMT, \
C_D_AMT, \
C_M_AMT \
FROM B_MERCH_LIMIT \
where USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') FOR UPDATE ", pcMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }


    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, pstMerchLimit->sId);
        STRV(pstRes, 2, pstMerchLimit->sUserCode);
        DOUV(pstRes, 3, pstMerchLimit->dDDLimit);
        DOUV(pstRes, 4, pstMerchLimit->dDWXLimit);
        DOUV(pstRes, 5, pstMerchLimit->dDBBLimit);
        DOUV(pstRes, 6, pstMerchLimit->dDMLimit);
        DOUV(pstRes, 7, pstMerchLimit->dMWXLimit);
        DOUV(pstRes, 8, pstMerchLimit->dMBBLimit);
        DOUV(pstRes, 9, pstMerchLimit->dCDLimit);
        DOUV(pstRes, 10, pstMerchLimit->dCMLimit);

        tTrim(pstMerchLimit->sId);
        tTrim(pstMerchLimit->sUserCode);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "检索到商户[%s]累计限额信息.", pcMerchId);

    return 0;
}

int UptMerchLimitEx(double dAmount, char cType, char *pcMerchId) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    switch (cType) {
            //pos
        case 'D':
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " D_AMT= D_AMT+%f, "
                    " M_AMT = M_AMT+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
        case 'C':
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " C_D_AMT= C_D_AMT+%f, "
                    " C_M_AMT = C_M_AMT+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
        case 'B':
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " D_BB_AMT= D_BB_AMT+%f, "
                    " M_BB_AMT = M_BB_AMT+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
        case 'W':
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " D_WX_AMT= D_WX_AMT+%f, "
                    " M_WX_AMT = M_WX_AMT+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
    }
    tLog(DEBUG, "sql[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新商户[%s]限额[%f]失败.", pcMerchId, dAmount);
        return -1;
    }

    tReleaseRes(pstRes);
    tLog(INFO, "更新商户[%s]限额[%f]成功.", pcMerchId, dAmount);

    return 0;
}
