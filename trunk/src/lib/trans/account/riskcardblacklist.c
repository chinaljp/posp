#include "t_cjson.h"
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
/*      ������:     RiskCardBlackList()                                        */
/*      ����˵��:   ���п�������                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int RiskCardBlackList(cJSON *pstJson, int *piFlag) {
    int iRet = 0;
    char sCardNo[19 + 1] = {0}, sECardNo[19 + 1] = {0}, sSql[128] = {0};
    char sKey[128 + 1] = {0};
    cJSON * pstTransJson;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
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
    /* ���п�������   */
    sprintf(sSql, "SELECT COUNT(1) FROM B_RISK_BLACK_CARD WHERE STATUS='1' AND CARD_NO_ENCRYPT='%s'", sECardNo);
    iRet = tQueryCount(sSql);
    if (iRet > 0) {
        ErrHanding(pstTransJson, "62", "������˿�[%s].", sCardNo);
        return -1;
    }
    tLog(INFO, "����[%s]���������ͨ��.", sCardNo);
    return 0;
}