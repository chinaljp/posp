#include "t_cjson.h"
#include <stdbool.h>
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "limitrule.h"
#include "merchlimit.h"
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/******************************************************************************/
/*      ������:     AmountProc1()                                             */
/*      ����˵��:   �ն��޶��鴦��                                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
#if 0
int AmountProc1(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0}, sMerchId[15 + 1] = {0}, sCardType[1 + 1] = {0};
    LimitRule stLimitRule;
    MerchLimit stMerchLimit;
    double dTranAmt = 0.0L;
    double dTranAmtAbs = 0.0L;
    bool bSignFlag;
    char sTransCode[9] = {0}, sOTransCode[9] = {0};
    cJSON * pstTransJson = NULL, *pstMerchLimitJson = NULL;


    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "o_trans_code", sOTransCode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    dTranAmt /= 100;
    if (FindLimitRule(&stLimitRule, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���޶����.", sMerchId);
        return -1;
    }

    if (FindMerchLimit(&stMerchLimit, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]���ۼƶ�����.", sMerchId);
        return -1;
    }

    if (NULL == strstr(stLimitRule.sATransCode, sTransCode)) {
        ErrHanding(pstTransJson, "40", "������[%s]��������", sTransCode);
        return -1;
    }

    /* ������ӵĽ���: ���ѡ�Ԥ��Ȩ��ɡ����ѳ���������Ԥ��Ȩ��ɳ������� */
    if (NULL != strstr("020000,024100,020023,024123,M20000,M20023,02W100,02W200,02B100,02B200", sTransCode)) {
        bSignFlag = false;
    }/* �����ٵĽ���: �˻������ѳ�����Ԥ��Ȩ��ɳ��� */
    else if (NULL != strstr("020001,020002,024102,M20002", sTransCode)) {
        bSignFlag = true;
    } else {
        tLog(INFO, "����[%s]�������ն��޶�!", sTransCode);
        return 0;
    }

    //΢�ŵ����޶�
    if (NULL != strstr("02W100,02W200", sTransCode)) {
        if (dTranAmt > stLimitRule.dWXLimit && stLimitRule.dWXLimit > 0.000000) {
            ErrHanding(pstTransJson, "C3", "�̻�[%s]΢�ŵ������ѽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, stLimitRule.dWXLimit, dTranAmt);
            return -1;
        }
    }//֧���������޶�
    else if (NULL != strstr("02B100,02B200", sTransCode)) {
        if (dTranAmt > stLimitRule.dBBLimit && stLimitRule.dBBLimit > 0.000000) {
            ErrHanding(pstTransJson, "C3", "�̻�[%s]֧�����������ѽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, stLimitRule.dBBLimit, dTranAmt);
            return -1;
        }
    } else {//���������޶�
        if (dTranAmt > stLimitRule.dPLimit && stLimitRule.dPLimit > 0.000000) {
            ErrHanding(pstTransJson, "C3", "�̻�[%s]����[%s]���ʽ���! �����޶�[%f], ��ǰ���׽��[%f].", sMerchId, sTransCode, stLimitRule.dPLimit, dTranAmt);
            return -1;
        }
    }
    dTranAmt = bSignFlag ? (-dTranAmt) : dTranAmt;
    //΢�ŵ����޶�
    if (NULL != strstr("02W100,02W200", sTransCode)) {
        if (stMerchLimit.dDWXLimit + dTranAmt > stLimitRule.dDWXLimit && stLimitRule.dDWXLimit > 0.000000) {
            ErrHanding(pstTransJson, "C4", "�̻�[%s]΢�������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dDWXLimit, stMerchLimit.dDWXLimit, dTranAmt);
            return -1;
        }

    }//֧���������޶�
    else if (NULL != strstr("02B100,02B200", sTransCode)) {
        if (stMerchLimit.dDBBLimit + dTranAmt > stLimitRule.dDBBLimit && stLimitRule.dDBBLimit > 0.000000) {
            ErrHanding(pstTransJson, "C4", "�̻�[%s]֧���������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dDBBLimit, stMerchLimit.dDBBLimit, dTranAmt);
            return -1;
        }

    }//���������޶�

    else if (stMerchLimit.dDLimit + dTranAmt > stLimitRule.dDLimit && stLimitRule.dDLimit > 0.000000) {
        ErrHanding(pstTransJson, "C4", "�̻�[%s]�����ѽ��׽���! \n"
                "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                , sMerchId, stLimitRule.dDLimit, stMerchLimit.dDLimit, dTranAmt);
        return -1;
    }

    //΢�ŵ����޶�
    if (NULL != strstr("02W100,02W200", sTransCode)) {
        if (stMerchLimit.dMWXLimit + dTranAmt > stLimitRule.dMWXLimit && stLimitRule.dMWXLimit > 0.000000) {
            ErrHanding(pstTransJson, "C5", "�̻�[%s]΢�������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dMWXLimit, stMerchLimit.dMWXLimit, dTranAmt);
            return -1;
        }
    }//֧���������޶�
    else if (NULL != strstr("02B100,02B200", sTransCode)) {
        if (stMerchLimit.dMBBLimit + dTranAmt > stLimitRule.dMBBLimit && stLimitRule.dMBBLimit > 0.000000) {
            ErrHanding(pstTransJson, "C5", "�̻�[%s]֧���������ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dMBBLimit, stMerchLimit.dMBBLimit, dTranAmt);
            return -1;
        }
    }//���������޶�
    else {
        if (stMerchLimit.dMLimit + dTranAmt > stLimitRule.dMLimit && stLimitRule.dMLimit > 0.000000) {
            ErrHanding(pstTransJson, "C5", "�̻�[%s]�����ѽ��׽���! \n"
                    "���޶�[%f], ��ǰ��ͳ�ƽ��[%f], ��ǰ���׽��[%f]."
                    , sMerchId, stLimitRule.dMLimit, stMerchLimit.dMLimit, dTranAmt);
            return -1;
        }
    }
    /*���ڸ����ۼ��޶�*/
    if (NULL != strstr("02B100,02B200", sTransCode)) {
        stMerchLimit.dDBBLimit += dTranAmt;
        stMerchLimit.dMBBLimit += dTranAmt;
    }

    else if (NULL != strstr("02W100,02W200", sTransCode)) {
        stMerchLimit.dDWXLimit += dTranAmt;
        stMerchLimit.dMWXLimit += dTranAmt;
    }
    else {
        stMerchLimit.dDLimit += dTranAmt;
        stMerchLimit.dMLimit += dTranAmt;
    }
    pstMerchLimitJson = cJSON_CreateObject();
    if (NULL == pstMerchLimitJson) {
        ErrHanding(pstTransJson, "96", "�����̻�[%s]��MerchLimitJson�ṹʧ��.", sMerchId);
        return -1;
    }
    SET_DOU_KEY(pstMerchLimitJson, "d_limit", stMerchLimit.dDLimit);
    SET_DOU_KEY(pstMerchLimitJson, "m_limit", stMerchLimit.dMLimit);
    SET_DOU_KEY(pstMerchLimitJson, "d_wx_limit", stMerchLimit.dDWXLimit);
    SET_DOU_KEY(pstMerchLimitJson, "m_wx_limit", stMerchLimit.dMWXLimit);
    SET_DOU_KEY(pstMerchLimitJson, "d_bb_limit", stMerchLimit.dDBBLimit);
    SET_DOU_KEY(pstMerchLimitJson, "m_bb_limit", stMerchLimit.dMBBLimit);

    SET_JSON_KEY(pstJson, "merch_limit", pstMerchLimitJson);
    tLog(INFO, "�̻�[%s]�����޶���ͨ��."
            , sMerchId);
    /*   �ն��޶��鴦��    */
    return 0;
}
#endif
/******************************************************************************/
/*      ������:     AmountProc2()                                             */
/*      ����˵��:   �ն��޶�ˢ�´���                                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int AmountProc2(cJSON *pstJson, int *piFlag) {
    char sRespCode[2 + 1] = {0}, sIstRespCode[2 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sTransCode[9] = {0};
    cJSON * pstTransJson = NULL, *pstMerchLimitJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    pstMerchLimitJson = GET_JSON_KEY(pstJson, "merch_limit");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "istresp_code", sIstRespCode);

    if (0 != memcmp(sIstRespCode, "00", 2)) {
        return -1;
    }
    /*  �ն��޶�ˢ�´���    */
    if (pstMerchLimitJson == NULL)
        return 0;
    if (0 == memcmp(sRespCode, "00", 2)) {
        if (UptMerchLimit(pstMerchLimitJson, sMerchId, sTransCode) < 0) {
            tLog(ERROR, "�̻�[%s]����ͳ�ƶ�ˢ��ʧ��!", sMerchId);
            return -1;
        }
        tLog(INFO, "�̻�[%s]����ͳ�ƶ�ˢ�³ɹ�.", sMerchId);
    }
    return 0;
}