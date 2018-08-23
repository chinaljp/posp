/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   merchlimit.h
 * Author: Pay
 *
 * Created on 2017年3月31日, 下午8:23
 */

#ifndef MERCHLIMIT_H
#define MERCHLIMIT_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char sId[32 + 1];
        char sUserCode[15 + 1]; /*              */
        double dDDLimit; /* 借记卡单日累计额，整数*/
        double dDWXLimit; /* 微信单日累计，整数*/
        double dDBBLimit; /* 支付宝单日累计，整数*/
        double dDMLimit; /* 借记卡单月累计额，整数*/
        double dMWXLimit; /* 微信单月累计额，整数*/
        double dMBBLimit; /* 支付宝单月累计额，整数*/
        double dCDLimit; /* 信用卡单日累计额，整数*/
        double dCMLimit;/* 信用卡单月累计额，整数*/
    } MerchLimit;

#ifdef __cplusplus
}
#endif

#endif /* MERCHLIMIT_H */

