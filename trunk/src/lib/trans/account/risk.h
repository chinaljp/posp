/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   risk.h
 * Author: Administrator
 *
 * Created on 2017年7月20日, 下午1:31
 */

#ifndef RISK_H
#define RISK_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char sRuleCode[4 + 1];
        char sRuleName[255 + 1];
        char sStatus[1 + 1];
        char sTransCode[255 + 1];
        int iTransCnt;
        int iQueueCnt;
        double dTransAmt;
        int iInterval;
        char sStartTime[6 + 1];
        char sEndTime[6 + 1];
        char sRespCode[255 + 1];
        char sMcc[255 + 1];
        char sRemark[255 + 1];
        char sIsStop[1 + 1];
        char sIsDel[1 + 1];
        char sIsFrozen[1 + 1];
        char sIsWarnning[1 + 1];
        char sIsLife[1 + 1];
        char sLifeControl[1 + 1];
        char sTransControl[1 + 1];
    } RiskRule;




#ifdef __cplusplus
}
#endif

#endif /* RISK_H */

