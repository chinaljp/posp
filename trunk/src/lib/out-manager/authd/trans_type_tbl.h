/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trans_type_tbl.h
 * Author: Administrator
 *
 * Created on 2017年3月11日, 上午11:54
 */

#ifndef TRANS_TYPE_TBL_H
#define TRANS_TYPE_TBL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char sMsgType[5]; /* 消息类型码       */
    char sProcCode[7]; /* 交易处理码       */
    char sProcCodeEx[3]; /* 扩展的交易处理码 */
    char sNmiCode[4]; /* 网络管理信息码   */
    char sTransCode[7]; /* 交易类型码       */
    char sOldTransCode[7]; /* 原交易类型码     */
    char sOOldTransCode[7]; /* 原原交易类型码   */
} TranTbl;

typedef struct {
    char sTransCode[7]; /* 交易类型码       */
    char sOldTransCode[7]; /* 原交易类型码     */
    char sOOldTransCode[7]; /* 原原交易类型码   */
    unsigned char *pcUnpackBitmap; /* 拆包域位图       */
    unsigned char *pcPackBitmap; /* 组包域位图       */
} TranBitMap;

typedef struct {
    char sTransCode[7];
    unsigned char *pcMacBitmap;
} TransMacMap;

#define BITMAP_SIZE     128
#define BITMAP_BYTE     (BITMAP_SIZE / 8)

/* 从NetTran.sAddData1中划出一段区域, 存储原始报文头信息 */
#define ORGINFO_OFFSET  512


#ifdef __cplusplus
}
#endif

#endif /* TRANS_TYPE_TBL_H */

