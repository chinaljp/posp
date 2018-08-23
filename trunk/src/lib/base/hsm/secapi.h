/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   secapi.h
 * Author: feng.gaoo
 *
 * Created on 2017年2月28日, 下午10:05
 */

#ifndef SECAPI_H
#define SECAPI_H

#ifdef __cplusplus
extern "C" {
#endif


    /***************************************
     * 生成MAC                              *
     * 入掺:LMK加密下的TAK                  *
     * 入掺:掺于计算的MAC数据               *
     * 入掺:掺于计算的MAC数据长度           *
     * 入掺:MAC 16位16进制数                *
     ****************************************/
    int HSM_GenMac(char *sTakuLmk, unsigned char *sMacData, int iDataLen, unsigned char *sMac);


    /**************************************
     *  产生终端用的TMK                    *
     *  用于第一次产生TMK                  *
     *  入参1: LMK加密下的ZMK              *
     *  出参1: LMK加密下的TMK              *
     *  出掺2: ZMK加密下的TMK              *
     **************************************/
    int HSM_FirstGenTmk(char *sZmkuLmk, char *sTmkuZmk, char *sTmkuLmk, char *sCheckValue);


    /**************************
     *  生成TPK                *
     *  入参1: LMK加密下的tmk  *
     *  出参1: LMK加密下的原tpk*
     *  出参2: LMK加密下的tpk  *
     *  出参3: tpk校验值16位   *
     ***************************/
    int HSM_GenTpk(char *sTmkuLmk, char *sTpkuZmk, char *sTpkuLmk, char *sCheckValue);



    /**************************
     *  生成TAK                *
     *  入参1: LMK加密下的tmk  *
     *  出参1: LMK加密下的原tak*
     *  出参2: LMK加密下的tak  *
     *  出参3: tpk校验值6位    *
     ***************************/
    int HSM_GenTak(char *sTmkuLmk, char *sTakuZmk, char *sTakuLmk, char *sCheckValue);


    /************************
     *  生成ZPK              *
     *  入参1: LMK加密下的zmk*
     *  出参1: ZMK加密下的zpk*
     *  出参2: LMK加密下的zpk*
     *  出参3: zpk校验值16位 *
     ************************/
    int HSM_GenZpk(char *sZmkuLmk, char *sZpkuZmk, char *sZpkuLmk, char *sCheckValue);

    int HSM_GenTdk(char *sTmkuLmk, char *sTdkuZmk, char *sTdkuLmk, char *sCheckValue);

    /************************
     *  生成ZAK              *
     *  入参1: LMK加密下的zmk*
     *  出参1: ZMK加密下的zak*
     *  出参2: LMK加密下的zak*
     *  出参3: zak校验值6位 *
     ************************/
    int HSM_GenZak(char *sZmkuLmk, char *sZakuZmk, char *sZakuLmk, char *sCheckValue);


    /***************************************
     * 生成 发卡机构MAC                     *
     * 入掺:LMK加密下的ZAK                  *
     * 入掺:掺于计算的MAC数据               *
     * 入掺:掺于计算的MAC数据长度           *
     * 入掺:MAC 16位16进制数                *
     ****************************************/
    int HSM_GenHostMac(char *sZakuLmk, unsigned char *sMacData, int iDataLen, unsigned char *sMac);


    /********************************
     * 转换PIN从TPK到ZPK             *
     * 入掺:卡号                     *
     * 入掺:原PIN 16位               *
     * 入掺:LMK加密下的TPK           *
     * 入掺:LMK加密下的ZPK           *
     * 出掺:ZPK加密下的PIN 16位      *
     *********************************/
    int HSM_TranPin_Tpk2Zpk(char *sCardNo, char *sOldPin, char *sTpkuLmk, char *sZpkuLmk, char *sNewPin);

    /* 加密机长连接拆链 */
    void HSM_UNLINK(void);
    int HSM_LINK(unsigned char *in, int wlen, unsigned char *out);
    int HSM_Dec_Track(char *pcClearData, char *pcDecData, int iDataLen, char *pcTdkuLmk);
    int HSM_Dec_Pcmsg(char *pcClearData, char *pcDecData, int iDataLen, char *pcTdkuLmk);
#ifdef __cplusplus
}
#endif

#endif /* SECAPI_H */

