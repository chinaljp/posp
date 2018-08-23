/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "t0limit.h"
#include "postransdetail.h"



/* 日结交易检查 */
int ChkD0Limit(cJSON *pstTransJson, int *piFlag) {
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
    int iTnum = 0, iSwth = 0;
    
    char sSingleAmt[32 + 1] = {0};
    double dSingleAmt = 0.0;
    
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    dTranAmt = dTranAmt / 100;
    if (sCardType[0] == '0') {
        ErrHanding(pstTransJson, "57", "日结提现只支持信用卡,当前交易为借记卡.");
        return -1;
    }
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
            if ( (iNum > 0 &&  !DBL_CMP(dTranAmt,dSingleAmt)) || (iNum > 0 && DBL_EQ(dTranAmt,dSingleAmt)) ) {
                tLog(DEBUG, "商户[%s]已在消费认证白名单中且磁条信用卡交易额[%.02f]小于等于[%.02f],可以直接提现", sMerchId,dTranAmt,dSingleAmt);
            }
            else { 
                tLog(WARN, "商户[%s]不在消费认证白名单 或者 此次交易磁条信用卡交易额超限.",sMerchId);
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





int UptT0Limit(double dAmount, char *pcUserCode) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE  B_MERCH_AUTH_LIMIT SET USED_LIMIT = USED_LIMIT+%f "
            ",USABLE_LIMIT = USABLE_LIMIT-%f \
         WHERE USER_CODE = '%s'", dAmount, dAmount, pcUserCode);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0)
    {
        tLog(ERROR, "更新额度失败USER_CODE[%s].", pcUserCode);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

/* 更新日结额度 */
int UpdD0Limit(cJSON *pstTransJson, int *piFlag) {
    char sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0;

    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    /* 更新商户T0限额  */
    if (UptT0Limit(dTranAmt / 100, sUserCode) < 0)
    {
        ErrHanding(pstTransJson, "96", "商户[%s]D0交易额度[+]更新失败.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]D0交易额度[%.02f]更新成功.", sMerchId, dTranAmt/100);
    return 0;
}


/*  app更新trans_type*/
int UpdAppTransType(cJSON *pstTransJson, int *piFlag) {
    char sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    double dTranAmt = 0.0;
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    if (UpTransType("S", sOTransDate, sORrn) < 0) {
        ErrHanding(pstTransJson, "96", "交易[%s:%s]交易类型trans_type更新失败.", sOTransDate, sORrn);
        return -1;
    }
    return 0;
}

/* 手机APP 消费认证列表转D0钱包交易 转账 到 提现 后台自动实现  add by gjq at 2017114*/
int AppTransToD0Wallet(cJSON *pstTransJson, int *piFlag) {
    char sTransType[1 + 1] = {0}, sSettleFlag[1 + 1] = {0}, sOldRrn[12 + 1] = {0};
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "settle_flag", sSettleFlag);
    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    tLog( DEBUG,"trans_type = [%s],settle_flag = [%s]", sTransType,sSettleFlag );
     
    if( sSettleFlag[0] == 'N' ) {
        ErrHanding(pstTransJson, "96", "原交易[%s]清算标志settle_flag = [%s ：未入账]，禁止转账、提现", sOldRrn,sSettleFlag);
        return ( -1 );
    }
    
    if( (sTransType[0] == '2' && sSettleFlag[0] == 'M') || (sTransType[0] == 'S' && sSettleFlag[0] == 'M') ) {
        /*转账到DO钱包可提现钱包*/
        if( AddAppPosWallet(pstTransJson, piFlag) < 0 ) {
            tLog(INFO,"原交易[%s]交易金额转账到D0可提现钱包失败！",sOldRrn);
            return ( -1 );
        }
        /*提现操作*/
        if( AddD0PosWallet(pstTransJson, piFlag) < 0 )  {
            tLog(INFO,"原交易[%s]交易金额提现失败！",sOldRrn);
            return ( -1 );
        }
    }
    else if( sTransType[0] == 'S' && sSettleFlag[0] == 'F' ) {
        /* 已转账到 DO钱包可提现钱包 只做提现操作*/
        tLog(INFO,"原交易[%s]交易金额已转账到D0可提现钱包，进行提现操作！",sOldRrn);
        if( AddD0PosWallet(pstTransJson, piFlag) < 0 )  {
            return ( -1 );
        }
    }
    else if ( sTransType[0] == 'S' && sSettleFlag[0] == 'Y' ) {
        tLog(INFO,"原交易[%s]交易金额已转账到D0可提现钱包 且 已经提现成功！",sOldRrn);
    }
    else {
        ErrHanding(pstTransJson, "96", "系统异常，请检查原交易[%s]流水信息", sOldRrn);
        return ( -1 );
    }
    
    return ( 0 );
}
