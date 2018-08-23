/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   card.h
 * Author: Pay
 *
 * Created on 2017年3月8日, 上午11:11
 */

#ifndef CARD_H
#define CARD_H

#ifdef __cplusplus
extern "C" {
#endif
#define CARDID_LEN 10

    typedef struct {
        char sCardId[CARDID_LEN + 1];
        char sCardType[1 + 1];
        char sCardName[20 + 1];
        char sIssBankId[8 + 1];
        char sCardBin[19 + 1];
        long lCardBinLen;
        long lCardNoLen;
    } CardBin;

/* 商户卡包中卡信息 */
    typedef struct  {
        char sCardNoE[255+1];         /*交易卡号*/
        char sCardHolderNameE[32+1]; /* 持卡人姓名 */
        char sMobileNoE[32+1];       /* 持卡人手机号  */
        char sCardCvnNoE[32+1];       /* 银行卡安全码 */
        char sCardExpDateE[32+1];     /* 银行卡有效期 */
        char sKey[32+1];               /*加密使用的KEY*/
    } CardMsg;

#ifdef __cplusplus
}
#endif

#endif /* CARD_H */

