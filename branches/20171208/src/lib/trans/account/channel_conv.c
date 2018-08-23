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
#include "settle_date.h"
#include "t_macro.h"
#include "status.h"
#include "limitrule.h"
#include "trans_code.h"

extern TransCode g_stTransCode;

int IsAgentLife(char *pcAgentId) {
    char sSqlStr[512] = {0}, sStatus[2] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT STATUS FROM B_LIFE_CTRL WHERE OBJECT_ID='%s'", pcAgentId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sStatus);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (sStatus[0] == '1') {
        return 1;
    }
    return 0;
}

int FindLifeMerch(char *pcLifeMerchId, char *pcLifeTermId, char *pcLifeMerchName, char *pcUserCode, char *pcAgentId) {
    char sSqlStr[512] = {0}, sTmp[1024] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;

    /* �ж��ܿ��� */
    if (FindValueByKey(sTmp, "LIFE_AGENT_SWITCH") < 0) {
        tLog(ERROR, "����key[LIFE_AGENT_SWITCH]����Ȧ�ܿ���,ʧ��.");
        return -1;
    }
    if (sTmp[0] == '0') {
        tLog(ERROR, "����Ȧ�ܿ���״̬[%s].", sTmp[0] == '0' ? "�ر�" : "��ͨ");
        return -1;
    }

    /* �̻�ֱ�������̿��� */
    if (IsAgentLife(pcAgentId) <= 0) {
        tLog(ERROR, "������[%s]δ��ͨ����Ȧ.", pcAgentId);
        return -1;
    }

    snprintf(sSqlStr, sizeof (sSqlStr), "select mc.b_life_merch_id,mc.life_term_id,lm.merch_name"
            " from B_LIFE_MERCH_CONV mc "
            " join B_LIFE_MERCH lm on lm.MERCH_ID=mc.b_life_merch_id "
            " WHERE mc.user_id='%s' and mc.status='1'"
            , pcUserCode);

    //tLog(DEBUG, "%s", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcLifeMerchId);
        STRV(pstRes, 2, pcLifeTermId);
        STRV(pstRes, 3, pcLifeMerchName);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    /* �������Ȧ�̻�ȡ����ѡ�����ݿ�����̻��ź��ն˺Żᱻ��� */
    if (pcLifeMerchId[0] == '\0' || pcLifeTermId[0] == '\0')
        return -1;
    return 0;
}

/* ͨ������Ȧ�̻��Ų����̻����� */
int FindLifeMerchName(char *pcLifeMerchName, char *pcLifeMerchId) {
    char sSqlStr[512] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select merch_name from B_LIFE_MERCH "
            " WHERE merch_id='%s'", pcLifeMerchId);

    //tLog(DEBUG, "%s", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcLifeMerchName);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

/* ͨ���Ż��������̻��Ų����̻����� */
int FindChannelMerchName(char *pcLifeMerchName, char *pcLifeMerchId) {
    char sSqlStr[512] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select channel_merch_name from B_CHANNEL_MERCH "
            " WHERE CHANNEL_MERCH='%s'", pcLifeMerchId);

    //tLog(DEBUG, "%s", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcLifeMerchName);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int ChkChannelMerch(cJSON *pstJson, char * pcChanneMerchId) {

    char sCardType[1 + 1] = {0}, sTransCode[9] = {0};
    double dTranAmt = 0.0L;
    char sMerchId[15 + 1] = {0}, sChannelMerchId[15 + 1] = {0};
    int iCnt = 0;

    PufaLimitRule stPufaLimitRule;

    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "card_type", sCardType);
    GET_DOU_KEY(pstJson, "amount", dTranAmt);
    GET_STR_KEY(pstJson, "merch_id", sMerchId);

    dTranAmt /= 100;

    /* �����ַ��̻����� */
    if (FindChannelRuleNoUp(&stPufaLimitRule, pcChanneMerchId) < 0) {
        tLog(INFO, "δ�ҵ������̻�[%s]���޶����.", pcChanneMerchId);
        return -1;
    }

    /* ���ұ����̻����� */
    if (FindMerchLimitRuleNoUp(&stPufaLimitRule, sMerchId) < 0) {
        tLog(INFO, "δ�ҵ������̻�[%s]���޶����.", pcChanneMerchId);
        return -1;
    }

    /*�����̻�Ϊ��ͨ�̻��ͻ����̻���Ҫ�жϵĹ���*/
    if (stPufaLimitRule.sCardType[0] != '2') {/* ����׼Ϊ2ʱ���޽���� */
        if (sCardType[0] != stPufaLimitRule.sCardType[0]) {
            tLog(INFO, "�����̻�[%s]���׿�����Ϊ[%c],Ҫ�����ѿ�����Ϊ[%s]", sMerchId, sCardType[0], (stPufaLimitRule.sCardType[0] ? "���ÿ�" : "��ǿ�"));
            return -1;
        }
    }

    if (dTranAmt < stPufaLimitRule.dLocalPassAmt) {
        tLog(INFO, "�����̻�[%s]���ѽ��С���������Ϊ[%f]", sMerchId, stPufaLimitRule.dLocalPassAmt);
        return -1;
    }

    //��齻���Ƿ�����
    if (sTransCode[5] != '3' && NULL == strstr(stPufaLimitRule.sATransCode, sTransCode)) {
        tLog(INFO, "��ֹ�Ľ���[%s].", sTransCode);
        return -1;
    }
    /* ��ȡ�����̻������ױ��� */
    iCnt = FindLocalMerchTransCnt(sMerchId, stPufaLimitRule.sChannelId);
    if (iCnt < 0) {
        tLog(INFO, "�����ַ������̻�[%s]���ױ���ʧ��.", sMerchId);
        return -1;
    }
    iCnt += 1;
    //�����̻��������ѱ���
    if (DBL_CMP(iCnt, stPufaLimitRule.iLocalTransCnt)) {
        tLog(INFO, "�ַ������̻�[%s]�����ѽ��״�������,���޽��ױ���[%d],��ǰ���ױ���[%d]", sMerchId, stPufaLimitRule.iLocalTransCnt, iCnt);
        return -1;
    }
    return 0;
}

/* �ַ��̻���ת���������� */
int ConvChannelMerch(cJSON *pstJson, int *piFlag) {

    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sMerchName[256 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sChannelTermId[8 + 1] = {0}, sChanneMerchId[15 + 1] = {0}, sAgentId[8 + 1] = {0}, sChannelId[8 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sChannelMerchName[256 + 1] = {0}, sGroupId[1 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    /*�����̻���û������·��*/
    if (FindChannelGroupId(sMerchId, sGroupId) < 0) {
        tLog(DEBUG, "�̻�[%s]���������̻���IDʧ��,ʹ��ԭ����.", sMerchId);
    } else {
        /*�����̻�����·��*/
        if (FindChannelMerchId(sGroupId, sChanneMerchId, sChannelTermId, sChannelMerchName, sChannelId) < 0) {
            tLog(INFO, "δ�ҵ�������[%s]�����̻�����", sGroupId);
            ErrHanding(pstTransJson, "96", "��ȡ�Ż���������Ϣʧ��");
            return -1;
        }
        if (memcmp(sTransCode, "021000", 6)) { /*����ǲ���ֱ������*/
            /*���·�ɹ���*/
            if (ChkChannelMerch(pstTransJson, sChanneMerchId) < 0) {
                /*�жϲ�����·�ɹ���ʱ�Ƿ����������*/
                if (ChkTransContol(sMerchId) < 0) {
                    ErrHanding(pstTransJson, "W8", "�����̻�[%s]��������ת����,����ʧ��.", sMerchId);
                    tLog(INFO, "�����������̻��������ƽ��ף�������������");
                    return -1;
                } else {
                    tLog(INFO, "�����ַ��������Ʋ���ת�Ż��������̻�!,��������ͨ�̻�");
                    return 0;
                }
            }
        }
        SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
        SET_STR_KEY(pstTransJson, "channel_term_id", sChannelTermId);
        SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
        //�Ż��������̻�Ĭ�Ϸ�������Ϊ�Ż���
        SET_STR_KEY(pstTransJson, "channel_merch_type", "1");
        SET_STR_KEY(pstTransJson, "channel_id", sChannelId);
        tLog(INFO, "�Ż��������̻�[%s:%s:%s].", sChanneMerchId, sChannelTermId, sChannelId);
    }
    return 0;
}

int UpChannelLimit(cJSON *pstJson, int *piFlag) {

    char sChanneMerchId[15 + 1] = {0}, sChanneTermId[8 + 1] = {0}, sChannelId[8 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sTraceNo[6 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransCode[6 + 1] = {0};
    double dTranAmt = 0.0L;
    PufaLimitRule stPufaLimitRule;
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "channel_id", sChannelId);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
    GET_STR_KEY(pstTransJson, "channel_term_id", sChanneTermId);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);

    if (memcmp(sChannelId, "48560000", 8) && memcmp(sChannelId, "49000000", 8)) {
        tLog(INFO, "�̻���������ת�Ż���������");
        return 0;
    }

    if (!memcmp(sChannelId, "48560000", 8)) {
        if (!memcmp(sTransCode, "020023", 6)) {
            ErrHanding(pstTransJson, "40", "�ַ���������������������.");
            return -1;
        }
    }

    /* ����Ƿ��п��� */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "�ǿ��ƽ���[%s],������޶�.", sTransCode);
        return 0;
    }


    /* �̻�������ת�ַ�����,�޸��̻���� */
    {
        dTranAmt /= 100;

        /* �����ַ����� */
        if (FindChannelRule(&stPufaLimitRule, sChanneMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "δ�ҵ������̻�[%s]���޶����.", sChanneMerchId);
            return -1;
        }
        /* �����ַ������̻����� */
        if (FindMerchLimitRule(&stPufaLimitRule, sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "δ�ҵ������̻�[%s]���޶����.", sMerchId);
            return -1;
        }

        if ('-' == g_stTransCode.sAccFlag[0]) {/* ������Ϊ������ʱ�����ж��޶� */
            dTranAmt = -dTranAmt;
        } else {
            /*�����̻�Ϊ�����Խ��̻����жϹ���*/
            if (stPufaLimitRule.sMerchType[0] == '1') {
                //�����̻����������޶�ͱ���
                if (DBL_CMP(stPufaLimitRule.dLocalUsedlimit + dTranAmt, stPufaLimitRule.dLocalTotallimit)) {
                    tLog(INFO, "�ַ������̻�[%s]�����ѽ��׽���,���޶�[%f],��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]"
                            , sMerchId, stPufaLimitRule.dLocalTotallimit, stPufaLimitRule.dLocalUsedlimit, dTranAmt);
                    if (ChkTransContol(sMerchId) < 0) {
                        ErrHanding(pstTransJson, "W8", "�����̻�[%s]��������ת����,����ʧ��.", sMerchId);
                        tLog(INFO, "�����������̻��������ƽ��ף�������������");
                        return -1;
                    } else {
                        tLog(INFO, "�����ַ��������Ʋ���ת�Ż��������̻�!,��������ͨ�̻�");
                        DEL_KEY(pstTransJson, "channel_id");
                        return 0;
                    }
                }
            }
            //�����̻����������޶�
            if (DBL_CMP(stPufaLimitRule.dUsedlimit + dTranAmt, stPufaLimitRule.dTotallimit)) {
                tLog(INFO, "�����̻�[%s]�����ѽ��׽���,���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sChanneMerchId, stPufaLimitRule.dTotallimit, stPufaLimitRule.dUsedlimit, dTranAmt);
                if (ChkTransContol(sMerchId) < 0) {
                    ErrHanding(pstTransJson, "W8", "�����̻�[%s]��������ת����,����ʧ��.", sMerchId);
                    tLog(INFO, "�����������̻��������ƽ��ף�������������");
                    return -1;
                } else {
                    tLog(INFO, "�����ַ��������Ʋ���ת�Ż��������̻�!,��������ͨ�̻�");
                    DEL_KEY(pstTransJson, "channel_id");
                    return 0;
                }
            }
        }


        /*���ڸ����ۼ��޶�*/
        if (UptChannelMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
            ErrHanding(pstTransJson, "96", "�����̻�[%s]�����ۼƶ�[%f]ʧ��.", sChanneMerchId, dTranAmt);
            return -1;
        }

        /* �������̻����±����̻��޶� */
        if (stPufaLimitRule.sMerchType[0] == '1') {
            /*���ڸ��±����̻��ۼ��޶�*/
            if (UptChannelLocalMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
                ErrHanding(pstTransJson, "96", "�����̻�[%s]�����ۼƶ�[%f]ʧ��.", sMerchId, dTranAmt);
                return -1;
            }
        }
        tLog(INFO, "�Ż��̻�[%s]�ͱ����̻�[%s]���¶�ȳɹ�!", sChanneMerchId, sMerchId);
        tLog(INFO, "ʹ�������̻�[%s:%s:%s].", sChanneMerchId, sChanneTermId, sChannelId);
        /* ���½�����ˮ��channel_id �������ڵ��ʽ���û���ص�ʱ��������Ҳ���channel_id�����Ҳ����ַ�*/
        UpdatePufaChannelId(sMerchId, sTraceNo, sTransTime, sChannelId, sChanneMerchId, sChanneTermId);

    }
    return 0;
}

int UnChannelLimit(cJSON *pstJson, int *piFlag) {

    char sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    char sCardType[1 + 1] = {0};
    char sChanneMerchId[15 + 1] = {0}, sChannelId[8 + 1] = {0};
    cJSON * pstTransJson = NULL;
    PufaLimitRule stPufaLimitRule;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "channel_id", sChannelId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);


    if (memcmp(sChannelId, "48560000", 8) && memcmp(sChannelId, "49000000", 8)) {
        tLog(INFO, "�̻���������ת�Ż�����������");
        return 0;
    }

    dTranAmt /= 100;
    if (!memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "�̻�[%s]���׳ɹ�,����ָ��޶�[%f].", sMerchId, dTranAmt);
        return 0;
    }

    /* ����Ƿ��п��� */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "�ǿ��ƽ���[%s],������޶�.", sTransCode);
        return 0;
    }
    if ('+' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }

    /* ���ҹ��� */
    if (FindChannelRule(&stPufaLimitRule, sChanneMerchId) < 0) {
        tLog(INFO, "δ�ҵ������̻�[%s]���޶����.", sChanneMerchId);
        return -1;
    }

    /* �����ַ������̻����� */
    if (FindMerchLimitRule(&stPufaLimitRule, sMerchId) < 0) {
        tLog(INFO, "δ�ҵ������̻�[%s]���޶����.", sMerchId);
        return -1;
    }

    /* ���ڻָ��ۼ��޶� */
    if (UptChannelMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
        ErrHanding(pstTransJson, "96", "�����̻�[%s]�����ۼƶ�[%f]ʧ��.", sChanneMerchId, dTranAmt);
        return -1;
    }
    /* �������̻����±����̻��޶� */
    if (stPufaLimitRule.sMerchType[0] == '1') {
        /*���ڸ��±����̻��ۼ��޶�*/
        if (UptChannelLocalMerchLimitEx(dTranAmt, &stPufaLimitRule) < 0) {
            ErrHanding(pstTransJson, "96", "�����̻�[%s]�����ۼƶ�[%f]ʧ��.", sMerchId, dTranAmt);
            return -1;
        }
    }

    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f]", sMerchId, dTranAmt);
    return 0;

}

/* ����Ȧ�̻�ת�� */
int ConvLifeMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sMerchName[256 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sChannelTermId[8 + 1] = {0}, sChanneMerchId[15 + 1] = {0}, sAgentId[8 + 1] = {0}, sChannelId[8 + 1] = {0};
    char sChannelMerchName[256 + 1] = {0};
    cJSON * pstTransJson = NULL;
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "merch_p_name", sMerchName);
    GET_STR_KEY(pstTransJson, "agent_id", sAgentId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "channel_id", sChannelId);

    if (!memcmp(sChannelId, "48560000", 8) || !memcmp(sChannelId, "49000000", 8)) {
        tLog(INFO, "�̻�������ת�Ż��̻���������ת�Ż�����");
        return 0;
    }

    if (FindLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode, sAgentId) < 0) {
        tLog(DEBUG, "�̻�[%s]��������Ȧ�̻�ʧ��,ʹ��ԭ�̻���.", sMerchId);
        SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
        SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
        SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
        tLog(INFO, "ʹ�ñ����̻�[%s:%s].", sMerchId, sTermId);
    } else {
        if (sChanneMerchId[0] != '\0' && sChannelTermId[0] != '\0') {
            tLog(DEBUG, "�̻�[%s]ʹ������Ȧ�̻�[%s:%s].", sMerchId, sChanneMerchId, sChannelTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sChannelTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
            tLog(INFO, "�����̻�[%s:%s]�����̻�[%s:%s].", sMerchId, sTermId, sChanneMerchId, sChannelTermId);
        } else {
            SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            tLog(INFO, "ʹ�ñ����̻�[%s:%s].", sMerchId, sTermId);
        }
    }
    return 0;
}

/* ������Ȧ����ת�������̻��� */
int ConvCupsMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
    tLog(INFO, "ʹ�ñ����̻�[%s:%s].", sMerchId, sTermId);
    return 0;
}

/* ��������Ҫʹ�������׵������̻��� */
int GetMerchName(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sChannelMerchId[15 + 1] = {0};
    char sMerchName[40 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChannelMerchId);
    if (memcmp(sMerchId, sChannelMerchId, 15)) {
        /* ����Ȧ�̻� */
        if (FindLifeMerchName(sMerchName, sChannelMerchId) < 0) {
            tLog(ERROR, "����LIFE�̻�[%s]���̻���ʧ��.", sChannelMerchId);
            if (FindChannelMerchName(sMerchName, sChannelMerchId) < 0) {
                tLog(ERROR, "���������Ż����̻�[%s]���̻���ʧ��.", sChannelMerchId);

            } else {
                tLog(DEBUG, "ʹ���Ż��������̻�[%s]������[%s].", sChannelMerchId, sMerchName);
                SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            }
        } else {
            tLog(DEBUG, "ʹ��LIFE�̻�[%s]������[%s].", sChannelMerchId, sMerchName);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
        }
    }
    return 0;
}