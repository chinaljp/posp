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
/*      函数名:     ChkMerchCardLimit()                                       */
/*      功能说明:   商户快捷消费限额检查处理，请求端配置                     */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkMerchCardLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char sMerchLevel[1+1] = {0};
    LimitRule stLimitRule;
    MerchLimit stMerchLimit;
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sOTransCode[9] = {0};
    cJSON * pstTransJson = NULL;
    char cUpdWay;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "o_trans_code", sOTransCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);

    dTranAmt /= 100;
#if 0    
    /*Begin 冲正类交易、退货交易，根据原商户号获取商户级别 add by GJQ at 20180309*/
    if( sTransCode[5] == '3' || sTransCode[5] == '1') 
    {
        if( FindMerchLevel(sMerchId, sMerchLevel) < 0 ) {
            ErrHanding(pstTransJson, "96", "检索不到原商户[%s]级别.", sMerchId);
            return ( -1 );
        }
        tLog(INFO,"原商户[%s]的级别为[%s]",sMerchId,sMerchLevel);
    }
    /*End 冲正类交易，根据原商户号获取商户级别 add by GJQ at 20180309*/
#endif    
    /* 查找规则 */
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额规则.", sMerchId);
        return -1;
    }
    //检查交易是否允许
    if (sTransCode[5] != '3' && NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        ErrHanding(pstTransJson, "B1", "禁止的交易[%s].", sTransCode);
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
        if (!DBL_ZERO(stLimitRule.dQCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dQCPLimit)) {
            ErrHanding(pstTransJson, "C3", "商户[%s]交易[%s]信用卡快捷消费单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dQCPLimit, dTranAmt);
            return -1;
        }
        //单日消费限额
        if (!DBL_ZERO(stLimitRule.dQCDLimit) && DBL_CMP(stMerchLimit.dQCDLimit + dTranAmt, stLimitRule.dQCDLimit)) {
            ErrHanding(pstTransJson, "C4", "商户[%s]信用卡日消费交易金额超限! \n"
                    "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dQCDLimit, stMerchLimit.dQCDLimit, dTranAmt);
            return -1;
        }
        
        if (!DBL_ZERO(stLimitRule.dQCMLimit) && DBL_CMP(stMerchLimit.dQCMLimit + dTranAmt, stLimitRule.dQCMLimit)) {
            ErrHanding(pstTransJson, "C5", "商户[%s]信用卡月消费交易金额超限! \n"
                    "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dQCMLimit, stMerchLimit.dQCMLimit, dTranAmt);
            return -1;
        }
        
        /*用于更新累计限额*/
        cUpdWay = 'Q';
        tLog(INFO, "商户[%s]本次交易金额[%f],快捷消费贷记卡日累计[%f],快捷消费贷记卡月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dQCDLimit, stMerchLimit.dQCMLimit);
    } else {
        if (!DBL_ZERO(stLimitRule.dQDPLimit) && DBL_CMP(dTranAmt, stLimitRule.dQDPLimit)) {
            ErrHanding(pstTransJson, "B4", "商户[%s]交易[%s]借记卡单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dQDPLimit, dTranAmt);
            return -1;
        }
        //单日消费限额
        if (!DBL_ZERO(stLimitRule.dQDDLimit) && DBL_CMP(stMerchLimit.dQDDLimit + dTranAmt, stLimitRule.dQDDLimit)) {
            ErrHanding(pstTransJson, "B5", "商户[%s]借记卡日消费交易金额超限! \n"
                    "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dQDDLimit, stMerchLimit.dQDDLimit, dTranAmt);
            return -1;
        }
        if (!DBL_ZERO(stLimitRule.dQDMLimit) && DBL_CMP(stMerchLimit.dQDMLimit + dTranAmt, stLimitRule.dQDMLimit)) {
            ErrHanding(pstTransJson, "B6", "商户[%s]借记卡月消费交易金额超限! \n"
                    "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                    , sMerchId, stLimitRule.dQDMLimit, stMerchLimit.dQDMLimit, dTranAmt);
            return -1;
        }
        /*用于更新累计限额*/
        cUpdWay = 'P';
        tLog(INFO, "商户[%s]本次交易金额[%f],快捷消费借记卡日累计[%f],快捷消费借记卡月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dQDDLimit, stMerchLimit.dQDMLimit);
    }
    
    /*更新累计限额*/
    tLog( DEBUG,"cUpdWay = [%c]:[%s]", cUpdWay,GETMESG(cUpdWay) );
    if ( UptMerchLimitEx(dTranAmt, cUpdWay, sMerchId) < 0 ) {
        ErrHanding(pstTransJson, "96", "商户[%s]%s增加限额[%f]失败.", sMerchId, GETMESG(cUpdWay), dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]%s限额累计成功,交易金额[%f]", sMerchId, GETMESG(cUpdWay), dTranAmt);
    
    return 0;
}

/******************************************************************************/
/*      函数名:     UnMerchCupsLimit()                                             */
/*      功能说明:   商户无卡快捷消费交易失败,恢复限额                                         */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
/*此限额恢复暂不用到， 现无需修改-20180704*/
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

