/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   quickpay.h
 * Author: Gjq
 *
 * Created on 2018年7月9日, 下午4:54
 */

#ifndef QUICKPAY_H
#define QUICKPAY_H

#ifdef __cplusplus
extern "C" {
#endif
 typedef struct {
     double dAmount;
     char sTransTime[6+1];
     char sTransDate[8+1];
     char sMerchOrderNo[100+1];
     char sValidFlag[1+1];
 } QuickPayDetail;
#ifdef __cplusplus
}
#endif

#endif /* QUICKPAY_H */

