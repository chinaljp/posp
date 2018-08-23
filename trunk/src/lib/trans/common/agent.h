/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   inst.h
 * Author: Administrator
 *
 * Created on 2017年3月9日, 上午10:34
 */

#ifndef INST_H
#define INST_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char sAgentId[8 + 1];
        char sAgentname[60 + 1];
        char sZmk[32 + 1];
        char sStatus[2 + 1];
        char sAgentLevel[2 + 1];
        char sGradeAgentLevel[2 + 1];
        char sAgentOrganization[100 + 1];
    } Agent;

    typedef struct {
        char sAgentId[8 + 1];
        //char sAgentname[60 + 1];
        double dCRate;
        double dCMax;
        double dCRadio;
        double dCYRate;
        double dCYMax;
        double dCYRadio;
        double dCMRate;
        double dCMMax;
        double dCMRadio;
        double dDRate;
        double dDMax;
        double dDRadio;
        double dDYRate;
        double dDYMax;
        double dDYRadio;
        double dDMRate;
        double dDMMax;
        double dDMRadio;
        double dWxRate;
        double dWxMax;
        double dWxRadio;
        double dBbRate;
        double dBbMax;
        double dBbRadio;
        double dYc2FeeCRate;
        double dYc2FeeDRate;
        double dYcNfcCRate;
        double dYcNfcDRate;
        double dYcQrCRate;
        double dYcQrDRate;
        double dYcRadio;
        double dYdRadio;
        double dD0Rate;
    } AgentFee;

    int FindAgentByMerhId(Agent *pstAgent, char *pcMerchId);
    int FindAgentById(Agent *pstAgent, char *pcAgentId);
#ifdef __cplusplus
}
#endif

#endif /* INST_H */

