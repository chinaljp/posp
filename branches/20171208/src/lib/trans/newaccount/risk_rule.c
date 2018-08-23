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

    tLog(INFO, "�̻���������[%d]��", iLevel);
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSql);
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSql);
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return 1;
    }
    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, sRuleCode);
    }

    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "δ�ҵ���¼.");
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
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
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tLog(DEBUG, "�����Ϣ:sRuleCode[%s],sRuleName[%s],sStatus[%s],sTransCode[%s],iTransCnt[%d],\
     iQueueCnt[%d],dTransAmt[%f],iInterval[%d],sStartTime[%s],sEndTime[%s],sRespCode[%s],\
     sMcc[%s],sRemark[%s],sIsStop[%s],sIsDel[%s],sIsFrozen[%s],sIsWarnning[%s],sIsLife[%s],sLifeControl[%s],sTransControl[%s]",
            stRiskRule.sRuleCode, stRiskRule.sRuleName, stRiskRule.sStatus, stRiskRule.sTransCode, stRiskRule.iTransCnt,
            stRiskRule.iQueueCnt, stRiskRule.dTransAmt, stRiskRule.iInterval, stRiskRule.sStartTime, stRiskRule.sEndTime,
            stRiskRule.sRespCode, stRiskRule.sMcc, stRiskRule.sRemark, stRiskRule.sIsStop, stRiskRule.sIsDel, stRiskRule.sIsFrozen,
            stRiskRule.sIsWarnning, stRiskRule.sIsLife, stRiskRule.sLifeControl, stRiskRule.sTransControl);
    *pstRiskRule = stRiskRule;
    tLog(DEBUG, "���ҵ��ֿع�����[%s]����", sRuleCode);
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
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
1�����̻���ѯ������5�α���������10�ζ����̻���
2. ������ѯ������3�α���
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
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }

    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "��ع���[R001]OR[R002]OR[R015]ִ����...");

    if (!memcmp(stRiskRule->sRuleCode, "R015", 4))
    {
        tLog(INFO, "ͬ����ѯ���");
        if (FindValueByKey(sKey, "kms.encdata.key") < 0)
        {
            ErrHanding(pstJson, "96", "������Կʧ��");
            tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
            return -1;
        }
        if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
        {
            ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
            tLog(ERROR, "���ܿ�������ʧ��.");
            return -1;
        }
        snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
         TRANS_CODE = '021000' AND MERCH_ID ='%s' AND RESP_CODE='00' AND CARD_NO_ENCRYPT='%s'", sMerchId, sECardNo);

    } else
    {
        tLog(INFO, "ͬ�̻���ѯ���");
        snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
       TRANS_CODE = '021000' AND MERCH_ID ='%s' AND RESP_CODE='00'", sMerchId);

    }

    iNum = tQueryCount(sSqlStr);
    if (iNum < 0)
    {
        ErrHanding(pstJson, "96", "��ȡ����ʧ��");
        return -1;
    }
    if (iNum >= stRiskRule->iTransCnt)
    {
        tLog(INFO, "���̻���ѯ������[%d]�Σ�Ԥ����¼�򶳽��̻�", stRiskRule->iTransCnt);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }
    return 0;
}

/* R003*/

/*
3��������ˢ���Ƶ�3�α���
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
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    if (memcmp(sInputMode + 1, "2", 1))
    {
        tLog(INFO, "���ײ��Ǵ����������Ƹù���");
        return 0;
    }

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    tLog(ERROR, "���ܿ�������[%s]", sECardNo);


    sprintf(sSql1, "SELECT COUNT(1) FROM B_WHITE_SAMCARD_LIST WHERE STATUS='1' AND CARD_NO_ENCRYPT='%s' AND USER_CODE='%s'", sECardNo, sUserCode);

    iRet1 = tQueryCount(sSql1);
    if (iRet1 < 0)
    {
        ErrHanding(pstJson, "96", "��ȡ����ʧ��");
        return -1;

    } else if (iRet1 == 1)
    {
        tLog(INFO, "��������ͬ����ˢ������,������ͬ����ˢ����");
        return 0;
    }

    if (!memcmp(stRiskRule->sIsLife, "1", 1))
    {
        tLog(INFO, "�÷�ع����ж�����Ȧ");
        if (memcmp(sMerchId, sChannelMerchId, 15))
        {
            tLog(INFO, "��������Ȧ�̻�,���жϸù���");
            return 0;
        }
    }


    tLog(DEBUG, "��ع���R003ִ����...");
    /* �̻�ͬ����ˢ�����߼���*/
    {

        sprintf(sSql, "select count(1) from b_pos_trans_detail where merch_id='%s' "
                "and resp_code='00' and valid_flag='0' and trans_code in ('020000','T20000','024100' )"
                "and amount>%f and CARD_NO_ENCRYPT='%s' and input_mode like '02%%'", stRiskRule->dTransAmt, sMerchId, sECardNo);
        tLog(DEBUG, "%s", sSql);

        iRet = tQueryCount(sSql);
        if (iRet < 0)
        {
            ErrHanding(pstJson, "96", "��ȡ����ʧ��");
            return -1;
        }

        if (iRet >= stRiskRule->iTransCnt)
        {
            tLog(INFO, "������ˢ���ѽ�����[%f]��[%d]��,����(�������ѣ��սᣬԤ��Ȩ���)", stRiskRule->dTransAmt, iRet);
            iRet = RiskOpt(pstJson, stRiskRule);
            if (iRet < 0)
            {
                tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
            } else if (iRet == 1)
            {
                ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
                return -1;
            }
        }
    }

    return 0;
}

/*R004*/

/*
4��������ѯ�������2�����϶����˻���
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
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "��ع���R004ִ����...");

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
    TRANS_CODE = '021000' AND  CARD_NO_ENCRYPT='%s' AND RESP_CODE='55'", sECardNo);
    iRet = tQueryCount(sSqlStr);
    if (iRet < 0)
    {
        ErrHanding(pstJson, "96", "��ȡ����ʧ��");
        return -1;
    }

    if (iRet >= stRiskRule->iTransCnt)
    {
        /*�����˻�*/
        tLog(INFO, "������ѯ�������[%d]�����϶����˻�(ͬ��)", stRiskRule->iTransCnt);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }
    return 0;
}

/*R005*/

/*
5�����̻���ѯ�������5�����϶����˻���
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
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "��ع���R005ִ����...");

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
    TRANS_CODE = '021000' AND MERCH_ID='%s' AND RESP_CODE='55'", sMerchId);
    iRet = tQueryCount(sSqlStr);
    if (iRet < 0)
    {
        ErrHanding(pstJson, "96", "��ȡ����ʧ��");
        return -1;
    }


    if (iRet >= stRiskRule->iTransCnt)
    {
        /*�����˻�*/
        tLog(INFO, "���̻���ѯ�������[%d]�����϶����˻�(ͬ�̻�����ͬ��)", stRiskRule->iTransCnt);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }
    return 0;
}

/* 6�����̻����׵��ʳ���10��ı���  */
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
        tLog(INFO, "�˹�����Խ���[%s]��������", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���6ִ����...");
    if (iAmount > (int) (dRkAmount * 100))
    {
        tLog(DEBUG, "�̻�[%s]���׵��ʳ���[%.02f]Ԫ,Ԥ��!", sMerchId, dRkAmount);
        /*Ԥ����¼*/
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }

    return 0;
}

/* 7��ͬ��ǿ�����ͬ�̻���1�������������ף����ѣ�Ԥ��Ȩ��ɣ������˻� ROO7*/
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
        tLog(INFO, "�˹�����Խ���[%s]��������", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���7ִ����...");
    OCI_Resultset *pstRes = NULL;

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    tLog(DEBUG, "���ܿ�������[%s]", sECardNo);

    /*BUG ��������ʱ���transmit_time ����� ���Դ�sql �����������ʱ��ʱ �����*/
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
        ErrHanding(pstJson, "96", "��ȡ���׽��״���ʧ��");
        return ( -1);
    }

    if (iRet >= stRiskRule->iTransCnt)
    {
        /*�����˻�*/
        tLog(DEBUG, "��ǿ�[%s]���̻�[%s]1�����ڽ��ױʴ�[%d],�����˻�!", sCardNo, sMerchId, iRet + 1); /*iRet+1 �����ν��׼�������*/
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }
    return 0;
}

/*8�����з����������04��34��41��54��57��62 Ԥ����¼�� ע�⣺���ؽ������� �������ƾ��彻��*/
int RuleCodefunc8(cJSON * pstJson, RiskRule *stRiskRule) {
    //char sTransCode[6 + 1] = {0}, sIsRespCode[3 + 1] = {0};
    char sIsRespCode[3 + 1] = {0};
    int iRet = 0;

    //GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "istresp_code", sIsRespCode);

    /*���ؽ������� �������ƾ��彻��*/
    /*
    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL) {
        tLog(INFO, "�˹�����Խ���[%s]��������", sTransCode);
        return 0;
    }
     */
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���8ִ����...");
    if (sIsRespCode[0] == '\0')
    {
        tLog(INFO, "���������У��������˷�ع���!");
    } else if (sIsRespCode[0] != '\0' && strstr(stRiskRule->sRespCode, sIsRespCode))
    {
        /*Ԥ����¼*/
        tLog(DEBUG, "���з���[%s],Ԥ����¼!", sIsRespCode);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    } else
    {
        tLog(INFO, "����������Ϊ[%s]���������˷�ع���!", sIsRespCode);
    }

    return 0;
}

/*9�����з����������04��34��41��54��57��62 ���쵥�̻�2�α��������˻� ע�⣺���ؽ������� �������ƾ��彻�� 
 * motify by GuoJiaQing at 20170915   
 * Modify Content: �޸Ĵ˹��� �����ڴ������� */
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
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���9ִ����...");
    if (sIsRespCode[0] == '\0')
    {
        tLog(INFO, "���������У��������˷�ع���!");
    } else if (sIsRespCode[0] != '\0' && strstr(stRiskRule->sRespCode, sIsRespCode))
    {
        snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
                        MERCH_ID='%s' AND ISTRESP_CODE in ('04','34','41','54','57','62') AND SUBSTR(INPUT_MODE,1,2) = '02'", sMerchId);
        iCnt = tQueryCount(sSqlStr);
        if (iCnt < 0)
        {
            ErrHanding(pstJson, "96", "��ȡ����ʧ��");
            return -1;
        }
        /*�����̻����������룺 04��34��41��54��57��62 ����һ�����׼�¼ ����Ϊ�ڶ����������ر���*/
        if (sInputMode[0] == '0' && sInputMode[1] == '2')
        {
            tLog(DEBUG, "sInputMode = [%s]", sInputMode);
            iCnt = iCnt + 1;
        }
        if (iCnt >= stRiskRule->iTransCnt)
        {
            /*�����˻�*/
            tLog(DEBUG, "�̻�[%s]�������з���[%s][%d]��,�����˻�!", sMerchId, sIsRespCode, iCnt);
            iRet = RiskOpt(pstJson, stRiskRule);
            if (iRet < 0)
            {
                tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
            } else if (iRet == 1)
            {
                ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
                return -1;
            }
        }
    } else
    {
        tLog(INFO, "����������Ϊ[%s]���������˷�ع���!", sIsRespCode);
    }

    return 0;
}

/* 10��ͬ�̻�����������2�ζ����˻�*/
int RuleCodefunc10(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sSqlStr[1024] = {0}, sDesc[100] = {0};
    int iCnt = 0, iRet = 0;

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);

    if (strstr(stRiskRule->sTransCode, sTransCode) == NULL)
    {
        tLog(INFO, "�˹�����Խ���[%s]��������", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���10ִ����...");

    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from b_pos_trans_detail where merch_id = '%s' and trans_code = '%s' and resp_code='00'",
            sMerchId, sTransCode);

    iCnt = tQueryCount(sSqlStr);
    if (iCnt < 0)
    {
        tLog(DEBUG, "sSqlStr[%s]", sSqlStr);
        ErrHanding(pstJson, "96", "��ȡ���׳�������ʧ��");
        return ( -1);
    }

    if (iCnt >= stRiskRule->iTransCnt)
    {
        /*�����˻�*/
        tLog(DEBUG, "�̻�[%s]�����ʴ�[%d],�����˻�!", sMerchId, iCnt);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }

    return 0;
}

/* 11�� ���죬ͬ�̻���ͬ�����Ȳ�ѯ�������� Ԥ��  R011*/

/*
 * insert into  B_RISK_RULE (rule_code,rule_name,status,trans_code,interval,IS_FROZEN)
 values ('R011','����ͬ��ͬ�̻��Ȳ�ѯ�����ѱ���','1','020000,T20000,024100',1,'0');
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
        tLog(INFO, "�˹�����Խ���[%s]��������", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���11ִ����...");

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    tLog(DEBUG, "���ܿ�������[%s]", sECardNo);
    snprintf(sSqlStr, sizeof (sSqlStr), " select * from ( \
                    select to_number(to_date('%s','hh24miss') - to_date(trans_time,'hh24miss'))*24*60*60 from b_pos_trans_detail \
                    where merch_id = '%s' and card_no_encrypt = '%s' and trans_code = '021000' \
                    and trans_time < '%s' order by trans_time desc \
             ) where rownum = 1", sTransTime, sMerchId, sECardNo, sTransTime); /*�����ʱ��λΪ��*/

    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        ErrHanding(pstJson, "96", "�������ݳ���");
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        ErrHanding(pstJson, "96", "�������ݳ���");
        return -1;
    }
    while (OCI_FetchNext(pstRes))
    {
        INTV(pstRes, 1, iSec);
    }

    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return 0;
    }

    tReleaseRes(pstRes);
    tLog(DEBUG, "��ǰ����[%s]������һ�ʲ�ѯ���׼��[%d]��", sTransCode, iSec);

    if (iSec < (stRiskRule->iInterval)*60)
    {

        tLog(DEBUG, "��ǰ����[%s]������һ�ʲ�ѯ���׼��[%d]�룬��Ԥ�����ʱ��[%d]������", sTransCode, iSec, stRiskRule->iInterval);
        /*Ԥ����¼*/
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }

    return ( 0);
}

/* 12��ͬ��������ͬ�̻���1�������������ף����ѣ�Ԥ��Ȩ��ɣ������˻� RO12*/

/*
 * insert into  B_RISK_RULE (rule_code,rule_name,status,trans_code,trans_cnt,IS_FROZEN)
 values ('R012','ͬ������ͬ�̻�1�������������׶����˻�','1','020000,T20000,024100',2,'1')
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
        tLog(INFO, "�˹�����Խ���[%s]��������", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���12ִ����...");
    /*ͨ����ˢ ������Ȧ�̻������������� add by Guo JiaQing at 20170919  BENGIN*/


    if (memcmp(sBusiType, "02", 2))
    {
        if (memcmp(sMerchId, sChannelMerchId, 15))
        {
            tLog(INFO, "[%s]��������Ȧ�̻�", sMerchId);
            /*�ж� �˹��������Ȧ�̻��Ƿ����� 0 - �����ƣ�1 - ����*/
            if (!memcmp(stRiskRule->sIsLife, "0", 1))
            {
                tLog(INFO, "����Ȧ�̻�,�˹�����䲻������,�˷�ز�������", sMerchId);
                return ( 0);
            }
        }
    }
    /* add by Guo JiaQing at 20170918  END */

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    tLog(DEBUG, "���ܿ�������[%s]", sECardNo);

    /*BUG ��������ʱ���transmit_time ����� ���Դ�sql �����������ʱ��ʱ �����*/
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
        ErrHanding(pstJson, "96", "��ȡ���׽��״���ʧ��");
        return ( -1);
    }

    if (iRet >= stRiskRule->iTransCnt)
    {
        /*�����˻�*/
        tLog(DEBUG, "������[%s]���̻�[%s]1�����ڽ��ױʴ�[%d],�����˻�!", sCardNo, sMerchId, iRet + 1); /*iRet+1 �����ν��׼�������*/
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }
    return 0;
}

/*13�����п�������*/
int RuleCodefunc13(cJSON * pstJson, RiskRule *stRiskRule) {
    int iRet = 0;
    char sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0}, sSql[128] = {0};
    char sKey[128 + 1] = {0}, sTransCode[6 + 1] = {0};

    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    tLog(ERROR, "���ܿ�������[%s]", sECardNo);
    /* ���п�������   */
    sprintf(sSql, "SELECT COUNT(1) FROM B_RISK_BLACK_CARD WHERE STATUS='1' AND CARD_NO_ENCRYPT='%s'", sECardNo);
    iRet = tQueryCount(sSql);
    if (iRet > 0)
    {
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "62", "������˿�[%s].", sCardNo);
            return -1;
        }

    }
    //tLog(INFO, "����[%s]���������ͨ��.", sCardNo);
    return 0;
}

/*14������Ȧ�̻��ж�*/
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
        tLog(INFO, "�̻����ַ��̻����ж�����Ȧ");
        return 0;
    }

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    if (!memcmp(sMerchId, sChannelMerchId, 15))
    {
        tLog(INFO, "����������Ȧ�̻�,���жϸù���");
        return 0;
    }

    {/* �߼��ж� */

        iRet1 = FindCityLevel(sMerchId);
        if (iRet1 < 0)
        {
            ErrHanding(pstJson, "96", "��ȡ����ʧ��");
            return -1;
        }

        /*��ѯ�̻��������ʡ����*/
        if (FindCityCode(sMerchId, sMerchCityCode, sMerchPCityCode, sMerchCityName, sMerchPCityName, iRet1) < 0)
        {
            ErrHanding(pstJson, "96", "��ȡ����ʧ��");
            return -1;
        }
        /*��ѯ����Ȧ�̻��������ʡ����*/
        if (FindLifeCityCode(sChannelMerchId, sChMerchCityCode, sChMerchPCityCode, sChMerchCityName, sChMerchPCityName) < 0)
        {
            ErrHanding(pstJson, "96", "��ȡ����ʧ��");
            return -1;
        }
        tLog(INFO, "�̻�[%s]:��[%s]������[%s]��ʡ[%s]������[%s]������Ȧ�̻�[%s]����[%s]������[%s]��ʡ[%s]������[%s]",
                sMerchId, sMerchCityName, sMerchCityCode, sMerchPCityName, sMerchPCityCode, sChannelMerchId, sChMerchCityName, sChMerchCityCode, sChMerchPCityName, sChMerchPCityCode);
        sprintf(sInfo, "�̻�����[%s]ʡ[%s]������Ȧ�̻�����[%s]��ʡ[%s]",
                sMerchCityName, sMerchPCityName, sChMerchCityName, sChMerchPCityName);
        strcpy(stRiskRule->sRuleName, sInfo);
        if (!memcmp(stRiskRule->sLifeControl, "0", 1))
        {
            tLog(INFO, "����Ȧ������ʡ������");
            return 0;
        } else if (!memcmp(stRiskRule->sLifeControl, "2", 1))
        {
            tLog(INFO, "����Ȧ�̻�����ͬ��");
            if (memcmp(sMerchCityCode, sChMerchCityCode, 4))
            {
                iRet = RiskOpt(pstJson, stRiskRule);
                if (iRet < 0)
                {
                    tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
                } else if (iRet == 1)
                {
                    ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
                    return -1;
                }
            }
        } else if (!memcmp(stRiskRule->sLifeControl, "1", 1))
        {
            tLog(INFO, "����Ȧ�̻�����ͬʡ");
            if (memcmp(sMerchPCityCode, sChMerchPCityCode, 4))
            {
                iRet = RiskOpt(pstJson, stRiskRule);
                if (iRet < 0)
                {
                    tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
                } else if (iRet == 1)
                {
                    ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
                    return -1;
                }
            }
        }
    }

    return 0;
}

/*16�����̻�����һСʱ����������������ζ����˻�  */

/* motify by GuoJiaQing at 20170915   
 * Modify Content: �޸Ĵ˹���Ϊ: ���̻�������һСʱ�ڣ���������������Σ������ν�����һСʱ֮�ڣ�
 * ���������⽻�׶����˻��������ڴ���������*/
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
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "��ع���R016ִ����...");

    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }

    /*���̻�����һСʱ����������������ζ����˻�*/
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
        ErrHanding(pstJson, "96", "��ȡ���׽��״���ʧ��");
        return ( -1);
    }

    if (iRet >= stRiskRule->iTransCnt)
    {
        tLog(INFO, "�˿�[%s]���̻�[%s]��pos�����У�һСʱ������ <�������> [%d]��,������ع���!", sCardNo, sMerchId, iRet);
        tLog(DEBUG, "%s", stRiskRule->sRuleName);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }

    return 0;
}

/*17��MCC����6012�ķǽ�ǿ����ף������˻�*/
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
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    /*MCC����6012�ķǽ�ǿ����ף������˻�*/
    tLog(DEBUG, "��ع���R017ִ����...");
    if (!memcmp(stRiskRule->sMcc, sMcc, 4))
    {
        tLog(INFO, "�̻�MCC����[%s]", stRiskRule->sMcc);
        if (memcmp(sCardType, "0", 1))
        {
            tLog(INFO, "�ÿ��ǽ�ǿ��������");
            iRet = RiskOpt(pstJson, stRiskRule);
            if (iRet < 0)
            {
                tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
            } else if (iRet == 1)
            {
                ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
                return -1;
            }
        }
    }
    return 0;

}

/*18�������������μ�����(��2�����㣬�������κν��׶������˷��) �������
 * motify by GuoJiaQing at 20170915   Modify Content: �޸Ĵ˹��� �����ڴ������� */
int RuleCodefunc18(cJSON * pstJson, RiskRule *stRiskRule) {
    char sKey[128 + 1] = {0};
    char sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sSqlStr[1024] = {0}, sDesc[100] = {0};
    int iRet = 0, iCnt = 0;

    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "�˹�����Խ���[%s]��������", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���18ִ����...");
    if (FindValueByKey(sKey, "kms.encdata.key") < 0)
    {
        ErrHanding(pstJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0)
    {
        ErrHanding(pstJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    tLog(DEBUG, "���ܿ�������[%s]", sECardNo);

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_POS_TRANS_DETAIL WHERE \
                    CARD_NO_ENCRYPT='%s' AND ISTRESP_CODE = '%s' AND SUBSTR(INPUT_MODE,1,2) = '02'", sECardNo, stRiskRule->sRespCode);
    iCnt = tQueryCount(sSqlStr);
    if (iCnt < 0)
    {
        ErrHanding(pstJson, "96", "��ȡ����ʧ��");
        return -1;
    }

    if (iCnt >= stRiskRule->iTransCnt)
    {
        tLog(DEBUG, "�˿�[%s]���׷��� <��������> [%d]�Σ������˷�ع���!", sCardNo, iCnt);
        tLog(DEBUG, "%s", stRiskRule->sRuleName);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
            return -1;
        }
    }

    return ( 0);
}

/*  19���쳣ʱ��ν��״��� */
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
        tLog(INFO, "�˹�����Խ���[%s]��������", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }

    tLog(DEBUG, "��ع���19ִ����...");
    /* ���������� �쳣ʱ��� ����Ԥ����¼*/
    if (memcmp(sTransTime, stRiskRule->sStartTime, 6) >= 0 || memcmp(sTransTime, stRiskRule->sEndTime, 6) <= 0)
    {
        if (iAmount >= (int) (dRkAmount * 100))
        {
            tLog(DEBUG, "����ʱ��[%s]Ϊ�쳣����ʱ���ҽ��׽��Ϊ[%.02f]Ԫ���ڵ���[%.02f]Ԫ�������˷�ع���", sTransTime, (double) (iAmount / 100), dRkAmount);
            tLog(DEBUG, "%s", stRiskRule->sRuleName);
            iRet = RiskOpt(pstJson, stRiskRule);
            if (iRet < 0)
            {
                tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
            } else if (iRet == 1)
            {
                ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
                return -1;
            }
        } else
        {
            tLog(DEBUG, "����ʱ��[%s]Ϊ�쳣����ʱ����ҽ��׽��С��[%.2lf]Ԫ,�������˷�ع���!", sTransTime, dRkAmount);
        }
    } else
    {
        tLog(INFO, "���쳣ʱ��Σ��������˷�ع���");
    }

    return ( 0);
}

/*���̻�1Сʱ�� ������������3������������п�������2��(��Դ���������)
 * �ٴν������⽻�׶����˻������ν�����һСʱ֮�ڣ� add by guojiaqing  */
int RuleCodefunc20(cJSON * pstJson, RiskRule *stRiskRule) {
    char sMerchId[15 + 1] = {0}, sRrn[RRN_LEN + 1] = {0};
    char sSqlStr[1024] = {0}, sRuleCode[100] = {0}, sTransCode[6 + 1] = {0}, sCardNo[19 + 1] = {0}, sECardNo[128 + 1] = {0};
    char sKey[128 + 1] = {0}, sTransmitTime[10 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDate[8 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    int iRet = 0;
    int iCnt = 0, iTmp = 0; /*����������*/
    int iNum = 0; /*����������п�����*/

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "transmit_time", sTransmitTime);
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    GET_STR_KEY(pstJson, "trans_date", sTransDate);

    if (NULL == strstr(stRiskRule->sTransCode, sTransCode))
    {
        tLog(INFO, "����[%s]�����ж�", sTransCode);
        return 0;
    }
    if (memcmp(stRiskRule->sStatus, "1", 1))
    {
        tLog(INFO, "��Ч�ķ�ع���,sRuleCode = [%s],sStatus = [%s]", stRiskRule->sRuleCode, stRiskRule->sStatus);
        return 0;
    }
    tLog(DEBUG, "��ع���R020ִ����...");


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
        ErrHanding(pstJson, "96", "�������ݳ���");
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        ErrHanding(pstJson, "96", "�������ݳ���");
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
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return 0;
    }

    tReleaseRes(pstRes);

    tLog(DEBUG, "����������[%d],����������п�����[%d]", iCnt, iNum);

    if (iCnt >= stRiskRule->iTransCnt && iNum >= stRiskRule->iQueueCnt)
    {
        tLog(INFO, "�����̻�[%s]��pos�����У�һСʱ������ <�������> [%d]�� �� <����������п�����> [%d] ��,������ع���!", sMerchId, iCnt, iNum);
        tLog(DEBUG, "%s", stRiskRule->sRuleName);
        iRet = RiskOpt(pstJson, stRiskRule);
        if (iRet < 0)
        {
            tLog(INFO, "��ع���[%s]��������ִ��ʧ��!", stRiskRule->sRuleCode);
        } else if (iRet == 1)
        {
            ErrHanding(pstJson, "D9", "������ع��򣬽�ֹ����");
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
    tLog(INFO, "�̻�[%s]״̬[%s]���³ɹ�.", pcUserCode, pcStatus);
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

    //����Ѿ������˻�����ֱ�ӷ��أ���������
    snprintf(sSqlStr, sizeof (sSqlStr), "select count(1) from B_BALANCE_CONTROL where user_code = '%s' and WALLET_TYPE = 'all' and STATUS='1'",
            sUserCode);

    iCnt = tQueryCount(sSqlStr);
    if (iCnt > 0)
    {
        tLog(INFO, "�̻�[%s]�Ѷ����˻�,���ٴ���.", sUserCode);
        return 0;
    }

    memset(sSqlStr, 0, sizeof (sSqlStr));
    snprintf(sSqlStr, sizeof (sSqlStr), "insert into B_BALANCE_CONTROL(USER_CODE,REMARK,status,wallet_type,frozen_type,opt_no)"
            " values('%s','������ع���[%s:%s],ϵͳ�Զ��˻�����.','1','all','2','%s')", sUserCode, sRrn, pcRuleName, sRrn);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tLog(INFO, "�̻�[%s]�˻������¼��ӳɹ�.", sUserCode);
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
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson)
    {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    /* �°汾�ӿ�
    SET_STR_KEY(pstNetJson, "walletType", "0");
    SET_STR_KEY(pstNetJson, "frozenStatus", "1");
    SET_STR_KEY(pstNetJson, "frozenBusinessType", "1");
    SET_STR_KEY(pstNetJson, "trans_code", "00TB00");
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_STR_KEY(pstNetJson, "optNo", sRrn);
    SET_STR_KEY(pstNetJson, "merch_id", sMerchId);
    SET_STR_KEY(pstNetJson, "user_code", sUserCode);
    SET_STR_KEY(pstNetJson, "frozenRemark", "������ع���,ϵͳ�Զ��˻�����.");
     * */
    //�ϰ汾�ӿ�
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
            tLog(ERROR, "����[%s]�˻�����֪ͨ��ʱ.", sRrn);
        } else
            tLog(ERROR, "����[%s]�˻�����֪ͨʧ��.", sRrn);
        cJSON_Delete(pstNetJson);
        return -1;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
    if (memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "�˻�����ʧ��[%s:%s].", sRespCode, sResvDesc);
    } else
        tLog(INFO, "����[%s]�˻�����ɹ�.", sRrn);
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

    /* kms ���ܴ洢 ,ԭ�ֶ�������ʾ */
    if (FindValueByKey(sKeyName, "kms.encdata.key") < 0)
    {
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
    } else
    {
        if (sCardNo[0] != '\0')
        {
            if (tHsm_Enc_Asc_Data(sCardNoEnc, sKeyName, sCardNo) < 0)
                tLog(ERROR, "���ż���ʧ��.");
            else
                tLog(DEBUG, "���ż��ܳɹ�.");
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
    tLog(INFO, "�����̻�Ԥ����¼��ӳɹ�");
    tReleaseRes(pstRes);
    return 0;
}

int Rulefunction(cJSON *pstJson, char * pcRuleCode) {

    char sCode[3 + 1] = {0}, sRuleCode[4 + 1] = {0};
    int iCode = 0, iRet = 0;
    RiskRule stRiskRule;

    /* ���ҷ�ع��� */
    //tLog(INFO, "��ع���[%s]", pcRuleCode);
    tStrCpy(sRuleCode, pcRuleCode, 4);
    if (FindCodeRule(&stRiskRule, sRuleCode) < 0)
    {
        tLog(INFO, "δ�ҵ���ع���[%s]", sRuleCode);
        return 0;
    }

    memcpy(sCode, pcRuleCode + 1, 3);
    iCode = atoi(sCode);
    switch (iCode)
    {
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

int ChkRiskRule(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sRuleCode[200] = {0};
    char sStore[4 + 1] = {0}, *pcTemp = NULL;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    /* �����̻���ͨ�Ĺ��� */
    if (FindMerchRule(sRuleCode, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]�ķ�ع���.", sMerchId);
        return -1;
    }
    tLog(DEBUG, "�̻�[%s]��ع�����[%s]", sMerchId, sRuleCode);

    pcTemp = sRuleCode;
    while (pcTemp)
    {
        tCutOneField(&pcTemp, sStore);
        tLog(DEBUG, "ִ�й���[%s]..", sStore);
        if (Rulefunction(pstTransJson, sStore) < 0)
        {
            tLog(ERROR, "ִ�й���[%s]ʧ��.", sStore);
            return -1;
        }
        tLog(DEBUG, "����[%s]ִ�н���", sStore);
    }
    return 0;
}

/*���з���ʱ����ؼ��*/
int RespChkRiskRule(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sRuleCode[200] = {0}, sRespCode[RESP_CODE_LEN + 1] = {0};

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    /* �����̻���ͨ�Ĺ��� */
    if (FindMerchRule(sRuleCode, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, sRespCode, "δ�ҵ��̻�[%s]�ķ�ع���,����.", sMerchId);
        return 0;
    }
    tLog(DEBUG, "�̻�[%s]��ع�����[%s].", sMerchId, sRuleCode);

    if (strstr(sRuleCode, "R008"))
    {
        tLog(DEBUG, "���з��أ�ִ�й���[R008]..");
        Rulefunction(pstTransJson, "R008");
        tLog(DEBUG, "����[ROO8]ִ�н���");
    }
    if (strstr(sRuleCode, "R009"))
    {
        tLog(DEBUG, "���з��أ�ִ�й���[R009]..");
        Rulefunction(pstTransJson, "R009");
        tLog(DEBUG, "����[R009]ִ�н���");
    }

    return ( 0);
}

int RiskOpt(cJSON *pstTransJson, RiskRule *pstRiskRule) {
    char sUserCode[15 + 1] = {0};
    char sStatus[2] = {0};
    int iRet = -1;

    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    //�̻�ע��
    if (pstRiskRule->sIsDel[0] == '1')
    {
        sStatus[0] = 'X';
        if (UpdMerchStatus(sUserCode, sStatus) < 0)
        {
            tLog(ERROR, "�̻�[%s]ע������ʧ��.", sUserCode);
            return -1;
        }
    }
    //�̻�����
    if (pstRiskRule->sIsStop[0] == '1')
    {
        sStatus[0] = '0';
        if (UpdMerchStatus(sUserCode, sStatus) < 0)
        {
            tLog(ERROR, "�̻�[%s]���׶������ʧ��.", sUserCode);
            return -1;
        }
    }
    //�˻�����
    if (pstRiskRule->sIsFrozen[0] == '1')
    {
        //�����ؼ�¼
        iRet = InstRiskLog(pstTransJson, pstRiskRule->sRuleName);
        if (iRet < 0)
        {
            tLog(ERROR, "�̻�[%s]�˻������¼���ʧ��.", sUserCode);
            return -1;
        }
        if (iRet > 0 && MerchFrozen(pstTransJson) < 0)
        {
            tLog(ERROR, "�̻�[%s]�˻��������ʧ��.", sUserCode);
            return -1;
        }
    }
    //Ԥ����¼
    if (pstRiskRule->sIsWarnning[0] == '1')
    {
        if (InstRiskMerch(pstTransJson, pstRiskRule->sRuleName, pstRiskRule->sRuleCode) < 0)
        { /*motify by GJQ add 20171205*/
            tLog(ERROR, "�̻�[%s]Ԥ����¼����ʧ��.", sUserCode);
            return -1;
        }
    }

    if (pstRiskRule->sTransControl[0] == '1')
    {
        tLog(INFO, "������ع��򣬽�ֹ����");
        return 1;
    }

    return 0;
}
