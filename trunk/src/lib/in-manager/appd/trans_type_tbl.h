/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trans_type_tbl.h
 * Author: Administrator
 *
 * Created on 2017��3��10��, ����8:14
 */

#ifndef TRANS_TYPE_TBL_H
#define TRANS_TYPE_TBL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char    sMsgId[5];          /* ��ϢID           */
    char    sProcCode[7];       /* ������           */
    char    sMsgType[3];        /* ��Ϣ������       */
    char    sNmiCode[4];        /* ���������Ϣ��   */
    char    sTransCode[7];      /* ����������       */
    char    sOldTransCode[7];   /* ԭ����������     */
}TranTbl;

typedef struct {
    char    sTransCode[7];          /* ������       */
    char    sMsgType[3];            /* ��Ϣ������   */
    unsigned char *pcUnpackBitmap;  /* �����λͼ   */
    unsigned char *pcPackBitmap;    /* �����λͼ   */
}TranBitMap;

typedef struct {
    char    sTransCode[7];
    unsigned char *pcMacBitmap;
}TransMacMap;

typedef struct{
    char sTransCode[7];
    char sSvcName[64];
}TransRoute;



#ifdef __cplusplus
}
#endif

#endif /* TRANS_TYPE_TBL_H */

