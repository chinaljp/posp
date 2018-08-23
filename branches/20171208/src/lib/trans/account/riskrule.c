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

    /* ���ҷ�ع��� */
    tLog(INFO, "��ع���%s��", pcRuleCode);
    tStrCpy(sRuleCode, pcRuleCode, 4);
    if (FindCodeRule(&stRiskRule, sRuleCode) < 0) {
        tLog(INFO, "δ�ҵ���ع���[%s]", sRuleCode);
        return 0;
    }

    memcpy(sCode, pcRuleCode + 1, 3);
    iCode = atoi(sCode);
    switch (iCode) {
            /*��ش����� */
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
            tLog(INFO, "δ�ҵ���ع��������...");
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

    /* �����̻���ͨ�Ĺ��� */
    if (FindMerchRule(sRuleCode, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]�ķ�ع���.", sMerchId);
        return -1;
    }
    tLog(INFO, "�̻�[%s]��ع�����[%s]", sMerchId, sRuleCode);

    pcTemp = sRuleCode;
    while (pcTemp) {
        tCutOneField(&pcTemp, sStore);
        tLog(INFO, "ִ�й���[%s]..", sStore);
        if (Rulefunction(pstTransJson, sStore) < 0) {
            return -1;
        }
        tLog(INFO, "����[%s]ִ�н���", sStore);
    }
    return 0;
}

/*���з���ʱ����ؼ��*/
int RespChkRiskRule(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sRuleCode[200] = {0};

    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    /* �����̻���ͨ�Ĺ��� */
    if (FindMerchRule(sRuleCode, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]�ķ�ع���.", sMerchId);
        return -1;
    }
    tLog(INFO, "�̻�[%s]��ع�����[%s]", sMerchId, sRuleCode);

    if (strstr(sRuleCode, "R008")) {
        tLog(INFO, "���з��أ�ִ�й���[R008]..");
        Rulefunction(pstTransJson, "R008");
        tLog(INFO, "����[ROO8]ִ�н���");
    }
    if(strstr(sRuleCode, "R009")) {
        tLog(INFO, "���з��أ�ִ�й���[R009]..");
        Rulefunction(pstTransJson, "R009");
        tLog(INFO, "����[R009]ִ�н���");
    }
    
    return ( 0);
}

int RiskOpt(cJSON *pstTransJson, RiskRule *pstRiskRule) {
    char sUserCode[15 + 1] = {0};
    char sStatus[2] = {0};
    int iRet = -1;

    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    //�̻�ע��
    if (pstRiskRule->sIsDel[0] == '1') {
        sStatus[0] = 'X';
        if (UpdMerchStatus(sUserCode, sStatus) < 0) {
            tLog(ERROR, "�̻�[%s]ע������ʧ��.", sUserCode);
            return -1;
        }
    }
    //�̻�����
    if (pstRiskRule->sIsStop[0] == '1') {
        sStatus[0] = '0';
        if (UpdMerchStatus(sUserCode, sStatus) < 0) {
            tLog(ERROR, "�̻�[%s]���׶������ʧ��.", sUserCode);
            return -1;
        }
    }
    //�˻�����
    if (pstRiskRule->sIsFrozen[0] == '1') {
        //�����ؼ�¼
        iRet = InstRiskLog(pstTransJson, pstRiskRule->sRuleName);
        if (iRet < 0) {
            tLog(ERROR, "�̻�[%s]�˻������¼���ʧ��.", sUserCode);
            return -1;
        }
        if (iRet > 0 && MerchFrozen(pstTransJson) < 0) {
            tLog(ERROR, "�̻�[%s]�˻��������ʧ��.", sUserCode);
            return -1;
        }
    }
    //Ԥ����¼
    if (pstRiskRule->sIsWarnning[0] == '1') {
        if (InstRiskMerch(pstTransJson, pstRiskRule->sRuleName, pstRiskRule->sRuleCode) < 0) { /*motify by GJQ add 20171205*/
            tLog(ERROR, "�̻�[%s]Ԥ����¼����ʧ��.", sUserCode);
            return -1;
        }
    }

    if (pstRiskRule->sTransControl[0] == '1') {
        tLog(INFO,"������ع��򣬽�ֹ����");
        return 1;
    }

    return 0;
}
