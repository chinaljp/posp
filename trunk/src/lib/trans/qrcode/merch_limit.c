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
/******************************************************************************/
/*      函数名:     ChkMerchWxLimit()                                             */
/*      功能说明:   商户微信限额检查处理，请求端配置                                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkMerchWxLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char  sMerchLevel[1+1] = {0}; //add by GJQ at 20180306
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
    /*begin add by GJQ at 20180306*/
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
    /*end add by GJQ at 20180306*/
    
    dTranAmt /= 100;

    /* 查找规则 */
    //if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
    /*motify by GJQ at  20180305*/
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额规则.", sMerchId);
        return -1;
    }
    /* 查找累计额 */
    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额数据.", sMerchId);
        return -1;
    }

    /* 检查是否有控制 */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "非控制交易[%s],不检查限额.", sTransCode);
        return 0;
    }
    if (NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        //ErrHanding(pstTransJson, "40", "禁止的交易[%s].", sTransCode);
        ErrHanding(pstTransJson, "B1", "禁止的交易[%s].", sTransCode); //motify by GJQ at 20180306
        return -1;
    }
    if ('-' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }
    //单笔消费限额
    if (!DBL_ZERO(stLimitRule.dWXLimit) && DBL_CMP(dTranAmt, stLimitRule.dWXLimit)) {
        ErrHanding(pstTransJson, "B7", "商户[%s]交易[%s]单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dWXLimit, dTranAmt);
        return -1;
    }
    //单日消费限额
    if (!DBL_ZERO(stLimitRule.dDWXLimit) && DBL_CMP(stMerchLimit.dDWXLimit + dTranAmt, stLimitRule.dDWXLimit)) {
        ErrHanding(pstTransJson, "B8", "商户[%s]日消费交易金额超限! \n"
                "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                , sMerchId, stLimitRule.dDWXLimit, stMerchLimit.dDWXLimit, dTranAmt);
        return -1;
    }
    //月消费限额
    if (!DBL_ZERO(stLimitRule.dMWXLimit) && DBL_CMP(stMerchLimit.dMWXLimit + dTranAmt, stLimitRule.dMWXLimit)) {
        ErrHanding(pstTransJson, "B9", "商户[%s]月消费交易金额超限! \n"
                "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                , sMerchId, stLimitRule.dMWXLimit, stMerchLimit.dMWXLimit, dTranAmt);
        return -1;
    }
    /*用于更新累计限额*/
    if (UptMerchLimitEx(dTranAmt, 'W', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f],日累计[%f],月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dDWXLimit, stMerchLimit.dMWXLimit);
    return 0;
}

/******************************************************************************/
/*      函数名:     UnMerchWxLimit()                                             */
/*      功能说明:   交易失败,恢复限额                                         */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UnMerchWxLimit(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

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
    if (UptMerchLimitEx(dTranAmt, 'W', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f]", sMerchId, dTranAmt);
    return 0;
}

/******************************************************************************/
/*      函数名:     ChkMerchBbLimit()                                             */
/*      功能说明:   商户支部包限额检查处理，请求端配置                                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkMerchBbLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char  sMerchLevel[1+1] = {0}; //add by GJQ at 20180306
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
    /*begin add by GJQ at 20180306*/
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
    /*end add by GJQ at 20180306*/
    
    dTranAmt /= 100;

    /* 查找规则 */
    //if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
    /*motify by GJQ at  20180305*/
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额规则.", sMerchId);
        return -1;
    }
    /* 查找累计额 */
    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额数据.", sMerchId);
        return -1;
    }

    /* 检查是否有控制 */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "非控制交易[%s],不检查限额.", sTransCode);
        return 0;
    }
    if (NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        //ErrHanding(pstTransJson, "40", "禁止的交易[%s].", sTransCode);
        ErrHanding(pstTransJson, "B1", "禁止的交易[%s].", sTransCode); //motify by GJQ at 20180306
        return -1;
    }
    if ('-' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }
    //单笔消费限额
    if (!DBL_ZERO(stLimitRule.dBBLimit) && DBL_CMP(dTranAmt, stLimitRule.dBBLimit)) {
        ErrHanding(pstTransJson, "B7", "商户[%s]交易[%s]单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dBBLimit, dTranAmt);
        return -1;
    }
    //单日消费限额
    if (!DBL_ZERO(stLimitRule.dDBBLimit) && DBL_CMP(stMerchLimit.dDBBLimit + dTranAmt, stLimitRule.dDBBLimit)) {
        ErrHanding(pstTransJson, "B8", "商户[%s]日消费交易金额超限! \n"
                "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                , sMerchId, stLimitRule.dDBBLimit, stMerchLimit.dDBBLimit, dTranAmt);
        return -1;
    }
    if (!DBL_ZERO(stLimitRule.dMBBLimit) && DBL_CMP(stMerchLimit.dMBBLimit + dTranAmt, stLimitRule.dMBBLimit)) {
        ErrHanding(pstTransJson, "B9", "商户[%s]月消费交易金额超限! \n"
                "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                , sMerchId, stLimitRule.dMBBLimit, stMerchLimit.dMBBLimit, dTranAmt);
        return -1;
    }
    /*用于更新累计限额*/
    if (UptMerchLimitEx(dTranAmt, 'B', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f],日累计[%f],月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dDBBLimit, stMerchLimit.dMBBLimit);
    return 0;
}

/******************************************************************************/
/*      函数名:     UnMerchBbLimit()                                             */
/*      功能说明:   交易失败,恢复限额                                         */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UnMerchBbLimit(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

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
    if (UptMerchLimitEx(dTranAmt, 'B', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f]", sMerchId, dTranAmt);
    return 0;
}

/*pos二维码撤销 无法区分支付宝与微信，支付宝、微信 二维码撤销检查限额 皆使用此交易流*/
int ChkMerchTbCodeLimit(cJSON *pstJson, int *piFlag) {
    char sTransCode[6 + 1] = {0};
    cJSON * pstTransJson = NULL;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    
    tLog(DEBUG,"此次撤销交易码：sTransCode= [%s]",sTransCode);
    if(!memcmp(sTransCode,"02W600",6)) {
        if(ChkMerchWxLimit(pstJson, piFlag) < 0) {
            return ( -1 );
        }
            
    }
    else {
        if(ChkMerchBbLimit(pstJson, piFlag) < 0) {
            return (-1);
        }
    }
    return ( 0 );
}
/*pos二维码撤销 无法区分支付宝与微信，支付宝、微信 二维码撤销检查限额 皆使用此交易流*/
int UnMerchTbCodeLimit(cJSON *pstJson, int *piFlag) {
    char sTransCode[6 + 1] = {0};
    cJSON * pstTransJson = NULL;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    
    tLog(DEBUG,"此次撤销交易码：sTransCode= [%s]",sTransCode);
    if(!memcmp(sTransCode,"02W600",6)) {
        if( UnMerchWxLimit(pstJson, piFlag) < 0 ) {
            return ( -1 );
        }
    }
    else {
        if( UnMerchBbLimit(pstJson, piFlag) < 0 ) {
            return ( -1 );
        }
    }
    return ( 0 );
}


/******************************************************************************/
/*      函数名:     ChkMerchCupsLimit()                                             */
/*      功能说明:   商户银联二维码限额检查处理，请求端配置                                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
/*银联二维码限额检查 使用pos普通交易限额检查 此处按贷记卡限额进行限制*/
int ChkMerchCupsLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char  sMerchLevel[1+1] = {0}; //add by GJQ at 20180306
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
    /*begin add by GJQ at 20180306*/
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
    /*end add by GJQ at 20180306*/
    
    dTranAmt /= 100;

    /* 查找规则 */
     //if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
    /*motify by GJQ at  20180305*/
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额规则.", sMerchId);
        return -1;
    }
    /* 查找累计额 */
    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额数据.", sMerchId);
        return -1;
    }

    /* 检查是否有控制 */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "非控制交易[%s],不检查限额.", sTransCode);
        return 0;
    }
    if (NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        //ErrHanding(pstTransJson, "40", "禁止的交易[%s].", sTransCode);
        ErrHanding(pstTransJson, "B1", "禁止的交易[%s].", sTransCode); //motify by GJQ at 20180306
        return -1;
    }
    if ('-' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }
    //单笔消费限额
    if (!DBL_ZERO(stLimitRule.dCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dCPLimit)) {
        ErrHanding(pstTransJson, "B7", "商户[%s]交易[%s]单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dCPLimit, dTranAmt);
        return -1;
    }
    //单日消费限额
    if (!DBL_ZERO(stLimitRule.dCDLimit) && DBL_CMP(stMerchLimit.dCDLimit + dTranAmt, stLimitRule.dCDLimit)) {
        ErrHanding(pstTransJson, "B8", "商户[%s]日消费交易金额超限! \n"
                "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                , sMerchId, stLimitRule.dCDLimit, stMerchLimit.dCDLimit, dTranAmt);
        return -1;
    }
    //月消费限额
    if (!DBL_ZERO(stLimitRule.dCMLimit) && DBL_CMP(stMerchLimit.dCMLimit + dTranAmt, stLimitRule.dCMLimit)) {
        ErrHanding(pstTransJson, "B9", "商户[%s]月消费交易金额超限! \n"
                "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                , sMerchId, stLimitRule.dCMLimit, stMerchLimit.dCMLimit, dTranAmt);
        return -1;
    }
    /*用于更新累计限额*/
    if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败(银联二维码使用pos普通消费贷记卡限额).", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f],日累计[%f],月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dCDLimit, stMerchLimit.dCMLimit);
    return 0;
}

/******************************************************************************/
/*      函数名:     UnMerchCupsLimit()                                             */
/*      功能说明:   商户银联二维码交易失败,恢复限额                                         */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
/*银联二维码恢复限额 使用pos普通交易恢复限额 此处按贷记卡限额进行限制*/
int UnMerchCupsLimit(cJSON *pstJson, int *piFlag) {
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
    if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败.", sMerchId, dTranAmt);
        return -1;
    }
   
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f]", sMerchId, dTranAmt);
    
    return ( 0 );
}

