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
        tLog(ERROR, "����mcc[%s]�Ƿ��ճ�����mcc.", pcMcc);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sValue);
        tTrim(sValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "��Mcc[%s]�ļ�¼.", pcMcc);
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
     * ����������
     *  NFC�ƶ�֧��
     */

    /* �ж����~ */
    if (FindValueByKey(sTmp, "yc.nfc.amount") < 0) {
        tLog(ERROR, "����key[yc.nfc.amount]������nfc�޶�,ʧ��.");
        return -1;
    }
    dNfcAmount = atof(sTmp);
    /* �ճ�����mcc */
    /* ���׽�� */
    if (DBL_CMP(dNfcAmount, dAmount) && 1 == IsSaleMcc(sMcc)) { 
        return 1;
    }
    
    return 0;
}

/* �ж����������� */
int ChkMerchFeeType(char *pcMerchType,char *pcMcc,double dAmount,char *pcFeeType) {
    
    /* nfc */
    if ( CalcMerchFeeNfc(pcMcc,dAmount) > 0 ) {
        strcpy(pcFeeType,"YN");
        return 0;
    }
    /* MERCH_TYPE �̻�����: 0:��׼��1:�Żݣ�2:���� */
    if (pcMerchType[0] == '1') {
        strcpy(pcFeeType,"Y");
    } else if (pcMerchType[0] == '2') {
        strcpy(pcFeeType,"M");
    } else {
        strcpy(pcFeeType,"B");
    }
    
    return 0;
}

/*������ά���첽֪ͨ�м�����������*/
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
    
    /*��ȡ�̻���sMerchType��sMcc*/
    if( FindMerchMsg(sMerchId,sMerchType,sMcc) < 0 ) {
        tLog(ERROR,"��ȡ�̻�[%s]�����͡�MCCʧ��!",sMerchId);
        return ( -1 );
    }
    tLog(DEBUG,"��ȡ��sMerchType = [%s],sMcc = [%s]",sMerchType,sMcc);
    
    /*�ж�����������*/
    ChkMerchFeeType(sMerchType,sMcc,dAmount,sFeeType);
    
    /*����������*/
    if (FindMerchFee(&stMerchFee, sMerchId) < 0) {
        tLog(ERROR, "δ�ҵ��̻�[%s]�ķ�������.", sMerchId);
        return -1;
    }
    if (!memcmp(sFeeType, "YN", 2)) {
        tLog(INFO, "����ʹ��������NFC����.");
        /* 
         * ����������
         *  NFC�ƶ�֧��
         */
        if (sCardType[0] == '0') {
            tLog(DEBUG, "��ǿ�[%f].", stMerchFee.dYcNfcDRate);
            dRate = stMerchFee.dYcNfcDRate / 100;
            sprintf(sFeeDesc, "%f", dRate);
        } else {
            tLog(DEBUG, "���ÿ�[%f].", stMerchFee.dYcNfcCRate);
            dRate = stMerchFee.dYcNfcCRate / 100;
            sprintf(sFeeDesc, "%f", dRate);
        }
    } 
    else {
        tLog(INFO, "����ʹ��һ�����[%s]����.", sFeeType);
        /* һ����ʼ��� */
        if (sCardType[0] == '0') {
            tLog(DEBUG, "��ǿ�[%f,%f].", stMerchFee.dDRate, stMerchFee.dDMax);
            dRate = stMerchFee.dDRate / 100;
            dMax = stMerchFee.dDMax;
            sprintf(sFeeDesc, "%f,%.0f", dRate, dMax);
        } else {
            tLog(DEBUG, "���ÿ�[%f,%f].", stMerchFee.dCRate, stMerchFee.dCMax);
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
    
    /*���������ѵ�������ˮ��¼��*/
    tTrim(sFeeType);
    if( UpFeeMsg(sOrderNo,sFeeType,dFee,sFeeDesc) < 0) {
        tLog(ERROR,"�����̻�[%s]�Ķ���[%s]����������Ϣʧ�ܣ�",sMerchId,sOrderNo);
        return ( -1 );
    }
    
    return ( 0 );
}