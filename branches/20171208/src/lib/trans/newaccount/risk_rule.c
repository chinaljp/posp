/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "t_cjson.h"
#include <stdbool.h>
#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"
#include "limitrule.h"
#include "merchlimit.h"
#include "trans_code.h"
#include "risk.h"

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
#include "risk.h"
#include "t_macro.h"
#include "t_redis.h"

int FindCityCode(char * pcMerchId, char * pcMerchCityCode, char * pcMerchPCityCode, char * pcMerchCityName, char *pcMerchPCityName, int iLevel) {

    char sMerchCityCode[4 + 1] = {0}, sMerchPCityCode[4 + 1] = {0}, sMerchPCityName[50] = {0}, sMerchCityName[50] = {0}, sSql[512] = {0};
    OCI_Resultset *pstRes = NULL;

    tLog(INFO, "商户地区所属[%d]级", iLevel);
    if (iLevel == 4)
    {
        snprintf(sSql, sizeof (sSql), "SELECT a.CITY_CODE,a.P_CITY_CODE,a.city_name,b.city_name FROM S_CITY a \
                   join s_city b on a.p_city_code=b.city_code \
                   WHERE a.CITY_CODE =  (SELECT P_CITY_CODE FROM S_CITY WHERE CITY_CODE =(SELECT CITY_CODE FROM B_MERCH WHERE MERCH_ID='%s'))", pcMerchId);
    } else
    {
        snprintf(sSql, sizeof (sSql), "SELECT a.CITY_CODE,a.P_CITY_CODE,a.city_name,b.city_name FROM S_CITY a \
                 join s_city b on a.p_city_code=b.city_code  \
                 WHERE a.CITY_CODE =(SELECT CITY_CODE FROM B_MERCH WHERE MERCH_ID='%s')", pcMerchId);
    }
    if (tExecute(&pstRes, sSql) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSql);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSql);
        return -1;
    }

    while (OCI_FetchNext(pstRes))
    {

        STRV(pstRes, 1, sMerchCityCode);
        STRV(pstRes, 2, sMerchPCityCode);
        STRV(pstRes, 3, sMerchCityName);
        STRV(pstRes, 4, sMerchPCityName);
    }

    strcpy(pcMerchCityCode, sMerchCityCode);
    strcpy(pcMerchPCityCode, sMerchPCityCode);
    strcpy(pcMerchCityName, sMerchCityName);
    strcpy(pcMerchPCityName, sMerchPCityName);

    tReleaseRes(pstRes);
    return 0;
}

int FindLifeCityCode(char * pcMerchId, char * pcMerchCityCode, char * pcMerchPCityCode, char * pcMerchCityName, char *pcMerchPCityName) {

    char sMerchCityCode[4 + 1] = {0}, sMerchPCityCode[4 + 1] = {0}, sSql[512] = {0};
    char sMerchPCityName[50] = {0}, sMerchCityName[50] = {0};
    OCI_Resultset *pstRes = NULL, *pstRes1 = NULL;

    snprintf(sSql, sizeof (sSql), "SELECT a.CITY_CODE,a.PROVINCE_CODE,b.city_name,c.city_name FROM B_LIFE_MERCH a \
                          join s_city b on b.city_code=a.city_code \
                          join s_city c on c.city_code=a.PROVINCE_CODE  WHERE MERCH_ID='%s'", pcMerchId);
    if (tExecute(&pstRes, sSql) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSql);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSql);
        return -1;
    }

    while (OCI_FetchNext(pstRes))
    {

        STRV(pstRes, 1, sMerchCityCode);
        STRV(pstRes, 2, sMerchPCityCode);
        STRV(pstRes, 3, sMerchCityName);
        STRV(pstRes, 4, sMerchPCityName);
    }

    strcpy(pcMerchCityCode, sMerchCityCode);
    strcpy(pcMerchPCityCode, sMerchPCityCode);
    strcpy(pcMerchCityName, sMerchCityName);
    strcpy(pcMerchPCityName, sMerchPCityName);

    tReleaseRes(pstRes);
    return 0;
}

int FindMerchRule(char * pstRuleCode, char * pstMerchId) {

    char sSqlStr[512] = {0}, sRuleCode[100] = {0};

    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT RULE_CODE FROM B_MERCH_RISK WHERE USER_CODE \
     in((SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID = '%s'),'AAAAAAAAAAAAAAA') AND STATUS='1'  \
     order by user_code desc", pstMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return 1;
    }
    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, sRuleCode);
    }

    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    strcpy(pstRuleCode, sRuleCode);
    tReleaseRes(pstRes);

    return 0;
}

int FindCodeRule(RiskRule *pstRiskRule, char * pcRuleCode) {

    char sSqlStr[512] = {0}, sRuleCode[4 + 1] = {0};
    RiskRule stRiskRule;
    OCI_Resultset *pstRes = NULL;
    strcpy(sRuleCode, pcRuleCode);
    tLog(DEBUG, "RULECODE[%s]", sRuleCode);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT RULE_CODE, RULE_NAME, STATUS, TRANS_CODE, TRANS_CNT,QUEUE_CNT,TRANS_AMT, \
             INTERVAL,START_TIME,END_TIME,RESP_CODE,MCC,REMARK,IS_STOP,IS_DEL,IS_FROZEN,IS_WARNNING,IS_LIFE,LIFE_CONTROL,TRANS_CONTROL \
            FROM B_RISK_RULE WHERE RULE_CODE = '%s' ", sRuleCode);
    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return 1;
    }
    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, stRiskRule.sRuleCode);
        STRV(pstRes, 2, stRiskRule.sRuleName);
        STRV(pstRes, 3, stRiskRule.sStatus);
        STRV(pstRes, 4, stRiskRule.sTransCode);
        INTV(pstRes, 5, stRiskRule.iTransCnt);
        INTV(pstRes, 6, stRiskRule.iQueueCnt);
        DOUV(pstRes, 7, stRiskRule.dTransAmt);
        INTV(pstRes, 8, stRiskRule.iInterval);
        STRV(pstRes, 9, stRiskRule.sStartTime);
        STRV(pstRes, 10, stRiskRule.sEndTime);
        STRV(pstRes, 11, stRiskRule.sRespCode);
        STRV(pstRes, 12, stRiskRule.sMcc);
        STRV(pstRes, 13, stRiskRule.sRemark);
        STRV(pstRes, 14, stRiskRule.sIsStop);
        STRV(pstRes, 15, stRiskRule.sIsDel);
        STRV(pstRes, 16, stRiskRule.sIsFrozen);
        STRV(pstRes, 17, stRiskRule.sIsWarnning);
        STRV(pstRes, 18, stRiskRule.sIsLife);
        STRV(pstRes, 19, stRiskRule.sLifeControl);
        STRV(pstRes, 20, stRiskRule.sTransControl);
    }

    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tLog(DEBUG, "风控信息:sRuleCode[%s],sRuleName[%s],sStatus[%s],sTransCode[%s],iTransCnt[%d],\
     iQueueCnt[%d],dTransAmt[%f],iInterval[%d],sStartTime[%s],sEndTime[%s],sRespCode[%s],\
     sMcc[%s],sRemark[%s],sIsStop[%s],sIsDel[%s],sIsFrozen[%s],sIsWarnning[%s],sIsLife[%s],sLifeControl[%s],sTransControl[%s]",
            stRiskRule.sRuleCode, stRiskRule.sRuleName, stRiskRule.sStatus, stRiskRule.sTransCode, stRiskRule.iTransCnt,
            stRiskRule.iQueueCnt, stRiskRule.dTransAmt, stRiskRule.iInterval, stRiskRule.sStartTime, stRiskRule.sEndTime,
            stRiskRule.sRespCode, stRiskRule.sMcc, stRiskRule.sRemark, stRiskRule.sIsStop, stRiskRule.sIsDel, stRiskRule.sIsFrozen,
            stRiskRule.sIsWarnning, stRiskRule.sIsLife, stRiskRule.sLifeControl, stRiskRule.sTransControl);
    *pstRiskRule = stRiskRule;
    tLog(DEBUG, "已找到分控规则码[%s]参数", sRuleCode);
    tReleaseRes(pstRes);
    return 0;
}

int FindCityLevel(char * pcMerchId) {
    char sSqlStr[512] = {0}, sCityLevel[1 + 1] = {0};
    int i = 0;
    OCI_Resultset *pstRes = NULL;
    tLog(INFO, "MERCH_ID[%s]", pcMerchId);
    snprintf(sSqlStr, sizeof (sSqlStr), "select city_level from s_city where city_code in (select city_code from b_merch where merch_id = '%s') ", pcMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, sCityLevel);
    }
    i = atoi(sCityLevel);
    return i;
}

/*
1、单商户查询余额高于5次报警，超过10次冻结商户；
2. 单卡查询余额大于3次报警
 */

int RuleCodefunc1(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0};
    char sKey[128 + 1] = {0}, sSqlStr[512] = {0}, sRuleCode[100] = {0}, sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0};
    int iNum = 0, iRet = 0;
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "card_no", sCardNo);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }

    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "风控规则[R001]OR[R002]OR[R015]执行中...");

    if (!memcmp(stRiskRule->sRuleCode, "R015", 4))
    {
        tLog(INFO, "同卡查询风控");
        if (FindValueByKey(sKey, "kms.encdata.key") < 0)
        {
            ErrHanding(pstJson, "96", "查找秘钥失败");
            tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
            return -1;
        }
        if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
        {
            ErrHanding(pstJson, "96", "加密卡号数据失败.");
            tLog(ERROR, "加密卡号数据失败.");
            return -1;
        }
        snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
         TRANS_CODE = '021000' AND MERCH_ID ='%s' AND RESP_CODE='00' AND CARD_NO_ENCRYPT='%s'", sMerchId, sECardNo);

    } else
    {
        tLog(INFO, "同商户查询风控");
        snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
       TRANS_CODE = '021000' AND MERCH_ID ='%s' AND RESP_CODE='00'", sMerchId);

    }

    iNum = tQueryCount(sSqlStr);
    if (iNum < 0)
    {
        ErrHanding(pstJson, "96", "获取次数失败");
        return -1;
    }
    if (iNum >= stRiskRule->iTransCnt)
    {
        tLog(INFO, "单商户查询余额高于[%d]次，预警记录或冻结商户", stRiskRule->iTransCnt);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }
    return 0;
}

/* R003*/

/*
3、单卡连刷限制第3次报警
 */
int RuleCodefunc3(cJSON * pstJson, RiskRule *stRiskRule) {


    char sSql[512] = {0}, sSql1[512] = {0}, sMerchId[16] = {0}, sChannelMerchId[16] = {0};
    char sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0}, sUserCode[15 + 1] = {0};
    int iRet = 0, iRet1 = 0;
    char sKey[128 + 1] = {0}, sTransCode[6 + 1] = {0}, sInputMode[3 + 1] = {0};

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "user_code", sUserCode);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
    GET_STR_KEY(pstJson, "input_mode", sInputMode);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    if (memcmp(sInputMode + 1, "2", 1))
    {
        tLog(INFO, "交易不是磁条卡不限制该规则");
        return 0;
    }

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(ERROR, "加密卡号数据[%s]", sECardNo);


    sprintf(sSql1, "SELECT COUNT(1) FROM B_WHITE_SAMCARD_LIST WHERE STATUS='1' AND CARD_NO_ENCRYPT='%s' AND USER_CODE='%s'", sECardNo, sUserCode);

    iRet1 = tQueryCount(sSql1);
    if (iRet1 < 0)
    {
        ErrHanding(pstJson, "96", "获取数据失败");
        return -1;

    } else if (iRet1 == 1)
    {
        tLog(INFO, "卡号属于同卡连刷白名单,不限制同卡连刷规则");
        return 0;
    }

    if (!memcmp(stRiskRule->sIsLife, "1", 1))
    {
        tLog(INFO, "该风控规则判断生活圈");
        if (memcmp(sMerchId, sChannelMerchId, 15))
        {
            tLog(INFO, "属于生活圈商户,不判断该规则");
            return 0;
        }
    }


    tLog(DEBUG, "风控规则R003执行中...");
    /* 商户同卡连刷规则逻辑块*/
    {

        sprintf(sSql, "select count(1) from b_pos_trans_detail where merch_id='%s' "
                "and resp_code='00' and valid_flag='0' and trans_code in ('020000','T20000','024100' )"
                "and amount>%f and CARD_NO_ENCRYPT='%s' and input_mode like '02%%'", stRiskRule->dTransAmt, sMerchId, sECardNo);
        tLog(DEBUG, "%s", sSql);

        iRet = tQueryCount(sSql);
        if (iRet < 0)
        {
            ErrHanding(pstJson, "96", "获取次数失败");
            return -1;
        }

        if (iRet >= stRiskRule->iTransCnt)
        {
            tLog(INFO, "单卡连刷消费金额大于[%f]第[%d]次,报警(包括消费，日结，预授权完成)", stRiskRule->dTransAmt, iRet);
            iRet = RiskOpt(pstJson, stRiskRule);
            if (iRet < 0)
            {
                tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
            } else if (iRet == 1)
            {
                ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
                return -1;
            }
        }
    }

    return 0;
}

/*R004*/

/*
4、单卡查询密码错误2次以上冻结账户；
 */
int RuleCodefunc4(cJSON * pstJson, RiskRule *stRiskRule) {

    char sMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0}, sRuleCode[100] = {0}, sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0};
    char sKey[128 + 1] = {0}, sTermId[8 + 1] = {0};
    int iRet = 0;
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "term_id", sTermId);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "card_no", sCardNo);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "风控规则R004执行中...");

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
    TRANS_CODE = '021000' AND  CARD_NO_ENCRYPT='%s' AND RESP_CODE='55'", sECardNo);
    iRet = tQueryCount(sSqlStr);
    if (iRet < 0)
    {
        ErrHanding(pstJson, "96", "获取次数失败");
        return -1;
    }

    if (iRet >= stRiskRule->iTransCnt)
    {
        /*冻结账户*/
        tLog(INFO, "单卡查询密码错误[%d]次以上冻结账户(同卡)", stRiskRule->iTransCnt);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }
    return 0;
}

/*R005*/

/*
5、单商户查询密码错误5次以上冻结账户；
 */
int RuleCodefunc5(cJSON * pstJson, RiskRule *stRiskRule) {

    char sMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0}, sRuleCode[100] = {0}, sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0};
    char sKey[128 + 1] = {0}, sTermId[8 + 1] = {0};
    int iRet = 0;
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "风控规则R005执行中...");

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
    TRANS_CODE = '021000' AND MERCH_ID='%s' AND RESP_CODE='55'", sMerchId);
    iRet = tQueryCount(sSqlStr);
    if (iRet < 0)
    {
        ErrHanding(pstJson, "96", "获取次数失败");
        return -1;
    }


    if (iRet >= stRiskRule->iTransCnt)
    {
        /*冻结账户*/
        tLog(INFO, "单商户查询密码错误[%d]次以上冻结账户(同商户，不同卡)", stRiskRule->iTransCnt);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }
    return 0;
}

/* 6、单商户大额交易单笔超过10万的报警  */
int RuleCodefunc6(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0};
    char sTransCode[6 + 1] = {0}, sDesc[100] = {0}, sSqlStr[1024] = {0};
    int iAmount = 0, iRet = 0;
    double dRkAmount = 0.00;

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_INT_KEY(pstJson, "amount", iAmount);

    dRkAmount = stRiskRule->dTransAmt;

    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL)
    {
        tLog(INFO, "此规则针对交易[%s]不做限制", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则6执行中...");
    if (iAmount > (int) (dRkAmount * 100))
    {
        tLog(DEBUG, "商户[%s]大额交易单笔超过[%.02f]元,预警!", sMerchId, dRkAmount);
        /*预警记录*/
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }

    return 0;
}

/* 7、同借记卡卡，同商户，1分钟内连续交易（消费，预授权完成）冻结账户 ROO7*/
int RuleCodefunc7(cJSON * pstJson, RiskRule *stRiskRule) {
    char sKey[128 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sCardNo[19 + 1] = {0}, sTransCode[6 + 1] = {0}, sTransmitTime[10 + 1] = {0};
    char sSqlStr[1024] = {0}, sDesc[100] = {0};
    char sECardNo[128 + 1] = {0};
    char iRet = 0;

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "transmit_time", sTransmitTime)

    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL)
    {
        tLog(INFO, "此规则针对交易[%s]不做限制", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则7执行中...");
    OCI_Resultset *pstRes = NULL;

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(DEBUG, "加密卡号数据[%s]", sECardNo);

    /*BUG ！！！！时间戳transmit_time 无年份 所以此sql 遇到跨年计算时间时 会出错*/
    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from b_pos_trans_detail \
                    where card_type = '0' and merch_id = '%s' and card_no_encrypt = '%s' \
                    and transmit_time between (select substr( to_char(((to_number(to_date(trans_date||trans_time,'yyyymmddhh24miss') \
                                                    - to_date('19700101000000', 'yyyymmddhh24miss'))*24*60*60 - 60)/60/60/24) \
                                                    + to_date('19700101000000', 'yyyymmddhh24miss'), 'yyyymmddhh24miss'), 5, 10 ) \
                                               from b_pos_trans_detail where transmit_time = '%s' and card_no_encrypt = '%s') \
                    and '%s' and resp_code='00'", sMerchId, sECardNo, sTransmitTime, sECardNo, sTransmitTime);

    iRet = tQueryCount(sSqlStr);
    if (iRet < 0)
    {
        tLog(DEBUG, "sSqlStr[%s]", sSqlStr);
        ErrHanding(pstJson, "96", "获取交易交易次数失败");
        return ( -1);
    }

    if (iRet >= stRiskRule->iTransCnt)
    {
        /*冻结账户*/
        tLog(DEBUG, "借记卡[%s]在商户[%s]1分钟内交易笔次[%d],冻结账户!", sCardNo, sMerchId, iRet + 1); /*iRet+1 将本次交易计算在内*/
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }
    return 0;
}

/*8、银行返回码包含：04、34、41、54、57、62 预警记录， 注意：返回交易流中 无需限制具体交易*/
int RuleCodefunc8(cJSON * pstJson, RiskRule *stRiskRule) {
    //char sTransCode[6 + 1] = {0}, sIsRespCode[3 + 1] = {0};
    char sIsRespCode[3 + 1] = {0};
    int iRet = 0;

    //GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "istresp_code", sIsRespCode);

    /*返回交易流中 无需限制具体交易*/
    /*
    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL) {
        tLog(INFO, "此规则针对交易[%s]不做限制", sTransCode);
        return 0;
    }
     */
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则8执行中...");
    if (sIsRespCode[0] == '\0')
    {
        tLog(INFO, "请求交易流中，不触发此风控规则!");
    } else if (sIsRespCode[0] != '\0' && strstr(stRiskRule->sRespCode, sIsRespCode))
    {
        /*预警记录*/
        tLog(DEBUG, "银行返回[%s],预警记录!", sIsRespCode);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    } else
    {
        tLog(INFO, "银联返回码为[%s]，不触发此风控规则!", sIsRespCode);
    }

    return 0;
}

/*9、银行返回码包含：04、34、41、54、57、62 当天单商户2次报出冻结账户 注意：返回交易流中 无需限制具体交易 
 * motify by GuoJiaQing at 20170915   
 * Modify Content: 修改此规则 适用于磁条卡。 */
int RuleCodefunc9(cJSON * pstJson, RiskRule *stRiskRule) {
    //char sTransCode[6 + 1] = {0}, sIsRespCode[3 + 1] = {0}, sMerchId[15+1] = {0};
    char sIsRespCode[3 + 1] = {0}, sMerchId[15 + 1] = {0}, sInputMode[3 + 1] = {0};
    char sSqlStr[1024] = {0};
    int iRet = 0, iCnt = 0;

    //GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "istresp_code", sIsRespCode);
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "input_mode", sInputMode);


    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则9执行中...");
    if (sIsRespCode[0] == '\0')
    {
        tLog(INFO, "请求交易流中，不触发此风控规则!");
    } else if (sIsRespCode[0] != '\0' && strstr(stRiskRule->sRespCode, sIsRespCode))
    {
        snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
                        MERCH_ID='%s' AND ISTRESP_CODE in ('04','34','41','54','57','62') AND SUBSTR(INPUT_MODE,1,2) = '02'", sMerchId);
        iCnt = tQueryCount(sSqlStr);
        if (iCnt < 0)
        {
            ErrHanding(pstJson, "96", "获取次数失败");
            return -1;
        }
        /*单个商户银联返回码： 04、34、41、54、57、62 已有一条交易记录 本次为第二次银联返回报错*/
        if (sInputMode[0] == '0' && sInputMode[1] == '2')
        {
            tLog(DEBUG, "sInputMode = [%s]", sInputMode);
            iCnt = iCnt + 1;
        }
        if (iCnt >= stRiskRule->iTransCnt)
        {
            /*冻结账户*/
            tLog(DEBUG, "商户[%s]交易银行返回[%s][%d]次,冻结账户!", sMerchId, sIsRespCode, iCnt);
            iRet = RiskOpt(pstJson, stRiskRule);
            if (iRet < 0)
            {
                tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
            } else if (iRet == 1)
            {
                ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
                return -1;
            }
        }
    } else
    {
        tLog(INFO, "银联返回码为[%s]，不触发此风控规则!", sIsRespCode);
    }

    return 0;
}

/* 10、同商户，撤销大于2次冻结账户*/
int RuleCodefunc10(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sSqlStr[1024] = {0}, sDesc[100] = {0};
    int iCnt = 0, iRet = 0;

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);

    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL)
    {
        tLog(INFO, "此规则针对交易[%s]不做限制", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则10执行中...");

    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from b_pos_trans_detail where merch_id = '%s' and trans_code = '%s' and resp_code='00'",
            sMerchId, sTransCode);

    iCnt = tQueryCount(sSqlStr);
    if (iCnt < 0)
    {
        tLog(DEBUG, "sSqlStr[%s]", sSqlStr);
        ErrHanding(pstJson, "96", "获取交易撤销次数失败");
        return ( -1);
    }

    if (iCnt >= stRiskRule->iTransCnt)
    {
        /*冻结账户*/
        tLog(DEBUG, "商户[%s]撤销笔次[%d],冻结账户!", sMerchId, iCnt);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }

    return 0;
}

/* 11、 当天，同商户，同卡，先查询，再消费 预警  R011*/

/*
 * insert into  B_RISK_RULE (rule_code,rule_name,status,trans_code,interval,IS_FROZEN)
 values ('R011','当天同卡同商户先查询再消费报警','1','020000,T20000,024100',1,'0');
 */
int RuleCodefunc11(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0}, sCardNo[19 + 1] = {0}, sTransCode[6 + 1] = {0}, sTransTime[6 + 1] = {0};
    char sECardNo[128 + 1] = {0}, sKey[128 + 1] = {0};
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    int iSec = 0;
    int iRet = 0;

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "trans_time", sTransTime);

    tTrim(sTransCode);
    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL)
    {
        tLog(INFO, "此规则针对交易[%s]不做限制", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则11执行中...");

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(DEBUG, "加密卡号数据[%s]", sECardNo);
    snprintf(sSqlStr, sizeof (sSqlStr), " select * from ( \
                    select to_number(to_date('%s','hh24miss') - to_date(trans_time,'hh24miss'))*24*60*60 from b_pos_trans_detail \
                    where merch_id = '%s' and card_no_encrypt = '%s' and trans_code = '021000' \
                    and trans_time < '%s' order by trans_time desc \
             ) where rownum = 1", sTransTime, sMerchId, sECardNo, sTransTime); /*计算出时间差单位为秒*/

    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        ErrHanding(pstJson, "96", "查找数据出错");
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        ErrHanding(pstJson, "96", "查找数据出错");
        return -1;
    }
    while (OCI_FetchNext(pstRes))
    {
        INTV(pstRes, 1, iSec);
    }

    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return 0;
    }

    tReleaseRes(pstRes);
    tLog(DEBUG, "当前交易[%s]与其上一笔查询交易间隔[%d]秒", sTransCode, iSec);

    if (iSec < (stRiskRule->iInterval)*60)
    {

        tLog(DEBUG, "当前交易[%s]与其上一笔查询交易间隔[%d]秒，在预警间隔时间[%d]分钟内", sTransCode, iSec, stRiskRule->iInterval);
        /*预警记录*/
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }

    return ( 0);
}

/* 12、同磁条卡，同商户，1分钟内连续交易（消费，预授权完成）冻结账户 RO12*/

/*
 * insert into  B_RISK_RULE (rule_code,rule_name,status,trans_code,trans_cnt,IS_FROZEN)
 values ('R012','同磁条卡同商户1分钟内连续交易冻结账户','1','020000,T20000,024100',2,'1')
 */
int RuleCodefunc12(cJSON * pstJson, RiskRule *stRiskRule) {
    char sKey[128 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sCardNo[19 + 1] = {0}, sTransCode[6 + 1] = {0}, sTransmitTime[10 + 1] = {0};
    char sChannelMerchId[15 + 1] = {0}, sChannelId[8 + 1] = {0};
    char sSqlStr[1024] = {0}, sDesc[100] = {0};
    char sECardNo[128 + 1] = {0},sBusiType[2 + 1]={0};
    char iRet = 0;

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "transmit_time", sTransmitTime);
    GET_STR_KEY(pstJson, "channel_id", sChannelId);
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
    GET_STR_KEY(pstJson, "busi_type",sBusiType);

    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL)
    {
        tLog(INFO, "此规则针对交易[%s]不做限制", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则12执行中...");
    /*通卡连刷 对生活圈商户不做规则限制 add by Guo JiaQing at 20170919  BENGIN*/


    if (memcmp(sBusiType, "02", 2))
    {
        if (memcmp(sMerchId, sChannelMerchId, 15))
        {
            tLog(INFO, "[%s]属于生活圈商户", sMerchId);
            /*判断 此规则对生活圈商户是否限制 0 - 不限制，1 - 限制*/
            if (!memcmp(stRiskRule->sIsLife, "0", 1))
            {
                tLog(INFO, "生活圈商户,此规则对其不做限制,此风控不触发！", sMerchId);
                return ( 0);
            }
        }
    }
    /* add by Guo JiaQing at 20170918  END */

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(DEBUG, "加密卡号数据[%s]", sECardNo);

    /*BUG ！！！！时间戳transmit_time 无年份 所以此sql 遇到跨年计算时间时 会出错*/
    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from b_pos_trans_detail \
                    where substr(input_mode,1,2) = '02' and merch_id = '%s' and card_no_encrypt = '%s' \
                    and transmit_time between (select substr( to_char(((to_number(to_date(trans_date||trans_time,'yyyymmddhh24miss') \
                                                    - to_date('19700101000000', 'yyyymmddhh24miss'))*24*60*60 - 60)/60/60/24) \
                                                    + to_date('19700101000000', 'yyyymmddhh24miss'), 'yyyymmddhh24miss'), 5, 10 ) \
                                               from b_pos_trans_detail where transmit_time = '%s' and card_no_encrypt = '%s') \
                    and '%s' and resp_code='00'", sMerchId, sECardNo, sTransmitTime, sECardNo, sTransmitTime);

    iRet = tQueryCount(sSqlStr);
    if (iRet < 0)
    {
        tLog(DEBUG, "sSqlStr[%s]", sSqlStr);
        ErrHanding(pstJson, "96", "获取交易交易次数失败");
        return ( -1);
    }

    if (iRet >= stRiskRule->iTransCnt)
    {
        /*冻结账户*/
        tLog(DEBUG, "磁条卡[%s]在商户[%s]1分钟内交易笔次[%d],冻结账户!", sCardNo, sMerchId, iRet + 1); /*iRet+1 将本次交易计算在内*/
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }
    return 0;
}

/*13、银行卡黑名单*/
int RuleCodefunc13(cJSON * pstJson, RiskRule *stRiskRule) {
    int iRet = 0;
    char sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0}, sSql[128] = {0};
    char sKey[128 + 1] = {0}, sTransCode[6 + 1] = {0};

    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(ERROR, "加密卡号数据[%s]", sECardNo);
    /* 银行卡黑名单   */
    sprintf(sSql, "SELECT COUNT(1) FROM B_RISK_BLACK_CARD WHERE STATUS='1' AND CARD_NO_ENCRYPT='%s'", sECardNo);
    iRet = tQueryCount(sSql);
    if (iRet > 0)
    {
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "62", "不受理此卡[%s].", sCardNo);
            return -1;
        }

    }
    //tLog(INFO, "卡号[%s]黑名单检查通过.", sCardNo);
    return 0;
}

/*14、生活圈商户判断*/
int RuleCodefunc14(cJSON * pstJson, RiskRule *stRiskRule) {
    char sTransCode[6 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sChannelMerchId[15 + 1] = {0},sBusiType[2 + 1]={0};
    char sMerchCityCode[4 + 1] = {0}, sMerchPCityCode[4 + 1] = {0}, sChMerchCityCode[4 + 1] = {0}, sChMerchPCityCode[4 + 1] = {0};
    char sMerchCityName[50] = {0}, sMerchPCityName[50] = {0}, sChMerchCityName[50] = {0}, sChMerchPCityName[50] = {0}, sInfo[100] = {0}, sChannelId[8 + 1] = {0};
    int iRet = 0, iRet1 = 0;
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
    GET_STR_KEY(pstJson, "channel_id", sChannelId);
    GET_STR_KEY(pstJson, "busi_type",sBusiType);

    if (!memcmp(sBusiType, "02", 2))
    {
        tLog(INFO, "商户是浦发商户不判断生活圈");
        return 0;
    }

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    if (!memcmp(sMerchId, sChannelMerchId, 15))
    {
        tLog(INFO, "不属于生活圈商户,不判断该规则");
        return 0;
    }

    {/* 逻辑判断 */

        iRet1 = FindCityLevel(sMerchId);
        if (iRet1 < 0)
        {
            ErrHanding(pstJson, "96", "获取数据失败");
            return -1;
        }

        /*查询商户市区码和省级码*/
        if (FindCityCode(sMerchId, sMerchCityCode, sMerchPCityCode, sMerchCityName, sMerchPCityName, iRet1) < 0)
        {
            ErrHanding(pstJson, "96", "获取数据失败");
            return -1;
        }
        /*查询生活圈商户市区码和省级码*/
        if (FindLifeCityCode(sChannelMerchId, sChMerchCityCode, sChMerchPCityCode, sChMerchCityName, sChMerchPCityName) < 0)
        {
            ErrHanding(pstJson, "96", "获取数据失败");
            return -1;
        }
        tLog(INFO, "商户[%s]:市[%s]地区码[%s]，省[%s]地区码[%s]，生活圈商户[%s]：市[%s]地区码[%s]，省[%s]地区码[%s]",
                sMerchId, sMerchCityName, sMerchCityCode, sMerchPCityName, sMerchPCityCode, sChannelMerchId, sChMerchCityName, sChMerchCityCode, sChMerchPCityName, sChMerchPCityCode);
        sprintf(sInfo, "商户：市[%s]省[%s]，生活圈商户：市[%s]，省[%s]",
                sMerchCityName, sMerchPCityName, sChMerchCityName, sChMerchPCityName);
        strcpy(stRiskRule->sRuleName, sInfo);
        if (!memcmp(stRiskRule->sLifeControl, "0", 1))
        {
            tLog(INFO, "生活圈不限制省市限制");
            return 0;
        } else if (!memcmp(stRiskRule->sLifeControl, "2", 1))
        {
            tLog(INFO, "生活圈商户限制同市");
            if (memcmp(sMerchCityCode, sChMerchCityCode, 4))
            {
                iRet = RiskOpt(pstJson, stRiskRule);
                if (iRet < 0)
                {
                    tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
                } else if (iRet == 1)
                {
                    ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
                    return -1;
                }
            }
        } else if (!memcmp(stRiskRule->sLifeControl, "1", 1))
        {
            tLog(INFO, "生活圈商户限制同省");
            if (memcmp(sMerchPCityCode, sChMerchPCityCode, 4))
            {
                iRet = RiskOpt(pstJson, stRiskRule);
                if (iRet < 0)
                {
                    tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
                } else if (iRet == 1)
                {
                    ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
                    return -1;
                }
            }
        }
    }

    return 0;
}

/*16、单商户单卡一小时内连续密码错误两次冻结账户  */

/* motify by GuoJiaQing at 20170915   
 * Modify Content: 修改此规则为: 单商户单卡在一小时内：连续密码错误两次（第三次交易在一小时之内）
 * 第三次任意交易冻结账户（适用于磁条卡。）*/
int RuleCodefunc16(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0};
    char sSqlStr[1024] = {0}, sRuleCode[100] = {0}, sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0};
    char sKey[128 + 1] = {0}, sTermId[8 + 1] = {0}, sTransmitTime[10 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDate[8 + 1] = {0};
    int iRet = 0;
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "term_id", sTermId);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "transmit_time", sTransmitTime);
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    GET_STR_KEY(pstJson, "trans_date", sTransDate);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "风控规则R016执行中...");

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }

    /*单商户单卡一小时内连续密码错误两次冻结账户*/
    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from b_pos_trans_detail \
                    where  merch_id = '%s' and card_no_encrypt = '%s' and substr(input_mode,1,2) = '02'\
                    and trans_date||trans_time between (select to_char(((to_number(to_date(trans_date||trans_time,'yyyymmddhh24miss') \
                                                    - to_date('19700101000000', 'yyyymmddhh24miss'))*24*60*60 - 3600)/60/60/24) \
                                                    + to_date('19700101000000', 'yyyymmddhh24miss'), 'yyyymmddhh24miss') \
                                               from b_pos_trans_detail where transmit_time = '%s' and card_no_encrypt = '%s') \
                    and '%s%s' and istresp_code='%s'", sMerchId, sECardNo, sTransmitTime, sECardNo, sTransDate, sTransTime, stRiskRule->sRespCode);
    tLog(DEBUG, "SQL[%s]", sSqlStr);
    iRet = tQueryCount(sSqlStr);
    if (iRet < 0)
    {
        tLog(ERROR, "sSqlStr[%s]", sSqlStr);
        ErrHanding(pstJson, "96", "获取交易交易次数失败");
        return ( -1);
    }

    if (iRet >= stRiskRule->iTransCnt)
    {
        tLog(INFO, "此卡[%s]在商户[%s]的pos交易中，一小时内连续 <密码错误> [%d]次,触发风控规则!", sCardNo, sMerchId, iRet);
        tLog(DEBUG, "%s", stRiskRule->sRuleName);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }

    return 0;
}

/*17、MCC等于6012的非借记卡交易，冻结账户*/
int RuleCodefunc17(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0};
    char sSqlStr[512] = {0}, sRuleCode[100] = {0}, sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0};
    char sKey[128 + 1] = {0}, sTermId[8 + 1] = {0}, sMcc[4 + 1], sCardType[1 + 1];
    int iRet = 0;
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "term_id", sTermId);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "mcc", sMcc);
    GET_STR_KEY(pstJson, "card_type", sCardType);


    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    /*MCC等于6012的非借记卡交易，冻结账户*/
    tLog(DEBUG, "风控规则R017执行中...");
    if (!memcmp(stRiskRule->sMcc, sMcc, 4))
    {
        tLog(INFO, "商户MCC属于[%s]", stRiskRule->sMcc);
        if (memcmp(sCardType, "0", 1))
        {
            tLog(INFO, "该卡非借记卡触发风控");
            iRet = RiskOpt(pstJson, stRiskRule);
            if (iRet < 0)
            {
                tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
            } else if (iRet == 1)
            {
                ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
                return -1;
            }
        }
    }
    return 0;

}

/*18、单卡余额不足两次及以上(报2次余额不足，第三次任何交易都触发此风控) 触发风控
 * motify by GuoJiaQing at 20170915   Modify Content: 修改此规则 适用于磁条卡。 */
int RuleCodefunc18(cJSON * pstJson, RiskRule *stRiskRule) {
    char sKey[128 + 1] = {0};
    char sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sSqlStr[1024] = {0}, sDesc[100] = {0};
    int iRet = 0, iCnt = 0;

    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "此规则针对交易[%s]不做限制", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则18执行中...");
    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(DEBUG, "加密卡号数据[%s]", sECardNo);

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
                    CARD_NO_ENCRYPT='%s' AND ISTRESP_CODE = '%s' AND SUBSTR(INPUT_MODE,1,2) = '02'", sECardNo, stRiskRule->sRespCode);
    iCnt = tQueryCount(sSqlStr);
    if (iCnt < 0)
    {
        ErrHanding(pstJson, "96", "获取次数失败");
        return -1;
    }

    if (iCnt >= stRiskRule->iTransCnt)
    {
        tLog(DEBUG, "此卡[%s]交易返回 <可用余额不足> [%d]次，触发此风控规则!", sCardNo, iCnt);
        tLog(DEBUG, "%s", stRiskRule->sRuleName);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }

    return ( 0);
}

/*  19、异常时间段交易大额报警 */
int RuleCodefunc19(cJSON * pstJson, RiskRule *stRiskRule) {
    char sTransTime[6 + 1] = {0}, sTransCode[6 + 1] = {0};
    int iAmount = 0, iRet = 0;
    double dRkAmount = 0.00;

    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    GET_INT_KEY(pstJson, "amount", iAmount);
    dRkAmount = stRiskRule->dTransAmt;

    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL)
    {
        tLog(INFO, "此规则针对交易[%s]不做限制", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "风控规则19执行中...");
    /* 请求交易流中 异常时间段 交易预警记录*/
    if (memcmp(sTransTime, stRiskRule->sStartTime, 6) >= 0 || memcmp(sTransTime, stRiskRule->sEndTime, 6) <= 0)
    {
        if (iAmount >= (int) (dRkAmount * 100))
        {
            tLog(DEBUG, "交易时间[%s]为异常交易时间且交易金额为[%.02f]元大于等于[%.02f]元，触发此风控规则", sTransTime, (double) (iAmount / 100), dRkAmount);
            tLog(DEBUG, "%s", stRiskRule->sRuleName);
            iRet = RiskOpt(pstJson, stRiskRule);
            if (iRet < 0)
            {
                tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
            } else if (iRet == 1)
            {
                ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
                return -1;
            }
        } else
        {
            tLog(DEBUG, "交易时间[%s]为异常交易时间段且交易金额小于[%.2lf]元,不触发此风控规则!", sTransTime, dRkAmount);
        }
    } else
    {
        tLog(INFO, "非异常时间段，不触发此风控规则");
    }

    return ( 0);
}

/*单商户1小时内 密码错误次数≥3，密码错误银行卡张数≥2，(针对磁条卡交易)
 * 再次进行任意交易冻结账户（本次交易在一小时之内） add by guojiaqing  */
int RuleCodefunc20(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0}, sRrn[RRN_LEN + 1] = {0};
    char sSqlStr[1024] = {0}, sRuleCode[100] = {0}, sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0};
    char sKey[128 + 1] = {0}, sTransmitTime[10 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDate[8 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    int iRet = 0;
    int iCnt = 0, iTmp = 0; /*密码错误次数*/
    int iNum = 0; /*密码错误银行卡张数*/

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "transmit_time", sTransmitTime);
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    GET_STR_KEY(pstJson, "trans_date", sTransDate);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "交易[%s]不做判断", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "无效的风控规则,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "风控规则R020执行中...");


    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1),card_no_encrypt from b_pos_trans_detail "
            "where merch_id = '%s' and substr(input_mode,1,2) = '02' and istresp_code = '%s' "
            "and trans_date||trans_time between (select to_char(((to_number(to_date(trans_date||trans_time,'yyyymmddhh24miss') "
            "- to_date('19700101000000', 'yyyymmddhh24miss'))*24*60*60 - 3600)/60/60/24) "
            "+ to_date('19700101000000', 'yyyymmddhh24miss'), 'yyyymmddhh24miss') "
            "from b_pos_trans_detail where transmit_time = '%s' and rrn = '%s') "
            "and '%s%s' group by card_no_encrypt", sMerchId, stRiskRule->sRespCode, sTransmitTime, sRrn, sTransDate, sTransTime);
    tLog(DEBUG, "SQL[%s]", sSqlStr);

    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        ErrHanding(pstJson, "96", "查找数据出错");
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        ErrHanding(pstJson, "96", "查找数据出错");
        return -1;
    }
    while (OCI_FetchNext(pstRes))
    {
        INTV(pstRes, 1, iTmp);
        iCnt += iTmp;
        iNum++;
    }

    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return 0;
    }

    tReleaseRes(pstRes);

    tLog(DEBUG, "密码错误次数[%d],密码错误银行卡张数[%d]", iCnt, iNum);

    if (iCnt >= stRiskRule->iTransCnt && iNum >= stRiskRule->iQueueCnt)
    {
        tLog(INFO, "此在商户[%s]的pos交易中，一小时内连续 <密码错误> [%d]次 且 <密码错误银行卡张数> [%d] 张,触发风控规则!", sMerchId, iCnt, iNum);
        tLog(DEBUG, "%s", stRiskRule->sRuleName);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "风控规则[%s]操作动作执行失败!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "触发风控规则，禁止交易");
            return -1;
        }
    }

    return ( 0);
}

int UpdMerchStatus(char *pcUserCode, char *pcStatus) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "update b_merch set status='%s' where user_code='%s'", pcStatus, pcUserCode);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tLog(INFO, "商户[%s]状态[%s]更新成功.", pcUserCode, pcStatus);
    tReleaseRes(pstRes);
    return 0;
}

int InstRiskLog(cJSON *pstTransJson, char *pcRuleName) {
    char sSqlStr[512] = {0}, sRrn[RRN_LEN + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    char sUserCode[15 + 1] = {0};
    char sStatus[2] = {0};
    int iCnt = 0;

    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);

    //如果已经冻结账户，则直接返回，不做处理
    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from B_BALANCE_CONTROL where user_code = '%s' and WALLET_TYPE = 'all' and STATUS='1'",
            sUserCode);

    iCnt = tQueryCount(sSqlStr);
    if (iCnt > 0)
    {
        tLog(INFO, "商户[%s]已冻结账户,不再处理.", sUserCode);
        return 0;
    }

    memset(sSqlStr, 0, sizeof (sSqlStr));
    snprintf(sSqlStr, sizeof (sSqlStr), "insert into B_BALANCE_CONTROL(USER_CODE,REMARK,status,wallet_type,frozen_type,opt_no)"
            " values('%s','触发风控规则[%s:%s],系统自动账户冻结.','1','all','2','%s')", sUserCode, sRrn, pcRuleName, sRrn);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tLog(INFO, "商户[%s]账户冻结记录添加成功.", sUserCode);
    tReleaseRes(pstRes);
    return 1;
}

int MerchFrozen(cJSON * pstTransJson) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sMerchId[15 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sUserCode[15 + 1] = {0}, sResvDesc[4096 + 1] = {0}, sRespCode[2 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson)
    {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    /* 新版本接口
    SET_STR_KEY(pstNetJson, "walletType", "0");
    SET_STR_KEY(pstNetJson, "frozenStatus", "1");
    SET_STR_KEY(pstNetJson, "frozenBusinessType", "1");
    SET_STR_KEY(pstNetJson, "trans_code", "00TB00");
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_STR_KEY(pstNetJson, "optNo", sRrn);
    SET_STR_KEY(pstNetJson, "merch_id", sMerchId);
    SET_STR_KEY(pstNetJson, "user_code", sUserCode);
    SET_STR_KEY(pstNetJson, "frozenRemark", "触发风控规则,系统自动账户冻结.");
     * */
    //老版本接口
    SET_STR_KEY(pstNetJson, "trans_code", "00TB00");
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_STR_KEY(pstNetJson, "merch_id", sMerchId);
    SET_STR_KEY(pstNetJson, "user_code", sUserCode);
    SET_STR_KEY(pstNetJson, "frozenType", "2");

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00TB00%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("00TB00_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0)
    {
        if (MSG_TIMEOUT == iRet)
        {
            tLog(ERROR, "交易[%s]账户冻结通知超时.", sRrn);
        } else
            tLog(ERROR, "交易[%s]账户冻结通知失败.", sRrn);
        cJSON_Delete(pstNetJson);
        return -1;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
    if (memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "账户冻结失败[%s:%s].", sRespCode, sResvDesc);
    } else
        tLog(INFO, "交易[%s]账户冻结成功.", sRrn);
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);
    return 0;
}

int InstRiskMerch(cJSON *pstNetTran, char * pcDesc, char *pcCode) {

    char sSqlStr[512] = {0}, sMerchId[15 + 1] = {0};
    char sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0};
    char sRrn[12 + 1] = {0}, sAmount[12 + 1] = {0}, sMerchName[60 + 1] = {0};
    char sCardNoEnc[255 + 1] = {0}, sKeyName[255 + 1] = {0};
    double dAmount = 0;
    OCI_Resultset *pstRes = NULL;
    GET_STR_KEY(pstNetTran, "merch_id", sMerchId);
    GET_STR_KEY(pstNetTran, "trans_code", sTransCode);
    GET_STR_KEY(pstNetTran, "card_no", sCardNo);
    GET_STR_KEY(pstNetTran, "trans_date", sTransDate);
    GET_STR_KEY(pstNetTran, "trans_time", sTransTime);
    GET_STR_KEY(pstNetTran, "rrn", sRrn);
    GET_STR_KEY(pstNetTran, "merch_name", sMerchName);
    GET_DOU_KEY(pstNetTran, "amount", dAmount);
#if 0

    /* kms 加密存储 ,原字段屏蔽显示 */
    if (FindValueByKey(sKeyName, "kms.encdata.key") < 0)
    {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
    } else
    {
        if (sCardNo[0] != '\0')
        {
            if (tHsm_Enc_Asc_Data(sCardNoEnc, sKeyName, sCardNo) < 0)
                tLog(ERROR, "卡号加密失败.");
            else
                tLog(DEBUG, "卡号加密成功.");
        }
    }
    if (sCardNo[0] != '\0')
        tScreenStr(sCardNo, strlen(sCardNo), '*', 6, 4);

#endif
    sprintf(sAmount, "%.2f", dAmount / 100);

    snprintf(sSqlStr, sizeof (sSqlStr), "insert into b_risk_merch (TRANS_CODE,TRANS_DATE,TRANS_TIME,CARD_NO, \
                      AMOUNT,MERCH_ID,MERCH_NAME,RRN,DESCRIBE,RULE_CODE ) VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')", \
                 sTransCode, sTransDate, sTransTime, sCardNo, sAmount, sMerchId, sMerchName, sRrn, pcDesc, pcCode);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tLog(INFO, "风险商户预警记录添加成功");
    tReleaseRes(pstRes);
    return 0;
}

int Rulefunction(cJSON *pstJson, char * pcRuleCode) {

    char sCode[3 + 1] = {0}, sRuleCode[4 + 1] = {0};
    int iCode = 0, iRet = 0;
    RiskRule stRiskRule;

    /* 查找风控规则 */
    //tLog(INFO, "风控规则[%s]", pcRuleCode);
    tStrCpy(sRuleCode, pcRuleCode, 4);
    if (FindCodeRule(&stRiskRule, sRuleCode) < 0)
    {
        tLog(INFO, "未找到风控规则[%s]", sRuleCode);
        return 0;
    }

    memcpy(sCode, pcRuleCode + 1, 3);
    iCode = atoi(sCode);
    switch (iCode)
    {
            /*风控处理函数 */
        case 1:
        case 2:
        case 15:
            iRet = RuleCodefunc1(pstJson, &stRiskRule);
            return iRet;
        case 3:
            iRet = RuleCodefunc3(pstJson, &stRiskRule);
            return iRet;
        case 4:
            iRet = RuleCodefunc4(pstJson, &stRiskRule);
            return iRet;
        case 5:
            iRet = RuleCodefunc5(pstJson, &stRiskRule);
            return iRet;
        case 6:
            iRet = RuleCodefunc6(pstJson, &stRiskRule);
            return iRet;
        case 7:
            iRet = RuleCodefunc7(pstJson, &stRiskRule);
            return iRet;
        case 8:
            iRet = RuleCodefunc8(pstJson, &stRiskRule);
            return iRet;
        case 9:
            iRet = RuleCodefunc9(pstJson, &stRiskRule);
            return iRet;
        case 10:
            iRet = RuleCodefunc10(pstJson, &stRiskRule);
            return iRet;
        case 11:
            iRet = RuleCodefunc11(pstJson, &stRiskRule);
            return iRet;
        case 12:
            iRet = RuleCodefunc12(pstJson, &stRiskRule);
            return iRet;
        case 13:
            iRet = RuleCodefunc13(pstJson, &stRiskRule);
            return iRet;
        case 14:
            iRet = RuleCodefunc14(pstJson, &stRiskRule);
            return iRet;
        case 16:
            iRet = RuleCodefunc16(pstJson, &stRiskRule);
            return iRet;
        case 17:
            iRet = RuleCodefunc17(pstJson, &stRiskRule);
            return iRet;
        case 18:
            iRet = RuleCodefunc18(pstJson, &stRiskRule);
            return iRet;
        case 19:
            iRet = RuleCodefunc19(pstJson, &stRiskRule);
            return iRet;
        case 20:
            iRet = RuleCodefunc20(pstJson, &stRiskRule);
            return iRet;

        default:
            tLog(INFO, "未找到风控规则处理程序...");
            break;

    }

    return 0;
}

int ChkRiskRule(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sRuleCode[200] = {0};
    char sStore[4 + 1] = {0}, *pcTemp = NULL;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    /* 查找商户开通的规则 */
    if (FindMerchRule(sRuleCode, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的风控规则.", sMerchId);
        return -1;
    }
    tLog(DEBUG, "商户[%s]风控规则码[%s]", sMerchId, sRuleCode);

    pcTemp = sRuleCode;
    while (pcTemp)
    {
        tCutOneField(&pcTemp, sStore);
        tLog(DEBUG, "执行规则[%s]..", sStore);
        if (Rulefunction(pstTransJson, sStore) < 0)
        {
            tLog(ERROR, "执行规则[%s]失败.", sStore);
            return -1;
        }
        tLog(DEBUG, "规则[%s]执行结束", sStore);
    }
    return 0;
}

/*银行返回时，风控检查*/
int RespChkRiskRule(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sRuleCode[200] = {0}, sRespCode[RESP_CODE_LEN + 1] = {0};

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    /* 查找商户开通的规则 */
    if (FindMerchRule(sRuleCode, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, sRespCode, "未找到商户[%s]的风控规则,忽略.", sMerchId);
        return 0;
    }
    tLog(DEBUG, "商户[%s]风控规则码[%s].", sMerchId, sRuleCode);

    if (strstr(sRuleCode, "R008"))
    {
        tLog(DEBUG, "银行返回，执行规则[R008]..");
        Rulefunction(pstTransJson, "R008");
        tLog(DEBUG, "规则[ROO8]执行结束");
    }
    if (strstr(sRuleCode, "R009"))
    {
        tLog(DEBUG, "银行返回，执行规则[R009]..");
        Rulefunction(pstTransJson, "R009");
        tLog(DEBUG, "规则[R009]执行结束");
    }

    return ( 0);
}

int RiskOpt(cJSON *pstTransJson, RiskRule *pstRiskRule) {
    char sUserCode[15 + 1] = {0};
    char sStatus[2] = {0};
    int iRet = -1;

    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    //商户注销
    if (pstRiskRule->sIsDel[0] == '1')
    {
        sStatus[0] = 'X';
        if (UpdMerchStatus(sUserCode, sStatus) < 0)
        {
            tLog(ERROR, "商户[%s]注销操作失败.", sUserCode);
            return -1;
        }
    }
    //商户冻结
    if (pstRiskRule->sIsStop[0] == '1')
    {
        sStatus[0] = '0';
        if (UpdMerchStatus(sUserCode, sStatus) < 0)
        {
            tLog(ERROR, "商户[%s]交易冻结操作失败.", sUserCode);
            return -1;
        }
    }
    //账户冻结
    if (pstRiskRule->sIsFrozen[0] == '1')
    {
        //插入风控记录
        iRet = InstRiskLog(pstTransJson, pstRiskRule->sRuleName);
        if (iRet < 0)
        {
            tLog(ERROR, "商户[%s]账户冻结记录添加失败.", sUserCode);
            return -1;
        }
        if (iRet > 0 && MerchFrozen(pstTransJson) < 0)
        {
            tLog(ERROR, "商户[%s]账户冻结操作失败.", sUserCode);
            return -1;
        }
    }
    //预警记录
    if (pstRiskRule->sIsWarnning[0] == '1')
    {
        if (InstRiskMerch(pstTransJson, pstRiskRule->sRuleName, pstRiskRule->sRuleCode) < 0)
        { /*motify by GJQ add 20171205*/
            tLog(ERROR, "商户[%s]预警记录操作失败.", sUserCode);
            return -1;
        }
    }

    if (pstRiskRule->sTransControl[0] == '1')
    {
        tLog(INFO, "触发风控规则，禁止交易");
        return 1;
    }

    return 0;
}
