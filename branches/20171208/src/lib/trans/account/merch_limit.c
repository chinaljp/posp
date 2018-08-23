#include "t_cjson.h"
#include <stdbool.h>
#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"
#include "limitrule.h"
#include "merchlimit.h"
#include "trans_code.h"
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
extern TransCode g_stTransCode;
/*******************************************strstr***********************************/
/*      函数名:     ChkMerchCardLimit()                                             */
/*      功能说明:   商户银行卡限额检查处理，请求端配置                                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkMerchCardLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    LimitRule stLimitRule;
    MerchLimit stMerchLimit;
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sOTransCode[9] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "o_trans_code", sOTransCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    dTranAmt /= 100;
    /* 查找规则 */
    if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额规则.", sMerchId);
        return -1;
    }
    //检查交易是否允许
    if (sTransCode[5] != '3' && NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        ErrHanding(pstTransJson, "40", "禁止的交易[%s].", sTransCode);
        return -1;
    }
    /* 检查是否有控制 */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "非控制交易[%s],不检查限额.", sTransCode);
        return 0;
    }
    tLog(DEBUG, "规则已经找到");
    /* 查找累计额 */
    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的累计额数据.", sMerchId);
        return -1;
    }

    if ('-' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }
    //单笔消费限额
    /*信用卡额度检查*/
    if (sCardType[0] == '1') {
        if (!DBL_ZERO(stLimitRule.dCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dCPLimit)) {
            ErrHanding(pstTransJson, "C3", "商户[%s]交易[%s]信用卡单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dCPLimit, dTranAmt);
            return -1;
        }
        //单日消费限额
        if (!DBL_ZERO(stLimitRule.dCDLimit) && DBL_CMP(stMerchLimit.dCDLimit + dTranAmt, stLimitRule.dCDLimit)) {
            ErrHanding(pstTransJson, "C4", "商户[%s]信用卡日消费交易金额超限! \n"
                    "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dCDLimit, stMerchLimit.dCDLimit, dTranAmt);
            return -1;
        }
        
        if (!DBL_ZERO(stLimitRule.dCMLimit) && DBL_CMP(stMerchLimit.dCMLimit + dTranAmt, stLimitRule.dCMLimit)) {
            ErrHanding(pstTransJson, "C5", "商户[%s]信用卡月消费交易金额超限! \n"
                    "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dCMLimit, stMerchLimit.dCMLimit, dTranAmt);
            return -1;
        }
        
        /*用于更新累计限额*/
        if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "商户[%s]信用卡增加限额[%f]失败.", sMerchId, dTranAmt);
            return -1;
        }
        tLog(INFO, "商户[%s]信用卡限额恢复成功,交易金额[%f],日累计[%f],月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dCDLimit, stMerchLimit.dCMLimit);

    } else {
        if (!DBL_ZERO(stLimitRule.dDPLimit) && DBL_CMP(dTranAmt, stLimitRule.dDPLimit)) {
            ErrHanding(pstTransJson, "C3", "商户[%s]交易[%s]信用卡单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dDPLimit, dTranAmt);
            return -1;
        }
        //单日消费限额
        if (!DBL_ZERO(stLimitRule.dDDLimit) && DBL_CMP(stMerchLimit.dDDLimit + dTranAmt, stLimitRule.dDDLimit)) {
            ErrHanding(pstTransJson, "C4", "商户[%s]信用卡日消费交易金额超限! \n"
                    "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dDDLimit, stMerchLimit.dDDLimit, dTranAmt);
            return -1;
        }
        if (!DBL_ZERO(stLimitRule.dDMLimit) && DBL_CMP(stMerchLimit.dDMLimit + dTranAmt, stLimitRule.dDMLimit)) {
            ErrHanding(pstTransJson, "C5", "商户[%s]信用卡月消费交易金额超限! \n"
                    "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dDMLimit, stMerchLimit.dDMLimit, dTranAmt);
            return -1;
        }
        /*用于更新累计限额*/
        if (UptMerchLimitEx(dTranAmt, 'D', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "商户[%s]借记卡增加限额[%f]失败.", sMerchId, dTranAmt);
            return -1;
        }
        tLog(INFO, "商户[%s]借记卡限额恢复成功,交易金额[%f],日累计[%f],月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dDDLimit, stMerchLimit.dDMLimit);
    }

    return 0;
}

/******************************************************************************/
/*      函数名:     UnMerchCardLimit()                                             */
/*      功能说明:   交易失败,恢复限额                                         */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UnMerchCardLimit(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    char sCardType[1 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);

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
    /*用于恢复累计限额*/
    if (sCardType[0] == '1') {
        if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败.", sMerchId, dTranAmt);
            return -1;
        }
    } else {
        if (UptMerchLimitEx(dTranAmt, 'D', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败.", sMerchId, dTranAmt);
            return -1;
        }
    }
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f]", sMerchId, dTranAmt);
    return 0;
}