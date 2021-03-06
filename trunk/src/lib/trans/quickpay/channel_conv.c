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


int IsAgentLife(char *pcAgentId) {
    char sSqlStr[512] = {0}, sStatus[2] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT STATUS FROM B_LIFE_CTRL WHERE OBJECT_ID='%s'", pcAgentId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sStatus);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (sStatus[0] == '1') {
        return 1;
    }
    return 0;
}

int FindLifeMerch(char *pcLifeMerchId, char *pcLifeMerchName, char *pcUserCode, char *pcAgentId,int * piUseCnt) {
    char sSqlStr[512] = {0}, sTmp[1024] = {0};
    char sIsYes[2] = {0};
    int iUseCnt = 0;
    OCI_Resultset *pstRes = NULL;

    /* 判断总开关 */
    if (FindValueByKey(sTmp, "LIFE_AGENT_SWITCH") < 0) {
        tLog(ERROR, "查找key[LIFE_AGENT_SWITCH]生活圈总开关,失败.");
        return -1;
    }
    if (sTmp[0] == '0') {
        tLog(ERROR, "生活圈总开关状态[%s].", sTmp[0] == '0' ? "关闭" : "开通");
        return -1;
    }

    /* 商户直属代理商开关 */
    if (IsAgentLife(pcAgentId) <= 0) {
        tLog(ERROR, "代理商[%s]未开通生活圈.", pcAgentId);
        return -1;
    }

    snprintf(sSqlStr, sizeof (sSqlStr), "select mc.b_life_merch_id,lm.merch_name,lm.use_cnt"
            " from B_LIFE_MERCH_CONV mc "
            " join B_LIFE_MERCH lm on lm.MERCH_ID=mc.b_life_merch_id "
            " WHERE mc.user_id='%s' and mc.status='1'"
            , pcUserCode);

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
        STRV(pstRes, 1, pcLifeMerchId);
        STRV(pstRes, 2, pcLifeMerchName);
        INTV(pstRes, 3, iUseCnt);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    /* 如果生活圈商户取消不选，数据库里的商户号和终端号会被清空 */
    if (pcLifeMerchId[0] == '\0')
        return -1;
    *piUseCnt = iUseCnt;
    return 0;
}

int AddLifeMerchUseCnt(char * psChanneMerchId) {
    char sSqlStr[512] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "update b_life_merch set use_cnt = use_cnt+1 where merch_id = '%s'", psChanneMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新生活圈商户[%s]累计交易次数失败", psChanneMerchId);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}


/*无卡快捷消费交易跳转生活圈商户*/
int ConvLifeMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sMerchName[256 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sChanneMerchId[15 + 1] = {0}, sAgentId[8 + 1] = {0}, sChannelId[8 + 1] = {0};
    char sChannelMerchName[256 + 1] = {0},sMerchLevel[1+1] = {0}, sUseCnt[8 + 1] = {0};
    cJSON * pstTransJson = NULL;
    int iRet = -1,iUseCnt = 0, iFindUseCnt = 0;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "merch_p_name", sMerchName);
    GET_STR_KEY(pstTransJson, "agent_id", sAgentId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);

    tLog(DEBUG,"agent_id[%s],user_code[%s]",sAgentId,sUserCode);
    
    /*BEGIN time:20180925 fanghui 生活圈商户规则优化*/
    /* 获取生活圈商户跳转次数 */
    if (FindValueByKey(sUseCnt, "LIFE_MERCH_USE_COUNT") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[LIFE_MERCH_USE_COUNT]生活圈跳转次数,失败.");
        return -1;
    }
    iUseCnt = atoi(sUseCnt);
    tLog(INFO,"生活圈商户允许跳转次数[%d]",iUseCnt);
    /* END */

    if (FindLifeMerch(sChanneMerchId, sChannelMerchName, sUserCode, sAgentId,&iFindUseCnt) < 0) {
        /*BEGIN C类商户 没有开通跳转生活圈商户的，按照规则选择一个生活圈商户进行交易 */
        GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
        if( sMerchLevel[0] == '3' || sMerchLevel[0] == '4' ) {
            tLog(INFO, "sMerchLevel = [%s]: C类商户[%s]没有开通跳转生活圈,进行按规则自动获取生活圈商户.", sMerchLevel,sMerchId);
            /*取一个本地商户所在市的有效状态的生活圈商户 (此处包含 经济特区 直接跳转本市 )*/
            if ( FindLocalCityLifeMerch(sChanneMerchId, sChannelMerchName, sUserCode,iUseCnt) < 0 ){
                /*本市没有，随机取本地商户所在省的省会城市的一个有效状态的生活圈商户 （此处 需要排除经济特区 不跳转本省省会城市的 生活圈商户 ）*/
                if( FindCapitalLifeMerch(sChanneMerchId, sChannelMerchName, sUserCode,iUseCnt) < 0 ) {
                    tLog(ERROR,"此商户[%s]无可用生活圈商户跳转",sMerchId);
                    ErrHanding(pstTransJson, "Z0", "商户[%s]关注的生活圈商户跳转次数已达上限！",sMerchId);
                    return ( -1 );
                }
            }
            
            /*BEGIN time:20180925 fanghui 生活圈商户跳转次数加一*/
            if (AddLifeMerchUseCnt(sChanneMerchId)) {
                ErrHanding(pstTransJson, "96", "更新生活圈商户累计笔数失败", sMerchId);
                return -1;
            }
            /*END*/
            
            SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
            tLog(INFO, "使用按规则跳转的生活圈商户[%s].", sChanneMerchId);
        }
        /*END C类商户 没有开通跳转生活圈商户的，按照规则选择一个生活圈商户进行交易 */
        else {
            tLog(DEBUG, "商户[%s]查找生活圈商户失败,使用原商户号.", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            tLog(INFO, "使用本地商户[%s].", sMerchId);
        }
    } else {
        
        
        /* BEGIN fanghui 判断关注的生活圈是否满足跳转次数限制，满足次数累加一，不满足返回pos*/
        if (iFindUseCnt >= iUseCnt) {
            tLog(ERROR, "关注的此生活区商户[%s]已达到跳转次数限制，已跳[%d]次", sChanneMerchId, iFindUseCnt);
            ErrHanding(pstTransJson, "Z0", "商户[%s]关注的生活圈商户跳转次数已达上限！", sMerchId);
            return -1;
        } else {
            /*生活圈商户跳转次数加一*/
            if (AddLifeMerchUseCnt(sChanneMerchId)) {
                ErrHanding(pstTransJson, "96", "更新生活圈商户累计笔数失败", sMerchId);
                return -1;
            }
             tLog(INFO,"关注的生活圈商户[%s]已跳转次数[%d]",sChanneMerchId,iFindUseCnt);
        }
        /*END*/
        
        if (sChanneMerchId[0] != '\0') {
            tLog(DEBUG, "商户[%s]使用生活圈商户[%s].", sMerchId, sChanneMerchId);
            SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
            tLog(INFO, "本地商户[%s]银联商户[%s].", sMerchId, sChanneMerchId);
        } else {
            SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            tLog(INFO, "使用本地商户[%s].", sMerchId);
        }
    }
    return 0;
}