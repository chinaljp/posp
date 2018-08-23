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
#include "limitrule.h"

int FindChannelGroupId(char * pcMerchId, char * pcGroupId) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select group_id from b_channel_merch_conv where merch_id='%s' and status = '1'"
            , pcMerchId);

    //tLog(DEBUG, "%s", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcGroupId);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int FindChannelRule(PufaLimitRule * pstPufaLimitRule, char * pcChanneMerchId) {
    PufaLimitRule stPufaLimitRule;
    char sChannelMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sChannelMerchId, pcChanneMerchId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select channel_merch,channel_merch_name,total_limit,used_limit,usable_limit,trans_code,channel_id"
            " from b_channel_merch WHERE channel_merch = '%s' and status='1' for update", sChannelMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, stPufaLimitRule.sMerchId);
        STRV(pstRes, 2, stPufaLimitRule.sMerchName);
        DOUV(pstRes, 3, stPufaLimitRule.dTotallimit);
        DOUV(pstRes, 4, stPufaLimitRule.dUsedlimit);
        DOUV(pstRes, 5, stPufaLimitRule.dUsable_limit);
        STRV(pstRes, 6, stPufaLimitRule.sATransCode);
        STRV(pstRes, 7, stPufaLimitRule.sChannelId);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }

    memcpy(pstPufaLimitRule, &stPufaLimitRule, sizeof (PufaLimitRule));
    tReleaseRes(pstRes);

    return 0;
}

int FindChannelRuleNoUp(PufaLimitRule * pstPufaLimitRule, char * pcChanneMerchId) {
    PufaLimitRule stPufaLimitRule;
    char sChannelMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sChannelMerchId, pcChanneMerchId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select channel_merch,channel_merch_name,total_limit,used_limit,usable_limit,trans_code,channel_id"
            " from b_channel_merch WHERE channel_merch = '%s' and status='1'", sChannelMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, stPufaLimitRule.sMerchId);
        STRV(pstRes, 2, stPufaLimitRule.sMerchName);
        DOUV(pstRes, 3, stPufaLimitRule.dTotallimit);
        DOUV(pstRes, 4, stPufaLimitRule.dUsedlimit);
        DOUV(pstRes, 5, stPufaLimitRule.dUsable_limit);
        STRV(pstRes, 6, stPufaLimitRule.sATransCode);
        STRV(pstRes, 7, stPufaLimitRule.sChannelId);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }

    memcpy(pstPufaLimitRule, &stPufaLimitRule, sizeof (PufaLimitRule));
    tReleaseRes(pstRes);

    return 0;
}


int FindMerchLimitRule(PufaLimitRule * pstPufaLimitRule, char * pcMerchId) {

    char sMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sMerchId, pcMerchId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select merch_id,total_limit,used_limit,trans_cnt,group_id,pass_amt,card_type,merch_type "
            " from b_channel_merch_conv WHERE merch_id = '%s' and status='1' for update", sMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstPufaLimitRule->sLocalMerch);
        DOUV(pstRes, 2, pstPufaLimitRule->dLocalTotallimit);
        DOUV(pstRes, 3, pstPufaLimitRule->dLocalUsedlimit);
        INTV(pstRes, 4, pstPufaLimitRule->iLocalTransCnt);
        STRV(pstRes, 5, pstPufaLimitRule->sLocalGroupId);
        DOUV(pstRes, 6, pstPufaLimitRule->dLocalPassAmt);
        STRV(pstRes, 7, pstPufaLimitRule->sCardType);
        STRV(pstRes, 8, pstPufaLimitRule->sMerchType);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    return 0;

}


int FindMerchLimitRuleNoUp(PufaLimitRule * pstPufaLimitRule, char * pcMerchId) {

    char sMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sMerchId, pcMerchId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select merch_id,total_limit,used_limit,trans_cnt,group_id,pass_amt,card_type,merch_type "
            " from b_channel_merch_conv WHERE merch_id = '%s' and status='1'", sMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstPufaLimitRule->sLocalMerch);
        DOUV(pstRes, 2, pstPufaLimitRule->dLocalTotallimit);
        DOUV(pstRes, 3, pstPufaLimitRule->dLocalUsedlimit);
        INTV(pstRes, 4, pstPufaLimitRule->iLocalTransCnt);
        STRV(pstRes, 5, pstPufaLimitRule->sLocalGroupId);
        DOUV(pstRes, 6, pstPufaLimitRule->dLocalPassAmt);
        STRV(pstRes, 7, pstPufaLimitRule->sCardType);
        STRV(pstRes, 8, pstPufaLimitRule->sMerchType);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    return 0;

}

int FindLocalMerchTransCnt(char * pcMerchId, char * pcChannelId) {
    char sMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    int iCnt = 0;

    strcpy(sMerchId, pcMerchId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from b_pos_trans_detail  where  merch_id = '%s' and resp_code='00' and valid_flag='0' and channel_id in ('49000000','48560000') and trans_code in ('020000','T20000')", sMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iCnt);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    return iCnt;
}

int UptChannelMerchLimitEx(double dTranAmt, PufaLimitRule * pstPufaLimitRule) {

    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_CHANNEL_MERCH SET "
            " USED_LIMIT = USED_LIMIT + %f, "
            " USABLE_LIMIT = USABLE_LIMIT - %f "
            " WHERE CHANNEL_MERCH='%s'"
            , dTranAmt, dTranAmt, pstPufaLimitRule->sMerchId);

    tLog(DEBUG, "sql[%s]", sSqlStr);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新商户[%s]限额失败.金额[%f]", pstPufaLimitRule->sMerchId, dTranAmt);
        return -1;
    }

    tReleaseRes(pstRes);

    tLog(INFO, "更新商户[%s]限额成功.金额[%f]", pstPufaLimitRule->sMerchId, dTranAmt);

    return 0;
}

int UptChannelLocalMerchLimitEx(double dTranAmt, PufaLimitRule * pstPufaLimitRule) {

    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_CHANNEL_MERCH_CONV SET "
            " USED_LIMIT = USED_LIMIT + %f "
            " WHERE MERCH_ID='%s'"
            , dTranAmt, pstPufaLimitRule->sLocalMerch);

    tLog(DEBUG, "sql[%s]", sSqlStr);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新商户[%s]限额失败.金额[%f]", pstPufaLimitRule->sLocalMerch, dTranAmt);
        return -1;
    }

    tReleaseRes(pstRes);

    tLog(INFO, "更新商户[%s]限额成功.金额[%f]", pstPufaLimitRule->sLocalMerch, dTranAmt);

    return 0;
}

int UpdatePufaChannelId(char * pcMerchId, char * pcTraceNo, char * pcTransTime, char * pcChannelId, char * pcChannelMerchId, char * pcChannelTermId) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL SET "
            " CHANNEL_ID = '%s',CHANNEL_MERCH_ID='%s',CHANNEL_TERM_ID='%s' "
            " WHERE MERCH_ID='%s' AND TRACE_NO = '%s' AND TRANS_TIME = '%s'"
            , pcChannelId, pcChannelMerchId, pcChannelTermId, pcMerchId, pcTraceNo, pcTransTime);

    tLog(DEBUG, "sql[%s]", sSqlStr);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新渠道ID失败.");
        return -1;
    }

    tReleaseRes(pstRes);

    tLog(INFO, "更新交易流水渠道ID成功");

    return 0;

}

int ChkTransContol(char * pcMerchId) {
    char sMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0}, sTransControl[1 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sMerchId, pcMerchId);
    snprintf(sSqlStr, sizeof (sSqlStr), "select trans_control from b_channel_merch_conv  where  merch_id = '%s'", sMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sTransControl);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (sTransControl[0] == '1') {
        return 0;
    } else {
        return -1;
    }
}

int FindChannelMerchId(char * pcGroupId, char * pcChannelMerchId, char * pcChannelTermId, char *pcChannelMerchName, char * pcChannelId) {

    char sChannelMerchId[15 + 1] = {0}, sChannelTermId[8 + 1] = {0}, sChannelMerchName[100] = {0}, sGroupId[8 + 1] = {0};
    char sSqlStr[512] = {0}, sChannelId[8 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    int iCnt = 0;

    strcpy(sGroupId, pcGroupId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select a.channel_merch,b.channel_term,a.channel_merch_name,a.channel_id from b_channel_merch a "
            " join b_channel_term b on a.channel_merch = b.channel_merch "
            " where instr(a.group_id,'%s') != 0 and b.status='1' and a.status='1' order by a.usable_limit desc", pcGroupId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sChannelMerchId);
        STRV(pstRes, 2, sChannelTermId);
        STRV(pstRes, 3, sChannelMerchName);
        STRV(pstRes, 4, sChannelId);
        break;
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }

    strcpy(pcChannelMerchId, sChannelMerchId);
    strcpy(pcChannelTermId, sChannelTermId);
    strcpy(pcChannelMerchName, sChannelMerchName);
    strcpy(pcChannelId, sChannelId);

    tReleaseRes(pstRes);
    return 0;
}