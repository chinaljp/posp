#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

int QuickPayOrder(cJSON *pstTranData, int *piFlag) {

    char sOrder_no[32+1] = {0},sTransDate[8+1] = {0}, sTransCode[6 + 1] = {0};
    char sMerchId[16] = {0}, sRrn[13] = {0}, sDate[8 + 1] = {0};
    cJSON * pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "settle_date", sDate);

    sprintf(sOrder_no, "%s%s%s", sTransDate+2,sRrn+6, sMerchId);
    tLog(DEBUG, "merch_order_no[%s]", sOrder_no);
    /* ���������н������Ÿ��µ����ν��׵���ˮ��¼��*/
    if( UpMerchOrderNo(sDate,sOrder_no,sMerchId,sRrn,sTransCode) < 0 ) {
        ErrHanding(pstTransJson, "96", "�����̻�[%s]������[%s]ʧ��.", sMerchId,sOrder_no);
        return ( -1 );
    }
    
    SET_STR_KEY(pstTransJson, "merch_order_no", sOrder_no);
    return 0;
}

/*EPOS ����޿����Ѳ�ѯ ����ԭ�����ѣ�����valid_flag*/
int UpValidProc(cJSON *pstJson, int *piFlag) {
    char sRespCode[2 + 1] = {0},sRrn[12+1] = {0},sORrn[12+1] = {0};
    char sORespDesc[200] = {0};
    int iTxStatus = 0;
    
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "rrn", sRrn);
    GET_STR_KEY(pstDataJson, "o_rrn", sORrn);
    
    /*ԭ����֧�����*/
    GET_STR_KEY(pstDataJson, "o_resp_desc", sORespDesc);
    GET_INT_KEY(pstDataJson, "tx_state", iTxStatus);
    
    if ( !memcmp(sRespCode, "00", 2) && iTxStatus == 2 ) {
        if( UpValidflag(sORrn) < 0 ) {
            tLog(ERROR, "����EPOSԭ����valid_flag��־ʧ�ܣ�ԭ����rrn[%s]", sORrn);
            return -1;
        }
        tLog(INFO, "����EPOSԭ����[%s]�������־�ɹ�.", sORrn);
        if ( UpRespDesc( "00", sORespDesc, sORrn ) < 0 ) {
            tLog(ERROR, "����EPOSԭ����֧���������ʧ�ܣ�ԭ����rrn[%s]", sORrn);
            return -1;
        }
    }
    else if( !memcmp(sRespCode, "00", 2) && ( iTxStatus == 0 || iTxStatus == 1 || iTxStatus == 3 ) ) {//strlen(sORespDesc) != 0 && 
        tLog(WARN,"ԭ����֧��������,iTxStatus[%d]",iTxStatus);
        if ( UpRespDesc( "FD", sORespDesc, sORrn ) < 0 ) {
            tLog(ERROR, "����EPOSԭ����֧���������ʧ�ܣ�ԭ����rrn[%s]", sORrn);
            return -1;
        }
    }
    else if ( !memcmp(sRespCode, "00", 2) && iTxStatus == 4 ) {
        tLog(WARN,"ԭ����֧��ʧ��,iTxStatus[%d]",iTxStatus);
        if ( UpRespDesc( "96", sORespDesc, sORrn ) < 0 ) {
            tLog(ERROR, "����EPOSԭ����֧���������ʧ�ܣ�ԭ����rrn[%s]", sORrn);
            return -1;
        }
    }/*�˻� ������ ��ѯʱ �˴���ӽ���ж�*/
    else {
        tLog(ERROR,"RESP_CODE=[%s],���Ѳ�ѯ����ʧ��,[%s]", sRespCode,sRrn);
        //ErrHanding(pstDataJson, sRespCode);
        return ( -1 );
    }
    /* ����״̬ Ϊ 0��1��3 ʱ Ϊ�м�״̬ ������APP����֪ͨ */
    if ( iTxStatus != 0 && iTxStatus != 1 && iTxStatus != 3 ) {
        if(  UpNoticeFlag( sORrn ) < 0 ) {
            tLog(ERROR, "���½���[%s]�����ͱ�־notice_flagʧ��.",sRrn);
            return ( -1 );
        }
    }
    return ( 0 );
}