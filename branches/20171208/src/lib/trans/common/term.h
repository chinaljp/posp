/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   term.h
 * Author: Pay
 *
 * Created on 2017年3月8日, 上午10:09
 */

#ifndef TERM_H
#define TERM_H

#ifdef __cplusplus
extern "C" {
#endif

    /* 终端信息 */
    typedef struct {
        char sMerchId[15 + 1]; /* 所属商户号                               */
        char sTermId[8 + 1]; /* 终端编号                                 */
        char sStatus[1 + 1]; /* 终端状态,0:正常,1:关闭                   */
        char sTermType[2 + 1]; /* 终端类型(高危POS以3X标识)                **
                                ** 00:有线, 01:无线, 02:网络,  03网络,      **
                                ** 04:分体, 05:蓝牙, 06:多媒体,07便民       */
        char sPinKey[32 + 1]; /* PIN密钥                                  */
        char sMacKey[32 + 1]; /* MAC密钥                                  */
        char sTmk[32 + 1]; /* TMK密钥                                  */
        char sTdk[32 + 1]; /* TDK密钥                                  */
        char sDownFlag[1 + 1]; /* 参数下载标志,0:未下载; 1:已下载          */
        char sTermSn[50+1]; /* 终端序列号 */
    } Terminal;

    int FindTermInfoById(Terminal *pcTerminal, char *pcMerchId, char *pcTermId);
    int FindTermIdBySn(char *pcMerchId, char *pcTermId, char *pcTermVender, char *pcTermSN);
    int UpdTermKey(char *psTmkuLmk, char * psTpkuTmk, char *psTakuTmk, char *psTdkuTmk, char *pstMerchId, char *pstTermId);
    int FindTermParam(char *pcMerchName, char *pcAdvertise, char *pcMerchId, char *pcTermId);
#ifdef __cplusplus
}
#endif

#endif /* TERM_H */

