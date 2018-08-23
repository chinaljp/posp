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
/*******************************************strstr***********************************/
/*      ������:     ChkMerchCardLimit()                                             */
/*      ����˵��:   �̻����п��޶��鴦�����������                                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int ChkMerchCardLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    LimitRule stLimitRule;
    MerchLimit stMerchLimit;
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sOTransCode[9] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "o_trans_code", sOTransCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    dTranAmt /= 100;
    /* ���ҹ��� */
    if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶����.", sMerchId);
        return -1;
    }
    //��齻���Ƿ�����
    if (sTransCode[5] != '3' && NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        ErrHanding(pstTransJson, "40", "��ֹ�Ľ���[%s].", sTransCode);
        return -1;
    }
    /* ����Ƿ��п��� */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "�ǿ��ƽ���[%s],������޶�.", sTransCode);
        return 0;
    }
    tLog(DEBUG, "�����Ѿ��ҵ�");
    /* �����ۼƶ� */
    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���ۼƶ�����.", sMerchId);
        return -1;
    }

    if ('-' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }
    //���������޶�
    /*���ÿ���ȼ��*/
    if (sCardType[0] == '1') {
        if (!DBL_ZERO(stLimitRule.dCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dCPLimit)) {
            ErrHanding(pstTransJson, "C3", "�̻�[%s]����[%s]���ÿ����ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dCPLimit, dTranAmt);
            return -1;
        }
        //���������޶�
        if (!DBL_ZERO(stLimitRule.dCDLimit) && DBL_CMP(stMerchLimit.dCDLimit + dTranAmt, stLimitRule.dCDLimit)) {
            ErrHanding(pstTransJson, "C4", "�̻�[%s]���ÿ������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dCDLimit, stMerchLimit.dCDLimit, dTranAmt);
            return -1;
        }
        
        if (!DBL_ZERO(stLimitRule.dCMLimit) && DBL_CMP(stMerchLimit.dCMLimit + dTranAmt, stLimitRule.dCMLimit)) {
            ErrHanding(pstTransJson, "C5", "�̻�[%s]���ÿ������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dCMLimit, stMerchLimit.dCMLimit, dTranAmt);
            return -1;
        }
        
        /*���ڸ����ۼ��޶�*/
        if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "�̻�[%s]���ÿ������޶�[%f]ʧ��.", sMerchId, dTranAmt);
            return -1;
        }
        tLog(INFO, "�̻�[%s]���ÿ��޶�ָ��ɹ�,���׽��[%f],���ۼ�[%f],���ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dCDLimit, stMerchLimit.dCMLimit);

    } else {
        if (!DBL_ZERO(stLimitRule.dDPLimit) && DBL_CMP(dTranAmt, stLimitRule.dDPLimit)) {
            ErrHanding(pstTransJson, "C3", "�̻�[%s]����[%s]���ÿ����ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dDPLimit, dTranAmt);
            return -1;
        }
        //���������޶�
        if (!DBL_ZERO(stLimitRule.dDDLimit) && DBL_CMP(stMerchLimit.dDDLimit + dTranAmt, stLimitRule.dDDLimit)) {
            ErrHanding(pstTransJson, "C4", "�̻�[%s]���ÿ������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dDDLimit, stMerchLimit.dDDLimit, dTranAmt);
            return -1;
        }
        if (!DBL_ZERO(stLimitRule.dDMLimit) && DBL_CMP(stMerchLimit.dDMLimit + dTranAmt, stLimitRule.dDMLimit)) {
            ErrHanding(pstTransJson, "C5", "�̻�[%s]���ÿ������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dDMLimit, stMerchLimit.dDMLimit, dTranAmt);
            return -1;
        }
        /*���ڸ����ۼ��޶�*/
        if (UptMerchLimitEx(dTranAmt, 'D', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "�̻�[%s]��ǿ������޶�[%f]ʧ��.", sMerchId, dTranAmt);
            return -1;
        }
        tLog(INFO, "�̻�[%s]��ǿ��޶�ָ��ɹ�,���׽��[%f],���ۼ�[%f],���ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dDDLimit, stMerchLimit.dDMLimit);
    }

    return 0;
}

/******************************************************************************/
/*      ������:     UnMerchCardLimit()                                             */
/*      ����˵��:   ����ʧ��,�ָ��޶�                                         */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UnMerchCardLimit(cJSON *pstJson, int *piFlag) {
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
    GET_STR_KEY(pstTransJson, "card_type", sCardType);

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
    /*���ڻָ��ۼ��޶�*/
    if (sCardType[0] == '1') {
        if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��.", sMerchId, dTranAmt);
            return -1;
        }
    } else {
        if (UptMerchLimitEx(dTranAmt, 'D', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��.", sMerchId, dTranAmt);
            return -1;
        }
    }
    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f]", sMerchId, dTranAmt);
    return 0;
}