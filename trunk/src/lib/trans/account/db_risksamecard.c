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
#include "card.h"
#include "t_cjson.h"

int FindSameCardRule(SameCardRule * pstSameCardRule) {

    char sSqlStr[1024] = {0}, sCnt[2] = {0};
    OCI_Resultset *pstRes = NULL;
    SameCardRule stSameCardRule;
    memset(&stSameCardRule, 0x00, sizeof (SameCardRule));
    memset(sSqlStr, 0, sizeof (sSqlStr));

    snprintf(sSqlStr, sizeof (sSqlStr), "select  sum(cnt),sum(amount),sum(querycnt),sum(querycardcnt),sum(querynmintrans), \
    sum(queryntrans) FROM (select \
    decode(KEY,'CNT',KEY_VALUE,'') as cnt, \
    decode(KEY,'AMOUNT',KEY_VALUE,'') as AMOUNT, \
    decode(KEY,'QUERY_CNT',KEY_VALUE,'') as querycnt, \
    decode(KEY,'QUERY_CARD_CNT',KEY_VALUE,'') as querycardcnt, \
    decode(KEY,'QUERY_N_MIN_TRANS',KEY_VALUE,'') as querynmintrans, \
    decode(KEY,'QUERY_N_TRANS',KEY_VALUE,'') as queryntrans \
    from S_PARAM where KEY in ('CNT','AMOUNT','QUERY_CNT','QUERY_CARD_CNT','QUERY_N_MIN_TRANS','QUERY_N_TRANS'))");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        tReleaseRes(pstRes);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        INTV(pstRes, 1, stSameCardRule.lCnt);
        DOUV(pstRes, 2, stSameCardRule.dAmount);
        INTV(pstRes, 3, stSameCardRule.lQueryCnt);
        INTV(pstRes, 4, stSameCardRule.lQueryCardCnt);
        INTV(pstRes, 5, stSameCardRule.lQUERYNMINTRANS);
        INTV(pstRes, 6, stSameCardRule.lQUERYNTRANS);

    }

    *pstSameCardRule = stSameCardRule;
    tReleaseRes(pstRes);
    tLog(INFO, "同卡连刷规则:连刷次数[%ld]，金额[%.02f]，商户查余次数[%ld],商户同卡查余次数[%ld],同卡查余[%ld]次后做交易,同卡查余后[%ld]分钟内做交易",
            stSameCardRule.lCnt, stSameCardRule.dAmount, stSameCardRule.lQueryCnt, stSameCardRule.lQueryCardCnt,
            stSameCardRule.lQUERYNTRANS, stSameCardRule.lQUERYNMINTRANS);
    return 0;
}

int FindLifeflag(char * psUserId) {
    char sUserId[16] = {0};
    int sNum = 0;
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;

    strcpy(sUserId, psUserId);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_LIFE_MERCH_CONV WHERE STATUS='1' AND USER_ID='%s'", sUserId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        tReleaseRes(pstRes);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        INTV(pstRes, 1, sNum);
    }
    tReleaseRes(pstRes);
    if (sNum == 0) {
        tLog(DEBUG, "商户没开通生活圈功能.");
        return 0;
    } else {
        tLog(DEBUG, "商户已开通生活圈功能.");
        return 1;
    }
}

int tQueryCount(char *pcSqlStr) {
    int iCounts;
    char *pcSql;
    pcSql = pcSqlStr;
    OCI_Resultset *pstRes = NULL;

    if (tExecute(&pstRes, pcSql) < 0) {
        tLog(ERROR, "sql[%s] err!", pcSql);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", pcSql);
        tReleaseRes(pstRes);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iCounts);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        iCounts = 0;
    }
    tReleaseRes(pstRes);
    return ( iCounts);
}

int ChkWhiteCard(char * psUserId) {
    char sUserId[16] = {0};
    int sNum = 0;
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;

    strcpy(sUserId, psUserId);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM b_merch_card_cont WHERE STATUS='1' AND USER_code='%s'", sUserId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        tReleaseRes(pstRes);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        INTV(pstRes, 1, sNum);
    }
    tReleaseRes(pstRes);
    if (sNum == 0) {
        tLog(DEBUG, "商户不在同卡连刷白名单.");
        return 0;
    } else {
        tLog(DEBUG, "商户是同卡连刷白名单.");
        return 1;
    }
}

int InstRiskMerch(cJSON *pstNetTran, char * pcDesc, char *pcCode) {

    char sSqlStr[512] = {0}, sMerchId[15 + 1] = {0};
    char sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0};
    char sRrn[12 + 1] = {0}, sAmount[12 + 1] = {0}, sMerchName[60 + 1] = {0};
    char sCardNoEnc[255 + 1] = {0}, sKeyName[255 + 1] = {0};
    double dAmount = 0;
    OCI_Resultset *pstRes = NULL;
    GET_STR_KEY(pstNetTran, "merch_id", sMerchId);
    GET_STR_KEY(pstNetTran, "trans_code", sTransCode);
    GET_STR_KEY(pstNetTran, "card_no", sCardNo);
    GET_STR_KEY(pstNetTran, "trans_date", sTransDate);
    GET_STR_KEY(pstNetTran, "trans_time", sTransTime);
    GET_STR_KEY(pstNetTran, "rrn", sRrn);
    GET_STR_KEY(pstNetTran, "merch_name", sMerchName);
    GET_DOU_KEY(pstNetTran, "amount", dAmount);
#if 0

    /* kms 加密存储 ,原字段屏蔽显示 */
    if (FindValueByKey(sKeyName, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
    } else {
        if (sCardNo[0] != '\0') {
            if (tHsm_Enc_Asc_Data(sCardNoEnc, sKeyName, sCardNo) < 0)
                tLog(ERROR, "卡号加密失败.");
            else
                tLog(DEBUG, "卡号加密成功.");
        }
    }
    if (sCardNo[0] != '\0')
        tScreenStr(sCardNo, strlen(sCardNo), '*', 6, 4);

#endif
    sprintf(sAmount, "%.2f", dAmount / 100);

    snprintf(sSqlStr, sizeof (sSqlStr), "insert into b_risk_merch (TRANS_CODE,TRANS_DATE,TRANS_TIME,CARD_NO, \
                      AMOUNT,MERCH_ID,MERCH_NAME,RRN,DESCRIBE,RULE_CODE ) VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')", \
                 sTransCode, sTransDate, sTransTime, sCardNo, sAmount, sMerchId, sMerchName, sRrn, pcDesc, pcCode);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tLog(INFO, "风险商户预警记录添加成功");
    tReleaseRes(pstRes);
    return 0;
}

int GetQueryTime(char * pcMerchId, char * pcCardNo, char * pcQueryTime) {
    char sSqlStr[512], sQueryTime[6 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT TRANS_TIME FROM B_POS_TRANS_DETAIL WHERE \
    MERCH_ID='%s' AND CARD_NO_ENCRYPT='%s' AND TRANS_CODE='021000' AND RESP_CODE='00' ORDER BY TRANS_TIME ASC ", pcMerchId, pcCardNo);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        tReleaseRes(pstRes);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sQueryTime);
    }
    strcpy(pcQueryTime, sQueryTime);
    tReleaseRes(pstRes);

    return 0;
}