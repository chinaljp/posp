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
#include "postransdetail.h"

/******************************************************************************/
/*      ������:     ChkHisOriginInfo()                                        */
/*      ����˵��:   �����ʷԭ��ˮ                                            */
/*      �������:   cJSON *pstJson                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int ChkHisOriginInfo(cJSON *pstJson, int *piFlag) {
    char sOrgTraceNo[6 + 1] = {0}, sAddData[100] = {0};
    char sRrn[12 + 1] = {0}, sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0}, sTransDate[8 + 1] = {0},sOTmpDate[4 + 1] = {0};
    char sTraceNo[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0};
    char sOMsgType[4 + 1] = {0}, sAcqInstId[8 + 1] = {0};
    int iCnt = -1;
    char sYear[4 + 1] = {0};
    double dTranAmt = 0L, dUnrefundAmt = 0L, dFee = 0;
    PosTransDetail stPosTransDetail;
    cJSON * pstTransJson;

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    //GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTmpDate); //motify by gjq at 20171220
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate); //�������� add by gjq at 20171220
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    dTranAmt /= 100;

    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));

    /* ��ȡԭ������Ϣ */
    tLog(ERROR, "rrn[%s]", sORrn);

    //tLog(ERROR, "transcode  old [%s]", pstNetTran->sOldTransCode);
    /* ֻ��������ˮ������鵱����ˮ */
    
    /*BEGIN add by gjq at  20171220 �˻���ԭ�������� ֻ������4λ���ڣ���Ҫ�ж�ԭ�������ڵ����  ����ԭ�����������4λ���*/
    tLog(DEBUG,"�˻����͵�ԭ��������sOTmpDate=[%s],���ν�������sTransDate = [%s],sTransDate+4 = [%s] ",sOTmpDate,sTransDate,sTransDate+4);
    if(  memcmp(sOTmpDate,sTransDate+4,4) <= 0 ) {
        memcpy(sOTransDate,sTransDate,4);
        memcpy(sOTransDate+4,sOTmpDate,4);
        sOTransDate[9] = '\0';
    }
    else {
        memcpy(sYear,sTransDate,4); 
        sprintf(sOTransDate,"%d%s",atoi(sYear)-1,sOTmpDate);
    }
    tLog(INFO,"�˻�ԭ��������ΪsOTransDate = [%s]",sOTransDate);
    /*END add by gjq at 20171220*/
    
    /*BEGIN ����ԭ����ʧ��ʱ ����ԭ�������ڸ��µ����ݿ�ʱΪ���� �� 4λ BUG�޸�*/
    DEL_KEY(pstTransJson, "o_trans_date");
    SET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    /*END ����ԭ����ʧ��ʱ ����ԭ�������ڸ��µ����ݿ�ʱΪ���� �� 4λ BUG�޸�*/
    
    if ((iCnt = FindAcctJonHis(&stPosTransDetail, sMerchId, sORrn, sOTransDate)) < 0) {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ�ܣ���ԭ������ˮ��ԭ�̻���[%s] ԭ�����ο���[%s] ԭ��������[%s] iCnt [%d].", \
                sMerchId, sORrn, sOTransDate, iCnt);
        return -1;
    }

    /* ��¼ԭʼ���� */
    /*
    strcpy(pstNetTran->sOldBatchNo, stAcctJon.sBatchNo);
    sprintf(pstNetTran->sOldTermTraceNo, "%ld", stAcctJon.lTraceNo);
     */
    dUnrefundAmt = dTranAmt + stPosTransDetail.dRefundAmt;
    SET_DOU_KEY(pstTransJson, "refund_amt", dUnrefundAmt);
  /*DEL_KEY(pstTransJson, "o_trans_date");
    SET_STR_KEY(pstTransJson, "o_trans_date", stPosTransDetail.sTransDate);
  */
    GetOMsgType(sOMsgType, stPosTransDetail.sTransCode);
    strcat(sAcqInstId, "0004900");
    tStrCpy(sAcqInstId + 7, sMerchId + 3, 4);
    sprintf(sAddData, "%s%06s%10s%11s%11s"
            , sOMsgType
            , stPosTransDetail.sSysTrace
            //  , stPosTransDetail.sTransDate + 4
            , stPosTransDetail.sTransmitTime
            , sAcqInstId
            , "00049000000");
    //SET_JSON_KEY(pstJson, "his_trans_detail", stPosTransDetail);
    PrtAcctJournal(&stPosTransDetail);

    if (ChkHisAcctJon(pstTransJson, &stPosTransDetail) != 0) {
        return -1;
    }

    tLog(INFO, "�˻�ԭ���׼��ɹ� ԭ�̻���[%s] ԭ�����ο���[%s] ԭ��������[%s]", \
            sMerchId, sORrn, sOTransDate);
    /* �˻�����ȡ��ԭ���׵� ���κš���ˮ�š��������� */
    /*
        if ( CMP_E == memcmp( pstNetTran->sTransCode, "020100", TRANSCODE_LEN ) )
        {
            sprintf( pstNetTran->sAddData1, "%6s%06ld%4s"
                    , stAcctJon.sBatchNo, stAcctJon.lSysTrace, stAcctJon.sChannelTransDate+4 );
        }
     */

    SET_STR_KEY(pstTransJson, "90_data", sAddData);
    SET_STR_KEY(pstTransJson, "auth_code", stPosTransDetail.sAuthCode);
    SET_STR_KEY(pstTransJson, "channel_id", stPosTransDetail.sChannelId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", stPosTransDetail.sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", stPosTransDetail.sChannelTermId);
    SET_STR_KEY(pstTransJson, "trans_type", stPosTransDetail.sTransType);
    SET_STR_KEY(pstTransJson, "fee_type", stPosTransDetail.sFeeType);
    SET_STR_KEY(pstTransJson, "card_type", stPosTransDetail.sCardType);   
    dFee = stPosTransDetail.dFee*dTranAmt/stPosTransDetail.dAmount;
    tMake(&dFee, 2, 'u');
    SET_DOU_KEY(pstTransJson,"fee",dFee);
    SET_STR_KEY(pstTransJson, "fee_flag", "M");
    SET_STR_KEY(pstTransJson, "fee_desc", stPosTransDetail.sFeeDesc);
    /*�˻�����o_amount*/
    SET_DOU_KEY(pstTransJson,"o_amount",stPosTransDetail.dAmount);
    /*add by gjq add 20180613 ��ȡԭ���׵����뷽ʽ �����жϽ���ʹ�õĿ�ΪIC�� ���Ǵ�����*/
    SET_STR_KEY(pstTransJson, "o_input_mode", stPosTransDetail.sInputMode);
    
    return 0;
}

int ChkHisAcctJon(cJSON *pstTransJson, PosTransDetail *pstPosTransDetail) {
    double dTranAmt = 0L, dUnrefundAmt = 0L;
    char sOrgTraceNo[6 + 1] = {0};
    char sRrn[12 + 1] = {0};
    int iCnt = -1;
    char sTransDate[8 + 1] = {0}, sCardNo[19 + 1] = {0};

    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    dTranAmt /= 100;

    /* �˻�ֻ�ܸ���,���ղ����˻� */
    if (!memcmp(pstPosTransDetail->sLogicDate, sTransDate, 8)) {
        ErrHanding(pstTransJson, "25", "ԭ�߼�����[%s]�뵱ǰ�߼�����[%s]һ��,�˻�ֻ�ܸ���.", pstPosTransDetail->sLogicDate, sTransDate);
        return -1;
    }

#if 0
    /* ����ֻ������һ���˻����� */
    if ((iCnt = DBGetRefundCnt(pstNetTran->sMerchId, pstNetTran->sTermId, sRrn)) < 0) {
        tErrLog(ERR, "��ȡ�̻�[%s]�ն�[%s]Rrn[%s]�˻����״���ʧ��.", \
                        pstNetTran->sMerchId, pstNetTran->sTermId, sRrn);
        SetRespCode(pstNetTran, "96", pcInstId);
        return -1;
    }


    if (iCnt > 0) {
        tLog(ERROR, "�̻�[%s]�ն�[%s]Rrn[%s]�˻����״���[%d]����.", \
                        pstNetTran->sMerchId, pstNetTran->sTermId, sRrn, iCnt);
        SetRespCode(pstNetTran, "89", pcInstId);
        return -1;
    }
#endif
    if (pstPosTransDetail->sValidFlag[0] != '0' && pstPosTransDetail->sValidFlag[0] != '3') {
        ErrHanding(pstTransJson, "12", "ԭ������Ч��־λ[%c].", pstPosTransDetail->sValidFlag[0]);
        return -1;
    }

    /* ���ԭ�����Ƿ�ɹ� */
    if (memcmp(pstPosTransDetail->sRespCode, "00", 2)) {
        ErrHanding(pstTransJson, "12", "ԭ����ʧ��,Ӧ����[%s].", pstPosTransDetail->sRespCode);
        return -1;
    }
    /* ��鿨�� */
    if (strcmp(pstPosTransDetail->sCardNo, sCardNo)) {
        ErrHanding(pstTransJson, "63", "���Ų�һ�� ԭ����[%s] ���Ϳ���[%s].", pstPosTransDetail->sCardNo, sCardNo);
        return -1;
    }
    /* ����� */
    dUnrefundAmt = pstPosTransDetail->dAmount - pstPosTransDetail->dRefundAmt;
    tLog(DEBUG, "�˿���[%.10f], δ�˽��[%.10f].", dTranAmt, dUnrefundAmt);

    /*
        if ( fabs(dTranAmt - dUnrefundAmt) > 0.00001 )
     */
    if (dTranAmt - dUnrefundAmt > 0.00001) {
        ErrHanding(pstTransJson, "13", "�˿���[%.02f]����δ�˽��[%.02f].", dTranAmt, dUnrefundAmt);
        return -1;
    }

    return 0;
}



