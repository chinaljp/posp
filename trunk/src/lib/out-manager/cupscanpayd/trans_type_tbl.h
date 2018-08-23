/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trans_type_tbl.h
 * Author: Administrator
 *
 * Created on 2017��3��10��, ����5:05
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

#define BITMAP_SIZE     128
#define BITMAP_BYTE     (BITMAP_SIZE / 8)

/* ��NetTran.sAddData1�л���һ������, �洢ԭʼ����ͷ��Ϣ */
#define ORGINFO_OFFSET  512


#define RESULT_0   "0"         /* ���׳ɹ� */

#define TYPE_A    "11"       /* ��ɨ֧�� */
#define TYPE_P    "12"       /* ��ɨ֧�� */
#define TYPE_Q    "13"       /* ��ѯ     */
#define TYPE_R    "14"       /* �˿�     */
#define TYPE_RQ   "15"       /* �˿��ѯ */
#define TYPE_RE   "16"       /* ����     */

#define WEIXIN  "ZXWeChat"         /* ����΢�� */
#define JINGDONG  "JD"             /* ����Ǯ�� */
#define ZHIFUBAO  "ZXAliPay"       /* ����֧���� */

#define XMWEIXIN  "XMMSWXZF"         /* ��������΢�� */
#define XMZHIFUBAO  "XMMSZFBZF"       /* ��������֧���� */

#ifdef __cplusplus
}
#endif

#endif /* TRANS_TYPE_TBL_H */

