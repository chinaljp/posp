/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   merch.h
 * Author: feng.gaoo
 *
 * Created on 2017年2月28日, 下午4:00
 */

#ifndef MERCH_H
#define MERCH_H

#ifdef __cplusplus
extern "C" {
#endif
#include "t_cjson.h"

    typedef struct {
        char sUserCode[15 + 1];
        char sMerchId[16];
        char sAgentId[11 + 1];
        char sMcc[4 + 1];
        char sCName[255];
        char sPName[255];
        char sStatus[2];
        char sOpenDate[7];
        char sExpireDate[7];
        char sMerchType[2];
    } Merch;

    typedef struct {
        char sUserCode[15 + 1];
        char sMerchId[16];
        double dCRate;
        double dCMax;
        double dDRate;
        double dDMax;
        double dMin;
        double dWxRate;
        double dWxMax;
        double dBbRate;
        double dBbMax;
        double dYc2FeeCRate;
        double dYc2FeeDRate;
        double dYcNfcCRate;
        double dYcNfcDRate;
        double dYcQrCRate;
        double dYcQrDRate;
    } MerchFee;


    int FindMerchInfoById(Merch *pstMerch, char *pcMerchId);
#ifdef __cplusplus
}
#endif

#endif /* MERCH_H */

