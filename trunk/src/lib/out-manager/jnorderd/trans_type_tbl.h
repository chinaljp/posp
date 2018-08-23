/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trans_type_tbl.h
 * Author: Administrator
 *
 * Created on 2017��3��22��, ����3:00
 */

#ifndef TRANS_TYPE_TBL_H
#define TRANS_TYPE_TBL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char sTransCode[7]; /* ����������       */
    char sOldTransCode[7]; /* ԭ����������     */
    char sOOldTransCode[7]; /* ԭԭ����������   */
    unsigned char *pcUnpackBitmap; /* �����λͼ       */
    unsigned char *pcPackBitmap; /* �����λͼ       */
} TranBitMap;

typedef struct {
    char sTransCode[7];
    unsigned char *pcMacBitmap;
} TransMacMap;


#define RETURNCODE1  "01"   /* �ύ�ɹ� */
#define RETURNCODE2  "02"   /* �ύʧ�� */
#define RETURNCODE3  "03"   /* ������������Ȩ����������� */
#define RETURNCODE4  "04"   /* �����쳣 */
#define RETURNCODE5  "05"   /* ��Ȩ���뵽������������ */
#define JSON_ASCLEN  5

#ifdef __cplusplus
}
#endif

#endif /* TRANS_TYPE_TBL_H */

