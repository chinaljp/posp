
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
/*      ������:     RiskSameCard()                                        */
/*      ����˵��:   ͬ����ˢ                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

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


    /* ��ȡ��ع��� */
    iRet = FindSameCardRule(&stSameCardRule);
    if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "��ȡͬ����ˢ����ʧ��.");
        return -1;
    }


    if (FindValueByKey(sKey, "kms.encdata.key") < 0) {
        ErrHanding(pstTransJson, "96", "������Կʧ��");
        tLog(ERROR, "����key[kms.encdata.key]ֵ,ʧ��.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0) {
        ErrHanding(pstTransJson, "96", "���ܿ�������ʧ��.");
        tLog(ERROR, "���ܿ�������ʧ��.");
        return -1;
    }
    tLog(ERROR, "���ܿ�������[%s]", sECardNo);


    /* �̻�ͬ������2�Σ��ж��Ƿ���һ�����������ѽ��� */
    {
        sprintf(sSql2, "select count(1) from b_pos_trans_detail where merch_id='%s' "
                "and resp_code='00' and trans_code='021000' and CARD_NO_ENCRYPT='%s'", sMerchId, sECardNo);
        tLog(DEBUG, "%s", sSql2);

        iRet2 = tQueryCount(sSql2);
        if (iRet2 < 0) {
            ErrHanding(pstTransJson, "96", "��ȡˢ������ʧ��");
            return -1;
        }
        tLog(DEBUG, "%d", iRet2);
        if (iRet2 >= stSameCardRule.lQUERYNTRANS) {
            tGetTime(sTime, "", -1);
            if (GetQueryTime(sMerchId, sECardNo, sQueryTime)) {
                ErrHanding(pstTransJson, "96", "��ȡ����ʱ��ʧ��");
                return -1;
            }
            /*������������60֮�ھͻ��¼�̻���Ϣ*/
            tLog(DEBUG, "��ǰ����ʱ��[%s]�����һ�β���ʱ��[%s]", sTime, sQueryTime);
            if (memcmp(sTime, sQueryTime, 2) == 0) {
                strncpy(sTimeMin, sTime + 2, 2);
                strcpy(sTimeSec, sTime + 4);

                strncpy(sTimeQMin, sQueryTime + 2, 2);
                strcpy(sTimeQSec, sQueryTime + 4);

                tLog(INFO, "����[%d]:[%d],����[%d]:[%d],�������[%d]", atoi(sTimeMin), atoi(sTimeSec), atoi(sTimeQMin), atoi(sTimeQSec), (atoi(sTimeMin)*60 + atoi(sTimeSec)) -(atoi(sTimeQMin)*60 + atoi(sTimeQSec)));
                if ((atoi(sTimeMin)*60 + atoi(sTimeSec)) -(atoi(sTimeQMin)*60 + atoi(sTimeQSec)) <= stSameCardRule.lQUERYNMINTRANS * 60) {
                    sprintf(sDesc, "�̻�ͬ������[%d]�Σ�[%d]�����������ѽ��ף���¼�̻���Ϣ",iRet2,stSameCardRule.lQUERYNMINTRANS);
                    if (InstRiskMerch(pstTransJson, sDesc, "") < 0) {
                        ErrHanding(pstTransJson, "96", "��������̻���Ϣʧ��");
                        return -1;
                    }
                }
            }
        }

    }

    /* �̻�ͬ����ˢ�����߼���*/
    {
        /* �ж��̻��ǲ�����ͬ����ˢ������*/
        iRet = ChkWhiteCard(sUserCode);
        if (iRet == 1) {
            tLog(INFO, "�̻�����ͬ����ˢ������,������ͬ����ˢ����");
            return 0;
        }


        /* �ж��̻���û�п�������Ȧ */
        iRet = FindLifeflag(sUserCode);
        if (iRet == 1) {
            tLog(INFO, "�̻��ѿ�ͨ����Ȧ����,������ͬ����ˢ����");
            return 0;
        }


        dTranAmt /= 100;



        sprintf(sSql, "select count(1) from b_pos_trans_detail where merch_id='%s' "
                "and resp_code='00' and valid_flag='0' and trans_code in ('020000','T20000' )"
                "and amount>%f and CARD_NO_ENCRYPT='%s'", sMerchId, stSameCardRule.dAmount, sECardNo);
        tLog(DEBUG, "%s", sSql);

        iRet = tQueryCount(sSql);
        if (iRet < 0) {
            ErrHanding(pstTransJson, "96", "��ȡˢ������ʧ��");
            return -1;
        }

        if (iRet >= stSameCardRule.lCnt && dTranAmt > stSameCardRule.dAmount) {
            sprintf(sDesc, "������ع���,ͬ����ˢ[%d]�Σ���¼�̻���Ϣ", stSameCardRule.lCnt);
            if (InstRiskMerch(pstTransJson, sDesc, "") < 0) {
                ErrHanding(pstTransJson, "96", "��������̻���Ϣʧ��");
                return -1;
            }
            ErrHanding(pstTransJson, "62", "ͬ������ˢ����[%ld]���׽��[%.02f],����[%ld][%.02f],��ֹ����.",
                    iRet, dTranAmt, stSameCardRule.lCnt, stSameCardRule.dAmount);
            return -1;
        }
    }
    return 0;
}