/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trans_type_tbl.h
 * Author: Administrator
 *
 * Created on 2017年3月10日, 下午5:05
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

#define BITMAP_SIZE     128
#define BITMAP_BYTE     (BITMAP_SIZE / 8)

/* 从NetTran.sAddData1中划出一段区域, 存储原始报文头信息 */
#define ORGINFO_OFFSET  512


#define RESULT_0   "0"         /* 交易成功 */

#define TYPE_A    "11"       /* 主扫支付 */
#define TYPE_P    "12"       /* 被扫支付 */
#define TYPE_Q    "13"       /* 查询     */
#define TYPE_R    "14"       /* 退款     */
#define TYPE_RQ   "15"       /* 退款查询 */
#define TYPE_RE   "16"       /* 撤销     */

#define WEIXIN  "ZXWeChat"         /* 中信微信 */
#define JINGDONG  "JD"             /* 京东钱包 */
#define ZHIFUBAO  "ZXAliPay"       /* 中信支付宝 */

#define XMWEIXIN  "XMMSWXZF"         /* 厦门民生微信 */
#define XMZHIFUBAO  "XMMSZFBZF"       /* 厦门民生支付宝 */

#ifdef __cplusplus
}
#endif

#endif /* TRANS_TYPE_TBL_H */

