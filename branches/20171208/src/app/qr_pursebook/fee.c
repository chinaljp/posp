/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "merch.h"
#include "t_macro.h"

int IsSaleMcc(char *pcMcc) {
    char sSqlStr[512] = {0}, sValue[128] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select consume_type from s_mcc where mcc='%s'", pcMcc);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找mcc[%s]是否日常消费mcc.", pcMcc);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sValue);
        tTrim(sValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无Mcc[%s]的记录.", pcMcc);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return atoi(sValue);
}

int CalcMerchFeeNfc(char *pcMcc,double dAmount) {
    
    char sMcc[4 + 1] = {0}, sTmp[12+1] = {0};
    double dNfcAmount = 0.00;
    
    strcpy(sMcc,pcMcc);
    /* 
     * 云闪付计算
     *  NFC移动支付
     */

    /* 判断限額 */
    if (FindValueByKey(sTmp, "yc.nfc.amount") < 0) {
        tLog(ERROR, "查找key[yc.nfc.amount]云闪付nfc限额,失败.");
        return -1;
    }
    dNfcAmount = atof(sTmp);
    /* 日常消费mcc */
    /* 交易金额 */
    if (DBL_CMP(dNfcAmount, dAmount) && 1 == IsSaleMcc(sMcc)) { 
        return 1;
    }
    
    return 0;
}

/* 判断手续费类型 */
int ChkMerchFeeType(char *pcMerchType,char *pcMcc,double dAmount,char *pcFeeType) {
    
    /* nfc */
    if ( CalcMerchFeeNfc(pcMcc,dAmount) > 0 ) {
        strcpy(pcFeeType,"YN");
        return 0;
    }
    /* MERCH_TYPE 商户类型: 0:标准，1:优惠；2:减免 */
    if (pcMerchType[0] == '1') {
        strcpy(pcFeeType,"Y");
    } else if (pcMerchType[0] == '2') {
        strcpy(pcFeeType,"M");
    } else {
        strcpy(pcFeeType,"B");
    }
    
    return 0;
}

/*银联二维码异步通知中计算手续续费*/
int MerchUpFee( char *pcMerchId,char *pcOrderNo,char *pcCardType,char *pcAmount ) {
    char sMerchType[2 + 1] = {0},sMcc[4 + 1] = {0},sFeeType[2 + 1] = {0};
    char sMerchId[15 + 1] = {0},sOrderNo[32 + 1] = {0},sCardType[2 + 1] = {0},sFeeDesc[255 + 1] = {0};
    double dAmount = 0.00;
    MerchFee stMerchFee;
    double dFee = 0.0, dRate = 0.0L, dMax = 0;
    
    strcpy(sMerchId,pcMerchId);
    strcpy(sOrderNo,pcOrderNo);
    strcpy(sCardType,pcCardType);
    dAmount = atof(pcAmount);
    dAmount = dAmount / 100;
    
    /*获取商户的sMerchType、sMcc*/
    if( FindMerchMsg(sMerchId,sMerchType,sMcc) < 0 ) {
        tLog(ERROR,"获取商户[%s]的类型、MCC失败!",sMerchId);
        return ( -1 );
    }
    tLog(DEBUG,"获取到sMerchType = [%s],sMcc = [%s]",sMerchType,sMcc);
    
    /*判断手续费类型*/
    ChkMerchFeeType(sMerchType,sMcc,dAmount,sFeeType);
    
    /*计算手续费*/
    if (FindMerchFee(&stMerchFee, sMerchId) < 0) {
        tLog(ERROR, "未找到商户[%s]的费率资料.", sMerchId);
        return -1;
    }
    if (!memcmp(sFeeType, "YN", 2)) {
        tLog(INFO, "交易使用云闪付NFC计算.");
        /* 
         * 云闪付计算
         *  NFC移动支付
         */
        if (sCardType[0] == '0') {
            tLog(DEBUG, "借记卡[%f].", stMerchFee.dYcNfcDRate);
            dRate = stMerchFee.dYcNfcDRate / 100;
            sprintf(sFeeDesc, "%f", dRate);
        } else {
            tLog(DEBUG, "信用卡[%f].", stMerchFee.dYcNfcCRate);
            dRate = stMerchFee.dYcNfcCRate / 100;
            sprintf(sFeeDesc, "%f", dRate);
        }
    } 
    else {
        tLog(INFO, "交易使用一般费率[%s]计算.", sFeeType);
        /* 一般费率计算 */
        if (sCardType[0] == '0') {
            tLog(DEBUG, "借记卡[%f,%f].", stMerchFee.dDRate, stMerchFee.dDMax);
            dRate = stMerchFee.dDRate / 100;
            dMax = stMerchFee.dDMax;
            sprintf(sFeeDesc, "%f,%.0f", dRate, dMax);
        } else {
            tLog(DEBUG, "信用卡[%f,%f].", stMerchFee.dCRate, stMerchFee.dCMax);
            dRate = stMerchFee.dCRate / 100;
            dMax = stMerchFee.dCMax;
            sprintf(sFeeDesc, "%f,%f", dRate, dMax);
        }
    }
    dFee = dAmount * dRate;
    if (DBL_CMP(dMax, 0)
            && DBL_CMP(dFee, dMax)) {
        dFee = dMax;
    }
    tMake(&dFee, 2, 'u');
    
    /*更新手续费到交易流水记录中*/
    tTrim(sFeeType);
    if( UpFeeMsg(sOrderNo,sFeeType,dFee,sFeeDesc) < 0) {
        tLog(ERROR,"更新商户[%s]的订单[%s]的手续费信息失败！",sMerchId,sOrderNo);
        return ( -1 );
    }
    
    return ( 0 );
}