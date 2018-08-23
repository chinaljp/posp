#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "qrcodetransdetail.h"

/*二维码 支付结果查询 从交易流水表中获取渠道商户号 (已撤销的交易允许查询)*/
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
        ErrHanding(pstTransJson, "25", "原交易检查失败，无原交易流水，商户订单号[%s].", sMerchOrderNo);
        return ( -1);
    }
    tLog(DEBUG, "sValidFlag = [%s]", sValidFlag);
    /* if(sValidFlag[0] == '4') {
         ErrHanding(pstTransJson, "R3", "未支付，商户订单号[%s].",sMerchOrderNo);
         return ( -1 );
     }*/

    /*微信、支付宝 被扫支付查询 上送原交易金额*/
    if(sTransCode[2] == 'B' || sTransCode[2] == 'W') {
        tLog(INFO,"原交易[%s]交易金额[%.2lf]",sTransCode,dAmount);
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
    //tLog(DEBUG,"交易金额 dAmount = [%.02f]",dAmount);
    //SET_DOU_KEY(pstTransJson, "amount", dAmount);
    return ( 0 );
}

/* 二维码 撤销交易 检查原交易(需要原交易订单号,渠道商户号) */
int ChkOrTransDetail(cJSON *pstJson, int *piFlag) {
    QrcodeTransDetail stQrcodeTransDetail;
    cJSON *pstTransJson = NULL;
    char sORrn[12 + 1]; /* 原交易rrn   */

    memset(&stQrcodeTransDetail, 0x00, sizeof (QrcodeTransDetail));

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    if (FindOriTransDetail(sORrn, &stQrcodeTransDetail) < 0) {
        ErrHanding(pstTransJson, "25", "原交易检查失败，无原交易流水，原交易rrn[%s].", sORrn);
        return ( -1);
    }

    tLog(DEBUG, "sValidFlag = [%s]", stQrcodeTransDetail.sValidFlag);
    /*判断原交易是否已撤销*/
    if (stQrcodeTransDetail.sValidFlag[0] == '1') {
        ErrHanding(pstTransJson, "C9", "原交易已撤销，原交易rrn[%s].", sORrn);
        return ( -1);
    }

    /*如果原交易为微信支付则 撤销交易码02B600改为02W600*/
    /*if (!memcmp(stQrcodeTransDetail.sTransCode, "02W100", 6) || !memcmp(stQrcodeTransDetail.sTransCode, "02W200", 6)) {
        DEL_KEY(pstTransJson, "trans_code");
        SET_STR_KEY(pstTransJson, "trans_code", "02W600");
    }*/

    /* 支付撤销  被扫支付与主扫支付 交易金额 还没有入钱包的（SETTLE_FLAG = ‘N’）  暂不允许撤销*/
    tLog(DEBUG, "sSettleFlag = [%s]", stQrcodeTransDetail.sSettleFlag);
    if (stQrcodeTransDetail.sSettleFlag[0] == 'N') {
        /*Z1 交易失败，请稍后重试*/
        ErrHanding(pstTransJson, "Z1", "现交易金额未充入钱包暂不允许撤销，请稍后重试，原交易rrn[%s].", sORrn);
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