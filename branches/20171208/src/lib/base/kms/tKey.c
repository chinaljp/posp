/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tKey.c
 * Author: pangpang
 *
 * Created on 2017年6月7日, 下午9:23
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "tKms.h"

void tXor(char *pcXorResult, char *pcData, int iLen) {
    char sTmp[8] = {0};
    int i, iPos = 0, iOff = 0;

    memcpy(sTmp, pcData, 8);
    for (iPos = 8; iPos < iLen; iPos += iOff) {
        iOff = ((iLen - iPos) >= 8) ? 8 : iLen - iPos;
        for (i = 0; i < iOff; i++)
            sTmp[i] ^= pcData[ i + iPos ];
    }
    memcpy(pcXorResult, sTmp, 8);
}

int tGetMac8(unsigned char *pcMacData, int iLen, char *pcMac8) {
    int iOff;
    int i, j, iPos;
    int iDataLen;
    char sMacTmp[100];
    char sMacAsc[100];

    memset(sMacTmp, 0, sizeof ( sMacTmp));
    memset(sMacAsc, 0, sizeof ( sMacAsc));

    j = 8 - iLen % 8;
    memcpy(pcMacData + iLen, sMacAsc, j);
    iDataLen = iLen + j;
    for (iPos = 0; iPos < iDataLen; iPos += iOff) {
        iOff = ((iDataLen - iPos) >= 8) ? 8 : iDataLen - iPos;
        for (i = 0; i < iOff; i++)
            sMacTmp[i] ^= pcMacData[ i + iPos ];
    }
    memcpy(pcMac8, sMacTmp, 8);
    return 0;
}

/* 生成key */
int tHsm_Gen_Key(char *pcKeyName, int iKeyLen, char *pcKeyType, int iExportFlag, char *pcKeyuKeyName, char *pcKeyuKey, char *pcEnKey, char *pcCheckValue) {
    int iRet = -1;
    iRet = UnionAPIServiceE110(-1, pcKeyName, GROUP_NAME, ALGM_DES, pcKeyType
            , iKeyLen, UPD_KEY_UPD, OLD_VER_KEY_USED, INPUT_USED, OUTPUT_USED, EFF_DAYS_100_YEAS, ENABLED_START, EFF_DAYS_DEF
            , KEY_DEF_VALUE, KEY_DEF_VALUE, KEY_MODE_YES, iExportFlag, EXP_UNLMK_KEY
            , pcKeyuKeyName, pcEnKey, pcCheckValue, pcKeyuKey);
    return iRet;
}

/* 生成并使用保护密钥输出zmk */
int tHsm_Gen_Exp_Zmk(char *pcKeyName, char *pcProtectKeyName, char *pcTmkuZmk, char *pcCheckValue) {
    char sKeyValue[129] = {0};
    return tHsm_Gen_Key(pcKeyName, KEY_3DES_LEN, KEY_ZMK, EXP_KEY, pcProtectKeyName, sKeyValue, pcTmkuZmk, pcCheckValue);
}

/* 生产zmk，不输出 */
int tHsm_Gen_Zmk(char *pcKeyName) {
    char sKeyValue[129] = {0}, sKeyValue2[128] = {0}, sCheckValue[128] = {0};
    return tHsm_Gen_Key(pcKeyName, KEY_3DES_LEN, KEY_ZMK, EXP_NO, KEY_DEF_VALUE, sKeyValue, sKeyValue2, sCheckValue);
}

/* 生成zpk */
int tHsm_Gen_Zpk(char *pcKeyName, char *pcProtectKeyName, char *pcZpkuTmk, char *pcCheckValue) {
    char sKeyValue[129] = {0};
    return tHsm_Gen_Key(pcKeyName, KEY_3DES_LEN, KEY_ZPK, EXP_KEY, pcProtectKeyName, sKeyValue, pcZpkuTmk, pcCheckValue);
}

/* 生成zek */
int tHsm_Gen_Zek(char *pcKeyName, char *pcProtectKeyName, char *pcZekuTmk, char *pcCheckValue) {
    char sKeyValue[129] = {0};
    return tHsm_Gen_Key(pcKeyName, KEY_3DES_LEN, KEY_ZEK, EXP_KEY, pcProtectKeyName, sKeyValue, pcZekuTmk, pcCheckValue);
}

/* 生成zak */
int tHsm_Gen_Zak(char *pcKeyName, char *pcProtectKeyName, char *pcZakuTmk, char *pcCheckValue) {
    char sKeyValue[129] = {0};
    return tHsm_Gen_Key(pcKeyName, KEY_3DES_LEN, KEY_ZAK, EXP_KEY, pcProtectKeyName, sKeyValue, pcZakuTmk, pcCheckValue);
}

int tHsm_Gen_Cpus_Zak(char *pcKeyName, char *pcProtectKeyName, char *pcZakuTmk, char *pcCheckValue) {
    char sKeyValue[129] = {0};
    return tHsm_Gen_Key(pcKeyName, KEY_DES_LEN, KEY_ZAK, EXP_KEY, pcProtectKeyName, sKeyValue, pcZakuTmk, pcCheckValue);
}

/* 通过保护密钥导入密钥 */
int tHsm_Imp_Key(char *pcKeyName, char *pcKeyValue, char *pcCheckValue, char *pcProtectKeyName) {
    return UnionAPIServiceE112(-1, pcKeyName, pcKeyValue, pcCheckValue, PROTECT_KEY_NAME, pcProtectKeyName, 0);
}

/* 导出密钥 */
int tHsm_Exp_Key(char *pcKeyName, char *pcKeyValue, char *pcCheckValue, char *pcProtectKeyName) {
    return UnionAPIServiceE113(-1, pcKeyName, pcKeyValue, pcCheckValue, PROTECT_KEY_NAME, pcProtectKeyName, 0);
}

/* ZEK加密数据 */
int tHsm_Enc_Data(char *pcEncData, char *pcKeyName, char *pcClearData) {
    return UnionAPIServiceE160(-1, EXP_KEY, pcKeyName, "", ALG_ECB, pcClearData, "", DATA_TYPE_HEX, FORMAT_00, "", pcEncData);
}

/* ZEK解密数据 */
int tHsm_Dec_Data(char *pcClearData, char *pcKeyName, char *pcEncData) {
    return UnionAPIServiceE161(-1, EXP_KEY, pcKeyName, "", ALG_ECB, DATA_TYPE_HEX, pcEncData, "", FORMAT_00, "", pcClearData);
}

/* ZEK加密数据 */
int tHsm_Enc_Asc_Data(char *pcEncData, char *pcKeyName, char *pcClearData) {
    return UnionAPIServiceE160(-1, EXP_KEY, pcKeyName, "", ALG_ECB, pcClearData, "", DATA_TYPE_ASC, FORMAT_00, "", pcEncData);
}

/* ZEK解密数据 */
int tHsm_Dec_Asc_Data(char *pcClearData, char *pcKeyName, char *pcEncData) {
    return UnionAPIServiceE161(-1, EXP_KEY, pcKeyName, "", ALG_ECB, DATA_TYPE_ASC, pcEncData, "", FORMAT_00, "", pcClearData);
}

/* 终端mac,pcMacBuf是异或或的数据 */
int tHsm_Gen_Pos_Mac(char *pcMac, char *pcKeyName, char *pcMacBuf, int iMacBufLen) {
    int iRet = -1;
    char sTmp[8 + 1] = {0}, sBuf[16 + 1] = {0}, sMac[16 + 1] = {0};
    tGetMac8(pcMacBuf, iMacBufLen, sTmp);
    iRet = UnionAPIServiceE150(-1, 1, EXP_KEY, pcKeyName, "", ALG_MAC_919, 1, DATA_TYPE_HEX, sTmp, 8, sMac);
    if (iRet == 0) {
        tAsc2Bcd(pcMac, sMac, 16, LEFT_ALIGN);
    }
    return iRet;
    //return tHsm_Enc_Data(pcMac, pcKeyName, pcMacBuf);
}
#define     GetFullLen( iLen, iUnitLen ) \
                ( ( ( iLen - 1 ) / iUnitLen + 1 ) * iUnitLen )

/* 浦发pos */
int tHsm_Gen_Pufa_Mac(char *pcMac, char *pcKeyName, char *pcMacBuf, int iMacBufLen) {
    int iRet = -1;
    char sTmp[8 + 1] = {0}, sBuf[16 + 1] = {0}, sMac[16 + 1] = {0};
    
    iRet = UnionAPIServiceE150(-1, 1, EXP_KEY, pcKeyName, "", ALG_MAC_CUPS, 1, DATA_TYPE_HEX, pcMacBuf, iMacBufLen, sMac);
    if (iRet == 0) {
        tAsc2Bcd(pcMac, sMac, 16, LEFT_ALIGN);
    }
    return iRet;
}

/* 银联mac */
int tHsm_Gen_Cups_Mac(char *pcMac, char *pcKeyName, char *pcMacBuf, int iMacBufLen) {
    return UnionAPIServiceE150(-1, 1, EXP_KEY, pcKeyName, "", ALG_MAC_CUPS, 1, DATA_TYPE_HEX, pcMacBuf, iMacBufLen, pcMac);
}

int tHsm_TranPin(char *pcNewPinBlock, char *pcCardNo, char *pcPinBlock, char *pcSKey, char *pcDKey) {
    return UnionAPIServiceE142(-1, 1, 1, pcPinBlock, pcSKey, pcDKey, pcCardNo, pcCardNo, "01", "01", 0, "", 0, "", pcNewPinBlock, "", "", "");
}
