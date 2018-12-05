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
/*      ������:     ChkMerchCardLimit()                                             */
/*      ����˵��:   �̻����п��޶��鴦�����������                                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

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
      * ���� �����ۼƽ��׶�ʱ�����ִ�������ǿ���IC����ǿ�
      * ���ֿ��Ľ��׽���ۼ�;
      *  C - IC���ÿ���D - IC��ǿ���
      *  F - �������ÿ���E - ������ǿ�
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
    
    /*begin add by GuoJiaQing at 20180625 Ԥ��Ȩ��ɳ������ף� ԭ���ף�Ԥ��Ȩ��ɣ������뷽ʽҲΪ���� ʱ����ȥ���� Ԥ��Ȩ��ɵ�ԭ���� ��ȡ ���뷽ʽ*/
    if( !memcmp(sInputMode,"01",2) ) {
        MEMSET(sInputMode);
        GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
        GET_STR_KEY(pstTransJson, "o_rrn", sRrn);
        if( FindOrigOInputMode(sInputMode,sRrn,sTransDate,"b_pos_trans_detail") < 0 ) {
            if( FindOrigOInputMode(sInputMode,sRrn,sTransDate,"b_pos_trans_detail_his") < 0 ) {
                ErrHanding(pstTransJson, "96", "��������ԭ���׵�ԭ�������뷽ʽ��");
                return ( -1 );
            }
        }
    }
    /*end add by GuoJiaQing at 20180625*/

    dTranAmt /= 100;
    
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
    
    /*Begin Z���̻����� ֻ�������еĿ� ��ˢ������ add by gjq at 20180919*/
    if( sMerchLevel[0] == '4' ) {
        GET_STR_KEY(pstTransJson,"card_encno",sCardNoEnc);
        if ( FindMerchCard( sMerchId,sCardNoEnc ) < 0 ) {
            ErrHanding(pstTransJson, "57", "�̻�[%s]ΪZ���̻�������ǿ������Ž���.", sMerchId);
            return -1;
        }
    }
    /*End Z���̻����� ֻ�������еĿ� ��ˢ������  add by gjq*/
    
    /* ���ҹ��� */
    //if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
    /*motify by GJQ at  20180305*/
    if (FindLimitRule(&stLimitRule, sMerchId, sMerchLevel) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶����.", sMerchId);
        return -1;
    }
    //��齻���Ƿ�����
    if (sTransCode[5] != '3' && NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        //ErrHanding(pstTransJson, "40", "��ֹ�Ľ���[%s].", sTransCode);
        ErrHanding(pstTransJson, "B1", "��ֹ�Ľ���[%s].", sTransCode); //motify by GJQ at 20180305
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
    
    /*���ÿ���ȼ��*/
    if (sCardType[0] == '1') {
        /*begin motify by GJQ at 20180613 */
        tLog(DEBUG,"sCardType[%s]:[���ÿ�]",sCardType);
        tLog(DEBUG,"sInputMode[%s],sMCFlag[%s],sICFlag[%s],(0����ֹ��1:����)",
                sInputMode,stLimitRule.sMCFlag,stLimitRule.sICFlag);
        
        if ( !memcmp(sInputMode,"02",2)  ) 
        {
            /*��MC_CARD_FLAG �������ǿ����ױ�־ 0����ֹ��1:����*/
            if( stLimitRule.sMCFlag[0] == '0' ) {  
                ErrHanding(pstTransJson, "B2", "��ֹ���̻�[%s]ʹ�ô������ǿ�����.", sMerchId);
                return ( -1 );
            }
            /*�����������޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dMCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dMCPLimit)) {
                ErrHanding(pstTransJson, "CC", "�̻�[%s]����[%s]�������ÿ����ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dMCPLimit, dTranAmt);
                return -1;
            }
            /*���������޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dMCDLimit) && DBL_CMP(stMerchLimit.dMCDLimit + dTranAmt, stLimitRule.dMCDLimit)) {
                ErrHanding(pstTransJson, "CD", "�̻�[%s]�������ÿ������ѽ��׽���! \n"
                        "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sMerchId, stLimitRule.dMCDLimit, stMerchLimit.dMCDLimit, dTranAmt);
                return -1;
            }
            /*���������޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dMCMLimit) && DBL_CMP(stMerchLimit.dMCMLimit + dTranAmt, stLimitRule.dMCMLimit)) {
                ErrHanding(pstTransJson, "CE", "�̻�[%s]�������ÿ������ѽ��׽���! \n"
                        "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sMerchId, stLimitRule.dMCMLimit, stMerchLimit.dMCMLimit, dTranAmt);
                return -1;
            }
            cUpdWay = 'F';
            tLog(INFO, "�̻�[%s]���ν��׽��[%f],�������ÿ����ۼ�[%f],�������ÿ����ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dMCDLimit, stMerchLimit.dMCMLimit);
        }
        else if ( !memcmp(sInputMode,"05",2) || !memcmp(sInputMode,"07",2) )
        {
            /*��IC_CARD_FLAG IC���ǿ����ױ�־ 0����ֹ��1:����*/
            if ( stLimitRule.sICFlag[0] == '0' ) {
                ErrHanding(pstTransJson, "B2", "��ֹ���̻�[%s]ʹ��IC���ǿ�����.", sMerchId);
                return ( -1 );
            }
            /*IC�������޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dCPLimit) && DBL_CMP(dTranAmt, stLimitRule.dCPLimit)) {
                ErrHanding(pstTransJson, "C3", "�̻�[%s]����[%s]IC���ÿ����ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dCPLimit, dTranAmt);
                return -1;
            }
            /*IC�����޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dCDLimit) && DBL_CMP(stMerchLimit.dCDLimit + dTranAmt, stLimitRule.dCDLimit)) {
                ErrHanding(pstTransJson, "C4", "�̻�[%s]IC���ÿ������ѽ��׽���! \n"
                        "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sMerchId, stLimitRule.dCDLimit, stMerchLimit.dCDLimit, dTranAmt);
                return -1;
            }
            /*IC�����޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dCMLimit) && DBL_CMP(stMerchLimit.dCMLimit + dTranAmt, stLimitRule.dCMLimit)) {
                ErrHanding(pstTransJson, "C5", "�̻�[%s]IC���ÿ������ѽ��׽���! \n"
                        "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sMerchId, stLimitRule.dCMLimit, stMerchLimit.dCMLimit, dTranAmt);
                return -1;
            }
            cUpdWay = 'C';
            tLog(INFO, "�̻�[%s]���ν��׽��[%f],IC���ÿ����ۼ�[%f],IC���ÿ����ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dCDLimit, stMerchLimit.dCMLimit);
        }
        /*end motify by GJQ at 20180613   */ 
        /*���ڸ����ۼ��޶�*/
        /*if (UptMerchLimitEx(dTranAmt, 'C', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "�̻�[%s]���ÿ������޶�[%f]ʧ��.", sMerchId, dTranAmt);
            return -1;
        }*/
        //tLog(INFO, "�̻�[%s]���ÿ��޶��ۼƳɹ�,���׽��[%f],���ۼ�[%f],���ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dCDLimit, stMerchLimit.dCMLimit);
    } else {
        /*  begin motify by GJQ at 20180613 */
        tLog(DEBUG,"sCardType[%s]:[��ǿ�]",sCardType);
        tLog(DEBUG,"sInputMode[%s],sMDFlag[%s],sIDFlag[%s],(0����ֹ��1:����)",
                sInputMode,stLimitRule.sMDFlag,stLimitRule.sIDFlag);
        
        if ( !memcmp(sInputMode,"02",2) ) 
        {   
            /*��MD_CARD_FLAG ������ǿ����ױ�־ 0����ֹ��1:����*/
            if ( stLimitRule.sMDFlag[0] == '0' ) {
                ErrHanding(pstTransJson, "B2", "��ֹ���̻�[%s]ʹ�ô�����ǿ�����.", sMerchId);
                return ( -1 );
            }
            /*�����������޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dMDPLimit) && DBL_CMP(dTranAmt, stLimitRule.dMDPLimit)) {
                ErrHanding(pstTransJson, "BA", "�̻�[%s]����[%s]������ǿ����ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dMDPLimit, dTranAmt);
                return -1;
            }
            /*���������޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dMDDLimit) && DBL_CMP(stMerchLimit.dMDDLimit + dTranAmt, stLimitRule.dMDDLimit)) {
                ErrHanding(pstTransJson, "BB", "�̻�[%s]������ǿ������ѽ��׽���! \n"
                        "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sMerchId, stLimitRule.dMDDLimit, stMerchLimit.dMDDLimit, dTranAmt);
                return -1;
            }
            /*���������޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dMDMLimit) && DBL_CMP(stMerchLimit.dMDMLimit + dTranAmt, stLimitRule.dMDMLimit)) {
                ErrHanding(pstTransJson, "BC", "�̻�[%s]������ǿ������ѽ��׽���! \n"
                        "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sMerchId, stLimitRule.dMDMLimit, stMerchLimit.dMDMLimit, dTranAmt);
                return -1;
            }
            cUpdWay = 'E';
            tLog(INFO, "�̻�[%s]���ν��׽��[%f],������ǿ����ۼ�[%f],������ǿ����ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dMDDLimit, stMerchLimit.dMDMLimit);
        }
        else if ( !memcmp(sInputMode,"05",2) || !memcmp(sInputMode,"07",2) )
        {   
            /*��ID_CARD_FLAG IC��ǿ����ױ�־ 0����ֹ��1:���� */
            if ( stLimitRule.sIDFlag[0] == '0' ) {
                ErrHanding(pstTransJson, "B2", "��ֹ���̻�[%s]ʹ��IC��ǿ�����.", sMerchId);
                return ( -1 );
            }
            /*IC�������޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dDPLimit) && DBL_CMP(dTranAmt, stLimitRule.dDPLimit)) {
                ErrHanding(pstTransJson, "B4", "�̻�[%s]����[%s]IC��ǿ����ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dDPLimit, dTranAmt);
                return -1;
            }
            /*IC�����޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dDDLimit) && DBL_CMP(stMerchLimit.dDDLimit + dTranAmt, stLimitRule.dDDLimit)) {
                ErrHanding(pstTransJson, "B5", "�̻�[%s]IC��ǿ������ѽ��׽���! \n"
                        "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sMerchId, stLimitRule.dDDLimit, stMerchLimit.dDDLimit, dTranAmt);
                return -1;
            }
            /*IC�����޶��ж�*/
            if (!DBL_ZERO(stLimitRule.dDMLimit) && DBL_CMP(stMerchLimit.dDMLimit + dTranAmt, stLimitRule.dDMLimit)) {
                ErrHanding(pstTransJson, "B6", "�̻�[%s]IC��ǿ������ѽ��׽���! \n"
                        "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                        , sMerchId, stLimitRule.dDMLimit, stMerchLimit.dDMLimit, dTranAmt);
                return -1;
            }
            cUpdWay = 'D';
            tLog(INFO, "�̻�[%s]���ν��׽��[%f],IC��ǿ����ۼ�[%f],IC��ǿ����ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dDDLimit, stMerchLimit.dDMLimit);
        }
        /*  end motify by GJQ at 20180613 */
        /*���ڸ����ۼ��޶�*/
        /*if (UptMerchLimitEx(dTranAmt, 'D', sMerchId) < 0) {
            ErrHanding(pstTransJson, "96", "�̻�[%s]��ǿ������޶�[%f]ʧ��.", sMerchId, dTranAmt);
            return -1;
        }*/
        //tLog(INFO, "�̻�[%s]��ǿ��޶�ָ��ɹ�,���׽��[%f],���ۼ�[%f],���ۼ�[%f].", sMerchId, dTranAmt, stMerchLimit.dDDLimit, stMerchLimit.dDMLimit);
    }
    
    /*�����ۼ��޶�*/
    tLog(DEBUG,"cUpdWay = [%c]:[%s]",cUpdWay,GETMESG(cUpdWay));
    if (UptMerchLimitEx(dTranAmt, cUpdWay, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]%s�����޶�[%f]ʧ��.", sMerchId, GETMESG(cUpdWay), dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]%s�޶��ۼƳɹ�,���׽��[%f]", sMerchId, GETMESG(cUpdWay), dTranAmt);
   
    
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
        ErrHanding(pstTransJson, "96", "�̻�[%s]�����޶�[%f]ʧ��.", sMerchId, dTranAmt);
        return -1;
    }
    
    tLog(INFO, "�̻�[%s]�޶�ָ��ɹ�,���׽��[%f]", sMerchId, dTranAmt);
    return 0;
}