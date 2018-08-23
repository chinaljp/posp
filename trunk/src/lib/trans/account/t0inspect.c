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
#include "t0limit.h"
#include "postransdetail.h"
#if 0
/******************************************************************************/
/*      ������:     T0Inspect()                                                */
/*      ����˵��:   ����̻��ս�ͨt0�����޶�����޶�                              */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int T0Inspect(cJSON *pstTranData, int *piFlag) {
    T0Merchlimit stT0Merchlimit;
    char sErr[128] = {0};
    char sTransTime[7] = {0}, sUserCode[15 + 1] = {0};
    char sTransCode[7] = {0}, sCardType[2] = {0}, sInputMode[4] = {0}, sTranAmt[14] = {0}, sMerchId[16] = {0}, sOldRrn[13] = {0};
    double dTranAmtAbs = 0.0L;
    int iNum = 0, iRet = 0, iTranAmt = 0;
    int iTnum = 0, iSwth = 0;
    cJSON *pstTransJson;

    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    DUMP_JSON(pstTransJson);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_INT_KEY(pstTransJson, "amount", iTranAmt);
    sprintf(sTranAmt, "%d", iTranAmt);

    /* �ս�����ֻ֧�� ���ÿ� */
    if (!memcmp(sTransCode, "T20000", 6)) {

        GET_STR_KEY(pstTransJson, "card_type", sCardType);
        GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
        GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
        if (sCardType[0] == '0') {
            tLog(ERROR, "�ս�����ֻ֧�����ÿ�,�������ֿ�����Ϊ[%c],���뷽ʽΪ[%c]",
                    sCardType[0], sInputMode[1]);
            sprintf(sErr, "�����Ͳ������ÿ�.");
            tLog(ERROR, sErr);
            ErrHanding(pstTransJson, "57", sErr);
            return -1;
        }
        if ((sCardType[0] == '1')&&(sInputMode[1] == '2')) {
            iRet = FindT0Limit(sTranAmt);
            if (iRet < 0) {
                sprintf(sErr, "��ȡ�޶����!");
                tLog(ERROR, sErr);
                ErrHanding(pstTransJson, "96", "��ȡ�޶����!");
                return -1;
            }
            if (iRet == 1) {
                sprintf(sErr, "�ſ��������ִ���10000");
                tLog(ERROR, sErr);
                ErrHanding(pstTransJson, "E9", sErr);
                return -1;
            }
            tLog(ERROR, "�ս�����");
        }
    }

    tLog(ERROR, "�ս�����");
    /* ����սύ���ܿ��� */
    iSwth = Chksettleswitch();
    if (iSwth < 0) {
        sprintf(sErr, "��ȡ�ս����ѿ���ʧ��.");
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    if (iSwth == 1) {
        sprintf(sErr, "�ս������ѹر�", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "D8", sErr);
        return -1;
    }
    /* �ֻ�app�ս�ͨ����û����ԭ���׽����ȡԭ���׽�� */
    if (!memcmp(sTransCode, "TA0010", 6)) {

        tLog(INFO, "APP�ս�֪ͨ����");
        /* ����ԭ���׽��ԭ������ԭ���뷽ʽ */

        GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);

        iRet = GetAmountFee(sOldRrn, sTranAmt, sCardType, sInputMode);
        if (iRet != 0) {
            ErrHanding(pstTransJson, "25", "RRN = %s �Ķ���û���ҵ�ԭ����", sOldRrn);
            return -1;
        }
        tLog(INFO, "APP-Card1Type[%c]InputMode[%s]", sCardType[0], sInputMode);
        if (sCardType[0] == '0') {

            tLog(ERROR, "�ս�����ֻ֧�����ÿ�,�������ֿ�����Ϊ[%c],���뷽ʽΪ[%c]",
                    sCardType[0], sInputMode[1]);

            sprintf(sErr, "�����Ͳ������ÿ�.");
            tLog(ERROR, sErr);
            ErrHanding(pstTransJson, "57", sErr);
            return -1;
        }
        /*
                if ((sCardType[0] == '1')&&(sInputMode[1] == '2')) {
                    iRet = FindT0Limit(sTranAmt);
                    if (iRet < 0) {
                        sprintf(sErr, "��ȡ�޶����!");
                        tLog(ERROR, sErr);
                        ErrHanding(pstTransJson, "96", sErr);
                        return -1;
                    }
                    if (iRet == 1) {
                        sprintf(sErr, "�ſ��������ִ���10000");
                        tLog(ERROR, sErr);
                        ErrHanding(pstTransJson, "E9", sErr);
                        return -1;
                    }
                }
         */
        tLog(INFO, "APP�ս�֪ͨ����");
    }

    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    /* ��֤����ʱ�估����޶� */
    iTnum = Findsettletime(sTransTime, sTranAmt);
    if (iTnum < 0) {
        sprintf(sErr, "��ȡT0����ʱ��ʧ��.");
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    if (iTnum == 2) {
        sprintf(sErr, "�̻�[%s]t0���ѵ�������޶�! \n", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "E6", sErr);
        return -1;
    }
    if (iTnum == 3) {
        sprintf(sErr, "�̻�[%s]t0���Ѵ�������޶�! \n", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "E7", sErr);
        return -1;
    }
    if (iTnum == 1) {
        sprintf(sErr, "����ʱ�䲻��T0����ʱ����.");
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "D7", sErr);
        return -1;

    }
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    iNum = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
    if (iNum < 0) {
        sprintf(sErr, "��ȡ�̻�[%s]T0�޶���Ϣʧ��.", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    if (iNum == 1) {
        sprintf(sErr, "�̻�[%s]δ��ͨ�ս�ͨ����.", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "D6", sErr);
        return -1;
    }

    tLog(INFO, "�̻�[%s]��ȡt0�޶���Ϣ�ɹ�.", sMerchId);

    /*�޶���*/

    /** ��齻�׽���Ƿ��� **/
    dTranAmtAbs = atof(sTranAmt) / 100;

    if (dTranAmtAbs > stT0Merchlimit.dUsable_limit) {
        sprintf(sErr, "�̻�[%s]t0���ö�ȳ���! \n" \
                "���ö��[%f], ��ǰ���׽��[%f].", sMerchId, stT0Merchlimit.dUsable_limit, dTranAmtAbs);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "D5", sErr);
        return -1;
    }

    tLog(INFO, "�̻�[%s]t0�����޶���ͨ��.", sMerchId);

    /* ���ö�ȼ�ȥ���׽����ö�ȼ��Ͻ��׽�� */
    tLog(INFO, "���׽��[%f]", dTranAmtAbs);
    tLog(INFO, "���׽��[%f]", stT0Merchlimit.dUsable_limit);
    stT0Merchlimit.dUsable_limit -= dTranAmtAbs;
    tLog(INFO, "���׽��[%f]", stT0Merchlimit.dUsable_limit);
    stT0Merchlimit.dUsedlimit += dTranAmtAbs;
    /* �����̻�T0�޶�  */
    if (UptT0limit(&stT0Merchlimit) < 0) {
        tLog(ERROR, "�̻�[%s]T0�����޶�ˢ��ʧ��!", sMerchId);
        return -1;
    }
    tLog(INFO, "�̻�[%s]T0�����޶�ˢ�³ɹ�.", sMerchId);

    return 0;
}

int ChkT0OldProc(cJSON * pstTransJson) {
    PosTransDetail stAcctJon;
    char sMerchId[16] = {0}, sTermId[9] = {0}, sOldTermTraceNo[7] = {0}, sOldRrn[13] = {0};

    GET_STR_KEY(pstTransJson, "o_trace_no", sOldTermTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    /* ����ԭ���� */
    if (FindRevAcctJon(&stAcctJon, sMerchId, sTermId, sOldTermTraceNo) != 0) {
        tLog(ERROR, "ԭ���׼��ʧ�ܣ���ԭ������ˮ��ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sOldTermTraceNo, sMerchId, sTermId);
        return -1;
    }
    /* rrn */
    tLog(DEBUG, "Rrn[%s][%s]", stAcctJon.sRrn, sOldRrn);
    if (memcmp(sOldRrn, stAcctJon.sRrn, 12)) {
        tLog(ERROR, "ԭ���׼��ʧ��,�����ο���RRN��һ��.");
        return -1;
    }
    /* ���������־  */
    if (UpT0flag(stAcctJon.sRrn) < 0) {
        tLog(ERROR, "�̻�[%s],RRN[%s]T0�����־����ʧ��!", sMerchId, stAcctJon.sRrn);
        return -1;
    }
    tLog(INFO, "�����̻���[%s]�ն˺�[%s]��ˮ��[%s]�������־�ɹ�.", sMerchId, sTermId, sOldTermTraceNo);
    return 0;
}
#endif
#if 0
/******************************************************************************/
/*      ������:     ChkT0Proc()                  	                      */
/*      ����˵��:   �ָ�T0ԭ����											  */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UnT2003D0Limit(cJSON *pstTranData, int *piFlag) {
    cJSON *pstTransJson;
    char sRespCode[2 + 1] = {0}, sErr[128] = {0}, sUserCode[16] = {0}, sTranAmt[13] = {0};
    char sMerchId[15 + 1] = {0};
    T0Merchlimit stT0Merchlimit;
    int iNum = 0;
    double dTranAmt = 0.0;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    if (!memcmp(sRespCode, "00", 2)) {
        iNum = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
        if (iNum < 0) {
            ErrHanding(pstTransJson, "96", "��ȡ�̻�[%s]D0���ö����Ϣʧ��.", sMerchId);
            return -1;
        }
        dTranAmt = dTranAmt / 100;
        /* T0���ѽ���ʧ��֮�����Ӧ�޶������ȥ */
        stT0Merchlimit.dUsable_limit += dTranAmt;
        stT0Merchlimit.dUsedlimit -= dTranAmt;
        /* �����̻�T0�޶�  */
        if (UptT0limit(&stT0Merchlimit) < 0) {
            tLog(ERROR, "�̻�[%s]D0���ö�ȸ���ʧ��.", sMerchId);
            return -1;
        } else {
            tLog(INFO, "�̻�[%s]D0���ö�Ȼָ��ɹ�.", sMerchId);
        }
    } else {
        tLog(INFO, "�ս����ѳ�������ʧ��,�̻�[%s]���ûָ�D0���.", sMerchId);
    }
    return 0;
}
#endif

/* �ս�ȷ�Ͻ��ף����ԭ���ף���Ҫ�Ǵ�ԭ������ȡֵ */
int ChkD0Trans(cJSON *pstTranData, int *piFlag) {
    PosTransDetail stAcctJon;
    char sMerchId[16] = {0}, sTermId[9] = {0}, sOldRrn[13] = {0}, sTransDate[8 + 1] = {0};
    cJSON *pstTransJson;

    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    /* ����ԭ���� */
    if (FindD0Trans(&stAcctJon, sTransDate, sOldRrn) != 0) {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ�ܣ���ԭ������ˮ��ԭrrn[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sOldRrn, sMerchId, sTermId);
        return -1;
    }
    tLog(DEBUG, "�̻�[%s]�ն�[%s]ԭ����rrn[%s]���ҳɹ�.", sMerchId, sTermId, sOldRrn);
    if (memcmp(stAcctJon.sRespCode, "00", 2)) {
        ErrHanding(pstTransJson, "25", "�̻�[%s]�ն�[%s]ԭ����rrn[%s]ʧ��, ��ֹ����.", sMerchId, sTermId, sOldRrn);
    }
    DEL_KEY(pstTransJson, "amount");
    SET_DOU_KEY(pstTransJson, "amount", stAcctJon.dAmount * 100);
    SET_DOU_KEY(pstTransJson, "fee", stAcctJon.dFee);
    SET_STR_KEY(pstTransJson, "settle_date", stAcctJon.sSettleDate);
    SET_STR_KEY(pstTransJson, "card_type", stAcctJon.sCardType);
    SET_STR_KEY(pstTransJson, "o_trans_date", stAcctJon.sTransDate);
    SET_STR_KEY(pstTransJson, "o_resp_code", stAcctJon.sRespCode);
    /*add by GJQ at 20171124 begin */
    SET_STR_KEY(pstTransJson, "trans_type", stAcctJon.sTransType);
    SET_STR_KEY(pstTransJson, "settle_flag", stAcctJon.sSettleFlag);
    /*add by GJQ at 20171124 end   */

    return 0;
}
#if 0

/* �սύ�����ʧ�ܣ��ָ���� */
int UnD0Limit(cJSON *pstTranData, int *piFlag) {
    cJSON *pstTransJson;
    char sRespCode[2 + 1] = {0}, sErr[128] = {0}, sUserCode[16] = {0}, sTranAmt[13] = {0};
    char sMerchId[15 + 1] = {0};
    T0Merchlimit stT0Merchlimit;
    int iNum = 0;
    double dTranAmt = 0.0;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    if (memcmp(sRespCode, "00", 2)) {
        iNum = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
        if (iNum < 0) {
            ErrHanding(pstTransJson, "96", "��ȡ�̻�[%s]D0���ö����Ϣʧ��.", sMerchId);
            return -1;
        }
        dTranAmt = dTranAmt / 100;
        /* T0���ѽ���ʧ��֮�����Ӧ�޶������ȥ */
        stT0Merchlimit.dUsable_limit += dTranAmt;
        stT0Merchlimit.dUsedlimit -= dTranAmt;
        /* �����̻�T0�޶�  */
        if (UptT0limit(&stT0Merchlimit) < 0) {
            tLog(ERROR, "�̻�[%s]D0���ö�ȸ���ʧ��.", sMerchId);
            return -1;
        } else {
            tLog(INFO, "�̻�[%s]D0���ö�Ȼָ��ɹ�.", sMerchId);
        }
    } else {

        tLog(INFO, "���׳ɹ�,�̻�[%s]���ûָ�D0���.", sMerchId);
    }
    return 0;
}
#endif

int ChkDebitD0Limit(cJSON *pstTransJson) {
    T0Merchlimit stT0Merchlimit;
    char sTmp[128] = {0}, sMerchId[16] = {0}, sUserCode[15 + 1] = {0}, sD0Type[1 + 1] = {0};
    char sSartTime[6 + 1] = {0}, sEndTime[6 + 1] = {0}, sTransTime[7] = {0};
    int iRet = 0;
    double dTranAmt = 0.0, dMinAmt = 0.0, dMaxAmt = 0.0;
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    dTranAmt = dTranAmt / 100;
    /*  ����̻��Ƿ�ͨ��ǿ�D0�������ͨ����ȡ�����Ϣ */
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    if (FindMerchD0Type(sD0Type, sUserCode) < 0 || sD0Type[0] != '1') {
        ErrHanding(pstTransJson, "B2", "���̻�[%s]������D0оƬ��ǿ�����.", sMerchId);
        return -1;
    }
    iRet = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
    if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "��ȡ�̻�[%s]D0���ö����Ϣʧ��.", sMerchId);
        return -1;
    }
    if (iRet == 1) {
        ErrHanding(pstTransJson, "D6", "�̻�[%s]δ��ͨ�ս�ͨ����.", sMerchId);
        return -1;
    }
    tLog(DEBUG, "�̻�[%s]�ѿ�ͨ�ս����.", sMerchId);
    if (dTranAmt > stT0Merchlimit.dUsable_limit) {
        ErrHanding(pstTransJson, "D5", "�̻�[%s]D0���ö�ȳ���,���ö��[%.02f],��ǰ���׽��[%.02f],ʧ��."
                , sMerchId, stT0Merchlimit.dUsable_limit, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]D0���ö��[%.02f],��ǰ���׽��[%.02f]."
            , sMerchId, stT0Merchlimit.dUsable_limit, dTranAmt);
    /* ��֤����ʱ�� */
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    if (FindValueByKey(sSartTime, "SETTLE_START_TIME") < 0) {
        ErrHanding(pstTransJson, "96", "����key[SETTLE_START_TIME]D0��ʼʱ��,ʧ��.");
        return -1;
    }
    if (FindValueByKey(sEndTime, "SETTLE_END_TIME") < 0) {
        ErrHanding(pstTransJson, "96", "����key[SETTLE_END_TIME]D0����ʱ��,ʧ��.");
        return -1;
    }
    if (memcmp(sTransTime, sSartTime, 6) < 0 || memcmp(sTransTime, sEndTime, 6) > 0) {
        ErrHanding(pstTransJson, "D7", "����ʱ��[%s]����D0����ʱ����[%s-%s].", sTransTime, sSartTime, sEndTime);
        return -1;
    }
    tLog(DEBUG, "����ʱ��[%s]��T0����ʱ����[%s-%s].", sTransTime, sSartTime, sEndTime);

    if (FindValueByKey(sTmp, "D0_SINGLE_CASH_MIN") < 0) {
        ErrHanding(pstTransJson, "96", "����key[D0_SINGLE_CASH_MIN]D0��������ֵ,ʧ��.");
        return -1;
    }
    dMinAmt = atof(sTmp);
    if (DBL_CMP(dMinAmt, dTranAmt)) {
        ErrHanding(pstTransJson, "E6", "�̻�[%s]D0����[%.02f]��������޶�[%.02f].", sMerchId, dTranAmt, dMinAmt);
        return -1;
    }
    if (FindValueByKey(sTmp, "D0_SINGLE_CASH_MAX") < 0) {
        ErrHanding(pstTransJson, "96", "����key[D0_SINGLE_CASH_MAX]D0��������ֵ,ʧ��.");
        return -1;
    }
    dMaxAmt = atof(sTmp);
    if (DBL_CMP(dTranAmt, dMaxAmt)) {
        ErrHanding(pstTransJson, "E7", "�̻�[%s]D0����[%.02f]��������޶�[%.02f].", sMerchId, dTranAmt, dMaxAmt);
        return -1;
    }
    tLog(DEBUG, "�̻�[%s]D0����[%.02f]����[%.02f]����[%.02f].", sMerchId, dTranAmt, dMinAmt, dMaxAmt);

    /* ���ö�ȼ�ȥ���׽����ö�ȼ��Ͻ��׽�� */
    stT0Merchlimit.dUsable_limit -= dTranAmt;
    stT0Merchlimit.dUsedlimit += dTranAmt;
    tLog(INFO, "�̻�[%s]D0оƬ��ǿ����ö��[%.02f],���ö��[%.02f],���׽��[%.02f]."
            , sMerchId, stT0Merchlimit.dUsable_limit, stT0Merchlimit.dUsedlimit, dTranAmt);
}

int ChkCreditD0Limit(cJSON *pstTransJson) {
    T0Merchlimit stT0Merchlimit;
    char sTmp[128] = {0}, sSql[256] = {0};
    char sTransTime[7] = {0}, sUserCode[15 + 1] = {0};
    char sTransCode[7] = {0}, sCardType[2] = {0}, sInputMode[4] = {0}, sMerchId[16] = {0}, sOldRrn[13] = {0};
    double dTranAmtAbs = 0.0L, dLimitAmt = 0.0;
    char sLimitAmt[32 + 1] = {0}, sSwitch[1 + 1] = {0};
    char sSartTime[6 + 1] = {0}, sEndTime[6 + 1] = {0};
    char sORrn[6 + 1] = {0}, sOTransDate[6 + 1] = {0}, sCardNo[32] = {0};
    int iNum = 0, iRet = 0;
    double dTranAmt = 0.0, dMinAmt = 0.0, dMaxAmt = 0.0;

    char sSingleAmt[32 + 1] = {0};
    double dSingleAmt = 0.0;

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    dTranAmt = dTranAmt / 100;

    /*  ����̻��Ƿ�ͨD0�������ͨ����ȡ�����Ϣ */
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    iRet = FindT0Merchlimit(&stT0Merchlimit, sUserCode);
    if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "��ȡ�̻�[%s]D0���ö����Ϣʧ��.", sMerchId);
        return -1;
    }
    if (iRet == 1) {
        ErrHanding(pstTransJson, "D6", "�̻�[%s]δ��ͨ�ս�ͨ����.", sMerchId);
        return -1;
    }
    tLog(DEBUG, "�̻�[%s]�ѿ�ͨ�ս����.", sMerchId);
    if (dTranAmt > stT0Merchlimit.dUsable_limit) {
        ErrHanding(pstTransJson, "D5", "�̻�[%s]D0���ö�ȳ���,���ö��[%.02f],��ǰ���׽��[%.02f],ʧ��."
                , sMerchId, stT0Merchlimit.dUsable_limit, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]D0���ö��[%.02f],��ǰ���׽��[%.02f]."
            , sMerchId, stT0Merchlimit.dUsable_limit, dTranAmt);
    /* �������ÿ�ֻ�����ڹ涨�Ľ�����սᣬ�����޶�ֵ��Ҫ��Ȩ */
    if (sCardType[0] == '1' && sInputMode[1] == '2') {
        tLog(DEBUG, "�������ÿ�.");
        tLog(DEBUG, "���D0���Ű�����.");
        snprintf(sSql, sizeof (sSql), "select count(1) from b_trans_card_white_list"
                " where account_no='%s'", sCardNo);
        iNum = tQueryCount(sSql);
        if (iNum > 0) {
            tLog(DEBUG, "����[%s]���ڰ�������,����ֱ������.", sCardNo);
        } else {
            tLog(DEBUG, "����[%s]���ڰ�������,������Ҫ���.", sCardNo);

            /*add by gjq at 20171019 BENGIN*/
            tLog(INFO, "����̻�������֤������.(d0)");
            if (FindValueByKey(sSingleAmt, "D0_TRANS_AMT") < 0) {
                tLog(ERROR, "����key[D0_TRANS_AMT]D0�޶�ֵ,ʧ��.");
                ErrHanding(pstTransJson, "96", "����key[D0_TRANS_AMT]�̻�������֤������D0�޶�ֵ,ʧ��.");
                return -1;
            }
            dSingleAmt = atof(sSingleAmt);
            tLog(DEBUG, "���׽��[%f],�������ÿ�D0�޶�[%f].", dTranAmt, dSingleAmt);

            snprintf(sSql, sizeof (sSql), "SELECT COUNT(1) FROM B_MERCH_D0_TRANS "
                    "WHERE MERCH_ID = '%s' AND STATUS = '1'", sMerchId);
            iNum = tQueryCount(sSql);
            if ((iNum > 0 && !DBL_CMP(dTranAmt, dSingleAmt)) || (iNum > 0 && DBL_EQ(dTranAmt, dSingleAmt))) {
                tLog(DEBUG, "�̻�[%s]����������֤���������Ҵ������ÿ����׶�[%.02f]С�ڵ���[%.02f],����ֱ������", sMerchId, dTranAmt, dSingleAmt);
            } else {
                tLog(WARN, "�̻�[%s]����������֤������ ���� �˴ν��״������ÿ����׶��.", sMerchId);
                /*add by gjq at 20171019 END*/
                if (FindValueByKey(sLimitAmt, "D0_SINGLE_CASH_MIDDLE") < 0) {
                    ErrHanding(pstTransJson, "96", "����key[D0_SINGLE_CASH_MIDDLE]D0�޶�ֵ,ʧ��.");
                    return -1;
                }
                /* �жϽ��׽���Ƿ���ڴ�ֵ */
                dLimitAmt = atof(sLimitAmt);
                if (DBL_CMP(dTranAmt, dLimitAmt)) {
                    ErrHanding(pstTransJson, "E9", "���׽��[%.02f]���ڴ������ÿ������޶�[%.02f]", dTranAmt, dLimitAmt);
                    return -1;
                }
            }
        }
    }
    /* ��֤����ʱ�� */
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    if (FindValueByKey(sSartTime, "SETTLE_START_TIME") < 0) {
        ErrHanding(pstTransJson, "96", "����key[SETTLE_START_TIME]D0��ʼʱ��,ʧ��.");
        return -1;
    }
    if (FindValueByKey(sEndTime, "SETTLE_END_TIME") < 0) {
        ErrHanding(pstTransJson, "96", "����key[SETTLE_END_TIME]D0����ʱ��,ʧ��.");
        return -1;
    }
    if (memcmp(sTransTime, sSartTime, 6) < 0 || memcmp(sTransTime, sEndTime, 6) > 0) {
        ErrHanding(pstTransJson, "D7", "����ʱ��[%s]����D0����ʱ����[%s-%s].", sTransTime, sSartTime, sEndTime);
        return -1;
    }
    tLog(DEBUG, "����ʱ��[%s]��T0����ʱ����[%s-%s].", sTransTime, sSartTime, sEndTime);

    if (FindValueByKey(sTmp, "D0_SINGLE_CASH_MIN") < 0) {
        ErrHanding(pstTransJson, "96", "����key[D0_SINGLE_CASH_MIN]D0��������ֵ,ʧ��.");
        return -1;
    }
    dMinAmt = atof(sTmp);
    if (DBL_CMP(dMinAmt, dTranAmt)) {
        ErrHanding(pstTransJson, "E6", "�̻�[%s]D0����[%.02f]��������޶�[%.02f].", sMerchId, dTranAmt, dMinAmt);
        return -1;
    }
    if (FindValueByKey(sTmp, "D0_SINGLE_CASH_MAX") < 0) {
        ErrHanding(pstTransJson, "96", "����key[D0_SINGLE_CASH_MAX]D0��������ֵ,ʧ��.");
        return -1;
    }
    dMaxAmt = atof(sTmp);
    if (DBL_CMP(dTranAmt, dMaxAmt)) {
        ErrHanding(pstTransJson, "E7", "�̻�[%s]D0����[%.02f]��������޶�[%.02f].", sMerchId, dTranAmt, dMaxAmt);
        return -1;
    }
    tLog(DEBUG, "�̻�[%s]D0����[%.02f]����[%.02f]����[%.02f].", sMerchId, dTranAmt, dMinAmt, dMaxAmt);

    /* ���ö�ȼ�ȥ���׽����ö�ȼ��Ͻ��׽�� */
    stT0Merchlimit.dUsable_limit -= dTranAmt;
    stT0Merchlimit.dUsedlimit += dTranAmt;
    tLog(INFO, "�̻�[%s]D0���ö��[%.02f],���ö��[%.02f],���׽��[%.02f]."
            , sMerchId, stT0Merchlimit.dUsable_limit, stT0Merchlimit.dUsedlimit, dTranAmt);
#if 0
    //�����Ľ���������
    /* �����̻�T0�޶�  */
    if (UptT0limit(&stT0Merchlimit) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]D0���׶�ȸ���ʧ��.", sMerchId);
        return -1;
    }
    tLog(INFO, "�̻�[%s]D0���׶�ȸ��³ɹ�.", sMerchId);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    if (UpTransType("S", sOTransDate, sORrn) < 0) {
        ErrHanding(pstTransJson, "96", "����[%s:%s]��������trans_type����ʧ��.", sOTransDate, sORrn);
        return -1;
    }
#endif
    return 0;
}

/* �սύ�׼�� */
int ChkD0Limit(cJSON *pstTranData, int *piFlag) {
    char sCardType[2] = {0}, sSwitch[1 + 1] = {0}, sInputMode[4] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    /* ����սύ���ܿ��� */
    if (FindValueByKey(sSwitch, "DAY_ACCOUNT_SWITCH") < 0) {
        ErrHanding(pstTransJson, "96", "����key[DAY_ACCOUNT_SWITCH]�սύ���ܿ���,ʧ��.");
        return -1;
    }
    if (sSwitch[0] == '0') {
        ErrHanding(pstTransJson, "D8", "�սύ���ܿ���״̬[%s].", sSwitch[0] == '0' ? "�ر�" : "��ͨ");
        return -1;
    }
    tLog(DEBUG, "�սύ���ܿ���״̬[%s].", sSwitch[0] == '0' ? "�ر�" : "��ͨ");

    /*��ǿ�,���ǿ��ж�*/
    if (sCardType[0] == '0') {//��ǿ�
        //ErrHanding(pstTransJson, "57", "�ս�����ֻ֧�����ÿ�,��ǰ����Ϊ��ǿ�.");
        //��ǿ���оƬ����������
        if (sInputMode[1] == '5' || sInputMode[1] == '7') {
            return ChkDebitD0Limit(pstTransJson);
        } else {
            ErrHanding(pstTransJson, "57", "�ս����ֲ�֧�ִ�����ǿ�.");
            return -1;
        }
    } else if (sCardType[0] == '1') { //���ǿ�
        return ChkCreditD0Limit(pstTransJson);
    } else {//������
        ErrHanding(pstTransJson, "57", "�����ʹ���");
        return -1;
    }
    return 0;
}

/* �����ս��� */
int UpdD0Limit(cJSON *pstTranData, int *piFlag) {
    char sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sCardType[2] = {0}, sOTransDate[8 + 1] = {0};
    double dTranAmt = 0.0;
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    /* �����̻�T0�޶�  */
    if (UptT0Limit(dTranAmt / 100, sUserCode, sCardType) < 0) {
        ErrHanding(pstTransJson, "96", "�̻�[%s]D0���׶��[+]����ʧ��.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]D0���׶�ȸ��³ɹ�.", sMerchId);
    return 0;
}

/*  app����trans_type*/
int UpdAppTransType(cJSON *pstTranData, int *piFlag) {
    char sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    double dTranAmt = 0.0;
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    if (UpTransType("S", sOTransDate, sORrn) < 0) {
        ErrHanding(pstTransJson, "96", "����[%s:%s]��������trans_type����ʧ��.", sOTransDate, sORrn);
        return -1;
    }
    return 0;
}

/* �ֻ�APP ������֤�б�תD0Ǯ������ ת�� �� ���� ��̨�Զ�ʵ��  add by gjq at 2017114*/
int AppTransToD0Wallet(cJSON *pstTranData, int *piFlag) {
    char sTransType[1 + 1] = {0}, sSettleFlag[1 + 1] = {0}, sOldRrn[12 + 1] = {0};
    cJSON *pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "settle_flag", sSettleFlag);
    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    tLog(DEBUG, "trans_type = [%s],settle_flag = [%s]", sTransType, sSettleFlag);

    if (sSettleFlag[0] == 'N') {
        ErrHanding(pstTransJson, "96", "ԭ����[%s]�����־settle_flag = [%s ��δ����]����ֹת�ˡ�����", sOldRrn, sSettleFlag);
        return ( -1);
    }

    if ((sTransType[0] == '2' && sSettleFlag[0] == 'M') || (sTransType[0] == 'S' && sSettleFlag[0] == 'M')) {
        /*ת�˵�DOǮ��������Ǯ��*/
        if (AddAppPosWallet(pstTranData, piFlag) < 0) {
            tLog(INFO, "ԭ����[%s]���׽��ת�˵�D0������Ǯ��ʧ�ܣ�", sOldRrn);
            return ( -1);
        }
        /*���ֲ���*/
        if (AddD0PosWallet(pstTranData, piFlag) < 0) {
            tLog(INFO, "ԭ����[%s]���׽������ʧ�ܣ�", sOldRrn);
            return ( -1);
        }
    } else if (sTransType[0] == 'S' && sSettleFlag[0] == 'F') {
        /* ��ת�˵� DOǮ��������Ǯ�� ֻ�����ֲ���*/
        tLog(INFO, "ԭ����[%s]���׽����ת�˵�D0������Ǯ�����������ֲ�����", sOldRrn);
        if (AddD0PosWallet(pstTranData, piFlag) < 0) {
            return ( -1);
        }
    } else if (sTransType[0] == 'S' && sSettleFlag[0] == 'Y') {
        tLog(INFO, "ԭ����[%s]���׽����ת�˵�D0������Ǯ�� �� �Ѿ����ֳɹ���", sOldRrn);
    } else {
        ErrHanding(pstTransJson, "96", "ϵͳ�쳣������ԭ����[%s]��ˮ��Ϣ", sOldRrn);
        return ( -1);
    }

    return ( 0);
}
