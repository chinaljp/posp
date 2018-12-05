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

//int FindLimitRule(LimitRule *pstLimitRule, char *pcMerchId) {
/*motify by GJQ at 20180305*/
int FindLimitRule(LimitRule *pstLimitRule, char *pcMerchId, char *pcMerchLevel) {
    char sTrack[105] = {0};
    char sSqlStr[1024];
    int iCount = 0;
    OCI_Resultset *pstRes = NULL;

    /*begin motify by GJQ at 20180613 */
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT ID, \
        USER_CODE, \
        IC_CARD_FLAG, \
        ID_CARD_FLAG, \
        MC_CARD_FLAG, \
        MD_CARD_FLAG, \
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
        D_P_LIMIT_M, \
        D_D_LIMIT_M, \
        D_M_LIMIT_M, \
        C_P_LIMIT_M, \
        C_D_LIMIT_M, \
        C_M_LIMIT_M, \
        DQ_LIMIT, \
        CQ_LIMIT, \
        D_DQ_LIMIT, \
        D_CQ_LIMIT, \
        Y_DQ_LIMIT, \
        Y_CQ_LIMIT, \
        TRANS_CODE FROM　B_LIMIT_RULE where USER_CODE in"
        "((SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s' AND GRADE_MERCH_LEVEL = '%s'),"
        "(SELECT USER_CODE FROM B_LIMIT_RULE WHERE USER_CODE IN ('AAAAAAAAAAAAAAA','BBBBBBBBBBBBBBB','CCCCCCCCCCCCCCC','ZZZZZZZZZZZZZZZ') AND GRADE_MERCH_LEVEL = '%s'))"
        "order by user_code desc ", pcMerchId,pcMerchLevel,pcMerchLevel);
        //"((SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s' AND GRADE_MERCH_LEVEL = '%s'),'AAAAAAAAAAAAAAA','BBBBBBBBBBBBBBB','CCCCCCCCCCCCCCC')"
        //"and GRADE_MERCH_LEVEL = '%s' order by user_code desc ", pcMerchId,pcMerchLevel,pcMerchLevel); 
    /*end motify by GJQ at 20180613 */
    
    tLog(DEBUG,"sql[%s]",sSqlStr);
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
        /*begin  motify by GJQ at 20180613*/
        STRV(pstRes, 3, pstLimitRule->sICFlag);
        STRV(pstRes, 4, pstLimitRule->sIDFlag);
        STRV(pstRes, 5, pstLimitRule->sMCFlag);
        STRV(pstRes, 6, pstLimitRule->sMDFlag); 
        /*end  motify by GJQ at 20180613*/
        DOUV(pstRes, 7, pstLimitRule->dDPLimit);
        DOUV(pstRes, 8, pstLimitRule->dWXLimit);
        DOUV(pstRes, 9, pstLimitRule->dBBLimit);
        DOUV(pstRes, 10, pstLimitRule->dDDLimit);
        DOUV(pstRes, 11, pstLimitRule->dDWXLimit);
        DOUV(pstRes, 12, pstLimitRule->dDBBLimit);
        DOUV(pstRes, 13, pstLimitRule->dDMLimit);
        DOUV(pstRes, 14, pstLimitRule->dMWXLimit);
        DOUV(pstRes, 15, pstLimitRule->dMBBLimit);
        DOUV(pstRes, 16, pstLimitRule->dCPLimit);
        DOUV(pstRes, 17, pstLimitRule->dCDLimit);
        DOUV(pstRes, 18, pstLimitRule->dCMLimit);
         /*begin  add by GJQ at 20180613*/
        DOUV(pstRes, 19, pstLimitRule->dMDPLimit);
        DOUV(pstRes, 20, pstLimitRule->dMDDLimit);
        DOUV(pstRes, 21, pstLimitRule->dMDMLimit);
        DOUV(pstRes, 22, pstLimitRule->dMCPLimit);
        DOUV(pstRes, 23, pstLimitRule->dMCDLimit);
        DOUV(pstRes, 24, pstLimitRule->dMCMLimit);
         /*end  add by GJQ at 20180613*/
        
        DOUV(pstRes, 25, pstLimitRule->dQDPLimit);
        DOUV(pstRes, 26, pstLimitRule->dQCPLimit);
        DOUV(pstRes, 27, pstLimitRule->dQDDLimit);
        DOUV(pstRes, 28, pstLimitRule->dQCDLimit);
        DOUV(pstRes, 29, pstLimitRule->dQDMLimit);
        DOUV(pstRes, 30, pstLimitRule->dQCMLimit);
        
        STRV(pstRes, 31, pstLimitRule->sATransCode);
        
        tTrim(pstLimitRule->sId);
        tTrim(pstLimitRule->sUserCode);
        tTrim(pstLimitRule->sICFlag);
        tTrim(pstLimitRule->sIDFlag);
        tTrim(pstLimitRule->sMCFlag);
        tTrim(pstLimitRule->sMDFlag);
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
C_M_AMT, \
D_AMT_M, \
M_AMT_M, \
C_D_AMT_M, \
C_M_AMT_M, \
DQ_AMT, \
DQ_AMT_M, \
CQ_AMT, \
CQ_AMT_M \
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
        /*begin  add by GJQ at 20180613*/
        DOUV(pstRes, 11, pstMerchLimit->dMDDLimit);
        DOUV(pstRes, 12, pstMerchLimit->dMDMLimit);
        DOUV(pstRes, 13, pstMerchLimit->dMCDLimit);
        DOUV(pstRes, 14, pstMerchLimit->dMCMLimit);
         /*end  add by GJQ at 20180613*/
        DOUV(pstRes, 15, pstMerchLimit->dQDDLimit);
        DOUV(pstRes, 16, pstMerchLimit->dQDMLimit);
        DOUV(pstRes, 17, pstMerchLimit->dQCDLimit);
        DOUV(pstRes, 18, pstMerchLimit->dQCMLimit);
        
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
        case 'E':
            tLog(DEBUG,"cType[%c]",cType);
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " D_AMT_M = D_AMT_M+%f, "
                    " M_AMT_M = M_AMT_M+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
        case 'F':
            tLog(DEBUG,"cType[%c]",cType);
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " C_D_AMT_M = C_D_AMT_M+%f, "
                    " C_M_AMT_M = C_M_AMT_M+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
        case 'D':
            tLog(DEBUG,"cType[%c]",cType);
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " D_AMT = D_AMT+%f, "
                    " M_AMT = M_AMT+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
        case 'C':
            tLog(DEBUG,"cType[%c]",cType);
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " C_D_AMT = C_D_AMT+%f, "
                    " C_M_AMT = C_M_AMT+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
        case 'P':
            tLog(DEBUG,"cType[%c]",cType);
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " DQ_AMT = DQ_AMT+%f, "
                    " DQ_AMT_M = DQ_AMT_M+%f "
                    " WHERE USER_CODE=(SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s') "
                    , dAmount, dAmount, pcMerchId);
            break;
        case 'Q':
            tLog(DEBUG,"cType[%c]",cType);
            snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_LIMIT SET "
                    " CQ_AMT = CQ_AMT+%f, "
                    " CQ_AMT_M = CQ_AMT_M+%f "
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
