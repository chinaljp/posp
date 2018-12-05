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
/*      函数名:     ChkMerchCardLimit()                                             */
/*      功能说明:   商户银行卡限额检查处理，请求端配置                                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkMerchCardLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char sInputMode[3+1] = {0}, sMerchLevel[1+1] = {0}, sTransDate[8+1] = {0}, sRrn[12+1] = {0}; //add by GJQ at 20180305
    LimitRule stLimitRule;
    MerchLimit stMerchLimit;
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sOTransCode[9] = {0};
    cJSON * pstTransJson = NULL;
     /*begin add by GJQ at 20180613 
      * 用于 更新累计交易额时，区分磁条借贷记卡，IC借贷记卡
      * 四种卡的交易金额累计;
      *  C - IC信用卡，D - IC借记卡，
      *  F - 磁条信用卡，E - 磁条借记卡
      */
    char cUpdWay;
    /* end  add by GJQ at 20180613 */
    char sCardNoEnc[255 + 1] = {0};
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "o_trans_code", sOTransCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    /*begin motify by GJQ at 20180613*/
    GET_STR_KEY(pstTransJson, "o_input_mode", sInputMode);
    if ( 0 == strlen(sInputMode) || !memcmp(sInputMode,"01",2) ) {
       MEMSET(sInputMode);
       GET_STR_KEY(pstTransJson, "input_mode", sInputMode); 
    }
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
    /*end motify by GJQ at 20180613*/
    
    /*begin add by GuoJiaQing at 20180625 预授权完成撤销交易， 原交易（预授权完成）的输入方式也为手输 时，就去检索 预授权完成的原交易 获取 输入方式*/
    if( !memcmp(sInputMode,"01",2) ) {
        MEMSET(sInputMode);
        GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
        GET_STR_KEY(pstTransJson, "o_rrn", sRrn);
        if( FindOrigOInputMode(sInputMode,sRrn,sTransDate,"b_pos_trans_detail") < 0 ) {
            if( FindOrigOInputMode(sInputMode,sRrn,sTransDate,"b_pos_trans_detail_his") < 0 ) {
                ErrHanding(pstTransJson, "96", "检索不到原交易的原交易输入方式！");
                return ( -1 );
            }
        }
    }
    /*end add by GuoJiaQing at 20180625*/

    dTranAmt /= 100;
    
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
    
    /*Begin Z类商户交易 只允许卡包中的卡 做刷卡交易 add by gjq at 20180919*/
    if( sMerchLevel[0] == '4' ) {
        GET_STR_KEY(pstTransJson,"card_encno",sCardNoEnc);
        if ( FindMerchCard( sMerchId,sCardNoEnc ) < 0 ) {
            ErrHanding(pstTransJson, "57", "商户[%s]为Z类商户不允许非卡包卡号交易.", sMerchId);
            return -1;
        }
    }
    /*End Z类商户交易 只允许卡包中的卡 做刷卡交易  add by gjq*/
    
    /* 查找规则 */
    //if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
    /*motify by GJQ at  20180305*/
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的限额规则.", sMerchId);
        return -1;
    }
    //检查交易是否允许
    if (sTransCode[5] != '3' && NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        //ErrHanding(pstTransJson, "40", "禁止的交易[%s].", sTransCode);
        ErrHanding(pstTransJson, "B1", "禁止的交易[%s].", sTransCode); //motify by GJQ at 20180305
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
    
    /*信用卡额度检查*/
    if (sCardType[0] == '1') {
        /*begin motify by GJQ at 20180613 */
        tLog(DEBUG,"sCardType[%s]:[信用卡]",sCardType);
        tLog(DEBUG,"sInputMode[%s],sMCFlag[%s],sICFlag[%s],(0：禁止，1:允许)",
                sInputMode,stLimitRule.sMCFlag,stLimitRule.sICFlag);
        
        if ( !memcmp(sInputMode,"02",2)  ) 
        {
            /*（MC_CARD_FLAG 磁条贷记卡交易标志 0：禁止，1:允许）*/
            if( stLimitRule.sMCFlag[0] == '0' ) {  
                ErrHanding(pstTransJson, "B2", "禁止此商户[%s]使用磁条贷记卡交易.", sMerchId);
                return ( -1 );
            }
            /*磁条卡单笔限额判断*/
            if (!DBL_ZERO(stLimitRule.dMCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dMCPLimit)) {
                ErrHanding(pstTransJson, "CC", "商户[%s]交易[%s]磁条信用卡单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dMCPLimit, dTranAmt);
                return -1;
            }
            /*磁条卡日限额判断*/
            if (!DBL_ZERO(stLimitRule.dMCDLimit) && DBL_CMP(stMerchLimit.dMCDLimit + dTranAmt, stLimitRule.dMCDLimit)) {
                ErrHanding(pstTransJson, "CD", "商户[%s]磁条信用卡日消费交易金额超限! \n"
                        "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                        , sMerchId, stLimitRule.dMCDLimit, stMerchLimit.dMCDLimit, dTranAmt);
                return -1;
            }
            /*磁条卡月限额判断*/
            if (!DBL_ZERO(stLimitRule.dMCMLimit) && DBL_CMP(stMerchLimit.dMCMLimit + dTranAmt, stLimitRule.dMCMLimit)) {
                ErrHanding(pstTransJson, "CE", "商户[%s]磁条信用卡月消费交易金额超限! \n"
                        "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                        , sMerchId, stLimitRule.dMCMLimit, stMerchLimit.dMCMLimit, dTranAmt);
                return -1;
            }
            cUpdWay = 'F';
            tLog(INFO, "商户[%s]本次交易金额[%f],磁条信用卡日累计[%f],磁条信用卡月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dMCDLimit, stMerchLimit.dMCMLimit);
        }
        else if ( !memcmp(sInputMode,"05",2) || !memcmp(sInputMode,"07",2) )
        {
            /*（IC_CARD_FLAG IC贷记卡交易标志 0：禁止，1:允许）*/
            if ( stLimitRule.sICFlag[0] == '0' ) {
                ErrHanding(pstTransJson, "B2", "禁止此商户[%s]使用IC贷记卡交易.", sMerchId);
                return ( -1 );
            }
            /*IC卡单笔限额判断*/
            if (!DBL_ZERO(stLimitRule.dCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dCPLimit)) {
                ErrHanding(pstTransJson, "C3", "商户[%s]交易[%s]IC信用卡单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dCPLimit, dTranAmt);
                return -1;
            }
            /*IC卡日限额判断*/
            if (!DBL_ZERO(stLimitRule.dCDLimit) && DBL_CMP(stMerchLimit.dCDLimit + dTranAmt, stLimitRule.dCDLimit)) {
                ErrHanding(pstTransJson, "C4", "商户[%s]IC信用卡日消费交易金额超限! \n"
                        "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                        , sMerchId, stLimitRule.dCDLimit, stMerchLimit.dCDLimit, dTranAmt);
                return -1;
            }
            /*IC卡月限额判断*/
            if (!DBL_ZERO(stLimitRule.dCMLimit) && DBL_CMP(stMerchLimit.dCMLimit + dTranAmt, stLimitRule.dCMLimit)) {
                ErrHanding(pstTransJson, "C5", "商户[%s]IC信用卡月消费交易金额超限! \n"
                        "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                        , sMerchId, stLimitRule.dCMLimit, stMerchLimit.dCMLimit, dTranAmt);
                return -1;
            }
            cUpdWay = 'C';
            tLog(INFO, "商户[%s]本次交易金额[%f],IC信用卡日累计[%f],IC信用卡月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dCDLimit, stMerchLimit.dCMLimit);
        }
        /*end motify by GJQ at 20180613   */ 
        /*用于更新累计限额*/
        /*if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "商户[%s]信用卡增加限额[%f]失败.", sMerchId, dTranAmt);
            return -1;
        }*/
        //tLog(INFO, "商户[%s]信用卡限额累计成功,交易金额[%f],日累计[%f],月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dCDLimit, stMerchLimit.dCMLimit);
    } else {
        /*  begin motify by GJQ at 20180613 */
        tLog(DEBUG,"sCardType[%s]:[借记卡]",sCardType);
        tLog(DEBUG,"sInputMode[%s],sMDFlag[%s],sIDFlag[%s],(0：禁止，1:允许)",
                sInputMode,stLimitRule.sMDFlag,stLimitRule.sIDFlag);
        
        if ( !memcmp(sInputMode,"02",2) ) 
        {   
            /*（MD_CARD_FLAG 磁条借记卡交易标志 0：禁止，1:允许）*/
            if ( stLimitRule.sMDFlag[0] == '0' ) {
                ErrHanding(pstTransJson, "B2", "禁止此商户[%s]使用磁条借记卡交易.", sMerchId);
                return ( -1 );
            }
            /*磁条卡单笔限额判断*/
            if (!DBL_ZERO(stLimitRule.dMDPLimit) && DBL_CMP(dTranAmt, stLimitRule.dMDPLimit)) {
                ErrHanding(pstTransJson, "BA", "商户[%s]交易[%s]磁条借记卡单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dMDPLimit, dTranAmt);
                return -1;
            }
            /*磁条卡日限额判断*/
            if (!DBL_ZERO(stLimitRule.dMDDLimit) && DBL_CMP(stMerchLimit.dMDDLimit + dTranAmt, stLimitRule.dMDDLimit)) {
                ErrHanding(pstTransJson, "BB", "商户[%s]磁条借记卡日消费交易金额超限! \n"
                        "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                        , sMerchId, stLimitRule.dMDDLimit, stMerchLimit.dMDDLimit, dTranAmt);
                return -1;
            }
            /*磁条卡月限额判断*/
            if (!DBL_ZERO(stLimitRule.dMDMLimit) && DBL_CMP(stMerchLimit.dMDMLimit + dTranAmt, stLimitRule.dMDMLimit)) {
                ErrHanding(pstTransJson, "BC", "商户[%s]磁条借记卡月消费交易金额超限! \n"
                        "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                        , sMerchId, stLimitRule.dMDMLimit, stMerchLimit.dMDMLimit, dTranAmt);
                return -1;
            }
            cUpdWay = 'E';
            tLog(INFO, "商户[%s]本次交易金额[%f],磁条借记卡日累计[%f],磁条借记卡月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dMDDLimit, stMerchLimit.dMDMLimit);
        }
        else if ( !memcmp(sInputMode,"05",2) || !memcmp(sInputMode,"07",2) )
        {   
            /*（ID_CARD_FLAG IC借记卡交易标志 0：禁止，1:允许） */
            if ( stLimitRule.sIDFlag[0] == '0' ) {
                ErrHanding(pstTransJson, "B2", "禁止此商户[%s]使用IC借记卡交易.", sMerchId);
                return ( -1 );
            }
            /*IC卡单笔限额判断*/
            if (!DBL_ZERO(stLimitRule.dDPLimit) && DBL_CMP(dTranAmt, stLimitRule.dDPLimit)) {
                ErrHanding(pstTransJson, "B4", "商户[%s]交易[%s]IC借记卡单笔金额超限! 单笔限额[%f], 当前交易金额[%f].", sMerchId, sTransCode, stLimitRule.dDPLimit, dTranAmt);
                return -1;
            }
            /*IC卡日限额判断*/
            if (!DBL_ZERO(stLimitRule.dDDLimit) && DBL_CMP(stMerchLimit.dDDLimit + dTranAmt, stLimitRule.dDDLimit)) {
                ErrHanding(pstTransJson, "B5", "商户[%s]IC借记卡日消费交易金额超限! \n"
                        "日限额[%f], 当前日统计金额[%f], 当前交易金额[%f]."
                        , sMerchId, stLimitRule.dDDLimit, stMerchLimit.dDDLimit, dTranAmt);
                return -1;
            }
            /*IC卡月限额判断*/
            if (!DBL_ZERO(stLimitRule.dDMLimit) && DBL_CMP(stMerchLimit.dDMLimit + dTranAmt, stLimitRule.dDMLimit)) {
                ErrHanding(pstTransJson, "B6", "商户[%s]IC借记卡月消费交易金额超限! \n"
                        "月限额[%f], 当前月统计金额[%f], 当前交易金额[%f]."
                        , sMerchId, stLimitRule.dDMLimit, stMerchLimit.dDMLimit, dTranAmt);
                return -1;
            }
            cUpdWay = 'D';
            tLog(INFO, "商户[%s]本次交易金额[%f],IC借记卡日累计[%f],IC借记卡月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dDDLimit, stMerchLimit.dDMLimit);
        }
        /*  end motify by GJQ at 20180613 */
        /*用于更新累计限额*/
        /*if (UptMerchLimitEx(dTranAmt, 'D', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "商户[%s]借记卡增加限额[%f]失败.", sMerchId, dTranAmt);
            return -1;
        }*/
        //tLog(INFO, "商户[%s]借记卡限额恢复成功,交易金额[%f],日累计[%f],月累计[%f].", sMerchId, dTranAmt, stMerchLimit.dDDLimit, stMerchLimit.dDMLimit);
    }
    
    /*更新累计限额*/
    tLog(DEBUG,"cUpdWay = [%c]:[%s]",cUpdWay,GETMESG(cUpdWay));
    if (UptMerchLimitEx(dTranAmt, cUpdWay, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]%s增加限额[%f]失败.", sMerchId, GETMESG(cUpdWay), dTranAmt);
        return -1;
    }
    tLog(INFO, "商户[%s]%s限额累计成功,交易金额[%f]", sMerchId, GETMESG(cUpdWay), dTranAmt);
   
    
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
    char sMerchId[15 + 1] = {0},sInputMode[3+1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    char sCardType[1 + 1] = {0};
    cJSON * pstTransJson = NULL;
    char cUpdWay;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    /*begin motify by GJQ at 20180613*/
    GET_STR_KEY(pstTransJson, "o_input_mode", sInputMode);
    if ( 0 == strlen(sInputMode) ) {
       GET_STR_KEY(pstTransJson, "input_mode", sInputMode); 
    }
    /*end motify by GJQ at 20180613*/
    
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
        if ( !memcmp(sInputMode,"02",2)  ) 
        { 
            cUpdWay = 'F';
        }
        else if ( !memcmp(sInputMode,"05",2) || !memcmp(sInputMode,"07",2) )
        {
            cUpdWay = 'C';
        }
    } else {
        if ( !memcmp(sInputMode,"02",2)  ) 
        {
            cUpdWay = 'E';
        }
        else if ( !memcmp(sInputMode,"05",2) || !memcmp(sInputMode,"07",2) )
        {
            cUpdWay = 'D';
        }
    }
    
    if (UptMerchLimitEx(dTranAmt, cUpdWay, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "商户[%s]增加限额[%f]失败.", sMerchId, dTranAmt);
        return -1;
    }
    
    tLog(INFO, "商户[%s]限额恢复成功,交易金额[%f]", sMerchId, dTranAmt);
    return 0;
}