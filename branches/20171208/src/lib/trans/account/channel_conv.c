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

int FindLifeMerch(char *pcLifeMerchId, char *pcLifeTermId, char *pcLifeMerchName, char *pcUserCode, char *pcAgentId) {
    char sSqlStr[512] = {0}, sTmp[1024] = {0};
    char sIsYes[2] = {0};
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

    snprintf(sSqlStr, sizeof (sSqlStr), "select mc.b_life_merch_id,mc.life_term_id,lm.merch_name"
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
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    /* 如果生活圈商户取消不选，数据库里的商户号和终端号会被清空 */
    if (pcLifeMerchId[0] == '\0' || pcLifeTermId[0] == '\0')
        return -1;
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

    dTranAmt /= 100;

    /* 查找浦发商户规则 */
    if (FindChannelRuleNoUp(&stPufaLimitRule, pcChanneMerchId) < 0) {
        tLog(INFO, "未找到渠道商户[%s]的限额规则.", pcChanneMerchId);
        return -1;
    }

    /* 查找本地商户规则 */
    if (FindMerchLimitRuleNoUp(&stPufaLimitRule, sMerchId) < 0) {
        tLog(INFO, "未找到渠道商户[%s]的限额规则.", pcChanneMerchId);
        return -1;
    }

    /*本地商户为普通商户和机构商户都要判断的规则*/
    if (stPufaLimitRule.sCardType[0] != '2') {/* 当标准为2时不限借贷记 */
        if (sCardType[0] != stPufaLimitRule.sCardType[0]) {
            tLog(INFO, "本地商户[%s]交易卡类型为[%c],要求消费卡类型为[%s]", sMerchId, sCardType[0], (stPufaLimitRule.sCardType[0] ? "信用卡" : "借记卡"));
            return -1;
        }
    }

    if (dTranAmt < stPufaLimitRule.dLocalPassAmt) {
        tLog(INFO, "本地商户[%s]消费金额小于起跳金额为[%f]", sMerchId, stPufaLimitRule.dLocalPassAmt);
        return -1;
    }

    //检查交易是否允许
    if (sTransCode[5] != '3' && NULL == strstr(stPufaLimitRule.sATransCode, sTransCode)) {
        tLog(INFO, "禁止的交易[%s].", sTransCode);
        return -1;
    }
    /* 获取本地商户正向交易笔数 */
    iCnt = FindLocalMerchTransCnt(sMerchId, stPufaLimitRule.sChannelId);
    if (iCnt < 0) {
        tLog(INFO, "查找浦发本地商户[%s]交易笔数失败.", sMerchId);
        return -1;
    }
    iCnt += 1;
    //本地商户单日消费笔数
    if (DBL_CMP(iCnt, stPufaLimitRule.iLocalTransCnt)) {
        tLog(INFO, "浦发本地商户[%s]日消费交易次数超限,日限交易笔数[%d],当前交易笔数[%d]", sMerchId, stPufaLimitRule.iLocalTransCnt, iCnt);
        return -1;
    }
    return 0;
}

/* 浦发商户号转换及规则检查 */
int ConvChannelMerch(cJSON *pstJson, int *piFlag) {

    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sMerchName[256 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sChannelTermId[8 + 1] = {0}, sChanneMerchId[15 + 1] = {0}, sAgentId[8 + 1] = {0}, sChannelId[8 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sChannelMerchName[256 + 1] = {0}, sGroupId[1 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    /*查找商户有没有配置路由*/
    if (FindChannelGroupId(sMerchId, sGroupId) < 0) {
        tLog(DEBUG, "商户[%s]查找渠道商户组ID失败,使用原流程.", sMerchId);
    } else {
        /*查找商户渠道路由*/
        if (FindChannelMerchId(sGroupId, sChanneMerchId, sChannelTermId, sChannelMerchName, sChannelId) < 0) {
            tLog(INFO, "未找到渠道组[%s]所属商户出错", sGroupId);
            ErrHanding(pstTransJson, "96", "获取优惠类渠道信息失败");
            return -1;
        }
        if (memcmp(sTransCode, "021000", 6)) { /*如果是查余直接跳过*/
            /*检查路由规则*/
            if (ChkChannelMerch(pstTransJson, sChanneMerchId) < 0) {
                /*判断不满足路由规则时是否继续走银联*/
                if (ChkTransContol(sMerchId) < 0) {
                    ErrHanding(pstTransJson, "W8", "渠道商户[%s]不满足跳转条件,交易失败.", sMerchId);
                    tLog(INFO, "不满足渠道商户规则，限制交易，不允许跳银联");
                    return -1;
                } else {
                    tLog(INFO, "触发浦发规则限制不跳转优惠类渠道商户!,走银联普通商户");
                    return 0;
                }
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

        /* 查找浦发规则 */
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
            /*本地商户为机构对接商户的判断规则*/
            if (stPufaLimitRule.sMerchType[0] == '1') {
                //本地商户单日消费限额和笔数
                if (DBL_CMP(stPufaLimitRule.dLocalUsedlimit + dTranAmt, stPufaLimitRule.dLocalTotallimit)) {
                    tLog(INFO, "浦发本地商户[%s]日消费交易金额超限,日限额[%f],当前日统计金额[%f], 当前交易金额[%f]"
                            , sMerchId, stPufaLimitRule.dLocalTotallimit, stPufaLimitRule.dLocalUsedlimit, dTranAmt);
                    if (ChkTransContol(sMerchId) < 0) {
                        ErrHanding(pstTransJson, "W8", "渠道商户[%s]不满足跳转条件,交易失败.", sMerchId);
                        tLog(INFO, "不满足渠道商户规则，限制交易，不允许跳银联");
                        return -1;
                    } else {
                        tLog(INFO, "触发浦发规则限制不跳转优惠类渠道商户!,走银联普通商户");
                        DEL_KEY(pstTransJson, "channel_id");
                        return 0;
                    }
                }
            }
            //渠道商户单日消费限额
            if (DBL_CMP(stPufaLimitRule.dUsedlimit + dTranAmt, stPufaLimitRule.dTotallimit)) {
                tLog(INFO, "渠道商户[%s]日消费交易金额超限,日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                        , sChanneMerchId, stPufaLimitRule.dTotallimit, stPufaLimitRule.dUsedlimit, dTranAmt);
                if (ChkTransContol(sMerchId) < 0) {
                    ErrHanding(pstTransJson, "W8", "渠道商户[%s]不满足跳转条件,交易失败.", sMerchId);
                    tLog(INFO, "不满足渠道商户规则，限制交易，不允许跳银联");
                    return -1;
                } else {
                    tLog(INFO, "触发浦发规则限制不跳转优惠类渠道商户!,走银联普通商户");
                    DEL_KEY(pstTransJson, "channel_id");
                    return 0;
                }
            }
        }


        /*用于更新累计限额*/
        if (UptChannelMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
            ErrHanding(pstTransJson, "96", "渠道商户[%s]增加累计额[%f]失败.", sChanneMerchId, dTranAmt);
            return -1;
        }

        /* 机构类商户更新本地商户限额 */
        if (stPufaLimitRule.sMerchType[0] == '1') {
            /*用于更新本地商户累计限额*/
            if (UptChannelLocalMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
                ErrHanding(pstTransJson, "96", "本地商户[%s]增加累计额[%f]失败.", sMerchId, dTranAmt);
                return -1;
            }
        }
        tLog(INFO, "优惠商户[%s]和本地商户[%s]更新额度成功!", sChanneMerchId, sMerchId);
        tLog(INFO, "使用渠道商户[%s:%s:%s].", sChanneMerchId, sChanneTermId, sChannelId);
        /* 更新交易流水的channel_id ，避免在当笔交易没返回的时候发起冲正找不到channel_id，而找不到浦发*/
        UpdatePufaChannelId(sMerchId, sTraceNo, sTransTime, sChannelId, sChanneMerchId, sChanneTermId);

    }
    return 0;
}

int UnChannelLimit(cJSON *pstJson, int *piFlag) {

    char sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    char sCardType[1 + 1] = {0};
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

    /* 用于恢复累计限额 */
    if (UptChannelMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
        ErrHanding(pstTransJson, "96", "渠道商户[%s]增加累计额[%f]失败.", sChanneMerchId, dTranAmt);
        return -1;
    }
    /* 机构类商户更新本地商户限额 */
    if (stPufaLimitRule.sMerchType[0] == '1') {
        /*用于更新本地商户累计限额*/
        if (UptChannelLocalMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
            ErrHanding(pstTransJson, "96", "本地商户[%s]增加累计额[%f]失败.", sMerchId, dTranAmt);
            return -1;
        }
    }

    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f]", sMerchId, dTranAmt);
    return 0;

}

/* 生活圈商户转换 */
int ConvLifeMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sMerchName[256 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sChannelTermId[8 + 1] = {0}, sChanneMerchId[15 + 1] = {0}, sAgentId[8 + 1] = {0}, sChannelId[8 + 1] = {0};
    char sChannelMerchName[256 + 1] = {0};
    cJSON * pstTransJson = NULL;
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "merch_p_name", sMerchName);
    GET_STR_KEY(pstTransJson, "agent_id", sAgentId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "channel_id", sChannelId);

    if (!memcmp(sChannelId, "48560000", 8) || !memcmp(sChannelId, "49000000", 8)) {
        tLog(INFO, "商户满足跳转优惠商户规则，已跳转优惠渠道");
        return 0;
    }

    if (FindLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode, sAgentId) < 0) {
        tLog(DEBUG, "商户[%s]查找生活圈商户失败,使用原商户号.", sMerchId);
        SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
        SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
        SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
        tLog(INFO, "使用本地商户[%s:%s].", sMerchId, sTermId);
    } else {
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
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
    tLog(INFO, "使用本地商户[%s:%s].", sMerchId, sTermId);
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