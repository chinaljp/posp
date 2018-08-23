/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   limitrule.h
 * Author: Pay
 *
 * Created on 2017年3月31日, 下午8:07
 */

#ifndef LIMITRULE_H
#define LIMITRULE_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char sId[32 + 1];
        char sUserCode[15 + 1]; /*              */
        double dDPLimit; /* 借记卡单笔限额，整数*/
        double dWXLimit; /* 微信限额，整数*/
        double dBBLimit; /* 支付宝单笔消费限额，整数*/
        double dDDLimit; /* 借记卡单日限额，整数*/
        double dDWXLimit; /* 微信单日限额，整数*/
        double dDBBLimit; /* 支付宝单日消费限额，整数*/
        double dDMLimit; /* 借记卡单月限额，整数*/
        double dMWXLimit; /* 微信单月限额，整数*/
        double dMBBLimit; /* 支付宝单月消费限额，整数*/
        double dCPLimit; /* 信用卡单笔限额，整数*/
        double dCDLimit; /* 信用卡单日限额，整数*/
        double dCMLimit; /* 信用卡单月限额，整数*/
        char sATransCode[512]; /*生效的交易码*/
    } LimitRule;

    typedef struct {
        char sMerchId[15 + 1]; /*  商户号   */
        char sMerchName[100 + 1]; /*  商户名   */
        double dTotallimit; /*  T0总额度   */
        double dUsedlimit; /*  T0已用额度 */
        double dUsable_limit; /*  T0可用额度 */
        char sATransCode[512]; /*生效的交易码*/
        char sLocalMerch[15 + 1]; /*  本地商户号   */
        double dLocalTotallimit; /*  T0总额度   */
        double dLocalUsedlimit; /*  T0已用额度 */
        int iLocalTransCnt; /* 本地商户交易笔数 */
        char sLocalGroupId[8 + 1]; /* 本地商户组id */
        double dLocalPassAmt; /* 本地商户起跳金额 */
        char sCardType[1 + 1]; /* 允许交易卡类型 */
        char sChannelId[8 + 1]; /* 渠道id */
        char sMerchType[1 + 1]; /* 商户类型 */
    } PufaLimitRule;



#ifdef __cplusplus
}
#endif

#endif /* LIMITRULE_H */

