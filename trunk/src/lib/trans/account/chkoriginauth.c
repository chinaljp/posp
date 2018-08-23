#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "postransdetail.h"

/******************************************************************************/
/*      ������:     ChkOriginAuth()                                           */
/*      ����˵��:   ���Ԥ��Ȩԭ��ˮ                                          */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int ChkOriginAuth(cJSON *pstJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sTrace[7] = {0}, sOTraceNo[6 + 1] = {0}, sSysTrace[6 + 1] = {0};
    char sOldDate[8 + 1] = {0}, sTransCode[6 + 1] = {0}, sAuthCode[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sOTransDate[8 + 1] = {0}, sCardNo[19 + 1] = {0}, sAddData[100] = {0};
    char sTraceNo[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0};
    int iRet = 0;
    cJSON * pstTransJson;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstTransJson, "auth_code", sAuthCode);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);

    //pstTranData->cJonFlag = 0;
    /*
     if ( pstNetTran->cSafFlag == '1' )
     {
         tErrLog( INFO, "SAF������ԭ����." );
         return 0;
     }
     */
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));

    /* ��ȡԭ������Ϣ */
    tStrCpy(sTrace, sOTraceNo, 6);


    /* pstPosTransDetail.lTraceNo,pstPosTransDetail.lSysTrace, pstPosTransDetail.lChannelTrace,  pstPosTransDetail.sRrn  */
    /* ��ԭ���׼���ʧ��ʱ��������ˮ��Ҫԭ���׼�¼ */
    //tStrCpy(stPosTransDetail.sTraceNo, sTrace, 6);
    //tStrCpy(stPosTransDetail.sSysTrace, pcSysTrace, 6);

    //if ( CMP_E == memcmp( pstNetTran->pcTransCode, "024100", TRANSCODE_LEN ) )
    if (0 == memcmp(sTransCode, "024100", 6)) {
        tStrCpy(stPosTransDetail.sTransCode, "024000", 6);
    } else {
        tStrCpy(stPosTransDetail.sTransCode, "024000", 6);
    }
    tLog(INFO, "trancode=[%s].", sTransCode);
    //pstTranData->stHisAcctJon = pstPosTransDetail;

    /* ��Ȩ�밴λ���� */
    tTrim(sAuthCode);
    tStrPad(sAuthCode, 6, 0x00 | RIGHT_ALIGN);

    /* Ԥ��Ȩ��ɡ�Ԥ��Ȩ����ֻ�����һ�������ڵ�Ԥ��Ȩ���� */
    tStrCpy(sOldDate, sTransDate, 4);
    strcat(sOldDate, sOTransDate);
    tLog(INFO, "sOldDate[%s] sTransDate[%s]", sOldDate, sTransDate);
    if (tDateCount(sOldDate, sTransDate) > 30) {
        ErrHanding(pstTransJson, "96", "�޷���һ���������Ԥ��Ȩ���׽���Ԥ��Ȩ��ɻ�Ԥ��Ȩ����������ԭԤ��Ȩ��������[%s]"
                , sOTransDate);
        return -1;
    }

    /* Ԥ��Ȩ��ɡ�Ԥ��Ȩ�������ܸ����Σ��������ˮ����ʷ��ˮ�� */
    iRet = FindRevAuthJon(&stPosTransDetail, sCardNo, sOTransDate, sAuthCode);
    if (iRet < 0) {
        iRet = FindRevAuthJonHis(&stPosTransDetail, sCardNo, sOTransDate, sAuthCode);
        //pstTranData->cJonFlag = 1;
    }

    if (iRet < 0) {
        ErrHanding(pstTransJson, "25", "ԭ����[%s]���ʧ�ܣ���ԭ������ˮ��ԭ����[%s] ԭ��������[%s] ԭ��Ȩ��[%s].", \
                stPosTransDetail.sTransCode, sCardNo, sOTransDate, sAuthCode);
        return -1;
    }

    tLog(DEBUG, "ԭ����[%s]ԭ����[%s] ԭ��������[%s] ԭ��Ȩ��[%s] ԭ��ˮ��[%ld] ԭ���κ�[%s].", \
        stPosTransDetail.sTransCode, sCardNo, sOTransDate, sAuthCode, \
        stPosTransDetail.sSysTrace, stPosTransDetail.sBatchNo);

    PrtAcctJournal(&stPosTransDetail);

    SET_STR_KEY(pstTransJson, "o_trans_code", stPosTransDetail.sTransCode);
    SET_STR_KEY(pstTransJson, "o_rrn", stPosTransDetail.sORrn);

    sprintf(sAddData, "%s%s%4s"
            , "000000", stPosTransDetail.sSysTrace, stPosTransDetail.sTransDate + 4);
    SET_STR_KEY(pstTransJson, "add_data", sAddData);
    /* ���ԭԤ��Ȩ�����Ƿ���Ч */
    if ((iRet = ChkAuthJon(pstTransJson, &stPosTransDetail)) != 0) {
        if (iRet == -1)
            ErrHanding(pstTransJson, "96", "ԭ���׼��ʧ��");
        else if (iRet == -2)

            ErrHanding(pstTransJson, "94", "ԭ���׼��ʧ��");
            //Change By LiuZe 2013-09-11 08:43
            //�޸Ľ�������ʱ������Ϊ64
        else if (iRet == -3)

            ErrHanding(pstTransJson, "64", "ԭ���׼��ʧ��");
            //End By 2013-09-11 08:43
            //Change by LiuZe 2014-01-10 14:14
            //Ԥ��Ȩ��ɽ��ô���Ԥ��Ȩ���
        else if (iRet == -4)

            ErrHanding(pstTransJson, "C8", "ԭ���׼��ʧ��");
        //End By 2014-01-10 14:14
        return -1;
    }

    SET_STR_KEY(pstTransJson, "channel_id", stPosTransDetail.sChannelId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", stPosTransDetail.sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", stPosTransDetail.sChannelTermId);
    SET_STR_KEY(pstTransJson, "his_trans_date", stPosTransDetail.sTransDate);
    SET_STR_KEY(pstTransJson, "his_rrn", stPosTransDetail.sRrn);
    SET_STR_KEY(pstTransJson, "trans_type", stPosTransDetail.sTransType);
    //pstTranData->stHisAcctJon = pstPosTransDetail;
    tLog(INFO, "ԭ���׼��ɹ� ԭ�̻���[%s] ԭ�ն˺�[%s] ԭ������ˮ��[%ld] ԭ��������[%s]", \
            sMerchId, sTermId, sTraceNo, stPosTransDetail.sSettleDate);
    return 0;   
}

int ChkAuthJon(cJSON *pstTransJson, PosTransDetail *pstPosTransDetail) {
    double dTranAmt;
    char sTransCode[6 + 1] = {0}, sInputMode[3 + 1] = {0}, sCardNo[19 + 1] = {0};
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    dTranAmt /= 100;
    /* �����ɶ�����ͣ��������Ч��־��ʧ�ܽ��ײ�������� */
    if (0 == memcmp(sTransCode, "020300", 6) ||
            0 == memcmp(sTransCode, "M20300", 6)) {
        return 0;
    }

    /* ���ԭ�����Ƿ���Ч */
    if (pstPosTransDetail->sValidFlag[0] != '0') {
        tLog(ERROR, "ԭ������Ȩ��[%s],��Ч��־λ[%s]."
                , pstPosTransDetail->sAuthCode, pstPosTransDetail->sValidFlag);
        return -1;
    }

    /* ���ܶ�����ɵ�Ԥ��Ȩ���������� */
    if ((0 == memcmp(sTransCode, "024002", 6))
            && ('1' == pstPosTransDetail->sAuthoFlag[0])) {
        tLog(ERROR, "ԭ������Ȩ��[%s],Ԥ��Ȩ�����, �޷����г���!"
                , pstPosTransDetail->sAuthCode);
        return -1;
    }

    /* ���ԭ�����Ƿ�ɹ� */
    if (memcmp(pstPosTransDetail->sRespCode, "00", 2)) {
        tLog(ERROR, "ԭ����[%s]ʧ��,��ˮ��[%s],Ӧ����[%s].", \
                        pstPosTransDetail->sTransCode, pstPosTransDetail->sTraceNo, pstPosTransDetail->sRespCode);
        return -1;
    }

    /* ���ԭԤ��Ȩ�����Ƿ������ */
    if ((0 == memcmp(sTransCode, "024100", 6) || 0 == memcmp(sTransCode, "027100", 6))
            &&('1' == pstPosTransDetail->sAuthoFlag[0])) {
        tLog(ERROR, "ԭԤ��Ȩ����[%s]�����! ", pstPosTransDetail->sAuthCode);
        return -2;
    }

    /* �ֹ�����ʱ��鿨�� */
    if ((0 == memcmp(sInputMode, "011", 3))
            || (0 == memcmp(sInputMode, "012", 3))) {
        tTrim(pstPosTransDetail->sCardNo);
        tTrim(sCardNo);
        if (0 != strcmp(pstPosTransDetail->sCardNo, sCardNo)) {
            tLog(ERROR, "���Ų�һ�� ԭ����[%s] ���Ϳ���[%s].", pstPosTransDetail->sCardNo, sCardNo);
            return -1;
        }
    }

    /* ����� */

    //Change By LiuZe 2013-09-11 08:43
    //�޸Ľ�����ʱ������Ϊ64
    //�޸ķ�Ԥ��Ȩ��ɽ���ʱΪ����Ƿ�һ��
    if (memcmp(sTransCode, "024100", 6)) {
        if (!((dTranAmt - pstPosTransDetail->dAmount) < 0.00001 && ((dTranAmt - pstPosTransDetail->dAmount) > -0.00001))) {
            tLog(ERROR, "��һ�� ԭ���[%.02f] ���ͽ��[%.02f].", pstPosTransDetail->dAmount, dTranAmt);
            return -3;
        }
#if 0
        if ((dTranAmt - pstPosTransDetail->dAmount) > 0.00001) {
            tErrLog(ERR, "Ԥ��Ȩ��ɽ��� ԭ���[%.02f] ���ͽ��[%.02f].", pstPosTransDetail->dAmount, dTranAmt);
            return -3;
        }
#endif
    }//Change By LiuZe 2014-01-10 14:07
        //�޸�Ԥ��Ȩ��ɽ��С�ڵ���Ԥ��Ȩ���
    else {
        if ((dTranAmt - pstPosTransDetail->dAmount) > 0.00001) {
            tLog(ERROR, "Ԥ��Ȩ��ɽ��� ԭ���[%.02f] ���ͽ��[%.02f].", pstPosTransDetail->dAmount, dTranAmt);
            return -4;
        }

    }

    /*
    if ( ( dTranAmt - pstPosTransDetail->dAmount ) > 0.00001 )
    {
        tErrLog( ERR, "���� ԭ���[%.02f] ���ͽ��[%.02f].", pstPosTransDetail->dAmount, dTranAmt );
        return -1;
    }
     */
    //End Change by 2013-09-11 08:43

    return 0;
}

