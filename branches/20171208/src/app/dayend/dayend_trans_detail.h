/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dayend_trans_detail.h
 * Author: Administrator
 *
 * Created on 2017年3月24日, 下午4:35
 */

#ifndef DAYEND_TRANS_DETAIL_H
#define DAYEND_TRANS_DETAIL_H

#ifdef __cplusplus
extern "C" {
#endif


#define S_NORMAL  '0'  /* 正常				*/
#define S_CUTOFF  '1'  /* 日切				*/    
#define S_CLEAR   '8'  /* 清理流水 */
#define S_LIMIT   '2'  /* 限额			*/ 
#define S_RECOVLIMIT  '3'  /* 恢复商户D0额度 */

#if 0
    typedef struct {
        char sId[32 + 1] = {0};
        char sTransCode[6 + 1] = {0};
        char sCardNo[19 + 1] = {0};
        char sCardId[4 + 1] = {0};
        char sCardType[1 + 1] = {0};
        char sCardExpDate[4 + 1] = {0};
        double dAmount;
        char sTransmitTime[10 + 1] = {0};
        char sTransDate[8 + 1] = {0};
        char sTransTime[16 + 1] = {0};
        char sSystrace[6 + 1] = {0};
        char sLogicDate[8 + 1] = {0};
        char sSettleDate[8 + 1] = {0};
        char sMerchId[15 + 1] = {0};
        char sTermId[8 + 1] = {0};
        char sTermSn[50 + 1] = {0};
        char sTraceNo[6 + 1] = {0};
        char sRrn[12 + 1] = {0};
        char sMcc[4 + 1] = {0};
        char sAcqId[11 + 1] = {0};
        char sAgentId[11 + 1] = {0};
        char sChannelId[11 + 1] = {0};
        char sChannelMerchId[15 + 1] = {0};
        char sChannelTermId[8 + 1] = {0};
        char sChannelTermSn[50 + 1] = {0};
        char sChannelMcc[4 + 1] = {0};
        char sTransType[3 + 1] = {0};
        char sValidFlag[1 + 1] = {0};
        char sCheckFlag[1 + 1] = {0};
        char sSettleFlag[1 + 1] = {0};
        char sBatchNo[6 + 1] = {0};
        double dRefundAmt;
        double dFee;
        char sFeeFlag[1 + 1] = {0};
        char sFeeDesc[50 + 1] = {0};
        char sRespCode[2 + 1] = {0};
        char sIstrespCode[3 + 1] = {0};
        char sRespDesc[200 + 1] = {0};
        char sRespId[8 + 1] = {0};
        char sMerchOrderNo[100 + 1] = {0};
        char sOrderNo[100 + 1] = {0};
        char sOrderUrl[255 + 1] = {0};
        char sMobile[20 + 1] = {0};
        char sAccountName[100 + 1] = {0};
        char sCertNo[50 + 1] = {0};
        char SettleOrderId[20 + 1] = {0};
        char sORrn[12 + 1] = {0};
        char sOTransDate[10 + 1] = {0};
        char sCreateTime[15 + 1] = {0};
        char sLastModTimeid[15 + 1] = {0};

    } POSTRANS;
#endif
#ifdef __cplusplus
}
#endif

#endif /* DAYEND_TRANS_DETAIL_H */

