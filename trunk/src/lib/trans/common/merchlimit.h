/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   merchlimit.h
 * Author: Pay
 *
 * Created on 2017��3��31��, ����8:23
 */

#ifndef MERCHLIMIT_H
#define MERCHLIMIT_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char sId[32 + 1];
        char sUserCode[15 + 1]; /*              */
        double dDDLimit; /* IC��ǿ������ۼƶ�*/
        double dDWXLimit; /* ΢�ŵ����ۼ�*/
        double dDBBLimit; /* ֧���������ۼ�*/
        double dDMLimit; /* IC��ǿ������ۼƶ�*/
        double dMWXLimit; /* ΢�ŵ����ۼƶ�*/
        double dMBBLimit; /* ֧���������ۼƶ�*/
        double dCDLimit; /* IC���ÿ������ۼƶ�*/
        double dCMLimit;/* IC���ÿ������ۼƶ�*/
        double dMDDLimit; /* ������ǿ������ۼƶ�*/
        double dMDMLimit; /* ������ǿ������ۼƶ�*/
        double dMCDLimit; /* �������ÿ������ۼƶ�*/
        double dMCMLimit;/*  �������ÿ������ۼƶ�*/
        double dQDDLimit; /* ��ǿ�����޿������ۼƶ�*/
        double dQDMLimit; /* ��ǿ�����޿������ۼƶ�*/
        double dQCDLimit; /* ���ÿ�����޿������ۼƶ�*/
        double dQCMLimit;/*  ���ÿ�����޿������ۼƶ�*/
    } MerchLimit;

#ifdef __cplusplus
}
#endif

#endif /* MERCHLIMIT_H */

