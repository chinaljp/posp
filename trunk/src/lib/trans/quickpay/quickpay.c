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
    /* 请求交易流中将订单号更新到本次交易的流水记录中*/
    if( UpMerchOrderNo(sDate,sOrder_no,sMerchId,sRrn,sTransCode) < 0 ) {
        ErrHanding(pstTransJson, "96", "更新商户[%s]订单号[%s]失败.", sMerchId,sOrder_no);
        return ( -1 );
    }
    
    SET_STR_KEY(pstTransJson, "merch_order_no", sOrder_no);
    return 0;
}

/*EPOS 快捷无卡消费查询 更新原（消费）交易valid_flag*/
int UpValidProc(cJSON *pstJson, int *piFlag) {
    char sRespCode[2 + 1] = {0},sRrn[12+1] = {0},sORrn[12+1] = {0};
    char sORespDesc[200] = {0};
    int iTxStatus = 0;
    
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "rrn", sRrn);
    GET_STR_KEY(pstDataJson, "o_rrn", sORrn);
    
    /*原交易支付结果*/
    GET_STR_KEY(pstDataJson, "o_resp_desc", sORespDesc);
    GET_INT_KEY(pstDataJson, "tx_state", iTxStatus);
    
    if ( !memcmp(sRespCode, "00", 2) && iTxStatus == 2 ) {
        if( UpValidflag(sORrn) < 0 ) {
            tLog(ERROR, "更新EPOS原交易valid_flag标志失败，原交易rrn[%s]", sORrn);
            return -1;
        }
        tLog(INFO, "更新EPOS原交易[%s]的清算标志成功.", sORrn);
        if ( UpRespDesc( "00", sORespDesc, sORrn ) < 0 ) {
            tLog(ERROR, "更新EPOS原交易支付结果描述失败，原交易rrn[%s]", sORrn);
            return -1;
        }
    }
    else if( !memcmp(sRespCode, "00", 2) && ( iTxStatus == 0 || iTxStatus == 1 || iTxStatus == 3 ) ) {//strlen(sORespDesc) != 0 && 
        tLog(WARN,"原交易支付处理中,iTxStatus[%d]",iTxStatus);
        if ( UpRespDesc( "FD", sORespDesc, sORrn ) < 0 ) {
            tLog(ERROR, "更新EPOS原交易支付结果描述失败，原交易rrn[%s]", sORrn);
            return -1;
        }
    }
    else if ( !memcmp(sRespCode, "00", 2) && iTxStatus == 4 ) {
        tLog(WARN,"原交易支付失败,iTxStatus[%d]",iTxStatus);
        if ( UpRespDesc( "96", sORespDesc, sORrn ) < 0 ) {
            tLog(ERROR, "更新EPOS原交易支付结果描述失败，原交易rrn[%s]", sORrn);
            return -1;
        }
    }/*退货 、撤销 查询时 此处添加结果判断*/
    else {
        tLog(ERROR,"RESP_CODE=[%s],消费查询交易失败,[%s]", sRespCode,sRrn);
        //ErrHanding(pstDataJson, sRespCode);
        return ( -1 );
    }
    /* 交易状态 为 0、1、3 时 为中间状态 放弃给APP推送通知 */
    if ( iTxStatus != 0 && iTxStatus != 1 && iTxStatus != 3 ) {
        if(  UpNoticeFlag( sORrn ) < 0 ) {
            tLog(ERROR, "更新交易[%s]的推送标志notice_flag失败.",sRrn);
            return ( -1 );
        }
    }
    return ( 0 );
}