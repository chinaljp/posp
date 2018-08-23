/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trans_type_tbl.h
 * Author: Administrator
 *
 * Created on 2017年3月10日, 下午8:14
 */

#ifndef TRANS_TYPE_TBL_H
#define TRANS_TYPE_TBL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char    sMsgId[5];          /* 消息ID           */
    char    sProcCode[7];       /* 处理码           */
    char    sMsgType[3];        /* 消息类型码       */
    char    sNmiCode[4];        /* 网络管理信息码   */
    char    sTransCode[7];      /* 交易类型码       */
    char    sOldTransCode[7];   /* 原交易类型码     */
}TranTbl;

typedef struct {
    char    sTransCode[7];          /* 交易码       */
    char    sMsgType[3];            /* 消息类型码   */
    unsigned char *pcUnpackBitmap;  /* 拆包域位图   */
    unsigned char *pcPackBitmap;    /* 组包域位图   */
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

