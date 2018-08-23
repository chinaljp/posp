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
        double dDDLimit; /* IC借记卡单日累计额*/
        double dDWXLimit; /* 微信单日累计*/
        double dDBBLimit; /* 支付宝单日累计*/
        double dDMLimit; /* IC借记卡单月累计额*/
        double dMWXLimit; /* 微信单月累计额*/
        double dMBBLimit; /* 支付宝单月累计额*/
        double dCDLimit; /* IC信用卡单日累计额*/
        double dCMLimit;/* IC信用卡单月累计额*/
        double dMDDLimit; /* 磁条借记卡单日累计额*/
        double dMDMLimit; /* 磁条借记卡单月累计额*/
        double dMCDLimit; /* 磁条信用卡单日累计额*/
        double dMCMLimit;/*  磁条信用卡单月累计额*/
        double dQDDLimit; /* 借记卡快捷无卡单日累计额*/
        double dQDMLimit; /* 借记卡快捷无卡单月累计额*/
        double dQCDLimit; /* 信用卡快捷无卡单日累计额*/
        double dQCMLimit;/*  信用卡快捷无卡单月累计额*/
    } MerchLimit;

#ifdef __cplusplus
}
#endif

#endif /* MERCHLIMIT_H */

