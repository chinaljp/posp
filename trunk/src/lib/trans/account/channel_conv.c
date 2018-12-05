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
#include "settle_date.h"
#include "t_macro.h"
#include "status.h"
#include "limitrule.h"
#include "trans_code.h"

extern TransCode g_stTransCode;

int JudgeAgentRoute() {

    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;
    char sState[2];
    int iState = 0;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT KEY_VALUE  FROM S_PARAM WHERE KEY='AGENT_ROUTE_PRIME_SWITCH'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
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

        tLog(ERROR, "优惠类路由已关闭");
        tReleaseRes(pstRes);
        return 1;

    }
    tReleaseRes(pstRes);
    return 0;

}

int DBFindMerchArea(char * pcMerchId, char * pcMerchCityCode, char * pcMerchPCityCode, int iLevel) {

    char sMerchCityCode[4 + 1] = {0}, sMerchPCityCode[4 + 1] = {0}, sSql[512] = {0};
    OCI_Resultset *pstRes = NULL;

    tLog(INFO, "商户地区所属[%d]级", iLevel);
    if (iLevel == 4) {
        snprintf(sSql, sizeof (sSql), "SELECT a.CITY_CODE,a.P_CITY_CODE FROM S_CITY a \
                   join s_city b on a.p_city_code=b.city_code \
                   WHERE a.CITY_CODE =  (SELECT P_CITY_CODE FROM S_CITY WHERE CITY_CODE =(SELECT CITY_CODE FROM B_MERCH WHERE MERCH_ID='%s'))", pcMerchId);
    } else {
        snprintf(sSql, sizeof (sSql), "SELECT a.CITY_CODE,a.P_CITY_CODE FROM S_CITY a \
                 join s_city b on a.p_city_code=b.city_code  \
                 WHERE a.CITY_CODE =(SELECT CITY_CODE FROM B_MERCH WHERE MERCH_ID='%s')", pcMerchId);
    }
    if (tExecute(&pstRes, sSql) < 0) {
        tLog(ERROR, "sql[%s] err!", sSql);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSql);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, sMerchCityCode);
        STRV(pstRes, 2, sMerchPCityCode);
    }

    strcpy(pcMerchCityCode, sMerchCityCode);
    strcpy(pcMerchPCityCode, sMerchPCityCode);

    tReleaseRes(pstRes);
    return 0;
}

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

int FindLifeMerch(char *pcLifeMerchId, char *pcLifeTermId, char *pcLifeMerchName, char *pcUserCode, char *pcAgentId, int * piUseCnt) {
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

    snprintf(sSqlStr, sizeof (sSqlStr), "select mc.b_life_merch_id,mc.life_term_id,lm.merch_name,lm.use_cnt"
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
        STRV(pstRes, 2, pcLifeTermId);
        STRV(pstRes, 3, pcLifeMerchName);
        INTV(pstRes, 4, iUseCnt);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    /* 如果生活圈商户取消不选，数据库里的商户号和终端号会被清空 */
    if (pcLifeMerchId[0] == '\0' || pcLifeTermId[0] == '\0')
        return -1;
    
    *piUseCnt = iUseCnt;
    
    return 0;
}

/* 通过生活圈商户号查找商户名称 */
int FindLifeMerchName(char *pcLifeMerchName, char *pcLifeMerchId) {
    char sSqlStr[512] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select merch_name from B_LIFE_MERCH "
            " WHERE merch_id='%s'", pcLifeMerchId);

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
        STRV(pstRes, 1, pcLifeMerchName);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

/* 通过优惠类渠道商户号查找商户名称 */
int FindChannelMerchName(char *pcLifeMerchName, char *pcLifeMerchId) {
    char sSqlStr[512] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select channel_merch_name from B_CHANNEL_MERCH "
            " WHERE CHANNEL_MERCH='%s'", pcLifeMerchId);

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
        STRV(pstRes, 1, pcLifeMerchName);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int ChkChannelMerch(cJSON *pstJson, char * pcChanneMerchId) {

    char sCardType[1 + 1] = {0}, sTransCode[9] = {0};
    double dTranAmt = 0.0L;
    char sMerchId[15 + 1] = {0}, sChannelMerchId[15 + 1] = {0};
    int iCnt = 0;

    PufaLimitRule stPufaLimitRule;

    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "card_type", sCardType);
    GET_DOU_KEY(pstJson, "amount", dTranAmt);
    GET_STR_KEY(pstJson, "merch_id", sMerchId);

    if (memcmp(sTransCode, "021000", 6)) { /*如果是查余直接跳过*/

        dTranAmt /= 100;

        /* 查找浦发商户规则 */
        if (FindChannelRuleNoUp(&stPufaLimitRule, pcChanneMerchId) < 0) {
            tLog(ERROR, "未找到渠道商户[%s]的限额规则.", pcChanneMerchId);
            return -1;
        }

        /* 查找本地商户规则 */
        if (FindMerchLimitRuleNoUp(&stPufaLimitRule, sMerchId) < 0) {
            tLog(ERROR, "未找到渠道商户[%s]的限额规则.", pcChanneMerchId);
            return -1;
        }
        /** 限制规则 */
        if (stPufaLimitRule.sCardType[0] != '2') {/* 当标准为2时不限借贷记 */
            if (sCardType[0] != stPufaLimitRule.sCardType[0]) {
                tLog(INFO, "本地商户[%s]交易卡类型为[%c],要求消费卡类型为[%s]", sMerchId, sCardType[0], (stPufaLimitRule.sCardType[0] == '1' ? "信用卡" : "借记卡"));
                return -1;
            }
        }

        if (dTranAmt < stPufaLimitRule.dLocalPassAmt) {
            tLog(INFO, "本地商户[%s]消费金额小于起跳金额为[%f]", sMerchId, stPufaLimitRule.dLocalPassAmt);
            return -1;
        }

        /* 获取本地商户正向交易笔数 */
        iCnt = FindLocalMerchTransCnt(sMerchId, stPufaLimitRule.sChannelId);
        if (iCnt < 0) {
            tLog(INFO, "查找本地商户[%s]路由交易笔数失败.", sMerchId);
            return -1;
        }
        iCnt += 1;
        //本地商户单日消费笔数
        if (DBL_CMP(iCnt, stPufaLimitRule.iLocalTransCnt)) {
            tLog(INFO, "本地商户[%s]日消费交易次数超限,日限交易笔数[%d],当前交易笔数[%d]", sMerchId, stPufaLimitRule.iLocalTransCnt, iCnt);
            return -1;
        }
    }
    //检查交易是否允许
    if (sTransCode[5] != '3' && NULL == strstr(stPufaLimitRule.sATransCode, sTransCode)) {
        tLog(ERROR, "禁止的交易[%s].", sTransCode);
        return -1;
    }

    return 0;
}

int FindMerchArea(char * pcMerchId, char * pcMerchPcityCode) {
    int iRet = 0;
    char sMerchCityCode[4 + 1] = {0}, sMerchPCityCode[4 + 1] = {0};
    /*  判断商户属于几级地区码 */
    iRet = FindCityLevel(pcMerchId);
    if (iRet < 0) {
        tLog(ERROR, "获取数据失败");
        return -1;
    }

    /*查询商户市区码和省级码*/
    if (DBFindMerchArea(pcMerchId, sMerchCityCode, sMerchPCityCode, iRet) < 0) {
        tLog(ERROR, "查询商户省级码失败");
        return -1;
    }
    strcpy(pcMerchPcityCode, sMerchPCityCode);
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

/* 优惠渠道商户号转换及规则检查 */
int ConvChannelMerch(cJSON *pstJson, int *piFlag) {

    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sMerchName[256 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sChannelTermId[8 + 1] = {0}, sChanneMerchId[15 + 1] = {0}, sAgentId[8 + 1] = {0}, sChannelId[8 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sChannelMerchName[256 + 1] = {0}, sGroupId[1 + 1] = {0};
    char sMerchPCityCode[4 + 1] = {0};
    char sAreaRoute[1 + 1] = {0};
    double dPassAmt = 0.0L;
    int iSwth = 0;
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "agent_id", sAgentId);

    /* 判断代理商路由总开关 */
    iSwth = JudgeAgentRoute();
    if (iSwth < 0) {
        ErrHanding(pstTransJson, "96", "获取代理商路由总开关失败.");
        return -1;
    }
    if (iSwth == 1) {
        tLog(INFO, "代理商优惠路由已关闭");
        return 0;
    }

    /* 判断商户是否开通优惠路由 */
    if (ChkMerchFlag(sMerchId) < 0) {
        tLog(INFO, "商户[%s]没有开通优惠路由功能", sMerchId);
        return 0;
    }

    /* 查询商户省级码 */
    if (FindMerchArea(sMerchId, sMerchPCityCode) < 0) {
        ErrHanding(pstTransJson, "03", "获取商户省级码失败.");
        return -1;
    }
    /*查找商户所属代理商有没有开通或配置路由并获取路由规则信息*/
    /*
    PASS_AMT	起跳金额
    area_route	路由区域
     */
    if (FindChannelGroupId(sAgentId, sGroupId, &dPassAmt, sAreaRoute) < 0) {
        tLog(INFO, "商户[%s]查找渠道商户组ID失败,使用原流程.", sMerchId);
    } else {
        tLog(INFO, "获取优惠商户规则信息:组[%s],起跳金额[%f],起跳省份限制[%c:(1.全国,0.同省)],商户省级码[%s]", sGroupId, dPassAmt, sAreaRoute[0], sMerchPCityCode);
        /*查找商户渠道路由*/
        if (FindChannelMerchId(sGroupId, sChanneMerchId, sChannelTermId, sChannelMerchName, sChannelId, sMerchPCityCode, sAreaRoute) < 0) {
            tLog(INFO, "未找到渠道组[%s]所属渠道商户", sGroupId);
            return 0;
        }
        /*检查路由规则*/
        if (ChkChannelMerch(pstTransJson, sChanneMerchId) < 0) {
            /*判断不满足路由规则时是否继续走银联*/
            if (ChkTransContol(sMerchId) < 0) {
                ErrHanding(pstTransJson, "W8", "不满足优惠渠道规则,不跳转,交易失败.", sMerchId);
                return -1;
            } else {
                tLog(INFO, "不满足优惠渠道规则,不跳转,走银联普通商户");
                return 0;
            }
        }
        SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
        SET_STR_KEY(pstTransJson, "channel_term_id", sChannelTermId);
        SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
        //优惠类渠道商户默认费率类型为优惠类
        SET_STR_KEY(pstTransJson, "channel_merch_type", "1");
        SET_STR_KEY(pstTransJson, "channel_id", sChannelId);
        tLog(INFO, "优惠类渠道商户[%s:%s:%s].", sChanneMerchId, sChannelTermId, sChannelId);
    }
    return 0;
}

int UpChannelLimit(cJSON *pstJson, int *piFlag) {

    char sChanneMerchId[15 + 1] = {0}, sChanneTermId[8 + 1] = {0}, sChannelId[8 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sTraceNo[6 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransCode[6 + 1] = {0};
    double dTranAmt = 0.0L;
    PufaLimitRule stPufaLimitRule;
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "channel_id", sChannelId);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
    GET_STR_KEY(pstTransJson, "channel_term_id", sChanneTermId);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);

    if (memcmp(sChannelId, "48560000", 8) && memcmp(sChannelId, "49000000", 8)) {
        tLog(INFO, "商户不满足跳转优惠渠道规则");
        return 0;
    }

    if (!memcmp(sChannelId, "48560000", 8)) {
        if (!memcmp(sTransCode, "020023", 6)) {
            ErrHanding(pstTransJson, "40", "浦发不允许做撤销冲正交易.");
            return -1;
        }
    }

    /* 检查是否有控制 */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "非控制交易[%s],不检查限额.", sTransCode);
        return 0;
    }


    /* 商户满足跳转浦发规则,修改商户额度 */
    {
        dTranAmt /= 100;

        /* 查找优惠渠道规则 */
        if (FindChannelRule(&stPufaLimitRule, sChanneMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "未找到渠道商户[%s]的限额规则.", sChanneMerchId);
            return -1;
        }
        /* 查找浦发本地商户规则 */
        if (FindMerchLimitRule(&stPufaLimitRule, sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "未找到本地商户[%s]的限额规则.", sMerchId);
            return -1;
        }

        if ('-' == g_stTransCode.sAccFlag[0]) {/* 当交易为反向交易时不用判断限额 */
            dTranAmt = -dTranAmt;
        } else {
            //渠道商户单日消费限额
            if (DBL_CMP(stPufaLimitRule.dUsedlimit + dTranAmt, stPufaLimitRule.dTotallimit)) {
                tLog(INFO, "渠道商户[%s]日消费交易金额超限,日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                        , sChanneMerchId, stPufaLimitRule.dTotallimit, stPufaLimitRule.dUsedlimit, dTranAmt);
                if (ChkTransContol(sMerchId) < 0) {
                    ErrHanding(pstTransJson, "W8", "商户[%s]不满足跳转条件,交易失败.", sMerchId);
                    tLog(INFO, "不满足渠道商户规则，限制交易，不允许跳银联");
                    return -1;
                } else {
                    tLog(INFO, "触发规则限制不跳转优惠类渠道商户!,走银联普通商户");
                    DEL_KEY(pstTransJson, "channel_id");
                    DEL_KEY(pstTransJson, "channel_merch_id");
                    DEL_KEY(pstTransJson, "channel_term_id");
                    DEL_KEY(pstTransJson, "channel_merch_name");
                    DEL_KEY(pstTransJson, "channel_merch_type");
                    return 0;
                }
            }
        }
        /*用于渠道商户更新累计限额*/
        if (UptChannelMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
            ErrHanding(pstTransJson, "96", "渠道商户[%s]增加累计额[%f]失败.", sChanneMerchId, dTranAmt);
            return -1;
        }

        /*用于更新本地商户累计限额*/
        if (UptChannelLocalMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
            ErrHanding(pstTransJson, "96", "本地商户[%s]增加累计额[%f]失败.", sMerchId, dTranAmt);
            return -1;
        }
        tLog(INFO, "优惠商户[%s]和本地商户[%s]更新额度成功!", sChanneMerchId, sMerchId);
        tLog(INFO, "使用渠道商户[%s:%s:%s].", sChanneMerchId, sChanneTermId, sChannelId);
        /* 更新交易流水的channel_id ，避免在当笔交易没返回的时候发起冲正找不到channel_id，而找不到优惠渠道*/
        UpdatePufaChannelId(sMerchId, sTraceNo, sTransTime, sChannelId, sChanneMerchId, sChanneTermId);
    }
    return 0;
}

int UnChannelLimit(cJSON *pstJson, int *piFlag) {

    char sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    char sChanneMerchId[15 + 1] = {0}, sChannelId[8 + 1] = {0};
    cJSON * pstTransJson = NULL;
    PufaLimitRule stPufaLimitRule;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "channel_id", sChannelId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);


    if (memcmp(sChannelId, "48560000", 8) && memcmp(sChannelId, "49000000", 8)) {
        tLog(INFO, "商户不满足跳转优惠类渠道规则");
        return 0;
    }

    dTranAmt /= 100;
    if (!memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "商户[%s]交易成功,无需恢复限额[%f].", sMerchId, dTranAmt);
        return 0;
    }

    /* 检查是否有控制 */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "非控制交易[%s],不检查限额.", sTransCode);
        return 0;
    }
    if ('+' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }

    /* 查找规则 */
    if (FindChannelRule(&stPufaLimitRule, sChanneMerchId) < 0) {
        tLog(INFO, "未找到渠道商户[%s]的限额规则.", sChanneMerchId);
        return -1;
    }

    /* 查找浦发本地商户规则 */
    if (FindMerchLimitRule(&stPufaLimitRule, sMerchId) < 0) {
        tLog(INFO, "未找到本地商户[%s]的限额规则.", sMerchId);
        return -1;
    }

    /* 用于恢复渠道商户累计限额 */
    if (UptChannelMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
        ErrHanding(pstTransJson, "96", "渠道商户[%s]增加累计额[%f]失败.", sChanneMerchId, dTranAmt);
        return -1;
    }

    /*用于更新本地商户累计限额*/
    if (UptChannelLocalMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
        ErrHanding(pstTransJson, "96", "本地商户[%s]增加累计额[%f]失败.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f]", sMerchId, dTranAmt);
    return 0;
}

/* 生活圈商户转换 */
int ConvLifeMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sMerchName[256 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sChannelTermId[8 + 1] = {0}, sChanneMerchId[15 + 1] = {0}, sAgentId[8 + 1] = {0}, sChannelId[8 + 1] = {0};
    char sChannelMerchName[256 + 1] = {0}, sMerchLevel[1 + 1] = {0}, sUseCnt[8 + 1] = {0};
    cJSON * pstTransJson = NULL;
    int iRet = -1, iUseCnt = 0, iFindUseCnt = 0;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "merch_p_name", sMerchName);
    GET_STR_KEY(pstTransJson, "agent_id", sAgentId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "channel_id", sChannelId);

    if (!memcmp(sChannelId, "48560000", 8) || !memcmp(sChannelId, "49000000", 8)) {
        tLog(INFO, "商户满足跳转优惠商户规则，已跳转优惠渠道");
        if (FindLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode, sAgentId,&iFindUseCnt) < 0) {
            tLog(INFO, "商户[%s]未配置生活圈商户，使用路由商户号", sMerchId);
            SET_STR_KEY(pstTransJson, "life_type", "000");/*跳转了优惠路由没有配置了生活圈模式*/
            return 0;
        }

        /*宝总要求只要配置了生活圈，即使跳转了优惠路由终端也要显示生活圈商户信息*/
        tLog(INFO, "商户已配置生活圈商户，终端显示生活圈商户信息[%s|%s|%s]", sChanneMerchId, sChannelTermId, sChannelMerchName);
        SET_STR_KEY(pstTransJson, "life_type", "001"); /*跳转了优惠路由还配置了生活圈模式*/
        SET_STR_KEY(pstTransJson, "life_channel_merch_id", sChanneMerchId);
        SET_STR_KEY(pstTransJson, "life_channel_term_id", sChannelTermId);
        SET_STR_KEY(pstTransJson, "life_channel_merch_name", sChannelMerchName);

        return 0;
    }

    /*BEGIN time:20180925 fanghui 生活圈商户规则优化*/
    /* 获取生活圈商户跳转次数 */
    if (FindValueByKey(sUseCnt, "LIFE_MERCH_USE_COUNT") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[LIFE_MERCH_USE_COUNT]生活圈跳转次数,失败.");
        return -1;
    }
    iUseCnt = atoi(sUseCnt);
    tLog(INFO,"生活圈商户允许跳转次数[%d]",iUseCnt);
    /* END */

    if (FindLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode, sAgentId, &iFindUseCnt) < 0) {
        /*BEGIN C类商户 没有开通跳转生活圈商户的，按照规则选择一个生活圈商户进行交易 */
        GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
        if (sMerchLevel[0] == '3' || sMerchLevel[0] == '4') {
            tLog(INFO, "sMerchLevel = [%s]: C类商户[%s]没有开通跳转生活圈,进行按规则自动获取生活圈商户.", sMerchLevel, sMerchId);
            /*取一个本地商户所在市的有效状态的生活圈商户 (此处包含 经济特区 直接跳转本市 )*/
            if (FindLocalCityLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode, iUseCnt) < 0) {
                /*本市没有，随机取本地商户所在省的省会城市的一个有效状态的生活圈商户 （此处 需要排除经济特区 不跳转本省省会城市的 生活圈商户 ）*/
                if (FindCapitalLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode, iUseCnt) < 0) {
                    tLog(ERROR, "此商户[%s]无可用生活圈商户跳转", sMerchId);
                    ErrHanding(pstTransJson, "Z0", "商户[%s]无可用生活圈商户跳转或已达跳转次数上限！", sMerchId);
                    return ( -1);
                }
            }

            /*BEGIN time:20180925 fanghui 生活圈商户跳转次数加一*/
            if (AddLifeMerchUseCnt(sChanneMerchId)) {
                ErrHanding(pstTransJson, "96", "更新生活圈商户累计笔数失败", sMerchId);
                return -1;
            }
            /*END*/

            SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sChannelTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
            tLog(INFO, "使用按规则跳转的生活圈商户[%s:%s].", sChanneMerchId, sChannelTermId);
        }            
        /*END C类商户 没有开通跳转生活圈商户的，按照规则选择一个生活圈商户进行交易 */
        else {
            tLog(DEBUG, "商户[%s]查找生活圈商户失败,使用原商户号.", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            tLog(INFO, "使用本地商户[%s:%s].", sMerchId, sTermId);
        }
    } else {

        /* BEGIN fanghui 判断关注的生活圈是否满足跳转次数限制，满足:次数累加一，不满足:返回pos*/
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

        if (sChanneMerchId[0] != '\0' && sChannelTermId[0] != '\0') {
            tLog(DEBUG, "商户[%s]使用生活圈商户[%s:%s].", sMerchId, sChanneMerchId, sChannelTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sChannelTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
            tLog(INFO, "本地商户[%s:%s]银联商户[%s:%s].", sMerchId, sTermId, sChanneMerchId, sChannelTermId);
        } else {
            SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            tLog(INFO, "使用本地商户[%s:%s].", sMerchId, sTermId);
        }
    }
    return 0;
}

/* 非生活圈交易转换银联商户号 */
int ConvCupsMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sChannelMerchId[15 + 1] = {0}, sChannelTermId[8 + 1] = {0};
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChannelMerchId);
    GET_STR_KEY(pstTransJson, "channel_term_id", sChannelTermId);

    if (sChannelMerchId[0] == '\0' || sChannelTermId[0] == '\0') {

        GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
        GET_STR_KEY(pstTransJson, "term_id", sTermId);
        SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
        SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
        tLog(INFO, "使用本地商户[%s:%s].", sMerchId, sTermId);
        return 0;
    }
    tLog(INFO, "使用原交易渠道商户终端号");
    return 0;
}

/* 反向交易需要使用正向交易的上送商户号 */
int GetMerchName(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sChannelMerchId[15 + 1] = {0};
    char sMerchName[40 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChannelMerchId);
    if (memcmp(sMerchId, sChannelMerchId, 15)) {
        /* 生活圈商户 */
        if (FindLifeMerchName(sMerchName, sChannelMerchId) < 0) {
            tLog(ERROR, "查找LIFE商户[%s]的商户名失败.", sChannelMerchId);
            if (FindChannelMerchName(sMerchName, sChannelMerchId) < 0) {
                tLog(ERROR, "查找渠道优惠类商户[%s]的商户名失败.", sChannelMerchId);

            } else {
                tLog(DEBUG, "使用优惠类渠道商户[%s]的名称[%s].", sChannelMerchId, sMerchName);
                SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            }
        } else {
            tLog(DEBUG, "使用LIFE商户[%s]的名称[%s].", sChannelMerchId, sMerchName);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
        }
    }
    return 0;
}