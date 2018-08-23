#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "qrcodetransdetail.h"

/*��ά�� ֧�������ѯ �ӽ�����ˮ���л�ȡ�����̻��� (�ѳ����Ľ��������ѯ)*/
int ChkOrTransMsg(cJSON *pstJson, int *piFlag) {

    cJSON *pstTransJson = NULL;
    char sMerchOrderNo[100];
    char sTransCode[6 + 1];
    char sChannelMerchId[15 + 1];
    char sValidFlag[1 + 1];

    MEMSET(sMerchOrderNo);
    MEMSET(sTransCode);
    MEMSET(sChannelMerchId);
    MEMSET(sValidFlag);
    
    double dAmount = 0.00;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "qr_order_no", sMerchOrderNo);
    if (FindOriTransMsg(sMerchOrderNo, sTransCode, sChannelMerchId, sValidFlag, &dAmount) < 0) {
    //if (FindOriTransMsg(sMerchOrderNo, sTransCode, sChannelMerchId, sValidFlag) < 0) {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ�ܣ���ԭ������ˮ���̻�������[%s].", sMerchOrderNo);
        return ( -1);
    }
    tLog(DEBUG, "sValidFlag = [%s]", sValidFlag);
    /* if(sValidFlag[0] == '4') {
         ErrHanding(pstTransJson, "R3", "δ֧�����̻�������[%s].",sMerchOrderNo);
         return ( -1 );
     }*/

    /*΢�š�֧���� ��ɨ֧����ѯ ����ԭ���׽��*/
    if(sTransCode[2] == 'B' || sTransCode[2] == 'W') {
        tLog(INFO,"ԭ����[%s]���׽��[%.2lf]",sTransCode,dAmount);
        DEL_KEY(pstTransJson,"amount");
        SET_INT_KEY( pstTransJson, "amount", (int)(dAmount*100) );
    }

    tLog(DEBUG, "fuck sTransCode = [%s]", sTransCode);
    if (sTransCode[3] == '1') {
        SET_STR_KEY(pstTransJson, "scan_type", "ZS");
    }
    if (sTransCode[3] == '2') {
        SET_STR_KEY(pstTransJson, "scan_type", "BS");
    }

    //SET_STR_KEY(pstTransJson, "channel_merch_id", sChannelMerchId);
    //tLog(DEBUG,"���׽�� dAmount = [%.02f]",dAmount);
    //SET_DOU_KEY(pstTransJson, "amount", dAmount);
    return ( 0 );
}

/* ��ά�� �������� ���ԭ����(��Ҫԭ���׶�����,�����̻���) */
int ChkOrTransDetail(cJSON *pstJson, int *piFlag) {
    QrcodeTransDetail stQrcodeTransDetail;
    cJSON *pstTransJson = NULL;
    char sORrn[12 + 1]; /* ԭ����rrn   */

    memset(&stQrcodeTransDetail, 0x00, sizeof (QrcodeTransDetail));

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    if (FindOriTransDetail(sORrn, &stQrcodeTransDetail) < 0) {
        ErrHanding(pstTransJson, "25", "ԭ���׼��ʧ�ܣ���ԭ������ˮ��ԭ����rrn[%s].", sORrn);
        return ( -1);
    }

    tLog(DEBUG, "sValidFlag = [%s]", stQrcodeTransDetail.sValidFlag);
    /*�ж�ԭ�����Ƿ��ѳ���*/
    if (stQrcodeTransDetail.sValidFlag[0] == '1') {
        ErrHanding(pstTransJson, "C9", "ԭ�����ѳ�����ԭ����rrn[%s].", sORrn);
        return ( -1);
    }

    /*���ԭ����Ϊ΢��֧���� ����������02B600��Ϊ02W600*/
    /*if (!memcmp(stQrcodeTransDetail.sTransCode, "02W100", 6) || !memcmp(stQrcodeTransDetail.sTransCode, "02W200", 6)) {
        DEL_KEY(pstTransJson, "trans_code");
        SET_STR_KEY(pstTransJson, "trans_code", "02W600");
    }*/

    /* ֧������  ��ɨ֧������ɨ֧�� ���׽�� ��û����Ǯ���ģ�SETTLE_FLAG = ��N����  �ݲ�������*/
    tLog(DEBUG, "sSettleFlag = [%s]", stQrcodeTransDetail.sSettleFlag);
    if (stQrcodeTransDetail.sSettleFlag[0] == 'N') {
        /*Z1 ����ʧ�ܣ����Ժ�����*/
        ErrHanding(pstTransJson, "Z1", "�ֽ��׽��δ����Ǯ���ݲ������������Ժ����ԣ�ԭ����rrn[%s].", sORrn);
        return ( -1);
    }
    
    //SET_DOU_KEY(pstTransJson, "amount", stQrcodeTransDetail.dAmount);
    //SET_STR_KEY(pstTransJson, "channel_merch_id", stQrcodeTransDetail.sChannelMerchId);
    SET_STR_KEY(pstTransJson, "settle_date", stQrcodeTransDetail.sSettleDate);
    SET_STR_KEY(pstTransJson, "orig_order_no", stQrcodeTransDetail.sMerchOrderNo);
    SET_DOU_KEY(pstTransJson, "fee", stQrcodeTransDetail.dFee);
    SET_STR_KEY(pstTransJson, "fee_flag", stQrcodeTransDetail.sFeeFlag);
    SET_STR_KEY(pstTransJson, "fee_type", stQrcodeTransDetail.sFeeType);
    SET_STR_KEY(pstTransJson, "fee_desc", stQrcodeTransDetail.sFeeDesc);
    SET_STR_KEY(pstTransJson, "code_url", stQrcodeTransDetail.sOrderUrl);

    return ( 0);
}