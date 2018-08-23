
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
#include "postransdetail.h"
#include "card.h"


/******************************************************************************/
/*      函数名:     RiskSameCard()                                        */
/*      功能说明:   同卡连刷                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int RiskSameCard(cJSON *pstJson, int *piFlag) {

    SameCardRule stSameCardRule;
    double dAmt = 0.0f;
    char sTimeMin[2 + 1] = {0}, sTimeSec[2 + 1] = {0}, sTimeQMin[2 + 1] = {0}, sTimeQSec[2 + 1] = {0};
    char sErr[128] = {0}, sSql[256] = {0}, sSql2[256] = {0}, sCurDate[9] = {0}, sMerchId[16] = {0};
    char sCardNo[19 + 1] = {0},sECardNo[19 + 1] = {0}, sUserCode[15 + 1] = {0}, sDesc[100] = {0}, sTime[6 + 1] = {0}, sQueryTime[6 + 1] = {0};
    int iRet = 0, iRet2 = 0;
    char sKey[128 + 1] = {0};
    double dTranAmt = 0L;

    tGetDate(sCurDate, "", -1);
    cJSON * pstTransJson;
    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);


    /* 获取风控规则 */
    iRet = FindSameCardRule(&stSameCardRule);
    if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "获取同卡连刷规则失败.");
        return -1;
    }


    if (FindValueByKey(sKey, "kms.encdata.key") < 0) {
        ErrHanding(pstTransJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0) {
        ErrHanding(pstTransJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(ERROR, "加密卡号数据[%s]", sECardNo);


    /* 商户同卡查余2次，判断是否在一分钟内做消费交易 */
    {
        sprintf(sSql2, "select count(1) from b_pos_trans_detail where merch_id='%s' "
                "and resp_code='00' and trans_code='021000' and CARD_NO_ENCRYPT='%s'", sMerchId, sECardNo);
        tLog(DEBUG, "%s", sSql2);

        iRet2 = tQueryCount(sSql2);
        if (iRet2 < 0) {
            ErrHanding(pstTransJson, "96", "获取刷卡次数失败");
            return -1;
        }
        tLog(DEBUG, "%d", iRet2);
        if (iRet2 >= stSameCardRule.lQUERYNTRANS) {
            tGetTime(sTime, "", -1);
            if (GetQueryTime(sMerchId, sECardNo, sQueryTime)) {
                ErrHanding(pstTransJson, "96", "获取查余时间失败");
                return -1;
            }
            /*查余和消费相差60之内就会记录商户信息*/
            tLog(DEBUG, "当前交易时间[%s]，最后一次查余时间[%s]", sTime, sQueryTime);
            if (memcmp(sTime, sQueryTime, 2) == 0) {
                strncpy(sTimeMin, sTime + 2, 2);
                strcpy(sTimeSec, sTime + 4);

                strncpy(sTimeQMin, sQueryTime + 2, 2);
                strcpy(sTimeQSec, sQueryTime + 4);

                tLog(INFO, "消费[%d]:[%d],查余[%d]:[%d],相差秒数[%d]", atoi(sTimeMin), atoi(sTimeSec), atoi(sTimeQMin), atoi(sTimeQSec), (atoi(sTimeMin)*60 + atoi(sTimeSec)) -(atoi(sTimeQMin)*60 + atoi(sTimeQSec)));
                if ((atoi(sTimeMin)*60 + atoi(sTimeSec)) -(atoi(sTimeQMin)*60 + atoi(sTimeQSec)) <= stSameCardRule.lQUERYNMINTRANS * 60) {
                    sprintf(sDesc, "商户同卡查余[%d]次，[%d]分钟内做消费交易，记录商户信息",iRet2,stSameCardRule.lQUERYNMINTRANS);
                    if (InstRiskMerch(pstTransJson, sDesc, "") < 0) {
                        ErrHanding(pstTransJson, "96", "插入风险商户信息失败");
                        return -1;
                    }
                }
            }
        }

    }

    /* 商户同卡连刷规则逻辑块*/
    {
        /* 判断商户是不是在同卡连刷白名单*/
        iRet = ChkWhiteCard(sUserCode);
        if (iRet == 1) {
            tLog(INFO, "商户属于同卡连刷白名单,不限制同卡连刷规则");
            return 0;
        }


        /* 判断商户有没有开启生活圈 */
        iRet = FindLifeflag(sUserCode);
        if (iRet == 1) {
            tLog(INFO, "商户已开通生活圈功能,不限制同卡连刷规则");
            return 0;
        }


        dTranAmt /= 100;



        sprintf(sSql, "select count(1) from b_pos_trans_detail where merch_id='%s' "
                "and resp_code='00' and valid_flag='0' and trans_code in ('020000','T20000' )"
                "and amount>%f and CARD_NO_ENCRYPT='%s'", sMerchId, stSameCardRule.dAmount, sECardNo);
        tLog(DEBUG, "%s", sSql);

        iRet = tQueryCount(sSql);
        if (iRet < 0) {
            ErrHanding(pstTransJson, "96", "获取刷卡次数失败");
            return -1;
        }

        if (iRet >= stSameCardRule.lCnt && dTranAmt > stSameCardRule.dAmount) {
            sprintf(sDesc, "触发风控规则,同卡连刷[%d]次，记录商户信息", stSameCardRule.lCnt);
            if (InstRiskMerch(pstTransJson, sDesc, "") < 0) {
                ErrHanding(pstTransJson, "96", "插入风险商户信息失败");
                return -1;
            }
            ErrHanding(pstTransJson, "62", "同卡已连刷次数[%ld]交易金额[%.02f],规则[%ld][%.02f],禁止交易.",
                    iRet, dTranAmt, stSameCardRule.lCnt, stSameCardRule.dAmount);
            return -1;
        }
    }
    return 0;
}