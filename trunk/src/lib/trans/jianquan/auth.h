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
   
#define CARDID_LEN 4

    typedef struct {
        char sCardId[CARDID_LEN + 1];
        char cCardType;
        char sCardName[20 + 1];
        char sIssBankId[8 + 1];
        char sCardBin[19 + 1];
        long lCardBinLen;
        long lCardNoLen;
    } CardBin;

#ifdef __cplusplus
}
#endif

#endif /* CARD_H */

