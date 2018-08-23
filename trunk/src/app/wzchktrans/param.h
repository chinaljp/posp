/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   param.h
 * Author: Administrator
 *
 * Created on 2017年3月28日, 下午2:23
 */

#ifndef PARAM_H
#define PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

    int InitParam(int iArgc, char* pcArgv[]);
    char *GetLogName();
    char *GetSvrName();
    char *GetLibName();
    int GetAppNum();

    typedef struct {
        char sTansTime[50];
        char sAccountId[50];
        char sThMerchNo[50];
        char sMerchNo[50];
        char sSubMerchNo[50];
        char sTermId[50];
        char sWftOrderNo[50];
        char sThOrderNo[50];
        char sMerchOrderNo[50];
        char sUserId[50];
        char sTransType[50];
        char sTranStatus[50];
        char sPayBank[50];
        char sAmtType[50];
        double dDmount;
        double dRepakAmt;
        char sRefundNo[60];
        char sMerchRefundNo[50];
        double dRefundAmt;
        double dRepakRefundAmt;
        char sRefundType[50];
        char sRefundStatus[50];
        char sCommName[200];
        char sMerchData[200];
        double dFee;
        double dRate;
        char sTermType[50];
        char sChkMark[20];
        char sNumber[50];
        char sMerchName[50];
        char sMerchId[50];
        char sSubMerchId[50];
        char sExtend1[100];

    } ZxSettleTransDetail;

#ifdef __cplusplus
}
#endif

#endif /* PARAM_H */

