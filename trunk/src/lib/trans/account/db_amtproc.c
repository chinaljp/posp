#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "limitrule.h"
#include "merchlimit.h"
#include "t_cjson.h"

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#if 0
int FindLimitRule(LimitRule *pstLimitRule, char *pcMerchId) {
    char sTrack[105] = {0};
    char sSqlStr[1024];
    int  iCount = 0;
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT ID, \
USER_CODE, \
P_LIMIT, \
WX_LIMIT, \
BB_LIMIT, \
D_LIMIT, \
D_WX_LIMIT, \
D_BB_LIMIT, \
M_LIMIT, \
M_WX_LIMIT, \
M_BB_LIMIT, \
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
        DOUV(pstRes, 3, pstLimitRule->dPLimit);
        DOUV(pstRes, 4, pstLimitRule->dWXLimit);
        DOUV(pstRes, 5, pstLimitRule->dBBLimit);
        DOUV(pstRes, 6, pstLimitRule->dDLimit);
        DOUV(pstRes, 7, pstLimitRule->dDWXLimit);
        DOUV(pstRes, 8, pstLimitRule->dDBBLimit);
        DOUV(pstRes, 9, pstLimitRule->dMLimit);
        DOUV(pstRes, 10, pstLimitRule->dMWXLimit);
        DOUV(pstRes, 11, pstLimitRule->dMBBLimit);
        STRV(pstRes, 12, pstLimitRule->sATransCode);


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
    if(iCount == 1){
        tLog(INFO, "检索到默认商户限额规则信息.限制的交易码为[%s]", pstLimitRule->sATransCode);
    } else{
        tLog(INFO, "检索到商户[%s]限额规则信息.限制的交易码为[%s]", pcMerchId, pstLimitRule->sATransCode);
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
M_BB_AMT \
FROM B_MERCH_LIMIT \
where USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') ", pcMerchId);
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
        DOUV(pstRes, 3, pstMerchLimit->dDLimit);
        DOUV(pstRes, 4, pstMerchLimit->dDWXLimit);
        DOUV(pstRes, 5, pstMerchLimit->dDBBLimit);
        DOUV(pstRes, 6, pstMerchLimit->dMLimit);
        DOUV(pstRes, 7, pstMerchLimit->dMWXLimit);
        DOUV(pstRes, 8, pstMerchLimit->dMBBLimit);

        tTrim(pstMerchLimit->sId);
        tTrim(pstMerchLimit->sUserCode);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "检索到商户[%s]累计限额信息.", pcMerchId);

    return 0;
}
#endif
int UptMerchLimit(cJSON *pstMerchLimitJson, char *pcMerchId, char *pcTransCode) {
    double dDLimit = 0L, dMLimit = 0L;
    double dDWXLimit = 0L, dMWXLimit = 0L, dDBBLimit = 0L, dMBBLimit = 0L;
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;

    GET_DOU_KEY(pstMerchLimitJson, "d_limit", dDLimit);
    GET_DOU_KEY(pstMerchLimitJson, "m_limit", dMLimit);
    GET_DOU_KEY(pstMerchLimitJson, "d_wx_limit", dDWXLimit);
    GET_DOU_KEY(pstMerchLimitJson, "m_wx_limit", dMWXLimit);
    GET_DOU_KEY(pstMerchLimitJson, "d_bb_limit", dDBBLimit);
    GET_DOU_KEY(pstMerchLimitJson, "m_bb_limit", dMBBLimit);
    if (NULL != strstr("02B100,02B200", pcTransCode)) {
        snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                " D_BB_AMT= '%lf', "
                " M_BB_AMT = '%lf' "
                " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                , dDBBLimit, dMBBLimit, pcMerchId);
    }
    else if (NULL != strstr("02W100,02W200", pcTransCode)) {
        snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                " D_WX_AMT= '%lf', "
                " M_WX_AMT = '%lf' "
                " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                , dDWXLimit, dMWXLimit, pcMerchId);
    }
    else {
        snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                " D_AMT= '%lf', "
                " M_AMT = '%lf' "
                " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                , dDLimit, dMLimit, pcMerchId);
    }
    if (strlen(sSqlStr) == 0) {
        tLog(INFO, "无需更新商户[%s]交易[%s]限额.", pcMerchId, pcTransCode);
        return 0;
    }
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新商户[%s]限额失败.", pcMerchId);
        return -1;
    }

    tReleaseRes(pstRes);
    tLog(INFO, "更新商户[%s]限额成功.", pcMerchId);

    return 0;
}