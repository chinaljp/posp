/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trans_type_tbl.h
 * Author: Administrator
 *
 * Created on 2017年3月22日, 下午3:00
 */

#ifndef TRANS_TYPE_TBL_H
#define TRANS_TYPE_TBL_H

#ifdef __cplusplus
extern "C" {
#endif

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


#define RETURNCODE1  "01"   /* 提交成功 */
#define RETURNCODE2  "02"   /* 提交失败 */
#define RETURNCODE3  "03"   /* 卡号已做过鉴权，请更换卡号 */
#define RETURNCODE4  "04"   /* 其他异常 */
#define RETURNCODE5  "05"   /* 鉴权人与到账人姓名不符 */
#define JSON_ASCLEN  5

#ifdef __cplusplus
}
#endif

#endif /* TRANS_TYPE_TBL_H */

