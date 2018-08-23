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
/*      ������:     RiskQueryMerch()                                        */
/*      ����˵��:   �������                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int RiskQueryMerch(cJSON *pstJson, int *piFlag) {

    SameCardRule stSameCardRule;
    double dAmt = 0.0f;
    char sErr[128] = {0}, sSql[256] = {0}, sSql2[256] = {0}, sCurDate[9] = {0}, sMerchId[16] = {0};
    char sCardNo[19 + 1] = {0}, sUserCode[15 + 1] = {0}, sDesc[100] = {0};
    int iRet = 0, iRet2 = 0;
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
        ErrHanding(pstTransJson, "96", "��ȡ��ع���ʧ��.");
        return -1;
    }


    sprintf(sSql, "select count(1) from b_pos_trans_detail where merch_id='%s' "
            "and resp_code='00' and valid_flag='0' and trans_code = '021000' and card_no = '%s'", sMerchId, sCardNo);
    tLog(DEBUG, "%s", sSql);

    iRet = tQueryCount(sSql);
    if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "��ȡͬ���������ʧ��");
        return -1;
    }



    /* ͬ�̻���ѯ������ */
    sprintf(sSql2, "select count(1) from b_pos_trans_detail where merch_id='%s' "
            "and resp_code='00' and valid_flag='0' and trans_code = '021000'", sMerchId);
    tLog(DEBUG, "%s", sSql2);

    iRet2 = tQueryCount(sSql2);
    if (iRet2 < 0) {
        ErrHanding(pstTransJson, "96", "��ȡ�������ʧ��");
        return -1;
    }

    if (iRet >= stSameCardRule.lQueryCardCnt) {
        sprintf(sDesc, "���̻�ͬ����ѯ���[%d]�Σ����ڷ��գ���¼�̻���Ϣ", iRet);
        if (InstRiskMerch(pstTransJson, sDesc, "") < 0) {
            ErrHanding(pstTransJson, "96", "��������̻���Ϣʧ��");
            return -1;
        }
    }


    if (iRet2 >= stSameCardRule.lQueryCnt) {
        sprintf(sDesc, "���̻���ѯ���[%d]�Σ����ڷ��գ���¼�̻���Ϣ", iRet2);
        if (InstRiskMerch(pstTransJson, sDesc, "") < 0) {
            ErrHanding(pstTransJson, "96", "��������̻���Ϣʧ��");
            return -1;
        }
    }

    return 0;
}