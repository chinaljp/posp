/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trans_code.h
 * Author: feng.gaoo
 *
 * Created on 2017年3月9日, 上午10:59
 */

#ifndef TRANS_CODE_H
#define TRANS_CODE_H

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_TRANS_CODE   256
#define TRUE_FLAG   '1'   //有效
#define FALSE_FLAG '0'    //无效
#define POS_GROUP   "POS"
#define INLINE_GROUP    "INLINE"

    typedef struct {
        char sGroupCode[255 + 1];
        char sTransCode[6 + 1];
        char sTransName[255 + 1];
        char sSettleFlag[1 + 1];
        char sLogFlag[1 + 1];
        char sAccFlag[1 + 1];
        char sSafFlag[1 + 1];
        char sStatus[1 + 1];
    } TransCode;

    int FindTransCode(TransCode *pstTransCode, char *pcTransCode);
    int LoadTransCode();
    int LoadTransCodeByCode(TransCode *pstTransCode, char *pcTransCode);
    int LoadTransCodeByGroupId(char *pcGroupId);
#ifdef __cplusplus
}
#endif

#endif /* TRANS_CODE_H */

