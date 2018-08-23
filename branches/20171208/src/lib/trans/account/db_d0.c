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
#include "t0limit.h"

int FindT0Limit(char * pcTransAmt) {

    char sAmt[13] = {0}, sSqlStr[512] = {0};
    double TransAmt;
    double dAmt;
    OCI_Resultset *pstRes = NULL;
    TransAmt = atof(pcTransAmt) / 100;
    // TransAmt = atof(pcTransAmt);
    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key = 'D0_SINGLE_CASH_MIDDLE'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sAmt);
    }


    dAmt = atof(sAmt);
    if (DBL_CMP(TransAmt, dAmt)) {
        return 1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int Chksettleswitch() {

    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;
    char sState[2];
    int iState = 0;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT KEY_VALUE  FROM S_PARAM WHERE KEY='DAY_ACCOUNT_SWITCH'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return 1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sState);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }

    iState = atoi(sState);

    if (iState == 0) {

        tLog(ERROR, "日结消费已关闭");
        tReleaseRes(pstRes);
        return 1;

    }
    tReleaseRes(pstRes);
    return 0;

}

/* 查询原交易金额原卡类型,原输入方式  */
int GetAmountFee(char *pcRrn, char *pcAmt, char *pcCardType, char *pcInputMod) {
    char sRrn[12 + 1];
    double sAmt;
    char sSqlStr[512];
    char sCardType[2];
    char sInputMod[3 + 1];
    char sOamt;
    int iAmt;
    OCI_Resultset *pstRes = NULL;

    tStrCpy(sRrn, pcRrn, strlen(pcRrn));
    tLog(DEBUG, "sRrn=%s", sRrn);

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT AMOUNT,CARD_TYPE,INPUT_MODE  FROM B_POS_TRANS_DETAIL WHERE RRN = '%s' ", sRrn);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return 1;
    }
    while (OCI_FetchNext(pstRes)) {
        DOUV(pstRes, 1, sAmt);
        STRV(pstRes, 2, sCardType);
        STRV(pstRes, 3, sInputMod);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }


    /* 将浮点型金额转为分的字符串 */
    iAmt = sAmt * 100;
    sprintf(pcAmt, "%012d", iAmt);

    tTrim(sInputMod);

    pcCardType[0] = sCardType[0];
    strcpy(pcInputMod, sInputMod);
    tReleaseRes(pstRes);
    return 0;

}

int Findsettletime(char * pcTransTime, char * pcTransAmt) {

    char sTransTime[6 + 1];
    char sSettleStime[6 + 1];
    char sSettleEtime[6 + 1];
    double dMinAmt = 0, dMaxAmt = 0;
    char sSqlStr[512] = {0};
    int iStime;
    int iEtime;
    int iTtime;
    double TransAmt;
    OCI_Resultset *pstRes = NULL;
    TransAmt = atof(pcTransAmt) / 100;
    strcpy(sTransTime, pcTransTime);
    /*
     * modify by gaof 2016/12/28
     * 参数统一设置,增加最大值
     * */

    snprintf(sSqlStr, sizeof (sSqlStr), "select  LPAD(sum(starttime),6,'0'),LPAD(sum(endtime),6,'0'),sum(minamt),sum(maxamt) \
    from ( select decode(key,'SETTLE_START_TIME',key_value,'000000') as starttime, decode(key,'SETTLE_END_TIME',key_value,'0000000') as endtime \
            ,decode(key,'D0_SINGLE_CASH_MIN',key_value,'0') as minamt,decode(key,'D0_SINGLE_CASH_MAX',key_value,'0') as maxamt \
    from s_param where key in ('SETTLE_START_TIME','SETTLE_END_TIME','D0_SINGLE_CASH_MIN','D0_SINGLE_CASH_MAX'))");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, sSettleStime);
        STRV(pstRes, 2, sSettleEtime);
        DOUV(pstRes, 3, dMinAmt);
        DOUV(pstRes, 4, dMaxAmt);

        iTtime = atoi(sTransTime);
        iStime = atoi(sSettleStime);
        iEtime = atoi(sSettleEtime);
    }
    tLog(DEBUG, "sTransTime:[%d],sSettleStime:[%d],sSettleEtime:[%d],MinAmt[%f],MaxAmt[%f]",
            iTtime, iStime, iEtime, dMinAmt, dMaxAmt);

    tLog(INFO, "TransAmt[%f]", TransAmt);
    tReleaseRes(pstRes);
    if (DBL_CMP(dMinAmt, TransAmt)) {
        return 2;
    }
    if (DBL_CMP(TransAmt, dMaxAmt)) {
        return 3;
    }

    if (iTtime >= iStime && iTtime <= iEtime) {
        return 0;
    } else {
        return 1;
    }

}

int FindT0Merchlimit(T0Merchlimit * pstMerchlimit, char * pcUserCode) {


    T0Merchlimit st0Merchlimit;
    char sUserCode[15 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    memset(&st0Merchlimit, 0x00, sizeof (T0Merchlimit));
    strcpy(sUserCode, pcUserCode);
    tTrim(sUserCode);

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT USER_CODE, TOTAL_LIMIT, USED_LIMIT,USABLE_LIMIT FROM B_MERCH_AUTH_LIMIT  \
WHERE USER_CODE = '%s' FOR UPDATE", sUserCode);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return 1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, st0Merchlimit.sUserCode);
        DOUV(pstRes, 2, st0Merchlimit.dTotallimit);
        DOUV(pstRes, 3, st0Merchlimit.dUsedlimit);
        DOUV(pstRes, 4, st0Merchlimit.dUsable_limit);
    }
    tTrim(st0Merchlimit.sUserCode);
    memcpy(pstMerchlimit, &st0Merchlimit, sizeof (T0Merchlimit));
    tReleaseRes(pstRes);
    return 0;
}

/*****************************************************************************
 ** 函数: DBUptT0limit( )                                                **
 ** 类型: int                                                                **
 ** 入参：                                                                   **
 **       T0Merchlimit *psT0limit    -- T0限额额信息                   **
 ** 出参:                                                                    **
 **       NA                                                                 **
 ** 返回值：                                                                 **
 **       0      -- 更新库表成功                                             **
 **       -1     -- 更新库表失败                                             **
 ** 功能：                                                                   **
 **       根据商户编号更新指定商户的限额信息。                   **
 ** 涉及库表:                                                                **
 **       agent.authority_limit
 ** 创建日期:                                                                **
 **       2016.07.22   
 **
 *****************************************************************************/
int UptT0Limit(double dAmount, char *pcUserCode) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE  B_MERCH_AUTH_LIMIT SET USED_LIMIT = USED_LIMIT+%f "
            ",USABLE_LIMIT = USABLE_LIMIT-%f \
         WHERE USER_CODE = '%s'", dAmount, dAmount, pcUserCode);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新额度失败USER_CODE[%s].", pcUserCode);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

/*****************************************************************************
 ** 函数: DBUpT0flag( )                                                **
 ** 类型: int                                                                **
 ** 入参：                                                                   **
 **       char *psRrn    -- T0Rrn                   **
 ** 出参:                                                                    **
 **       NA                                                                 **
 ** 返回值：                                                                 **
 **       0      -- 更新库表成功                                             **
 **       -1     -- 更新库表失败                                             **
 ** 功能：                                                                   **
 **       根据rrn更新指定流水的清算算标志。                   **
 ** 涉及库表:                                                                **
 **       ACCOUNT_JOURNAL
 ** 创建日期:                                                                **
 **       2016.07.22   
 **
 *****************************************************************************/
int UpT0flag(char *pcRrn) {
    char sRrn[13];
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sRrn, pcRrn);
    tTrim(sRrn);

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL \
       SET TRANS_TYPE = '1' WHERE RESP_CODE= '00' AND SETTLE_FLAG = 'N' AND VALID_FLAG='0' AND  RRN  = '%s'", sRrn);
    tLog(INFO, "sql[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新RRN[%s]结算标志失败", sRrn);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int UpTransType(char *pcTransType, char *pcDate, char *pcRrn) {
    char sRrn[13];
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sRrn, pcRrn);
    tTrim(sRrn);

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL \
       SET TRANS_TYPE = '%s' WHERE  TRANS_DATE='%s' AND RRN  = '%s' "
            " and trans_code in (select trans_code from s_trans_code where saf_flag='1')", pcTransType, pcDate, sRrn);
    tLog(INFO, "sql[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新RRN[%s]trans_type标志失败", sRrn);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}