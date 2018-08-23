/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tKms.h
 * Author: pangpang
 *
 * Created on 2017年5月24日, 下午12:36
 */

#ifndef TKMS_H
#define TKMS_H

#ifdef __cplusplus
extern "C" {
#endif
#define KEY_DEF_VALUE    ""
    //密钥组
#define GROUP_NAME      "default"
    //算法标识
#define ALGM_DES        "DES"    
#define ALGM_SM1        "SM1"    
#define ALGM_SM2        "SM2"    
    //密钥类型
    //    ZPK、ZAK、ZMK、TMK、TPK、TAK、PVK、CVK、ZEK、WWK、BDK、EDK、MK-AC、MK-SMC
#define KEY_ZPK         "ZPK"
#define KEY_ZAK         "ZAK"
#define KEY_ZMK         "ZMK"
#define KEY_TMK         "TMK"
#define KEY_TPK         "TPK"
#define KEY_TAK         "TAK"
#define KEY_PVK         "PVK"
#define KEY_CVK         "CVK"
#define KEY_ZEK         "ZEK"
#define KEY_WWK         "WWK"
#define KEY_BDK         "BDK"
#define KEY_EDK         "EDK"
#define KEY_MK_AC       "MK-AC"
#define KEY_MK_SMC      "MK-SMC"
    //密钥长度
#define KEY_DES_LEN      64
#define KEY_3DES_LEN      128   
    //更新密钥标识    
#define UPD_KEY_DEF    0    //默认
#define UPD_KEY_UPD    1    //当为1时，且密钥名称已经存在，则更新密钥
    //允许使用旧密钥
#define OLD_VER_KEY_USED    1   //允许
#define OLD_VER_KEY_UNUSED    0
    //允许导入标识
#define INPUT_USED  1   //允许
#define INPUT_UNUSED  0  
    //允许导出标识
#define OUTPUT_USED  1
#define OUTPUT_UNUSED  0  
    //有效天数
#define EFF_DAYS_DEF    0
#define EFF_DAYS_1_YEAS    365
#define EFF_DAYS_10_YEAS    3650
#define EFF_DAYS_100_YEAS    36500
    /*enabled 启用标识	可选，默认值初始化时为0，生成密钥则为1 0：不启用
        1：启用，并生效
        2：启用，指定时间生效 */

#define ENABLED_DEF     0 
#define ENABLED_START   1 
#define ENABLED_TIME    2
    //模式  
#define KEY_MODE_NO     0  //不生成密钥
#define KEY_MODE_YES    1   //生成密钥
    /*exportFlag	1N	输出标识	可选，默认值为0，mode 为1和outputFlag为1时存在。*/
#define EXP_NO              0
#define EXP_KEY             1   //指定密钥名称保护输出
#define EXP_ZMK_KEY        2  //外带ZMK密钥保护输出
#define EXP_TMK_KEY        3  //外带TMK密钥保护输出
    /*
     exportLmkKey	1N	输出标识	可选，默认值为0。值为1时返回lmk加密钥的密钥
     */
#define EXP_LMK_KEY         1
#define EXP_UNLMK_KEY       0    
    /*
     protectKey	128A	保护密钥	可选
        当exportFlag为0时，不存在
        当exportFlag为1时：为密钥名称
        当exportFlag为2时，为ZMK密钥值
        当exportFlag为3时，为TMK密钥值
     */
#define PROTECT_KEY_NO   0
#define PROTECT_KEY_NAME    1
    /*algorithmID	1N	算法标识	可选，默认为0 
        0：ECB
        1：CBC
     */
#define ALG_ECB 0
#define ALG_CBC 1 
    /*
     algorithmID	1N	算法标识	可选，默认为1
        1：ANSIX9.19
        2：中国银联标准
        3：银联POS标准

     */
#define ALG_MAC_919 1
#define ALG_MAC_CUPS 2
#define ALG_MAC_CUPS_POS 3
    /*dataType	数据类型
0：ASC 字符（默认）
1：十六进制数
     */
#define DATA_TYPE_ASC   0
#define DATA_TYPE_HEX   1
    /*format	1N	密文数据格式	可选，默认为0
        0：不带填充
        1：4字节明文长度+明文+补位‘0’
        2：填充0x00
        3：填充0x80
     */
#define FORMAT_NO   0
#define FORMAT_0   1    
#define FORMAT_00  2
#define FORMAT_80  3      

    int tHsm_Gen_Zmk(char *pcKeyName);
    int tHsm_Gen_Exp_Zmk(char *pcKeyName, char *pcProtectKeyName, char *pcTmkuZmk, char *pcCheckValue);
    int tHsm_Gen_Zpk(char *pcKeyName, char *pcProtectKeyName, char *pcEnKey, char *pcCheckValue);
    int tHsm_Gen_Zek(char *pcKeyName, char *pcProtectKeyName, char *pcEnKey, char *pcCheckValue);
    int tHsm_Gen_Zak(char *pcKeyName, char *pcProtectKeyName, char *pcEnKey, char *pcCheckValue);
    int tHsm_Gen_Cpus_Zak(char *pcKeyName, char *pcProtectKeyName, char *pcZakuTmk, char *pcCheckValue);
    int tHsm_Imp_Key(char *pcKeyName, char *pcKeyValue, char *pcCheckValue, char *pcProtectKeyName);
    int tHsm_Exp_Key(char *pcKeyName, char *pcKeyValue, char *pcCheckValue, char *pcProtectKeyName);
    int tHsm_Dec_Data(char *pcClearData, char *pcKeyName, char *pcEncData);
    int tHsm_Enc_Data(char *pcEncData, char *pcKeyName, char *pcClearData);
    int tHsm_Gen_Pos_Mac(char *pcMac, char *pcKeyName, char *pcMacBuf, int iMacBufLen);
    int tHsm_Gen_Cups_Mac(char *pcMac, char *pcKeyName, char *pcMacBuf, int iMacBufLen);
    int tHsm_Gen_Pufa_Mac(char *pcMac, char *pcKeyName, char *pcMacBuf, int iMacBufLen);
    int tHsm_TranPin(char *pcNewPinBlock,char *pcCardNo,char *pcPinBlock,char *pcSKey,char *pcDKey);
    
    int tHsm_Enc_Asc_Data(char *pcEncData, char *pcKeyName, char *pcClearData);
    int tHsm_Dec_Asc_Data(char *pcClearData, char *pcKeyName, char *pcEncData);
#ifdef __cplusplus
}
#endif

#endif /* TKMS_H */

