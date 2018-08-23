#include "t_cjson.h"
#include <stdbool.h>
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "limitrule.h"
#include "merchlimit.h"
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/******************************************************************************/
/*      函数名:     AmountProc1()                                             */
/*      功能说明:   终端限额检查处理                                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
#if 0
int AmountProc1(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    LimitRule stLimitRule;
    MerchLimit stMerchLimit;
    double dTranAmt = 0.0L;
    double dTranAmtAbs = 0.0L;
    bool bSignFlag;
    char sTransCode[9] = {0}, sOTransCode[9] = {0};
    cJSON * pstTransJson = NULL, *pstMerchLimitJson = NULL;


    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "o_trans_code", sOTransCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    dTranAmt /= 100;
    if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额规则.", sMerchId);
        return -1;
    }

    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的累计额数据.", sMerchId);
        return -1;
    }

    if (NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        ErrHanding(pstTransJson, "40", "交易码[%s]不被允许", sTransCode);
        return -1;
    }

    /* 金额增加的交易: 消费、预授权完成、消费撤销冲正、预授权完成撤销冲正 */
    if (NULL != strstr("020000,024100,020023,024123,M20000,M20023,02W100,02W200,02B100,02B200", sTransCode)) {
        bSignFlag = false;
    }/* 金额减少的交易: 退货、消费撤销、预授权完成撤销 */
    else if (NULL != strstr("020001,020002,024102,M20002", sTransCode)) {
        bSignFlag = true;
    } else {
        tLog(INFO, "交易[%s]不需检查终端限额!", sTransCode);
        return 0;
    }

    //微信单笔限额
    if (NULL != strstr("02W100,02W200", sTransCode)) {
        if (dTranAmt > stLimitRule.dWXLimit && stLimitRule.dWXLimit > 0.000000) {
            ErrHanding(pstTransJson, "C3", "商户[%s]微信单笔消费金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, stLimitRule.dWXLimit, dTranAmt);
            return -1;
        }
    }//支付宝单笔限额
    else if (NULL != strstr("02B100,02B200", sTransCode)) {
        if (dTranAmt > stLimitRule.dBBLimit && stLimitRule.dBBLimit > 0.000000) {
            ErrHanding(pstTransJson, "C3", "商户[%s]支付宝单笔消费金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, stLimitRule.dBBLimit, dTranAmt);
            return -1;
        }
    } else {//单笔消费限额
        if (dTranAmt > stLimitRule.dPLimit && stLimitRule.dPLimit > 0.000000) {
            ErrHanding(pstTransJson, "C3", "商户[%s]交易[%s]单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dPLimit, dTranAmt);
            return -1;
        }
    }
    dTranAmt = bSignFlag ? (-dTranAmt) : dTranAmt;
    //微信单日限额
    if (NULL != strstr("02W100,02W200", sTransCode)) {
        if (stMerchLimit.dDWXLimit + dTranAmt > stLimitRule.dDWXLimit && stLimitRule.dDWXLimit > 0.000000) {
            ErrHanding(pstTransJson, "C4", "商户[%s]微信日消费交易金额超限! \n"
                    "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dDWXLimit, stMerchLimit.dDWXLimit, dTranAmt);
            return -1;
        }

    }//支付宝单日限额
    else if (NULL != strstr("02B100,02B200", sTransCode)) {
        if (stMerchLimit.dDBBLimit + dTranAmt > stLimitRule.dDBBLimit && stLimitRule.dDBBLimit > 0.000000) {
            ErrHanding(pstTransJson, "C4", "商户[%s]支付宝日消费交易金额超限! \n"
                    "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dDBBLimit, stMerchLimit.dDBBLimit, dTranAmt);
            return -1;
        }

    }//单日消费限额

    else if (stMerchLimit.dDLimit + dTranAmt > stLimitRule.dDLimit && stLimitRule.dDLimit > 0.000000) {
        ErrHanding(pstTransJson, "C4", "商户[%s]日消费交易金额超限! \n"
                "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                , sMerchId, stLimitRule.dDLimit, stMerchLimit.dDLimit, dTranAmt);
        return -1;
    }

    //微信单月限额
    if (NULL != strstr("02W100,02W200", sTransCode)) {
        if (stMerchLimit.dMWXLimit + dTranAmt > stLimitRule.dMWXLimit && stLimitRule.dMWXLimit > 0.000000) {
            ErrHanding(pstTransJson, "C5", "商户[%s]微信月消费交易金额超限! \n"
                    "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dMWXLimit, stMerchLimit.dMWXLimit, dTranAmt);
            return -1;
        }
    }//支付宝单月限额
    else if (NULL != strstr("02B100,02B200", sTransCode)) {
        if (stMerchLimit.dMBBLimit + dTranAmt > stLimitRule.dMBBLimit && stLimitRule.dMBBLimit > 0.000000) {
            ErrHanding(pstTransJson, "C5", "商户[%s]支付宝月消费交易金额超限! \n"
                    "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dMBBLimit, stMerchLimit.dMBBLimit, dTranAmt);
            return -1;
        }
    }//单月消费限额
    else {
        if (stMerchLimit.dMLimit + dTranAmt > stLimitRule.dMLimit && stLimitRule.dMLimit > 0.000000) {
            ErrHanding(pstTransJson, "C5", "商户[%s]月消费交易金额超限! \n"
                    "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dMLimit, stMerchLimit.dMLimit, dTranAmt);
            return -1;
        }
    }
    /*用于更新累计限额*/
    if (NULL != strstr("02B100,02B200", sTransCode)) {
        stMerchLimit.dDBBLimit += dTranAmt;
        stMerchLimit.dMBBLimit += dTranAmt;
    }

    else if (NULL != strstr("02W100,02W200", sTransCode)) {
        stMerchLimit.dDWXLimit += dTranAmt;
        stMerchLimit.dMWXLimit += dTranAmt;
    }
    else {
        stMerchLimit.dDLimit += dTranAmt;
        stMerchLimit.dMLimit += dTranAmt;
    }
    pstMerchLimitJson = cJSON_CreateObject();
    if (NULL == pstMerchLimitJson) {
        ErrHanding(pstTransJson, "96", "创建商户[%s]的MerchLimitJson结构失败.", sMerchId);
        return -1;
    }
    SET_DOU_KEY(pstMerchLimitJson, "d_limit", stMerchLimit.dDLimit);
    SET_DOU_KEY(pstMerchLimitJson, "m_limit", stMerchLimit.dMLimit);
    SET_DOU_KEY(pstMerchLimitJson, "d_wx_limit", stMerchLimit.dDWXLimit);
    SET_DOU_KEY(pstMerchLimitJson, "m_wx_limit", stMerchLimit.dMWXLimit);
    SET_DOU_KEY(pstMerchLimitJson, "d_bb_limit", stMerchLimit.dDBBLimit);
    SET_DOU_KEY(pstMerchLimitJson, "m_bb_limit", stMerchLimit.dMBBLimit);

    SET_JSON_KEY(pstJson, "merch_limit", pstMerchLimitJson);
    tLog(INFO, "商户[%s]交易限额检查通过."
            , sMerchId);
    /*   终端限额检查处理    */
    return 0;
}
#endif
/******************************************************************************/
/*      函数名:     AmountProc2()                                             */
/*      功能说明:   终端限额刷新处理                                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int AmountProc2(cJSON *pstJson, int *piFlag) {
    char sRespCode[2 + 1] = {0}, sIstRespCode[2 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sTransCode[9] = {0};
    cJSON * pstTransJson = NULL, *pstMerchLimitJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    pstMerchLimitJson = GET_JSON_KEY(pstJson, "merch_limit");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "istresp_code", sIstRespCode);

    if (0 != memcmp(sIstRespCode, "00", 2)) {
        return -1;
    }
    /*  终端限额刷新处理    */
    if (pstMerchLimitJson == NULL)
        return 0;
    if (0 == memcmp(sRespCode, "00", 2)) {
        if (UptMerchLimit(pstMerchLimitJson, sMerchId, sTransCode) < 0) {
            tLog(ERROR, "商户[%s]交易统计额刷新失败!", sMerchId);
            return -1;
        }
        tLog(INFO, "商户[%s]交易统计额刷新成功.", sMerchId);
    }
    return 0;
}