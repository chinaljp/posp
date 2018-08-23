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
        /*begin MOTIFY by GJQ at 20180613 */
        char sICFlag[1 + 1]; /*IC���ǿ�*/
        char sIDFlag[1 + 1]; /*IC��ǿ�*/
        char sMCFlag[1 + 1];/*�������ǿ�*/
        char sMDFlag[1 + 1];/*������ǿ�*/
        /*end MOTIFY by GJQ at 20180613 */
        double dDPLimit; /* IC��ǿ������޶����*/
        double dWXLimit; /* ΢���޶����*/
        double dBBLimit; /* ֧�������������޶����*/
        double dDDLimit; /* IC��ǿ������޶����*/
        double dDWXLimit; /* ΢�ŵ����޶����*/
        double dDBBLimit; /* ֧�������������޶����*/
        double dDMLimit; /* IC��ǿ������޶����*/
        double dMWXLimit; /* ΢�ŵ����޶����*/
        double dMBBLimit; /* ֧�������������޶����*/
        double dCPLimit; /* IC���ÿ������޶����*/
        double dCDLimit; /* IC���ÿ������޶����*/
        double dCMLimit; /* IC���ÿ������޶����*/
        double dMDPLimit; /* ������ǿ������޶����*/
        double dMDDLimit; /* ������ǿ������޶����*/
        double dMDMLimit; /* ������ǿ������޶����*/
        double dMCPLimit; /* �������ÿ������޶����*/
        double dMCDLimit; /* �������ÿ������޶����*/
        double dMCMLimit; /* �������ÿ������޶����*/
        double dQDPLimit; /* ��ǿ�����޿������޶� */
        double dQCPLimit; /* ���ÿ�����޿������޶� */
        double dQDDLimit; /* ��ǿ�����޿������޶� */
        double dQCDLimit; /* ���ÿ�����޿������޶� */
        double dQDMLimit; /* ��ǿ�����޿������޶� */
        double dQCMLimit; /* ���ÿ�����޿������޶� */
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

