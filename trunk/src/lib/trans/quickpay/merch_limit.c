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
/*      ������:     ChkMerchCardLimit()                                       */
/*      ����˵��:   �̻���������޶��鴦�����������                     */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int ChkMerchCardLimit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    char sMerchLevel[1+1] = {0};
    LimitRule stLimitRule;
    MerchLimit stMerchLimit;
    double dTranAmt = 0.0L;
    char sTransCode[9] = {0}, sOTransCode[9] = {0};
    cJSON * pstTransJson = NULL;
    char cUpdWay;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "o_trans_code", sOTransCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);

    dTranAmt /= 100;
#if 0    
    /*Begin �����ཻ�ס��˻����ף�����ԭ�̻��Ż�ȡ�̻����� add by GJQ at 20180309*/
    if( sTransCode[5] == '3' || sTransCode[5] == '1') 
    {
        if( FindMerchLevel(sMerchId, sMerchLevel) < 0 ) {
            ErrHanding(pstTransJson, "96", "��������ԭ�̻�[%s]����.", sMerchId);
            return ( -1 );
        }
        tLog(INFO,"ԭ�̻�[%s]�ļ���Ϊ[%s]",sMerchId,sMerchLevel);
    }
    /*End �����ཻ�ף�����ԭ�̻��Ż�ȡ�̻����� add by GJQ at 20180309*/
#endif    
    /* ���ҹ��� */
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶����.", sMerchId);
        return -1;
    }
    //��齻���Ƿ�����
    if (sTransCode[5] != '3' && NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        ErrHanding(pstTransJson, "B1", "��ֹ�Ľ���[%s].", sTransCode);
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
        if (!DBL_ZERO(stLimitRule.dQCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dQCPLimit)) {
            ErrHanding(pstTransJson, "C3", "�̻�[%s]����[%s]���ÿ�������ѵ��ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dQCPLimit, dTranAmt);
            return -1;
        }
        //���������޶�
        if (!DBL_ZERO(stLimitRule.dQCDLimit) && DBL_CMP(stMerchLimit.dQCDLimit + dTranAmt, stLimitRule.dQCDLimit)) {
            ErrHanding(pstTransJson, "C4", "�̻�[%s]���ÿ������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dQCDLimit, stMerchLimit.dQCDLimit, dTranAmt);
            return -1;
        }
        
        if (!DBL_ZERO(stLimitRule.dQCMLimit) && DBL_CMP(stMerchLimit.dQCMLimit + dTranAmt, stLimitRule.dQCMLimit)) {
            ErrHanding(pstTransJson, "C5", "�̻�[%s]���ÿ������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dQCMLimit, stMerchLimit.dQCMLimit, dTranAmt);
            return -1;
        }
        
        /*���ڸ����ۼ��޶�*/
        cUpdWay = 'Q';
        tLog(INFO, "�̻�[%s]���ν��׽��[%f],������Ѵ��ǿ����ۼ�[%f],������Ѵ��ǿ����ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dQCDLimit, stMerchLimit.dQCMLimit);
    } else {
        if (!DBL_ZERO(stLimitRule.dQDPLimit) && DBL_CMP(dTranAmt, stLimitRule.dQDPLimit)) {
            ErrHanding(pstTransJson, "B4", "�̻�[%s]����[%s]��ǿ����ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dQDPLimit, dTranAmt);
            return -1;
        }
        //���������޶�
        if (!DBL_ZERO(stLimitRule.dQDDLimit) && DBL_CMP(stMerchLimit.dQDDLimit + dTranAmt, stLimitRule.dQDDLimit)) {
            ErrHanding(pstTransJson, "B5", "�̻�[%s]��ǿ������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dQDDLimit, stMerchLimit.dQDDLimit, dTranAmt);
            return -1;
        }
        if (!DBL_ZERO(stLimitRule.dQDMLimit) && DBL_CMP(stMerchLimit.dQDMLimit + dTranAmt, stLimitRule.dQDMLimit)) {
            ErrHanding(pstTransJson, "B6", "�̻�[%s]��ǿ������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dQDMLimit, stMerchLimit.dQDMLimit, dTranAmt);
            return -1;
        }
        /*���ڸ����ۼ��޶�*/
        cUpdWay = 'P';
        tLog(INFO, "�̻�[%s]���ν��׽��[%f],������ѽ�ǿ����ۼ�[%f],������ѽ�ǿ����ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dQDDLimit, stMerchLimit.dQDMLimit);
    }
    
    /*�����ۼ��޶�*/
    tLog( DEBUG,"cUpdWay = [%c]:[%s]", cUpdWay,GETMESG(cUpdWay) );
    if ( UptMerchLimitEx(dTranAmt, cUpdWay, sMerchId) < 0 ) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]%s�����޶�[%f]ʧ��.", sMerchId, GETMESG(cUpdWay), dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]%s�޶��ۼƳɹ�,���׽��[%f]", sMerchId, GETMESG(cUpdWay), dTranAmt);
    
    return 0;
}

/******************************************************************************/
/*      ������:     UnMerchCupsLimit()                                             */
/*      ����˵��:   �̻��޿�������ѽ���ʧ��,�ָ��޶�                                         */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
/*���޶�ָ��ݲ��õ��� �������޸�-20180704*/
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

