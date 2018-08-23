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
        /*begin MOTIFY by GJQ at 20180613 */
        char sICFlag[1 + 1]; /*IC贷记卡*/
        char sIDFlag[1 + 1]; /*IC借记卡*/
        char sMCFlag[1 + 1];/*磁条贷记卡*/
        char sMDFlag[1 + 1];/*磁条借记卡*/
        /*end MOTIFY by GJQ at 20180613 */
        double dDPLimit; /* IC借记卡单笔限额，整数*/
        double dWXLimit; /* 微信限额，整数*/
        double dBBLimit; /* 支付宝单笔消费限额，整数*/
        double dDDLimit; /* IC借记卡单日限额，整数*/
        double dDWXLimit; /* 微信单日限额，整数*/
        double dDBBLimit; /* 支付宝单日消费限额，整数*/
        double dDMLimit; /* IC借记卡单月限额，整数*/
        double dMWXLimit; /* 微信单月限额，整数*/
        double dMBBLimit; /* 支付宝单月消费限额，整数*/
        double dCPLimit; /* IC信用卡单笔限额，整数*/
        double dCDLimit; /* IC信用卡单日限额，整数*/
        double dCMLimit; /* IC信用卡单月限额，整数*/
        double dMDPLimit; /* 磁条借记卡单笔限额，整数*/
        double dMDDLimit; /* 磁条借记卡单日限额，整数*/
        double dMDMLimit; /* 磁条借记卡单月限额，整数*/
        double dMCPLimit; /* 磁条信用卡单笔限额，整数*/
        double dMCDLimit; /* 磁条信用卡单日限额，整数*/
        double dMCMLimit; /* 磁条信用卡单月限额，整数*/
        double dQDPLimit; /* 借记卡快捷无卡单笔限额 */
        double dQCPLimit; /* 信用卡快捷无卡单笔限额 */
        double dQDDLimit; /* 借记卡快捷无卡单日限额 */
        double dQCDLimit; /* 信用卡快捷无卡单日限额 */
        double dQDMLimit; /* 借记卡快捷无卡单月限额 */
        double dQCMLimit; /* 信用卡快捷无卡单月限额 */
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

