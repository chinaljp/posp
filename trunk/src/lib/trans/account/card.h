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

/* 同卡连刷规则 */
    typedef struct  {
    long    lCnt;                    /* 交易次数 */
    double  dAmount;                /* 金额  */
    long    lQueryCnt;               /* 商户查余次数 */
    long    lQueryCardCnt;           /* 商户同卡查余次数 */
    long    lQUERYNMINTRANS;           /* 同卡查余后几分钟内做交易 */
    long    lQUERYNTRANS;               /* 同卡查余几次后做交易 */
    } SameCardRule;

#ifdef __cplusplus
}
#endif

#endif /* CARD_H */

