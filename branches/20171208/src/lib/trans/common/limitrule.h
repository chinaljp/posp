/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   limitrule.h
 * Author: Pay
 *
 * Created on 2017��3��31��, ����8:07
 */

#ifndef LIMITRULE_H
#define LIMITRULE_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char sId[32 + 1];
        char sUserCode[15 + 1]; /*              */
        double dDPLimit; /* ��ǿ������޶����*/
        double dWXLimit; /* ΢���޶����*/
        double dBBLimit; /* ֧�������������޶����*/
        double dDDLimit; /* ��ǿ������޶����*/
        double dDWXLimit; /* ΢�ŵ����޶����*/
        double dDBBLimit; /* ֧�������������޶����*/
        double dDMLimit; /* ��ǿ������޶����*/
        double dMWXLimit; /* ΢�ŵ����޶����*/
        double dMBBLimit; /* ֧�������������޶����*/
        double dCPLimit; /* ���ÿ������޶����*/
        double dCDLimit; /* ���ÿ������޶����*/
        double dCMLimit; /* ���ÿ������޶����*/
        char sATransCode[512]; /*��Ч�Ľ�����*/
    } LimitRule;

    typedef struct {
        char sMerchId[15 + 1]; /*  �̻���   */
        char sMerchName[100 + 1]; /*  �̻���   */
        double dTotallimit; /*  T0�ܶ��   */
        double dUsedlimit; /*  T0���ö�� */
        double dUsable_limit; /*  T0���ö�� */
        char sATransCode[512]; /*��Ч�Ľ�����*/
        char sLocalMerch[15 + 1]; /*  �����̻���   */
        double dLocalTotallimit; /*  T0�ܶ��   */
        double dLocalUsedlimit; /*  T0���ö�� */
        int iLocalTransCnt; /* �����̻����ױ��� */
        char sLocalGroupId[8 + 1]; /* �����̻���id */
        double dLocalPassAmt; /* �����̻�������� */
        char sCardType[1 + 1]; /* �����׿����� */
        char sChannelId[8 + 1]; /* ����id */
        char sMerchType[1 + 1]; /* �̻����� */
    } PufaLimitRule;



#ifdef __cplusplus
}
#endif

#endif /* LIMITRULE_H */

