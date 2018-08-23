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
        double dDDLimit; /* ��ǿ������ۼƶ����*/
        double dDWXLimit; /* ΢�ŵ����ۼƣ�����*/
        double dDBBLimit; /* ֧���������ۼƣ�����*/
        double dDMLimit; /* ��ǿ������ۼƶ����*/
        double dMWXLimit; /* ΢�ŵ����ۼƶ����*/
        double dMBBLimit; /* ֧���������ۼƶ����*/
        double dCDLimit; /* ���ÿ������ۼƶ����*/
        double dCMLimit;/* ���ÿ������ۼƶ����*/
    } MerchLimit;

#ifdef __cplusplus
}
#endif

#endif /* MERCHLIMIT_H */

