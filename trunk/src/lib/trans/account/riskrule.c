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

int Rulefunction(cJSON *pstJson, char * pcRuleCode) {

    char sCode[3 + 1] = {0}, sRuleCode[4 + 1] = {0};
    int iCode = 0, iRet = 0;
    RiskRule stRiskRule;

    /* 查找风控规则 */
    tLog(INFO, "风控规则【%s】", pcRuleCode);
    tStrCpy(sRuleCode, pcRuleCode, 4);
    if (FindCodeRule(&stRiskRule, sRuleCode) < 0) {
        tLog(INFO, "未找到风控规则[%s]", sRuleCode);
        return 0;
    }

    memcpy(sCode, pcRuleCode + 1, 3);
    iCode = atoi(sCode);
    switch (iCode) {
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

int ChkRiskRule(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sRuleCode[200] = {0};
    char sStore[4 + 1] = {0}, *pcTemp = NULL;
    int iRet = 0;
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    /* 查找商户开通的规则 */
    if (FindMerchRule(sRuleCode, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的风控规则.", sMerchId);
        return -1;
    }
    tLog(INFO, "商户[%s]风控规则码[%s]", sMerchId, sRuleCode);

    pcTemp = sRuleCode;
    while (pcTemp) {
        tCutOneField(&pcTemp, sStore);
        tLog(INFO, "执行规则[%s]..", sStore);
        if (Rulefunction(pstTransJson, sStore) < 0) {
            return -1;
        }
        tLog(INFO, "规则[%s]执行结束", sStore);
    }
    return 0;
}

/*银行返回时，风控检查*/
int RespChkRiskRule(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sRuleCode[200] = {0};

    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    /* 查找商户开通的规则 */
    if (FindMerchRule(sRuleCode, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的风控规则.", sMerchId);
        return -1;
    }
    tLog(INFO, "商户[%s]风控规则码[%s]", sMerchId, sRuleCode);

    if (strstr(sRuleCode, "R008")) {
        tLog(INFO, "银行返回，执行规则[R008]..");
        Rulefunction(pstTransJson, "R008");
        tLog(INFO, "规则[ROO8]执行结束");
    }
    if(strstr(sRuleCode, "R009")) {
        tLog(INFO, "银行返回，执行规则[R009]..");
        Rulefunction(pstTransJson, "R009");
        tLog(INFO, "规则[R009]执行结束");
    }
    
    return ( 0);
}

int RiskOpt(cJSON *pstTransJson, RiskRule *pstRiskRule) {
    char sUserCode[15 + 1] = {0};
    char sStatus[2] = {0};
    int iRet = -1;

    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    //商户注销
    if (pstRiskRule->sIsDel[0] == '1') {
        sStatus[0] = 'X';
        if (UpdMerchStatus(sUserCode, sStatus) < 0) {
            tLog(ERROR, "商户[%s]注销操作失败.", sUserCode);
            return -1;
        }
    }
    //商户冻结
    if (pstRiskRule->sIsStop[0] == '1') {
        sStatus[0] = '0';
        if (UpdMerchStatus(sUserCode, sStatus) < 0) {
            tLog(ERROR, "商户[%s]交易冻结操作失败.", sUserCode);
            return -1;
        }
    }
    //账户冻结
    if (pstRiskRule->sIsFrozen[0] == '1') {
        //插入风控记录
        iRet = InstRiskLog(pstTransJson, pstRiskRule->sRuleName);
        if (iRet < 0) {
            tLog(ERROR, "商户[%s]账户冻结记录添加失败.", sUserCode);
            return -1;
        }
        if (iRet > 0 && MerchFrozen(pstTransJson) < 0) {
            tLog(ERROR, "商户[%s]账户冻结操作失败.", sUserCode);
            return -1;
        }
    }
    //预警记录
    if (pstRiskRule->sIsWarnning[0] == '1') {
        if (InstRiskMerch(pstTransJson, pstRiskRule->sRuleName, pstRiskRule->sRuleCode) < 0) { /*motify by GJQ add 20171205*/
            tLog(ERROR, "商户[%s]预警记录操作失败.", sUserCode);
            return -1;
        }
    }

    if (pstRiskRule->sTransControl[0] == '1') {
        tLog(INFO,"触发风控规则，禁止交易");
        return 1;
    }

    return 0;
}
