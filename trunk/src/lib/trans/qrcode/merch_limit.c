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
/*      ������:     ChkMerchWxLimit()                                             */
/*      ����˵��:   �̻�΢���޶��鴦�����������                                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int ChkMerchWxLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char  sMerchLevel[1+1] = {0}; //add by GJQ at 20180306
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
    /*begin add by GJQ at 20180306*/
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
    /*end add by GJQ at 20180306*/
    
    dTranAmt /= 100;

    /* ���ҹ��� */
    //if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
    /*motify by GJQ at  20180305*/
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶����.", sMerchId);
        return -1;
    }
    /* �����ۼƶ� */
    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶�����.", sMerchId);
        return -1;
    }

    /* ����Ƿ��п��� */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "�ǿ��ƽ���[%s],������޶�.", sTransCode);
        return 0;
    }
    if (NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        //ErrHanding(pstTransJson, "40", "��ֹ�Ľ���[%s].", sTransCode);
        ErrHanding(pstTransJson, "B1", "��ֹ�Ľ���[%s].", sTransCode); //motify by GJQ at 20180306
        return -1;
    }
    if ('-' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }
    //���������޶�
    if (!DBL_ZERO(stLimitRule.dWXLimit) && DBL_CMP(dTranAmt, stLimitRule.dWXLimit)) {
        ErrHanding(pstTransJson, "B7", "�̻�[%s]����[%s]���ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dWXLimit, dTranAmt);
        return -1;
    }
    //���������޶�
    if (!DBL_ZERO(stLimitRule.dDWXLimit) && DBL_CMP(stMerchLimit.dDWXLimit + dTranAmt, stLimitRule.dDWXLimit)) {
        ErrHanding(pstTransJson, "B8", "�̻�[%s]�����ѽ��׽���! \n"
                "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                , sMerchId, stLimitRule.dDWXLimit, stMerchLimit.dDWXLimit, dTranAmt);
        return -1;
    }
    //�������޶�
    if (!DBL_ZERO(stLimitRule.dMWXLimit) && DBL_CMP(stMerchLimit.dMWXLimit + dTranAmt, stLimitRule.dMWXLimit)) {
        ErrHanding(pstTransJson, "B9", "�̻�[%s]�����ѽ��׽���! \n"
                "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                , sMerchId, stLimitRule.dMWXLimit, stMerchLimit.dMWXLimit, dTranAmt);
        return -1;
    }
    /*���ڸ����ۼ��޶�*/
    if (UptMerchLimitEx(dTranAmt, 'W', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f],���ۼ�[%f],���ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dDWXLimit, stMerchLimit.dMWXLimit);
    return 0;
}

/******************************************************************************/
/*      ������:     UnMerchWxLimit()                                             */
/*      ����˵��:   ����ʧ��,�ָ��޶�                                         */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UnMerchWxLimit(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

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
    if (UptMerchLimitEx(dTranAmt, 'W', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f]", sMerchId, dTranAmt);
    return 0;
}

/******************************************************************************/
/*      ������:     ChkMerchBbLimit()                                             */
/*      ����˵��:   �̻�֧�����޶��鴦�����������                                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int ChkMerchBbLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char  sMerchLevel[1+1] = {0}; //add by GJQ at 20180306
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
    /*begin add by GJQ at 20180306*/
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
    /*end add by GJQ at 20180306*/
    
    dTranAmt /= 100;

    /* ���ҹ��� */
    //if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
    /*motify by GJQ at  20180305*/
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶����.", sMerchId);
        return -1;
    }
    /* �����ۼƶ� */
    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶�����.", sMerchId);
        return -1;
    }

    /* ����Ƿ��п��� */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "�ǿ��ƽ���[%s],������޶�.", sTransCode);
        return 0;
    }
    if (NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        //ErrHanding(pstTransJson, "40", "��ֹ�Ľ���[%s].", sTransCode);
        ErrHanding(pstTransJson, "B1", "��ֹ�Ľ���[%s].", sTransCode); //motify by GJQ at 20180306
        return -1;
    }
    if ('-' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }
    //���������޶�
    if (!DBL_ZERO(stLimitRule.dBBLimit) && DBL_CMP(dTranAmt, stLimitRule.dBBLimit)) {
        ErrHanding(pstTransJson, "B7", "�̻�[%s]����[%s]���ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dBBLimit, dTranAmt);
        return -1;
    }
    //���������޶�
    if (!DBL_ZERO(stLimitRule.dDBBLimit) && DBL_CMP(stMerchLimit.dDBBLimit + dTranAmt, stLimitRule.dDBBLimit)) {
        ErrHanding(pstTransJson, "B8", "�̻�[%s]�����ѽ��׽���! \n"
                "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                , sMerchId, stLimitRule.dDBBLimit, stMerchLimit.dDBBLimit, dTranAmt);
        return -1;
    }
    if (!DBL_ZERO(stLimitRule.dMBBLimit) && DBL_CMP(stMerchLimit.dMBBLimit + dTranAmt, stLimitRule.dMBBLimit)) {
        ErrHanding(pstTransJson, "B9", "�̻�[%s]�����ѽ��׽���! \n"
                "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                , sMerchId, stLimitRule.dMBBLimit, stMerchLimit.dMBBLimit, dTranAmt);
        return -1;
    }
    /*���ڸ����ۼ��޶�*/
    if (UptMerchLimitEx(dTranAmt, 'B', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f],���ۼ�[%f],���ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dDBBLimit, stMerchLimit.dMBBLimit);
    return 0;
}

/******************************************************************************/
/*      ������:     UnMerchBbLimit()                                             */
/*      ����˵��:   ����ʧ��,�ָ��޶�                                         */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UnMerchBbLimit(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sRespCode[2 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

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
    if (UptMerchLimitEx(dTranAmt, 'B', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f]", sMerchId, dTranAmt);
    return 0;
}

/*pos��ά�볷�� �޷�����֧������΢�ţ�֧������΢�� ��ά�볷������޶� ��ʹ�ô˽�����*/
int ChkMerchTbCodeLimit(cJSON *pstJson, int *piFlag) {
    char sTransCode[6 + 1] = {0};
    cJSON * pstTransJson = NULL;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    
    tLog(DEBUG,"�˴γ��������룺sTransCode= [%s]",sTransCode);
    if(!memcmp(sTransCode,"02W600",6)) {
        if(ChkMerchWxLimit(pstJson, piFlag) < 0) {
            return ( -1 );
        }
            
    }
    else {
        if(ChkMerchBbLimit(pstJson, piFlag) < 0) {
            return (-1);
        }
    }
    return ( 0 );
}
/*pos��ά�볷�� �޷�����֧������΢�ţ�֧������΢�� ��ά�볷������޶� ��ʹ�ô˽�����*/
int UnMerchTbCodeLimit(cJSON *pstJson, int *piFlag) {
    char sTransCode[6 + 1] = {0};
    cJSON * pstTransJson = NULL;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    
    tLog(DEBUG,"�˴γ��������룺sTransCode= [%s]",sTransCode);
    if(!memcmp(sTransCode,"02W600",6)) {
        if( UnMerchWxLimit(pstJson, piFlag) < 0 ) {
            return ( -1 );
        }
    }
    else {
        if( UnMerchBbLimit(pstJson, piFlag) < 0 ) {
            return ( -1 );
        }
    }
    return ( 0 );
}


/******************************************************************************/
/*      ������:     ChkMerchCupsLimit()                                             */
/*      ����˵��:   �̻�������ά���޶��鴦�����������                                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
/*������ά���޶��� ʹ��pos��ͨ�����޶��� �˴������ǿ��޶��������*/
int ChkMerchCupsLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char  sMerchLevel[1+1] = {0}; //add by GJQ at 20180306
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
    /*begin add by GJQ at 20180306*/
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
    /*end add by GJQ at 20180306*/
    
    dTranAmt /= 100;

    /* ���ҹ��� */
     //if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
    /*motify by GJQ at  20180305*/
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶����.", sMerchId);
        return -1;
    }
    /* �����ۼƶ� */
    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶�����.", sMerchId);
        return -1;
    }

    /* ����Ƿ��п��� */
    if (g_stTransCode.sSettleFlag[0] == '0') {
        tLog(INFO, "�ǿ��ƽ���[%s],������޶�.", sTransCode);
        return 0;
    }
    if (NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        //ErrHanding(pstTransJson, "40", "��ֹ�Ľ���[%s].", sTransCode);
        ErrHanding(pstTransJson, "B1", "��ֹ�Ľ���[%s].", sTransCode); //motify by GJQ at 20180306
        return -1;
    }
    if ('-' == g_stTransCode.sAccFlag[0]) {
        dTranAmt = -dTranAmt;
    }
    //���������޶�
    if (!DBL_ZERO(stLimitRule.dCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dCPLimit)) {
        ErrHanding(pstTransJson, "B7", "�̻�[%s]����[%s]���ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dCPLimit, dTranAmt);
        return -1;
    }
    //���������޶�
    if (!DBL_ZERO(stLimitRule.dCDLimit) && DBL_CMP(stMerchLimit.dCDLimit + dTranAmt, stLimitRule.dCDLimit)) {
        ErrHanding(pstTransJson, "B8", "�̻�[%s]�����ѽ��׽���! \n"
                "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                , sMerchId, stLimitRule.dCDLimit, stMerchLimit.dCDLimit, dTranAmt);
        return -1;
    }
    //�������޶�
    if (!DBL_ZERO(stLimitRule.dCMLimit) && DBL_CMP(stMerchLimit.dCMLimit + dTranAmt, stLimitRule.dCMLimit)) {
        ErrHanding(pstTransJson, "B9", "�̻�[%s]�����ѽ��׽���! \n"
                "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                , sMerchId, stLimitRule.dCMLimit, stMerchLimit.dCMLimit, dTranAmt);
        return -1;
    }
    /*���ڸ����ۼ��޶�*/
    if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��(������ά��ʹ��pos��ͨ���Ѵ��ǿ��޶�).", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f],���ۼ�[%f],���ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dCDLimit, stMerchLimit.dCMLimit);
    return 0;
}

/******************************************************************************/
/*      ������:     UnMerchCupsLimit()                                             */
/*      ����˵��:   �̻�������ά�뽻��ʧ��,�ָ��޶�                                         */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
/*������ά��ָ��޶� ʹ��pos��ͨ���׻ָ��޶� �˴������ǿ��޶��������*/
int UnMerchCupsLimit(cJSON *pstJson, int *piFlag) {
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
    if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��.", sMerchId, dTranAmt);
        return -1;
    }
   
    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f]", sMerchId, dTranAmt);
    
    return ( 0 );
}

