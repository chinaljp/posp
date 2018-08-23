/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tKms.h
 * Author: pangpang
 *
 * Created on 2017��5��24��, ����12:36
 */

#ifndef TKMS_H
#define TKMS_H

#ifdef __cplusplus
extern "C" {
#endif
#define KEY_DEF_VALUE    ""
    //��Կ��
#define GROUP_NAME      "default"
    //�㷨��ʶ
#define ALGM_DES        "DES"    
#define ALGM_SM1        "SM1"    
#define ALGM_SM2        "SM2"    
    //��Կ����
    //    ZPK��ZAK��ZMK��TMK��TPK��TAK��PVK��CVK��ZEK��WWK��BDK��EDK��MK-AC��MK-SMC
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
    //��Կ����
#define KEY_DES_LEN      64
#define KEY_3DES_LEN      128   
    //������Կ��ʶ    
#define UPD_KEY_DEF    0    //Ĭ��
#define UPD_KEY_UPD    1    //��Ϊ1ʱ������Կ�����Ѿ����ڣ��������Կ
    //����ʹ�þ���Կ
#define OLD_VER_KEY_USED    1   //����
#define OLD_VER_KEY_UNUSED    0
    //�������ʶ
#define INPUT_USED  1   //����
#define INPUT_UNUSED  0  
    //��������ʶ
#define OUTPUT_USED  1
#define OUTPUT_UNUSED  0  
    //��Ч����
#define EFF_DAYS_DEF    0
#define EFF_DAYS_1_YEAS    365
#define EFF_DAYS_10_YEAS    3650
#define EFF_DAYS_100_YEAS    36500
    /*enabled ���ñ�ʶ	��ѡ��Ĭ��ֵ��ʼ��ʱΪ0��������Կ��Ϊ1 0��������
        1�����ã�����Ч
        2�����ã�ָ��ʱ����Ч */

#define ENABLED_DEF     0 
#define ENABLED_START   1 
#define ENABLED_TIME    2
    //ģʽ  
#define KEY_MODE_NO     0  //��������Կ
#define KEY_MODE_YES    1   //������Կ
    /*exportFlag	1N	�����ʶ	��ѡ��Ĭ��ֵΪ0��mode Ϊ1��outputFlagΪ1ʱ���ڡ�*/
#define EXP_NO              0
#define EXP_KEY             1   //ָ����Կ���Ʊ������
#define EXP_ZMK_KEY        2  //���ZMK��Կ�������
#define EXP_TMK_KEY        3  //���TMK��Կ�������
    /*
     exportLmkKey	1N	�����ʶ	��ѡ��Ĭ��ֵΪ0��ֵΪ1ʱ����lmk����Կ����Կ
     */
#define EXP_LMK_KEY         1
#define EXP_UNLMK_KEY       0    
    /*
     protectKey	128A	������Կ	��ѡ
        ��exportFlagΪ0ʱ��������
        ��exportFlagΪ1ʱ��Ϊ��Կ����
        ��exportFlagΪ2ʱ��ΪZMK��Կֵ
        ��exportFlagΪ3ʱ��ΪTMK��Կֵ
     */
#define PROTECT_KEY_NO   0
#define PROTECT_KEY_NAME    1
    /*algorithmID	1N	�㷨��ʶ	��ѡ��Ĭ��Ϊ0 
        0��ECB
        1��CBC
     */
#define ALG_ECB 0
#define ALG_CBC 1 
    /*
     algorithmID	1N	�㷨��ʶ	��ѡ��Ĭ��Ϊ1
        1��ANSIX9.19
        2���й�������׼
        3������POS��׼

     */
#define ALG_MAC_919 1
#define ALG_MAC_CUPS 2
#define ALG_MAC_CUPS_POS 3
    /*dataType	��������
0��ASC �ַ���Ĭ�ϣ�
1��ʮ��������
     */
#define DATA_TYPE_ASC   0
#define DATA_TYPE_HEX   1
    /*format	1N	�������ݸ�ʽ	��ѡ��Ĭ��Ϊ0
        0���������
        1��4�ֽ����ĳ���+����+��λ��0��
        2�����0x00
        3�����0x80
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

