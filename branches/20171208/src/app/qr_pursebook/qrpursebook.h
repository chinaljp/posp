/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   qrpursebook.h
 * Author: Administrator
 *
 * Created on 2017年12月25日, 下午5:15
 */

#ifndef QRPURSEBOOK_H
#define QRPURSEBOOK_H

#ifdef __cplusplus
extern "C" {
#endif
int FindMerchId( char *pcMerchId,char *pcOrderNo );
int FindMerchMsg( char *pcMerchId,char *pcMerchType,char *pcMcc );
int UpFeeMsg(char *pcOrderNo,char *pcFeeType,double dFee,char *pcFeeDesc);
int UpCupsSettleKey(char *pcCardType, char *pcSettleSysTrace, char *pcSettleTransTime, char *pcOrderNo);
int UpValidFlag( char *pcOrderNo );
int DealRespMsg(char *pcRespMsg);
int GoBackMsg(char *pcBackFlag);
int g_iQutLoop;



#ifdef __cplusplus
}
#endif

#endif /* QRPURSEBOOK_H */

