/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "t0limit.h"
#include "postransdetail.h"



/* �սύ�׼�� */
int ChkD0Limit(cJSON *pstTransJson, int *piFlag) {
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
    int iTnum = 0, iSwth = 0;
    
    char sSingleAmt[32 + 1] = {0};
    double dSingleAmt = 0.0;
    
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    dTranAmt = dTranAmt / 100;
    if (sCardType[0] == '0') {
        ErrHanding(pstTransJson, "57", "�ս�����ֻ֧�����ÿ�,��ǰ����Ϊ��ǿ�.");
        return -1;
    }
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
            if ( (iNum > 0 &&  !DBL_CMP(dTranAmt,dSingleAmt)) || (iNum > 0 && DBL_EQ(dTranAmt,dSingleAmt)) ) {
                tLog(DEBUG, "�̻�[%s]����������֤���������Ҵ������ÿ����׶�[%.02f]С�ڵ���[%.02f],����ֱ������", sMerchId,dTranAmt,dSingleAmt);
            }
            else { 
                tLog(WARN, "�̻�[%s]����������֤������ ���� �˴ν��״������ÿ����׶��.",sMerchId);
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





int UptT0Limit(double dAmount, char *pcUserCode) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE  B_MERCH_AUTH_LIMIT SET USED_LIMIT = USED_LIMIT+%f "
            ",USABLE_LIMIT = USABLE_LIMIT-%f \
         WHERE USER_CODE = '%s'", dAmount, dAmount, pcUserCode);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0)
    {
        tLog(ERROR, "���¶��ʧ��USER_CODE[%s].", pcUserCode);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

/* �����ս��� */
int UpdD0Limit(cJSON *pstTransJson, int *piFlag) {
    char sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    double dTranAmt = 0.0;

    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    /* �����̻�T0�޶�  */
    if (UptT0Limit(dTranAmt / 100, sUserCode) < 0)
    {
        ErrHanding(pstTransJson, "96", "�̻�[%s]D0���׶��[+]����ʧ��.", sMerchId, dTranAmt);
        return -1;
    }
    tLog(INFO, "�̻�[%s]D0���׶��[%.02f]���³ɹ�.", sMerchId, dTranAmt/100);
    return 0;
}


/*  app����trans_type*/
int UpdAppTransType(cJSON *pstTransJson, int *piFlag) {
    char sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    double dTranAmt = 0.0;
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    if (UpTransType("S", sOTransDate, sORrn) < 0) {
        ErrHanding(pstTransJson, "96", "����[%s:%s]��������trans_type����ʧ��.", sOTransDate, sORrn);
        return -1;
    }
    return 0;
}

/* �ֻ�APP ������֤�б�תD0Ǯ������ ת�� �� ���� ��̨�Զ�ʵ��  add by gjq at 2017114*/
int AppTransToD0Wallet(cJSON *pstTransJson, int *piFlag) {
    char sTransType[1 + 1] = {0}, sSettleFlag[1 + 1] = {0}, sOldRrn[12 + 1] = {0};
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "settle_flag", sSettleFlag);
    GET_STR_KEY(pstTransJson, "o_rrn", sOldRrn);
    tLog( DEBUG,"trans_type = [%s],settle_flag = [%s]", sTransType,sSettleFlag );
     
    if( sSettleFlag[0] == 'N' ) {
        ErrHanding(pstTransJson, "96", "ԭ����[%s]�����־settle_flag = [%s ��δ����]����ֹת�ˡ�����", sOldRrn,sSettleFlag);
        return ( -1 );
    }
    
    if( (sTransType[0] == '2' && sSettleFlag[0] == 'M') || (sTransType[0] == 'S' && sSettleFlag[0] == 'M') ) {
        /*ת�˵�DOǮ��������Ǯ��*/
        if( AddAppPosWallet(pstTransJson, piFlag) < 0 ) {
            tLog(INFO,"ԭ����[%s]���׽��ת�˵�D0������Ǯ��ʧ�ܣ�",sOldRrn);
            return ( -1 );
        }
        /*���ֲ���*/
        if( AddD0PosWallet(pstTransJson, piFlag) < 0 )  {
            tLog(INFO,"ԭ����[%s]���׽������ʧ�ܣ�",sOldRrn);
            return ( -1 );
        }
    }
    else if( sTransType[0] == 'S' && sSettleFlag[0] == 'F' ) {
        /* ��ת�˵� DOǮ��������Ǯ�� ֻ�����ֲ���*/
        tLog(INFO,"ԭ����[%s]���׽����ת�˵�D0������Ǯ�����������ֲ�����",sOldRrn);
        if( AddD0PosWallet(pstTransJson, piFlag) < 0 )  {
            return ( -1 );
        }
    }
    else if ( sTransType[0] == 'S' && sSettleFlag[0] == 'Y' ) {
        tLog(INFO,"ԭ����[%s]���׽����ת�˵�D0������Ǯ�� �� �Ѿ����ֳɹ���",sOldRrn);
    }
    else {
        ErrHanding(pstTransJson, "96", "ϵͳ�쳣������ԭ����[%s]��ˮ��Ϣ", sOldRrn);
        return ( -1 );
    }
    
    return ( 0 );
}
