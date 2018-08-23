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
#include "t_cjson.h"
#include "merch.h"
#include "t_macro.h"

/* 微信 */
int MerchWxFee(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sFeeDesc[255 + 1] = {0}, sOrderNo[32 + 1] = {0};
    MerchFee stMerchFee;
    double dAmount = 0.0, dFee = 0.0;
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "qr_order_no", sOrderNo);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);

    if (FindMerchFee(&stMerchFee, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的费率资料.", sMerchId);
        return -1;
    }

    dFee = (dAmount / 100) * stMerchFee.dWxRate / 100;
    sprintf(sFeeDesc, "%f", stMerchFee.dWxRate / 100);
    tMake(&dFee, 2, 'u');
    //SET_DOU_KEY(pstTransJson, "fee", dFee);
    //SET_STR_KEY(pstTransJson, "fee_flag", "M");
    //SET_STR_KEY(pstTransJson, "fee_desc", sFeeDesc);

    tLog(INFO, "商户[%s]微信手续费[%f]", sMerchId, dFee);
    if ( UpFeeMsg( sOrderNo,"B",dFee,sFeeDesc ) < 0 ) {
        //tLog(ERROR,"更新商户[%s]订单[%s]的手续费出错",sMerchId,sOrderNo);
        ErrHanding(pstTransJson, "96", "更新商户[%s]订单[%s]的手续费出错.", sMerchId,sOrderNo);
        return ( -1 );
    }
    
    
    return 0;
}

/* 支付宝 */
int MerchBbFee(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sFeeDesc[255 + 1] = {0}, sOrderNo[32 + 1] = {0};
    MerchFee stMerchFee;
    double dAmount = 0.0, dFee = 0.0;
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "qr_order_no", sOrderNo);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    
    if (FindMerchFee(&stMerchFee, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的费率资料.", sMerchId);
        return -1;
    }

    dFee = (dAmount / 100) * stMerchFee.dBbRate / 100;
    sprintf(sFeeDesc, "%f", stMerchFee.dBbRate / 100);
    tMake(&dFee, 2, 'u');
    //SET_DOU_KEY(pstTransJson, "fee", dFee);
    //SET_STR_KEY(pstTransJson, "fee_flag", "M");
    //SET_STR_KEY(pstTransJson, "fee_desc", sFeeDesc);

    tLog(INFO, "商户[%s]支付宝手续费[%f]", sMerchId, dFee);
    
    if ( UpFeeMsg( sOrderNo,"B",dFee,sFeeDesc ) < 0 ) {
        //tLog(ERROR,"更新商户[%s]订单[%s]的手续费出错",sMerchId,sOrderNo);
        ErrHanding(pstTransJson, "96", "更新商户[%s]订单[%s]的手续费出错.", sMerchId,sOrderNo);
        return ( -1 );
    }
    
    return 0;
}