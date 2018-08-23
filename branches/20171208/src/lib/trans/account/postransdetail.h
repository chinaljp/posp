/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   postransdetail.h
 * Author: Pay
 *
 * Created on 2017年3月8日, 下午4:42
 */

#ifndef POSTRANSDETAIL_H
#define POSTRANSDETAIL_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char sId[32 + 1];
        char sTransCode[6 + 1]; /* 交易代码             */
        char sCardNo[19 + 1]; /* 卡号1                */
        char sCardId[4 + 1]; /* 卡号卡类            */
        char sCardType[1 + 1]; /* 卡号1类型,借记/信用  */
        char sCardExpDate[4 + 1]; /* 卡号1有效期          */
        double dAmount; /* 交易发生额           */
        char sTransmitTime[10 + 1]; /* 发起方交易时间       */
        char sTransDate[8 + 1]; /* 交易日期             */
        char sTransTime[6 + 1]; /* 交易时间             */
        char sSysTrace[6 + 1]; /* 收单系统流水号       */
        char sLogicDate[8 + 1]; /* 收单系统逻辑日期     */
        char sSettleDate[8 + 1]; /* 清算日期             */
        char sMerchId[15 + 1]; /* 商户编号             */
        char sTermId[8 + 1]; /* 终端号               */
        char sTermSn[50]; /* 终端序列号 */
        char sTraceNo[6 + 1]; /* 终端流水号           */
        char sRrn[12 + 1]; /* 系统参考号           */
        char sPufaRrn[12 + 1]; /* 浦发系统参考号           */
        char sMcc[4 + 1]; /* 业态代码             */
        char sInputMode[3 + 1]; /* 服务点输入方式码     */
        char sCondCode[2 + 1]; /* 服务点条件码         */
        char sPinCode[2 + 1]; /* 服务点pin获取码      */
        char sAcqId[8 + 1]; /* 收单机构标识         */
        char sIssId[8 + 1]; /* 发卡机构标识         */
        char sAgentId[8 + 1]; /* 代理机构标识         */
        char sChannelId[8 + 1]; /* 渠道标识             */
        char sChannelMerchId[15 + 1]; /* 渠道商户编号         */
        char sChannelTermId[8 + 1]; /* 渠道终端号           */
        char sChannelTermSn[50 + 1]; /* 渠道终端号           */
        char sChannelMcc[4 + 1]; /* 渠道业态代码         */
        char sTransType[1 + 1]; /* 交易类型             */
        char sValidFlag[1 + 1]; /* 有效标志             */
        char sCheckFlag[1 + 1]; /* 对帐标志             */
        char sSettleFlag[1 + 1]; /* 清算标志             */
        char sAuthoFlag[1 + 1]; /* 预授权完成标志       */
        char sBatchNo[6 + 1]; /* 批次号               */
        char sAuthCode[6 + 1]; /* 授权码               */
        double dRefundAmt; /* 已退货金额               */
        double dFee; /* 交易手续费               */
        char sFeeFlag[1 + 1]; /* 手续费计算标志           */
        char sFeeDesc[50]; /*商户手续费描述             */
        char sRespCode[3 + 1]; /* 应答码               */
        char sIstRespCode[3 + 1]; /* 银联应答码               */
        char sRespDesc[200]; /*应答描述*/
        char sRespId[8 + 1]; /* 应答机构             */
        char sSignFlag[1 + 1]; /*电子签名标识*/
        char sMerchOrderNo[100]; /*订单号*/
        char sORrn[12 + 1]; /* 原系统参考号         */
        char sOTransDate[8 + 1]; /*原交易日期*/
        char sSettleOrderId[20]; /*结算单号*/
        char sFeeType[2 + 1]; /*手续费类型*/
        char sMerchPName[255 + 1];
        char sChannelMerchName[255 + 1];
        char sBusiType[2 + 1];
    } PosTransDetail;


#ifdef __cplusplus
}
#endif

#endif /* POSTRANSDETAIL_H */

