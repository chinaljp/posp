/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   saf_trans_detail.h
 * Author: feng.gaoo
 *
 * Created on 2017年3月7日, 下午1:54
 */

#ifndef SAF_TRANS_DETAIL_H
#define SAF_TRANS_DETAIL_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char    sTransCode[6+1];
    char    sChannelId[8+1];
    char    sSettleDate[8+1];
    char    sSysTrace[6+1];
    char    sTransDate[8+1];
    char    sTransTime[6+1];
    char    sTransmitTime[10+1];
    char    sCard1No[19+1];
    char    cCardType;
    double  dAmount;
    double  dFee;
    char    sMsgId[4+1];
    char    sProCode[6+1];
    char    sChannelMcc[4+1];
    char    sChannelMerchId[15+1];
    char    sChannelTermId[8+1];
    char    sChannelRrn[12+1];
    char    sAuthCode[6+1];
    char    sRespInstId[8+1];
    char    sOrgSysTrace[6+1];
    char    sRespCode[2+1];
    char    sInputMode[3+1];
    double  dIssFee;
    double  dChannelFee;
    double  dChannelFee2;
    char    sOrgTransmitTime[10+1];
    char    sCondCode[2+1];
    char    cTranArea;
    char    sIssInstId[8+1];
    char    sPayType[4+1];
    char    cChkFlag;
    char    cAdjustFlag;
    char    sRemark[100+1];
}ChannelSettleTransDetail;
#ifdef __cplusplus
}
#endif

#endif /* SAF_TRANS_DETAIL_H */

