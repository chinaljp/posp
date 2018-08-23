/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   qrcodetransdetail.h
 * Author: Administrator
 *
 * Created on 2017年8月17日, 下午8:10
 */

#ifndef QRCODETRANSDETAIL_H
#define QRCODETRANSDETAIL_H

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct {
        char sId[32+1];
        char sTransCode[6 + 1]; /* 交易代码             */
        double dAmount; /* 交易发生额           */
        char sSettleDate[8 + 1]; /* 清算日期             */
        char sRrn[12 + 1]; /* 系统参考号           */
        char sMcc[4 + 1]; /* 业态代码             */
        char sAgentId[8 + 1]; /* 代理机构标识         */
        char sChannelMerchId[15 + 1]; /* 渠道商户编号         */
        char sValidFlag[1+1]; /* 有效标志             */
        char sCheckFlag[1+1]; /* 对帐标志             */
        char sSettleFlag[1+1]; /*清算标志，sSettleFlag = 'N' 时， 金额未充入钱包，不允许撤销*/
        char sBatchNo[6 + 1]; /* 批次号               */
        double dFee;        /* 交易手续费               */
        char sFeeFlag[1+1]; /* 手续费计算标志           */
        char sFeeDesc[50]; /*商户手续费描述             */
        char sFeeType[2+1]; /*手续费类型*/
        char sMerchOrderNo[32+1];/* 原交易商户订单号 */
        char sOrderUrl[255];/*被扫支付二维码URL*/
        char sUserCode[15+1];
} QrcodeTransDetail;


#ifdef __cplusplus
}
#endif

#endif /* QRCODETRANSDETAIL_H */

