/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   qrcodetransdetail.h
 * Author: Administrator
 *
 * Created on 2017��8��17��, ����8:10
 */

#ifndef QRCODETRANSDETAIL_H
#define QRCODETRANSDETAIL_H

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct {
        char sId[32+1];
        char sTransCode[6 + 1]; /* ���״���             */
        double dAmount; /* ���׷�����           */
        char sSettleDate[8 + 1]; /* ��������             */
        char sRrn[12 + 1]; /* ϵͳ�ο���           */
        char sMcc[4 + 1]; /* ҵ̬����             */
        char sAgentId[8 + 1]; /* ���������ʶ         */
        char sChannelMerchId[15 + 1]; /* �����̻����         */
        char sValidFlag[1+1]; /* ��Ч��־             */
        char sCheckFlag[1+1]; /* ���ʱ�־             */
        char sSettleFlag[1+1]; /*�����־��sSettleFlag = 'N' ʱ�� ���δ����Ǯ������������*/
        char sBatchNo[6 + 1]; /* ���κ�               */
        double dFee;        /* ����������               */
        char sFeeFlag[1+1]; /* �����Ѽ����־           */
        char sFeeDesc[50]; /*�̻�����������             */
        char sFeeType[2+1]; /*����������*/
        char sMerchOrderNo[32+1];/* ԭ�����̻������� */
        char sOrderUrl[255];/*��ɨ֧����ά��URL*/
        char sUserCode[15+1];
} QrcodeTransDetail;


#ifdef __cplusplus
}
#endif

#endif /* QRCODETRANSDETAIL_H */

