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
#include "merch.h"
#include "t_cjson.h"
#include "resp_code.h"
#include "t0limit.h"
#include "postransdetail.h"
#if 0
/******************************************************************************/
/*      函数名:     T0Inspect()                                                */
/*      功能说明:   检查商户日结通t0消费限额更新限额                              */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int T0Inspect(cJSON *pstTranData, int *piFlag) {
    T0Merchlimit stT0Merchlimit;
    char sErr[128] = {0};
    char sTransTime[7] = {0}, sUserCode[15 + 1] = {0};
    char sTransCode[7] = {0}, sCardType[2] = {0}, sInputMode[4] = {0}, sTranAmt[14] = {0}, sMerchId[16] = {0}, sOldRrn[13] = {0};
    double dTranAmtAbs = 0.0L;
    int iNum = 0, iRet = 0, iTranAmt = 0;
    int iTnum = 0, iSwth = 0;
    cJSON *pstTransJson;

    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    DUMP_JSON(pstTransJson);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_INT_KEY(pstTransJson, "amount", iTranAmt);
    sprintf(sTranAmt, "%d", iTranAmt);

    /* 日结消费只支持 信用卡 */
    if (!memcmp(sTransCode, "T20000", 6)) {

        GET_STR_KEY(pstTransJson, "card_type", sCardType);
        GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
        GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
        if (sCardType[0] == '0') {
            tLog(ERROR, "日结提现只支持信用卡,本次提现卡类型为[%c],输入方式为[%c]",
                    sCardType[0], sInputMode[1]);
            sprintf(sErr, "卡类型不是信用卡.");
            tLog(ERROR, sErr);
            ErrHanding(pstTransJson, "57", sErr);
            return -1;
        }
        if ((sCardType[0] == '1')&&(sInputMode[1] == '2')) {
            iRet = FindT0Limit(sTranAmt);
            if (iRet < 0) {
                sprintf(sErr, "获取限额出错!");
                tLog(ERROR, sErr);
                ErrHanding(pstTransJson, "96", "获取限额出错!");
                return -1;
            }
            if (iRet == 1) {
                sprintf(sErr, "磁卡交易提现大于10000");
                tLog(ERROR, sErr);
                ErrHanding(pstTransJson, "E9", sErr);
                return -1;
            }
            tLog(ERROR, "日结提现");
        }
    }

    tLog(ERROR, "日结提现");
    /* 检查日结交易总开关 */
    iSwth = Chksettleswitch();
    if (iSwth < 0) {
        sprintf(sErr, "获取日结消费开关失败.");
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    if (iSwth == 1) {
        sprintf(sErr, "日结消费已关闭", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "D8", sErr);
        return -1;
    }
    /* 手机app日结通交易没上送原交易金额，需获取原交易金额 */
    if (!memcmp(sTransCode, "TA0010", 6)) {

        tLog(INFO, "APP日结通知交易");
        /* 查找原交易金额原卡类型原输入方式 */

        GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);

        iRet = GetAmountFee(sOldRrn, sTranAmt, sCardType, sInputMode);
        if (iRet != 0) {
            ErrHanding(pstTransJson, "25", "RRN = %s 的订单没有找到原交易", sOldRrn);
            return -1;
        }
        tLog(INFO, "APP-Card1Type[%c]InputMode[%s]", sCardType[0], sInputMode);
        if (sCardType[0] == '0') {

            tLog(ERROR, "日结提现只支持信用卡,本次提现卡类型为[%c],输入方式为[%c]",
                    sCardType[0], sInputMode[1]);

            sprintf(sErr, "卡类型不是信用卡.");
            tLog(ERROR, sErr);
            ErrHanding(pstTransJson, "57", sErr);
            return -1;
        }
        /*
                if ((sCardType[0] == '1')&&(sInputMode[1] == '2')) {
                    iRet = FindT0Limit(sTranAmt);
                    if (iRet < 0) {
                        sprintf(sErr, "获取限额出错!");
                        tLog(ERROR, sErr);
                        ErrHanding(pstTransJson, "96", sErr);
                        return -1;
                    }
                    if (iRet == 1) {
                        sprintf(sErr, "磁卡交易提现大于10000");
                        tLog(ERROR, sErr);
                        ErrHanding(pstTransJson, "E9", sErr);
                        return -1;
                    }
                }
         */
        tLog(INFO, "APP日结通知交易");
    }

    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    /* 验证结算时间及最低限额 */
    iTnum = Findsettletime(sTransTime, sTranAmt);
    if (iTnum < 0) {
        sprintf(sErr, "获取T0结算时间失败.");
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    if (iTnum == 2) {
        sprintf(sErr, "商户[%s]t0消费低于最低限额! \n", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "E6", sErr);
        return -1;
    }
    if (iTnum == 3) {
        sprintf(sErr, "商户[%s]t0消费大于最大限额! \n", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "E7", sErr);
        return -1;
    }
    if (iTnum == 1) {
        sprintf(sErr, "交易时间不在T0结算时间内.");
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "D7", sErr);
        return -1;

    }
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    iNum = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
    if (iNum < 0) {
        sprintf(sErr, "获取商户[%s]T0限额信息失败.", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    if (iNum == 1) {
        sprintf(sErr, "商户[%s]未开通日结通服务.", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "D6", sErr);
        return -1;
    }

    tLog(INFO, "商户[%s]获取t0限额信息成功.", sMerchId);

    /*限额检查*/

    /** 检查交易金额是否超限 **/
    dTranAmtAbs = atof(sTranAmt) / 100;

    if (dTranAmtAbs > stT0Merchlimit.dUsable_limit) {
        sprintf(sErr, "商户[%s]t0可用额度超限! \n" \
                "可用额度[%f], 当前交易金额[%f].", sMerchId, stT0Merchlimit.dUsable_limit, dTranAmtAbs);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "D5", sErr);
        return -1;
    }

    tLog(INFO, "商户[%s]t0交易限额检查通过.", sMerchId);

    /* 可用额度减去交易金额，已用额度加上交易金额 */
    tLog(INFO, "交易金额[%f]", dTranAmtAbs);
    tLog(INFO, "交易金额[%f]", stT0Merchlimit.dUsable_limit);
    stT0Merchlimit.dUsable_limit -= dTranAmtAbs;
    tLog(INFO, "交易金额[%f]", stT0Merchlimit.dUsable_limit);
    stT0Merchlimit.dUsedlimit += dTranAmtAbs;
    /* 更新商户T0限额  */
    if (UptT0limit(&stT0Merchlimit) < 0) {
        tLog(ERROR, "商户[%s]T0交易限额刷新失败!", sMerchId);
        return -1;
    }
    tLog(INFO, "商户[%s]T0交易限额刷新成功.", sMerchId);

    return 0;
}

int ChkT0OldProc(cJSON * pstTransJson) {
    PosTransDetail stAcctJon;
    char sMerchId[16] = {0}, sTermId[9] = {0}, sOldTermTraceNo[7] = {0}, sOldRrn[13] = {0};

    GET_STR_KEY(pstTransJson, "o_trace_no", sOldTermTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    /* 查找原交易 */
    if (FindRevAcctJon(&stAcctJon, sMerchId, sTermId, sOldTermTraceNo) != 0) {
        tLog(ERROR, "原交易检查失败，无原交易流水，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sOldTermTraceNo, sMerchId, sTermId);
        return -1;
    }
    /* rrn */
    tLog(DEBUG, "Rrn[%s][%s]", stAcctJon.sRrn, sOldRrn);
    if (memcmp(sOldRrn, stAcctJon.sRrn, 12)) {
        tLog(ERROR, "原交易检查失败,检索参考号RRN不一致.");
        return -1;
    }
    /* 更新清算标志  */
    if (UpT0flag(stAcctJon.sRrn) < 0) {
        tLog(ERROR, "商户[%s],RRN[%s]T0清算标志更新失败!", sMerchId, stAcctJon.sRrn);
        return -1;
    }
    tLog(INFO, "更新商户号[%s]终端号[%s]流水号[%s]的清算标志成功.", sMerchId, sTermId, sOldTermTraceNo);
    return 0;
}
#endif
#if 0
/******************************************************************************/
/*      函数名:     ChkT0Proc()                  	                      */
/*      功能说明:   恢复T0原交易											  */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UnT2003D0Limit(cJSON *pstTranData, int *piFlag) {
    cJSON *pstTransJson;
    char sRespCode[2 + 1] = {0}, sErr[128] = {0}, sUserCode[16] = {0}, sTranAmt[13] = {0};
    char sMerchId[15 + 1] = {0};
    T0Merchlimit stT0Merchlimit;
    int iNum = 0;
    double dTranAmt = 0.0;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    if (!memcmp(sRespCode, "00", 2)) {
        iNum = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
        if (iNum < 0) {
            ErrHanding(pstTransJson, "96", "获取商户[%s]D0可用额度信息失败.", sMerchId);
            return -1;
        }
        dTranAmt = dTranAmt / 100;
        /* T0消费交易失败之后把相应限额调整回去 */
        stT0Merchlimit.dUsable_limit += dTranAmt;
        stT0Merchlimit.dUsedlimit -= dTranAmt;
        /* 更新商户T0限额  */
        if (UptT0limit(&stT0Merchlimit) < 0) {
            tLog(ERROR, "商户[%s]D0可用额度更新失败.", sMerchId);
            return -1;
        } else {
            tLog(INFO, "商户[%s]D0可用额度恢复成功.", sMerchId);
        }
    } else {
        tLog(INFO, "日结消费冲正交易失败,商户[%s]不用恢复D0额度.", sMerchId);
    }
    return 0;
}
#endif

/* 日结确认交易，检查原交易，主要是从原交易中取值 */
int ChkD0Trans(cJSON *pstTranData, int *piFlag) {
    PosTransDetail stAcctJon;
    char sMerchId[16] = {0}, sTermId[9] = {0}, sOldRrn[13] = {0}, sTransDate[8 + 1] = {0};
    cJSON *pstTransJson;

    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    /* 查找原交易 */
    if (FindD0Trans(&stAcctJon, sTransDate, sOldRrn) != 0) {
        ErrHanding(pstTransJson, "25", "原交易检查失败，无原交易流水，原rrn[%s] 原商户号[%s] 原终端号[%s].", \
                sOldRrn, sMerchId, sTermId);
        return -1;
    }
    tLog(DEBUG, "商户[%s]终端[%s]原交易rrn[%s]查找成功.", sMerchId, sTermId, sOldRrn);
    if (memcmp(stAcctJon.sRespCode, "00", 2)) {
        ErrHanding(pstTransJson, "25", "商户[%s]终端[%s]原交易rrn[%s]失败, 禁止提现.", sMerchId, sTermId, sOldRrn);
    }
    DEL_KEY(pstTransJson, "amount");
    SET_DOU_KEY(pstTransJson, "amount", stAcctJon.dAmount * 100);
    SET_DOU_KEY(pstTransJson, "fee", stAcctJon.dFee);
    SET_STR_KEY(pstTransJson, "settle_date", stAcctJon.sSettleDate);
    SET_STR_KEY(pstTransJson, "card_type", stAcctJon.sCardType);
    SET_STR_KEY(pstTransJson, "o_trans_date", stAcctJon.sTransDate);
    SET_STR_KEY(pstTransJson, "o_resp_code", stAcctJon.sRespCode);
    /*add by GJQ at 20171124 begin */
    SET_STR_KEY(pstTransJson, "trans_type", stAcctJon.sTransType);
    SET_STR_KEY(pstTransJson, "settle_flag", stAcctJon.sSettleFlag);
    /*add by GJQ at 20171124 end   */

    return 0;
}
#if 0

/* 日结交易如果失败，恢复额度 */
int UnD0Limit(cJSON *pstTranData, int *piFlag) {
    cJSON *pstTransJson;
    char sRespCode[2 + 1] = {0}, sErr[128] = {0}, sUserCode[16] = {0}, sTranAmt[13] = {0};
    char sMerchId[15 + 1] = {0};
    T0Merchlimit stT0Merchlimit;
    int iNum = 0;
    double dTranAmt = 0.0;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    if (memcmp(sRespCode, "00", 2)) {
        iNum = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
        if (iNum < 0) {
            ErrHanding(pstTransJson, "96", "获取商户[%s]D0可用额度信息失败.", sMerchId);
            return -1;
        }
        dTranAmt = dTranAmt / 100;
        /* T0消费交易失败之后把相应限额调整回去 */
        stT0Merchlimit.dUsable_limit += dTranAmt;
        stT0Merchlimit.dUsedlimit -= dTranAmt;
        /* 更新商户T0限额  */
        if (UptT0limit(&stT0Merchlimit) < 0) {
            tLog(ERROR, "商户[%s]D0可用额度更新失败.", sMerchId);
            return -1;
        } else {
            tLog(INFO, "商户[%s]D0可用额度恢复成功.", sMerchId);
        }
    } else {

        tLog(INFO, "交易成功,商户[%s]不用恢复D0额度.", sMerchId);
    }
    return 0;
}
#endif

int ChkDebitD0Limit(cJSON *pstTransJson) {
    T0Merchlimit stT0Merchlimit;
    char sTmp[128] = {0}, sMerchId[16] = {0}, sUserCode[15 + 1] = {0}, sD0Type[1 + 1] = {0};
    char sSartTime[6 + 1] = {0}, sEndTime[6 + 1] = {0}, sTransTime[7] = {0};
    int iRet = 0;
    double dTranAmt = 0.0, dMinAmt = 0.0, dMaxAmt = 0.0;
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    dTranAmt = dTranAmt / 100;
    /*  检查商户是否开通借记卡D0，如果开通，获取额度信息 */
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    if (FindMerchD0Type(sD0Type, sUserCode) < 0 || sD0Type[0] != '1') {
        ErrHanding(pstTransJson, "B2", "该商户[%s]不允许D0芯片借记卡交易.", sMerchId);
        return -1;
    }
    iRet = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
    if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "获取商户[%s]D0可用额度信息失败.", sMerchId);
        return -1;
    }
    if (iRet == 1) {
        ErrHanding(pstTransJson, "D6", "商户[%s]未开通日结通服务.", sMerchId);
        return -1;
    }
    tLog(DEBUG, "商户[%s]已开通日结服务.", sMerchId);
    if (dTranAmt > stT0Merchlimit.dUsable_limit) {
        ErrHanding(pstTransJson, "D5", "商户[%s]D0可用额度超限,可用额度[%.02f],当前交易金额[%.02f],失败."
                , sMerchId, stT0Merchlimit.dUsable_limit, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]D0可用额度[%.02f],当前交易金额[%.02f]."
            , sMerchId, stT0Merchlimit.dUsable_limit, dTranAmt);
    /* 验证结算时间 */
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    if (FindValueByKey(sSartTime, "SETTLE_START_TIME") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[SETTLE_START_TIME]D0开始时间,失败.");
        return -1;
    }
    if (FindValueByKey(sEndTime, "SETTLE_END_TIME") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[SETTLE_END_TIME]D0结束时间,失败.");
        return -1;
    }
    if (memcmp(sTransTime, sSartTime, 6) < 0 || memcmp(sTransTime, sEndTime, 6) > 0) {
        ErrHanding(pstTransJson, "D7", "交易时间[%s]不在D0结算时间内[%s-%s].", sTransTime, sSartTime, sEndTime);
        return -1;
    }
    tLog(DEBUG, "交易时间[%s]在T0结算时间内[%s-%s].", sTransTime, sSartTime, sEndTime);

    if (FindValueByKey(sTmp, "D0_SINGLE_CASH_MIN") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[D0_SINGLE_CASH_MIN]D0交易下限值,失败.");
        return -1;
    }
    dMinAmt = atof(sTmp);
    if (DBL_CMP(dMinAmt, dTranAmt)) {
        ErrHanding(pstTransJson, "E6", "商户[%s]D0消费[%.02f]低于最低限额[%.02f].", sMerchId, dTranAmt, dMinAmt);
        return -1;
    }
    if (FindValueByKey(sTmp, "D0_SINGLE_CASH_MAX") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[D0_SINGLE_CASH_MAX]D0交易上限值,失败.");
        return -1;
    }
    dMaxAmt = atof(sTmp);
    if (DBL_CMP(dTranAmt, dMaxAmt)) {
        ErrHanding(pstTransJson, "E7", "商户[%s]D0消费[%.02f]大于最高限额[%.02f].", sMerchId, dTranAmt, dMaxAmt);
        return -1;
    }
    tLog(DEBUG, "商户[%s]D0消费[%.02f]下限[%.02f]上限[%.02f].", sMerchId, dTranAmt, dMinAmt, dMaxAmt);

    /* 可用额度减去交易金额，已用额度加上交易金额 */
    stT0Merchlimit.dUsable_limit -= dTranAmt;
    stT0Merchlimit.dUsedlimit += dTranAmt;
    tLog(INFO, "商户[%s]D0芯片借记卡可用额度[%.02f],已用额度[%.02f],交易金额[%.02f]."
            , sMerchId, stT0Merchlimit.dUsable_limit, stT0Merchlimit.dUsedlimit, dTranAmt);
}

int ChkCreditD0Limit(cJSON *pstTransJson) {
    T0Merchlimit stT0Merchlimit;
    char sTmp[128] = {0}, sSql[256] = {0};
    char sTransTime[7] = {0}, sUserCode[15 + 1] = {0};
    char sTransCode[7] = {0}, sCardType[2] = {0}, sInputMode[4] = {0}, sMerchId[16] = {0}, sOldRrn[13] = {0};
    double dTranAmtAbs = 0.0L, dLimitAmt = 0.0;
    char sLimitAmt[32 + 1] = {0}, sSwitch[1 + 1] = {0};
    char sSartTime[6 + 1] = {0}, sEndTime[6 + 1] = {0};
    char sORrn[6 + 1] = {0}, sOTransDate[6 + 1] = {0}, sCardNo[32] = {0};
    int iNum = 0, iRet = 0;
    double dTranAmt = 0.0, dMinAmt = 0.0, dMaxAmt = 0.0;

    char sSingleAmt[32 + 1] = {0};
    double dSingleAmt = 0.0;

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    dTranAmt = dTranAmt / 100;

    /*  检查商户是否开通D0，如果开通，获取额度信息 */
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    iRet = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
    if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "获取商户[%s]D0可用额度信息失败.", sMerchId);
        return -1;
    }
    if (iRet == 1) {
        ErrHanding(pstTransJson, "D6", "商户[%s]未开通日结通服务.", sMerchId);
        return -1;
    }
    tLog(DEBUG, "商户[%s]已开通日结服务.", sMerchId);
    if (dTranAmt > stT0Merchlimit.dUsable_limit) {
        ErrHanding(pstTransJson, "D5", "商户[%s]D0可用额度超限,可用额度[%.02f],当前交易金额[%.02f],失败."
                , sMerchId, stT0Merchlimit.dUsable_limit, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]D0可用额度[%.02f],当前交易金额[%.02f]."
            , sMerchId, stT0Merchlimit.dUsable_limit, dTranAmt);
    /* 磁条信用卡只允许在规定的金额内日结，超过限额值需要鉴权 */
    if (sCardType[0] == '1' && sInputMode[1] == '2') {
        tLog(DEBUG, "磁条信用卡.");
        tLog(DEBUG, "检查D0卡号白名单.");
        snprintf(sSql, sizeof (sSql), "select count(1) from b_trans_card_white_list"
                " where account_no='%s'", sCardNo);
        iNum = tQueryCount(sSql);
        if (iNum > 0) {
            tLog(DEBUG, "卡号[%s]已在白名单中,可以直接提现.", sCardNo);
        } else {
            tLog(DEBUG, "卡号[%s]不在白名单中,提现需要审核.", sCardNo);

            /*add by gjq at 20171019 BENGIN*/
            tLog(INFO, "检查商户消费认证白名单.(d0)");
            if (FindValueByKey(sSingleAmt, "D0_TRANS_AMT") < 0) {
                tLog(ERROR, "查找key[D0_TRANS_AMT]D0限额值,失败.");
                ErrHanding(pstTransJson, "96", "查找key[D0_TRANS_AMT]商户消费认证白名单D0限额值,失败.");
                return -1;
            }
            dSingleAmt = atof(sSingleAmt);
            tLog(DEBUG, "交易金额[%f],磁条信用卡D0限额[%f].", dTranAmt, dSingleAmt);

            snprintf(sSql, sizeof (sSql), "SELECT COUNT(1) FROM B_MERCH_D0_TRANS "
                    "WHERE MERCH_ID = '%s' AND STATUS = '1'", sMerchId);
            iNum = tQueryCount(sSql);
            if ((iNum > 0 && !DBL_CMP(dTranAmt, dSingleAmt)) || (iNum > 0 && DBL_EQ(dTranAmt, dSingleAmt))) {
                tLog(DEBUG, "商户[%s]已在消费认证白名单中且磁条信用卡交易额[%.02f]小于等于[%.02f],可以直接提现", sMerchId, dTranAmt, dSingleAmt);
            } else {
                tLog(WARN, "商户[%s]不在消费认证白名单 或者 此次交易磁条信用卡交易额超限.", sMerchId);
                /*add by gjq at 20171019 END*/
                if (FindValueByKey(sLimitAmt, "D0_SINGLE_CASH_MIDDLE") < 0) {
                    ErrHanding(pstTransJson, "96", "查找key[D0_SINGLE_CASH_MIDDLE]D0限额值,失败.");
                    return -1;
                }
                /* 判断交易金额是否大于此值 */
                dLimitAmt = atof(sLimitAmt);
                if (DBL_CMP(dTranAmt, dLimitAmt)) {
                    ErrHanding(pstTransJson, "E9", "交易金额[%.02f]大于磁条信用卡交易限额[%.02f]", dTranAmt, dLimitAmt);
                    return -1;
                }
            }
        }
    }
    /* 验证结算时间 */
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    if (FindValueByKey(sSartTime, "SETTLE_START_TIME") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[SETTLE_START_TIME]D0开始时间,失败.");
        return -1;
    }
    if (FindValueByKey(sEndTime, "SETTLE_END_TIME") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[SETTLE_END_TIME]D0结束时间,失败.");
        return -1;
    }
    if (memcmp(sTransTime, sSartTime, 6) < 0 || memcmp(sTransTime, sEndTime, 6) > 0) {
        ErrHanding(pstTransJson, "D7", "交易时间[%s]不在D0结算时间内[%s-%s].", sTransTime, sSartTime, sEndTime);
        return -1;
    }
    tLog(DEBUG, "交易时间[%s]在T0结算时间内[%s-%s].", sTransTime, sSartTime, sEndTime);

    if (FindValueByKey(sTmp, "D0_SINGLE_CASH_MIN") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[D0_SINGLE_CASH_MIN]D0交易下限值,失败.");
        return -1;
    }
    dMinAmt = atof(sTmp);
    if (DBL_CMP(dMinAmt, dTranAmt)) {
        ErrHanding(pstTransJson, "E6", "商户[%s]D0消费[%.02f]低于最低限额[%.02f].", sMerchId, dTranAmt, dMinAmt);
        return -1;
    }
    if (FindValueByKey(sTmp, "D0_SINGLE_CASH_MAX") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[D0_SINGLE_CASH_MAX]D0交易上限值,失败.");
        return -1;
    }
    dMaxAmt = atof(sTmp);
    if (DBL_CMP(dTranAmt, dMaxAmt)) {
        ErrHanding(pstTransJson, "E7", "商户[%s]D0消费[%.02f]大于最高限额[%.02f].", sMerchId, dTranAmt, dMaxAmt);
        return -1;
    }
    tLog(DEBUG, "商户[%s]D0消费[%.02f]下限[%.02f]上限[%.02f].", sMerchId, dTranAmt, dMinAmt, dMaxAmt);

    /* 可用额度减去交易金额，已用额度加上交易金额 */
    stT0Merchlimit.dUsable_limit -= dTranAmt;
    stT0Merchlimit.dUsedlimit += dTranAmt;
    tLog(INFO, "商户[%s]D0可用额度[%.02f],已用额度[%.02f],交易金额[%.02f]."
            , sMerchId, stT0Merchlimit.dUsable_limit, stT0Merchlimit.dUsedlimit, dTranAmt);
#if 0
    //单独的交易流处理
    /* 更新商户T0限额  */
    if (UptT0limit(&stT0Merchlimit) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]D0交易额度更新失败.", sMerchId);
        return -1;
    }
    tLog(INFO, "商户[%s]D0交易额度更新成功.", sMerchId);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    if (UpTransType("S", sOTransDate, sORrn) < 0) {
        ErrHanding(pstTransJson, "96", "交易[%s:%s]交易类型trans_type更新失败.", sOTransDate, sORrn);
        return -1;
    }
#endif
    return 0;
}

/* 日结交易检查 */
int ChkD0Limit(cJSON *pstTranData, int *piFlag) {
    char sCardType[2] = {0}, sSwitch[1 + 1] = {0}, sInputMode[4] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    /* 检查日结交易总开关 */
    if (FindValueByKey(sSwitch, "DAY_ACCOUNT_SWITCH") < 0) {
        ErrHanding(pstTransJson, "96", "查找key[DAY_ACCOUNT_SWITCH]日结交易总开关,失败.");
        return -1;
    }
    if (sSwitch[0] == '0') {
        ErrHanding(pstTransJson, "D8", "日结交易总开关状态[%s].", sSwitch[0] == '0' ? "关闭" : "开通");
        return -1;
    }
    tLog(DEBUG, "日结交易总开关状态[%s].", sSwitch[0] == '0' ? "关闭" : "开通");

    /*借记卡,贷记卡判断*/
    if (sCardType[0] == '0') {//借记卡
        //ErrHanding(pstTransJson, "57", "日结提现只支持信用卡,当前交易为借记卡.");
        //借记卡是芯片卡才允许交易
        if (sInputMode[1] == '5' || sInputMode[1] == '7') {
            return ChkDebitD0Limit(pstTransJson);
        } else {
            ErrHanding(pstTransJson, "57", "日结提现不支持磁条借记卡.");
            return -1;
        }
    } else if (sCardType[0] == '1') { //贷记卡
        return ChkCreditD0Limit(pstTransJson);
    } else {//其他卡
        ErrHanding(pstTransJson, "57", "卡类型错误");
        return -1;
    }
    return 0;
}

/* 更新日结额度 */
int UpdD0Limit(cJSON *pstTranData, int *piFlag) {
    char sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sCardType[2] = {0}, sOTransDate[8 + 1] = {0};
    double dTranAmt = 0.0;
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    /* 更新商户T0限额  */
    if (UptT0Limit(dTranAmt / 100, sUserCode, sCardType) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]D0交易额度[+]更新失败.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]D0交易额度更新成功.", sMerchId);
    return 0;
}

/*  app更新trans_type*/
int UpdAppTransType(cJSON *pstTranData, int *piFlag) {
    char sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    double dTranAmt = 0.0;
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    if (UpTransType("S", sOTransDate, sORrn) < 0) {
        ErrHanding(pstTransJson, "96", "交易[%s:%s]交易类型trans_type更新失败.", sOTransDate, sORrn);
        return -1;
    }
    return 0;
}

/* 手机APP 消费认证列表转D0钱包交易 转账 到 提现 后台自动实现  add by gjq at 2017114*/
int AppTransToD0Wallet(cJSON *pstTranData, int *piFlag) {
    char sTransType[1 + 1] = {0}, sSettleFlag[1 + 1] = {0}, sOldRrn[12 + 1] = {0};
    cJSON *pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "settle_flag", sSettleFlag);
    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    tLog(DEBUG, "trans_type = [%s],settle_flag = [%s]", sTransType, sSettleFlag);

    if (sSettleFlag[0] == 'N') {
        ErrHanding(pstTransJson, "96", "原交易[%s]清算标志settle_flag = [%s ：未入账]，禁止转账、提现", sOldRrn, sSettleFlag);
        return ( -1);
    }

    if ((sTransType[0] == '2' && sSettleFlag[0] == 'M') || (sTransType[0] == 'S' && sSettleFlag[0] == 'M')) {
        /*转账到DO钱包可提现钱包*/
        if (AddAppPosWallet(pstTranData, piFlag) < 0) {
            tLog(INFO, "原交易[%s]交易金额转账到D0可提现钱包失败！", sOldRrn);
            return ( -1);
        }
        /*提现操作*/
        if (AddD0PosWallet(pstTranData, piFlag) < 0) {
            tLog(INFO, "原交易[%s]交易金额提现失败！", sOldRrn);
            return ( -1);
        }
    } else if (sTransType[0] == 'S' && sSettleFlag[0] == 'F') {
        /* 已转账到 DO钱包可提现钱包 只做提现操作*/
        tLog(INFO, "原交易[%s]交易金额已转账到D0可提现钱包，进行提现操作！", sOldRrn);
        if (AddD0PosWallet(pstTranData, piFlag) < 0) {
            return ( -1);
        }
    } else if (sTransType[0] == 'S' && sSettleFlag[0] == 'Y') {
        tLog(INFO, "原交易[%s]交易金额已转账到D0可提现钱包 且 已经提现成功！", sOldRrn);
    } else {
        ErrHanding(pstTransJson, "96", "系统异常，请检查原交易[%s]流水信息", sOldRrn);
        return ( -1);
    }

    return ( 0);
}
