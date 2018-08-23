/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_extiso.h"
#include "trans_type_tbl.h"
#include "secapi.h"         /* 加解密   */
#include "trans_detail.h"
#include "trans_code.h"
#include "t_macro.h"
#include "trace.h"
#include "err.h"
#include "s_param.h"
#include "tKms.h"

/* dbop.pc */
int GetTermKey(char *pcMerchId, char *pcTermId, char *pcTak, char *pcTdk);

extern IsoData g_stIsoData;
char *g_pcMsg = NULL;
int g_iMsgLen = 0;

TranTbl g_staTranTbl[] = {
    /* 消息ID  处理码    消息类型码  NMI   交易代码  原交易代码  */
    { "0200", "310000", "01", "000", "021000", ""}, /* POS查余额            */
    { "0200", "000000", "22", "000", "020000", ""}, /* POS消费              */
    { "0400", "000000", "22", "000", "020003", ""}, /* POS消费冲正          */
    { "0200", "200000", "23", "000", "020002", ""}, /* POS消费撤销          */
    { "0400", "200000", "23", "000", "020023", ""}, /* POS消费撤销冲正      */
    { "0220", "200000", "25", "000", "020001", ""}, /* POS退货              */
    { "0200", "800000", "22", "000", "022100", ""}, /* 电子签购单 */
    
    { "0100", "030000", "10", "000", "024000", ""}, /* POS预授权            */
    { "0400", "030000", "10", "000", "024003", ""}, /* POS预授权冲正        */
    { "0100", "200000", "11", "000", "024002", ""}, /* POS预授权撤销        */
    { "0400", "200000", "11", "000", "024023", ""}, /* POS预授权撤销冲正    */
    { "0200", "000000", "20", "000", "024100", ""}, /* POS预授权完成        */
    { "0400", "000000", "20", "000", "024103", ""}, /* POS预授权完成冲正    */
    { "0200", "200000", "21", "000", "024102", ""}, /* POS预授权完成撤销    */
    { "0400", "200000", "21", "000", "024123", ""}, /* POS预授权完成撤销冲正*/
    
    { "0200", "000000", "36", "000", "U20000", ""},   /* POS脱机消费        */
    { "0220", "200000", "27", "000", "U20001", ""},   /* POS脱机消费退货     */
    
    { "0200", "311000", "01", "000", "M21000", ""},   /* 订单查询          */
    { "0200", "100000", "22", "000", "M20000", ""},   /* 订单消费          */ 
    { "0400", "100000", "22", "000", "M20003", ""},  /* 订单消费冲正      */
    { "0200", "210000", "23", "000", "M20002", ""},  /* 订单消费撤销      */
    { "0400", "210000", "23", "000", "M20023", ""},  /* 订单消费撤销冲正  */
    
    { "0200", "280000", "22", "000", "T20000", ""},   /* T0消费             */
    { "0400", "280000", "22", "000", "T20003", ""},   /* T0消费冲正          */    
    { "0620", "280000", "22", "000", "T20010", ""},   /* 日结消费通知         */

    { "0200", "330000", "50", "000", "02A000", ""}, /*  实名认证    */
    { "0200", "340000", "50", "000", "02A100", ""}, /* 查日结额度     */
    
        /* 微信 */
    { "0200", "710000", "60", "000", "02W100", "002"}, /* 微信主扫          */
    { "0200", "700000", "60", "000", "02W200", "002"}, /* 微信被扫支付           */     
    { "0200", "730000", "60", "000", "02W300", "002"}, /* 微信查询          */
    { "0200", "720000", "60", "000", "02W600", "002"},    /* 撤销         */
    
    /* 银联 */
    { "0200", "710000", "60", "000", "02Y100", "004"}, /* 银联主扫          */
    { "0200", "700000", "60", "000", "02Y200", "004"}, /* 银联被扫支付           */     
    { "0200", "730000", "60", "000", "02Y300", "004"}, /* 银联查询          */ 
    { "0200", "720000", "60", "000", "02Y600", "004"},    /* 银联撤销         */
    
    /* 支付宝 */
    { "0200", "710000", "60", "000", "02B100", "003"}, /* 支付宝主扫          */
    { "0200", "700000", "60", "000", "02B200", "003"}, /* 支付宝被扫支付           */     
    { "0200", "730000", "60", "000", "02B300", "003"}, /* 支付宝查询          */   
    { "0200", "720000", "60", "000", "02B600", "003"},    /* 撤销         */

//    { "0200", "740000", "60", "000", "S00004", ""}, /* 退款      */
//    { "0200", "750000", "60", "000", "S00005", ""}, /* 退款查询      */
//    { "0200", "720000", "60", "000", "S00006", ""}, /* 撤销         */   

    { "0620", "", "", "951", "020400", "021000"}, /* POS脚本通知*/
    { "0620", "", "", "951", "020400", "023000"}, /* POS脚本通知*/
    { "0620", "", "", "951", "020400", "020000"}, /* POS脚本通知*/
    { "0620", "", "", "951", "020400", "024000"}, /* POS脚本通知*/
    { "0620", "", "", "951", "020400", "024100"}, /* POS脚本通知*/
    { "0620", "", "", "951", "020400", "026000"}, /* POS脚本通知*/
    { "0620", "", "", "951", "020400", "027000"}, /* POS脚本通知*/
    
    { "0200", "800000", "22", "000", "022100", ""},      /* 电子签购单 */    
    
    /* 管理类交易 */
    { "0800", "", "00", "003", "029000", ""}, /* POS签到              */
    { "0800", "", "00", "101", "029300", ""}, /* POS初始化            */
    { "0800", "", "00", "360", "029400", ""}, /* POS参数下载          */
    { "0820", "", "07", "301", "029500", ""}, /* POS电子签名上传      */
    //{ "0000", "", "F0", "000", "033700", ""}, /* 库表更新通知(测试用) */

    { "0800", "", "00", "370", "028100", ""}, /* IC卡公钥下载          */
    { "0800", "", "00", "380", "028200", ""}, /* IC卡参数下载          */
    { "0800", "", "00", "390", "028300", ""}, /* IC卡黑名单下载          */
    { "0800", "", "00", "394", "028400", ""}, /* 非接参数下载下载          */
    { "0800", "", "00", "396", "028500", ""}, /* 免密新增卡bin下载          */
    { "0800", "", "00", "398", "028600", ""}, /* 免密卡bin黑名单下载          */

    { "0800", "", "00", "371", "028110", ""}, /* IC卡公钥下载结束          */
    { "0800", "", "00", "381", "028210", ""}, /* IC卡参数下载结束          */
    { "0800", "", "00", "391", "028310", ""}, /* IC卡黑名单下载结束          */
    { "0800", "", "00", "395", "028410", ""}, /* 非接参数下载下载结束          */
    { "0800", "", "00", "397", "028510", ""}, /* 免密新增卡bin下载结束          */
    { "0800", "", "00", "399", "028610", ""}, /* 免密卡bin黑名单下载结束          */

    { "0820", "", "00", "372", "027800", ""}, /* IC卡公钥下载状态上送          */
    { "0820", "", "00", "382", "027900", ""}, /* IC卡参数下载状态上送          */

    { "", "", "", "", "", ""},
};

/*************
 **  交易类  **
 *************/

/* POS查余额 */
static unsigned char g_caInqueryUnpackMap[] = {\
2, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 59, 60, 62, 64, 255};
static unsigned char g_caInqueryPackMap[] = {\
2, 11, 12, 13, 14, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 54, 55, 60, 63 ,64, 255};



/* POS消费 */
static unsigned char g_caPurchaseUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 57, 59, 60, 62, 63, 64, 255};
static unsigned char g_caPurchasePackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55,59, 60, 63, 64, 255};
/* POS脱机消费 */
static unsigned char g_caPbocUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 59, 60, 62, 63, 64, 255};
static unsigned char g_caPbocPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 60, 63, 64, 255};


/* 非指定账户圈存 */
static unsigned char g_caPurchaseFZDUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 59, 60, 62, 63, 64, 255};
static unsigned char g_caPurchaseFZDPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32,35,36, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55,59, 60, 63, 64, 255};

/* POS消费撤销 */
static unsigned char g_caUndoPurchaseUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 63, 64, 255};
static unsigned char g_caUndoPurchasePackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};

/* POS退货 */
static unsigned char g_caRefundUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 63, 64, 255};
static unsigned char g_caRefundPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};

/* POS预授权 */
static unsigned char g_caPreAuthoUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 41, 42, 35, 36, 49, 52, 53, 55, 57, 59, 60, 63, 64, 255};
static unsigned char g_caPreAuthoPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55, 59, 60, 63, 64, 255};

/* POS预授权撤销 */
static unsigned char g_caUndoPreAuthoUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 63, 64, 255};
static unsigned char g_caUndoPreAuthoPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};

/* POS预授权完成 */
static unsigned char g_caPreAuthoCompUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 38, 41, 42, 35, 36, 49, 52, 53, 55, 59, 60, 61, 63, 64, 255};
static unsigned char g_caPreAuthoCompPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55, 59, 60, 63, 64, 255};

/* POS预授权完成撤销 */
static unsigned char g_caUndoPreAuthoCompUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 63, 63, 255};
static unsigned char g_caUndoPreAuthoCompPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};

/* POS撤销冲正 */
static unsigned char g_caCorrectUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 32, 38, 39, 41, 42, 49, 53, 55, 60, 61, 62, 64, 255};
static unsigned char g_caCorrectPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 55, 60, 64, 255};

/* POS冲正 */
static unsigned char g_caPurchaseRevUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 32, 38, 39, 41, 42, 49, 53, 55, 60, 62, 64, 255};
static unsigned char g_caPurchaseRevPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 55, 60, 64, 255};

/* POS脚本通知 */
static unsigned char g_caJbtzUnpackMap[] = {\
2, 4, 11, 22, 23, 32, 37, 41, 42, 49, 55, 60, 61, 64, 255};
static unsigned char g_caJbtzPackMap[] = {\
2, 11, 12, 13, 15, 22, 23, 32, 38, 39, 41, 42, 49, 55, 60,  64, 255};

/* 电子签购单*/
static unsigned char g_caReceiptsUnpackMap[] = {\
11,14, 37, 41, 42, 60,62, 64, 255};
static unsigned char g_caReceiptsPackMap[] = {\
5,11, 12, 13, 14,32, 39, 41, 42, 60, 64, 255};


/* 订单查询 */
static unsigned char g_caOrderUnpackMap[] = {\
6, 11, 41, 42, 60,62, 64, 255};
static unsigned char g_caOrderPackMap[] = {\
6, 11, 12, 13, 37, 39, 41, 42, 44, 54, 60, 64, 255};


/* 订单消费 */
static unsigned char g_caorderPurchaseUnpackMap[] = {\
2, 4, 6, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 57, 59, 60,62, 63, 64, 255};
static unsigned char g_caorderPurchasePackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55,59, 60,63, 64, 255};

/* 订单消费冲正 */
static unsigned char g_caorderPurchaseRevUnpackMap[] = {\
2, 4, 6, 11, 14, 22, 23, 25, 32, 38, 39, 41, 42, 49, 53, 55, 60,62, 64, 255};
static unsigned char g_caorderPurchaseRevPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 55, 60, 64, 255};


/* 订单消费撤销 */
static unsigned char g_caUndorderUnpackMap[] = {\
2, 4, 6, 11, 14, 22, 23, 25, 26, 32, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 62, 63, 64, 255};
static unsigned char g_caUndorderPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};


/* 订单消费撤销冲正 */
static unsigned char g_caorderCorrectUnpackMap[] = {\
2, 4, 6, 11, 14, 22, 23, 25, 32, 38, 39, 41, 42, 49, 53, 55, 60, 61, 62,64, 255};
static unsigned char g_caorderCorrectPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 55, 60, 64, 255};

/* POS鉴权 */
static unsigned char g_AuthenUnpackMap[] = {\
2, 11, 41, 42, 53, 62, 63, 64, 255};

static unsigned char g_AuthenPackMap[] = {\
2, 11, 12, 13, 37, 39, 41, 42, 44, 63, 64, 255};

/* 日结额度查询 */
static unsigned char g_QuotaUnpackMap[] = {\
11, 41, 42, 64, 255};
static unsigned char g_QuotaPackMap[] = {\
11, 12, 13, 37, 39, 41, 42, 54, 63, 64, 255};

/* 主扫消费 */
static unsigned char g_UnActivescanpackMap[] = {\
4, 6, 11, 14, 32, 41, 42, 44, 49, 58, 62, 63, 64, 255};
static unsigned char g_ActivescanpackMap[] = {\
4, 6, 11, 12, 13, 14, 32, 37, 39, 41, 42, 44, 49, 63, 64, 255};

/* 被扫消费 */
static unsigned char g_UnPassivescanpackMap[] = {\
4, 7, 11, 14, 22, 32, 41, 42, 44, 49, 58, 62, 63, 64, 255};
static unsigned char g_PassivescanpackMap[] = {\
4, 5, 6, 11, 12, 13, 14, 32, 37, 39, 41, 42, 44, 49, 63, 64, 255};

/* 查询 */
static unsigned char g_UnQueryresultpackMap[] ={\
6, 11, 32, 41, 42, 44, 63, 64, 255};
static unsigned char g_QueryresultpackMap[] = {\
4, 6, 11, 12, 13, 32, 37, 39, 41, 42, 44, 63, 64, 255};

/*二维码撤销*/
static unsigned char g_UnTdRevokepackMap[] = {\
4, 11, 32, 37, 41, 42, 44, 49, 60, 61, 63, 64, 255};
static unsigned char g_TdRevokepackMap[] = {\
4, 11, 12, 13, 32, 37, 39, 41, 42, 44, 49, 60, 63, 64, 255};

/* 退款 */
static unsigned char g_UnRefundpackMap[] = {\
4, 6, 11, 32, 37, 41, 42, 44, 49, 58, 60, 63, 64, 255};
static unsigned char g_RefundpackMap[] ={\
4, 11, 12, 13, 32, 37, 39, 41, 42, 44, 49, 60, 63, 64, 255};

/* 退款查询 */
static unsigned char g_UnQueryRefundpackMap[] ={\
4, 6, 11, 37, 41, 42, 44, 60, 61, 63, 64, 255};
static unsigned char g_QueryRefundpackMap[] ={\
4, 6, 11, 12, 13, 37, 39, 41, 42, 44, 60, 63, 64, 255};

/* 订单撤销 */
static unsigned char g_UnRevokepackMap[] = {\
4, 6, 11, 32, 37, 41, 42, 44, 49, 58, 60, 63, 64, 255};
static unsigned char g_RevokepackMap[] = {\
4, 6, 11, 12, 13, 32, 37, 39, 41, 42, 44, 49, 60, 63, 64, 255};


/*************
 **  管理类  **
 *************/

/* POS签到 */
static unsigned char g_caLoginUnpackMap[] = {\
11, 32,41, 42, 60, 63, 255};
static unsigned char g_caLoginPackMap[] = {\
11, 12, 13, 32, 37, 39, 41, 42, 60, 62, 255};

/* POS状态上送 */
static unsigned char g_caZtssUnpackMap[] = {\
 37, 41, 42, 60, 62, 255};
static unsigned char g_caZtssPackMap[] = {\
 12, 13, 39, 41, 42, 60, 62, 255};

/* POS初始化 */
static unsigned char g_caInitUnpackMap[] = {\
11, 59, 60, 255};
static unsigned char g_caInitPackMap[] = {\
11, 12, 13, 32, 37, 39, 41, 42, 60, 255};

/* POS参数下载 */
static unsigned char g_caArgumentDownloadUnpackMap[] = {\
41, 42, 60, 255};
static unsigned char g_caArgumentDownloadPackMap[] = {\
12, 13, 37, 39, 41, 42, 60, 62, 255};

/* POS电子签名上传 */
static unsigned char g_caElecSigUploadUnpackMap[] = {\
2, 4, 11, 15, 37, 41, 42, 60, 62, 64, 255};
static unsigned char g_caElecSigUploadPackMap[] = {\
11, 37, 39, 41, 42, 60, 64, 255};


TranBitMap g_staBitMap[] = {
    /* 交易码   消息类型码 拆包域位图                      组包域位图                   */
    { "021000", "01", g_caInqueryUnpackMap, g_caInqueryPackMap}, /* POS查余额            */
    { "020000", "22", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* POS消费              */
    { "020003", "22", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* POS消费冲正          */
    { "020002", "23", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* POS消费撤销          */
    { "020023", "23", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* POS消费撤销冲正      */
    { "020001", "25", g_caRefundUnpackMap, g_caRefundPackMap}, /* POS退货              */

    { "024000", "10", g_caPreAuthoUnpackMap, g_caPreAuthoPackMap}, /* POS预授权            */
    { "024003", "10", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* POS预授权冲正        */
    { "024002", "11", g_caUndoPreAuthoUnpackMap, g_caUndoPreAuthoPackMap}, /* POS预授权撤销        */
    { "024023", "11", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* POS预授权撤销冲正    */
    { "024100", "20", g_caPreAuthoCompUnpackMap, g_caPreAuthoCompPackMap}, /* POS预授权完成        */
    { "024103", "20", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* POS预授权完成冲正    */
    { "024102", "21", g_caUndoPreAuthoCompUnpackMap, g_caUndoPreAuthoCompPackMap}, /* POS预授权完成撤销    */
    { "024123", "21", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* POS预授权完成撤销冲正*/

    { "U20000", "36", g_caPbocUnpackMap, g_caPbocPackMap}, /* POS脱机消费              */
    { "U20001", "27", g_caRefundUnpackMap, g_caRefundPackMap}, /* POS脱机消费退货              */

    { "T20000", "22", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* T0消费 */
    { "T20003", "22", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* T0消费冲正 */
    { "T20010", "22", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* T0日结通知          */

    { "M21000", "01", g_caOrderUnpackMap, g_caOrderPackMap}, /* 订单查询          */
    { "M20000", "22", g_caorderPurchaseUnpackMap, g_caorderPurchasePackMap}, /* 订单消费     */
    { "M20002", "23", g_caUndorderUnpackMap, g_caUndorderPackMap}, /* 订单消费撤销  */
    { "M20003", "22", g_caorderPurchaseRevUnpackMap, g_caorderPurchaseRevPackMap}, /* 订单消费冲正  */
    { "M20023", "23", g_caorderCorrectUnpackMap, g_caorderCorrectPackMap}, /* 订单消费撤销冲正 */

    { "02A000", "50", g_AuthenUnpackMap, g_AuthenPackMap}, /* POS鉴权 */
    { "02A100", "50", g_QuotaUnpackMap, g_QuotaPackMap}, /* 日结限额查询  */

    { "02Y100", "60", g_UnActivescanpackMap, g_ActivescanpackMap}, /* 银联主扫模式 */
    { "02Y200", "60", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* 银联被扫模式 */
    { "02Y300", "60", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* 银联支付查询 */
    { "02Y600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap}, /*二维码银联撤销*/
    
    { "02B100", "60", g_UnActivescanpackMap, g_ActivescanpackMap}, /* 支付宝主扫模式 */
    { "02B200", "60", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* 支付宝被扫模式 */
    { "02B300", "60", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* 支付宝支付查询 */
    { "02B600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap},      /*二维码支付宝撤销*/

    { "02W100", "60", g_UnActivescanpackMap, g_ActivescanpackMap}, /* 微信主扫模式 */
    { "02W200", "60", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* 微信被扫模式 */
    { "02W300", "60", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* 微信支付查询 */
    { "02W600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap},      /*二维码微信撤销*/
    
    { "S00004", "60", g_UnRefundpackMap, g_RefundpackMap}, /* 退款 */
    { "S00005", "60", g_UnQueryRefundpackMap, g_QueryRefundpackMap}, /* 退款查询 */
    { "S00006", "60", g_UnRevokepackMap, g_RevokepackMap}, /* 订单撤销 */

    { "023000", "45", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* 电子现金圈存              */
    { "020300", "45", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* 电子现金圈存冲正          */
    { "023000", "46", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* 电子现金现金充值              */
    { "020300", "46", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* 电子现金现金充值冲正         */
    { "023000", "47", g_caPurchaseFZDUnpackMap, g_caPurchaseFZDPackMap}, /* 电子现金非指定账户圈存              */
    { "020300", "47", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* 电子现金非指定账户圈存冲正          */
    { "020200", "51", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* 电子现金现金充值撤销       */
    { "020300", "51", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* 电子现金现金充值撤销冲正      */
    { "020400", "00", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* IC卡脚本通知              */

    { "029000", "00", g_caLoginUnpackMap, g_caLoginPackMap}, /* POS签到              */
    { "029300", "00", g_caInitUnpackMap, g_caInitPackMap}, /* POS初始化            */
    { "029400", "00", g_caArgumentDownloadUnpackMap, g_caArgumentDownloadPackMap}, /* POS参数下载          */
    { "029500", "07", g_caElecSigUploadUnpackMap, g_caElecSigUploadPackMap}, /* POS电子签名上传      */
    { "022100", "22", g_caReceiptsUnpackMap, g_caReceiptsPackMap}, /* 电子签购单 */

    { "028100", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡公钥下载              */
    { "028110", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡公钥下载结束              */
    { "028200", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡参数下载              */
    { "028210", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡参数下载结束              */
    { "028300", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡黑名单下载              */
    { "028310", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡黑名单下载结束              */
    { "028400", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* 非接参数下载下载              */
    { "028410", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* 非接参数下载下载结束              */
    { "028500", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* 免密新增卡bin下载              */
    { "028510", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* 免密新增卡bin下载结束              */
    { "028600", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* 免密卡bin黑名单下载              */
    { "028610", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* 免密卡bin黑名单下载结束              */

    { "027800", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡公钥下载状态上送              */
    { "027900", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡参数下载状态上送              */

    //    { "020500", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC卡状态上送(使用029800,这个废弃)              */
    { "", "", NULL, NULL}
};

/*
 * 根据POS报文的消息类型、处理码、消息类型码、网络管理信息码，在定义表中判断查找交易类型
 */
int GetTranType(cJSON *pstJson, char *pcMsgId, char *pcProcCode, char *pcMsgType, char *pcNmiCode, char *pcOldTransCode) {
    int i;

    /* 查取当前交易类型(根据从发卡机构报文解出的消息ID、处理码、消息类型码、网络管理码) */
    for (i = 0; g_staTranTbl[i].sMsgId[0] != '\0'; i++)
    {
        if (!memcmp(g_staTranTbl[i].sMsgId, pcMsgId, strlen(g_staTranTbl[i].sMsgId))
                && !memcmp(g_staTranTbl[i].sProcCode, pcProcCode, strlen(g_staTranTbl[i].sProcCode))
                && !memcmp(g_staTranTbl[i].sMsgType, pcMsgType, strlen(g_staTranTbl[i].sMsgType))
                && !memcmp(g_staTranTbl[i].sNmiCode, pcNmiCode, strlen(g_staTranTbl[i].sNmiCode))
                && !memcmp(g_staTranTbl[i].sOldTransCode, pcOldTransCode, strlen(pcOldTransCode)))
        {
            SET_STR_KEY(pstJson, "trans_code", g_staTranTbl[i].sTransCode);
            return ( 0);
        }
    }
    return ( -1);
}

/*
 * 根据内部报文的交易代码、原交易代码，在定义表中查找消息ID、处理码、消息类型码、网络管理码
 */
int GetMsgData(char *pcTransCode, char *pcOldTransCode, char *pcMsgId, char *pcProcCode, char *pcMsgType, char *pcNmiCode) {
    int i;

    for (i = 0; g_staTranTbl[i].sMsgId[0] != '\0'; i++)
    {
        if (!memcmp(g_staTranTbl[i].sTransCode, pcTransCode, 6))
        {
            strcpy(pcMsgId, g_staTranTbl[i].sMsgId);
            strcpy(pcProcCode, g_staTranTbl[i].sProcCode);
            strcpy(pcMsgType, g_staTranTbl[i].sMsgType);
            strcpy(pcNmiCode, g_staTranTbl[i].sNmiCode);

            return ( 0);
        }
    }
    return ( -1);
}

int DecTrack(char *pcClearTrack, char *pcDecTrack, int iDecTrackLen, char *pcTdkuLmk) {
    int iRet = -1, iLen = 0;
    char sBlock[17] = {0}, sTmp[17] = {0};

    tLog(DEBUG, "Track Data[%d][%s]", iDecTrackLen, pcDecTrack);
    tStrCpy(pcClearTrack, pcDecTrack, iDecTrackLen);
    /* 整除是偶数，奇数+1*/
    iLen = iDecTrackLen % 2 ? iDecTrackLen + 1 : iDecTrackLen;
    /* 从后数第二位向前取8字节*/
    tLog(DEBUG, "Track Len[%d]", iLen);
    memcpy(sBlock, pcDecTrack + iLen - 18, 16);
    //iRet = HSM_Dec_Track(sTmp, sBlock, 16, pcTdkuLmk);
    iRet = tHsm_Dec_Data(sTmp, pcTdkuLmk, sBlock);
    if (iRet < 0)
        return -1;
    tLog(DEBUG, "Track Clear[%s]", sTmp);
    memcpy(pcClearTrack + iLen - 18, sTmp, 16);

    return 0;
}

/* unpack */
int GetFld(int iBitNo, cJSON *pstJson) {
    char sNumBuf[3], sBuf1[2048], sBuf2[2048], caMsgBuf[1024], sTak[33] = {0}, caMac[8], caTmp[8], sTdk[33] = {0};
    int iLen = 0, iRet = -1;
    char sBuf[1024] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0}, sInputMode[3 + 1] = {0};
    char sErr[128] = {0}, sCtype[3] = {0}, sTrack2[38] = {0}, sTrack3[105] = {0}, sAcqId[11 + 1] = {0};
    char track2Buf[37];
    int pos = 0;

    memset(sNumBuf, 0, sizeof (sNumBuf));
    memset(sBuf, 0, sizeof (sBuf));
    memset(caTmp, 0, sizeof (caTmp));
    memset(caMsgBuf, 0, sizeof (caMsgBuf));
    memset(track2Buf, 0, sizeof (track2Buf));
    switch (iBitNo)
    {
        case 2: /* 主账号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet <= 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_no", sBuf);
            break;
        case 4: /* 交易金额 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_DOU_KEY(pstJson, "amount", atol(sBuf));
            break;
        case 6:/* 订单号 */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /* 用法一: 当交易为订单相关交易时订单号用sPrivateData域 */
            if (!memcmp(sTransCode, "MM", 1))
            {
                SET_STR_KEY(pstJson, "merch_order_no", sBuf);
            } else if (!memcmp(sTransCode, "02W100", 6) || !memcmp(sTransCode, "02B100", 6) || !memcmp(sTransCode, "02Y100", 6) ) 
			{
                SET_STR_KEY(pstJson, "seed", sBuf);
            } else
            {
                SET_STR_KEY(pstJson, "qr_order_no", sBuf);
            }
            break;
        case 7: /* 商户订单号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "merch_order_no", sBuf);
            tLog(DEBUG, "商户订单号[%s]", sBuf);
            break;
        case 11: /* 终端流水号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "trace_no", sBuf);
            break;
        case 14: /* 卡有效期 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_exp_date", sBuf);
            break;
        case 22: /* 服务点输入方式码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "input_mode", sBuf);
            break;
        case 23: /* 卡片序列号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            tLog(DEBUG, "23 card_sn[%s]", sBuf);
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_sn", sBuf);
            break;
        case 25: /* 服务点条件码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "cond_code", sBuf);
            break;
        case 26: /* 服务点PIN获取码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "pin_code", sBuf);
            break;
        case 32: /* 受理机构标识 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tTrim(sBuf);
            SET_STR_KEY(pstJson, "acq_id", sBuf);
            break;
        case 35: /* 2磁道数据 */
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            if (GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
            {
                SET_STR_KEY(pstJson, "resp_code", "A7");
                tLog(ERROR, "获取终端[%s:%s]的密钥出错.", sMerchId, sTermId);
                return ( -1);
            }
            tLog(DEBUG, "TDK[%s]", sTdk);
            tLog(DEBUG, "TAK[%s]", sTak);
            //有机构没有加密，特殊处理一下
            GET_STR_KEY(pstJson, "acq_id", sAcqId);
            if (NULL == strstr(sAcqId, "49000780"))
            {
                DecTrack(sTrack2, sBuf, strlen(sBuf), sTdk);
            } else
            {
                strcpy(sTrack2, sBuf);
            }
            SET_STR_KEY(pstJson, "track2", sTrack2);
            tLog(DEBUG, "Track2[%s]", sTrack2);
            if (iRet != 0)
            {
                pos = 0;
                memset(track2Buf, 0, sizeof (track2Buf));
                memcpy(track2Buf, sTrack2, 20);
                while (*(track2Buf + pos) != 'D' && *(track2Buf + pos) != '\0')
                {
                    if (0 == isdigit(track2Buf[pos]))
                        return -1;
                    pos++;
                }
                *(track2Buf + pos) = '\0';
                SET_STR_KEY(pstJson, "card_no", track2Buf);
                tLog(DEBUG, "track card_no[%s]", track2Buf);
            }
            break;
        case 36: /* 3磁道数据 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            if (sBuf[0] == '\0')
                return 0;
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);

            if (sTdk[0] == '\0' && GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
            {
                SET_STR_KEY(pstJson, "resp_code", "A7");
                tLog(ERROR, "获取终端[%s:%s]的密钥出错.", sMerchId, sTermId);
                return ( -1);
            }
            tLog(DEBUG, "TDK[%s]", sTdk);
            tLog(DEBUG, "TAK[%s]", sTak);
            DecTrack(sTrack3, sBuf, strlen(sBuf), sTdk);
            SET_STR_KEY(pstJson, "track3", sTrack3);
            break;
        case 37: /* 用法1: 收单系统流水号(原交易流水号) */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "o_rrn", sBuf);
            break;
        case 38: /* 授权码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "auth_code", sBuf);
            break;
        case 39: /* 应答码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            //SET_STR_KEY(pstJson, "resp_code", sBuf);
            /* 只有冲正上送39域，但是cups不需要上送 */
            break;
        case 41: /* 终端编号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "term_id", sBuf);
            break;
        case 42: /* 商户编号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "merch_id", sBuf);
            break;
        case 44: /* 支付渠道 001京东 002微信 003支付宝 */
            /* 在外层处理 */
            //            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            //            if (iRet < 0)
            //                return ( iRet);
            //            SET_STR_KEY(pstJson, "input_mode", sBuf);
            break;
        case 49: /* 交易货币代码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "ccy_code", sBuf);
            break;
        case 52: /* 持卡人密码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, caTmp, sizeof (caTmp));
            if (iRet < 0)
                return ( iRet);
            GET_STR_KEY(pstJson, "input_mode", sInputMode);
            if (sInputMode[2] == '1')
            {
                tBcd2Asc((UCHAR*) sBuf1, (UCHAR*) caTmp, 16, LEFT_ALIGN);
                sBuf1[16] = '\0';
                SET_STR_KEY(pstJson, "pin", sBuf1);
            }
            break;
        case 53: /* 安全控制信息 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "secure_ctrl", sBuf);
            break;
        case 55: /* IC卡数据域 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_INT_KEY(pstJson, "ic_len", iRet);
            tBcd2Asc((UCHAR*) sBuf1, (UCHAR*) sBuf, iRet << 1, LEFT_ALIGN);
            sBuf1[iRet << 1] = '\0';
            SET_STR_KEY(pstJson, "ic_data", sBuf1);
            tLog(DEBUG, "55 Ic[%s].", sBuf1);
            break;

        case 57: /* 终端上送硬件序列号及密文数据 */
            /*
               终端入网认证编号 03 
               设备类型 04 
               终端序列号 05 
               加密随机因子 06 
               硬件序列号密文数据 07 
               应用程序版本号 08              
             */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "pos_info", sBuf);
            tLog(DEBUG, "pos_info[%s].", sBuf);
            break;

        case 58: /* 订单标题 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "order_header", sBuf);
            tLog(DEBUG, "订单标题[%s].", sBuf);
            break;
        case 59: /* 自定义域，用法1: 终端厂家信息20、序列号 */
            /* 终端厂家信息 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /*
                        数据元长度 N3
                                59.1 终端厂家标识 N20
                                59.2 终端序列号sn N30
                                59.3 基站信息 N10
                                59.4 pos程序版本号 N2
                                59.4域上送版本号定义，根据终端类型和通讯方式的不同，自定义如下：
                                CDMA通讯：51 GPRS通讯：52 如有多种机型临时定义。
             */
            /* 厂家信息20 */
            tStrCpy(sBuf1, sBuf, 20);
            tTrim(sBuf1);
            SET_STR_KEY(pstJson, "term_fatory", sBuf1);
            /* 序列号30 */
            tStrCpy(sBuf1, sBuf + 20, 30);
            tTrim(sBuf1);
            SET_STR_KEY(pstJson, "term_sn", sBuf1);
            /* 
             * 59.3和59.4暂不处理
             *  */
            break;
        case 60: /* 自定义域60: 消息类型码、批次号、网络管理信息码 */
            /* 在外层ModuleUnpack里处理 */
            /*
             * 数据元长度 N3
             * 60.1 消息类型码 N2
             * 60.2 批次号 N6
             * 60.3 网络管理信息码 N3
             * 60.4 终端读取能力 N1
             * 60.5 基于PBOC借 / 贷记标准的IC卡条件代码 N1
             */
            break;
        case 61: /* 原始交易信息: 原始交易批次号6位、原始交易POS流水号位、原始交易日期4位 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /*
                        数据元长度 N3
                                61.1 原始交易批次号 N6
                                61.2 原始交易POS流水号 N6
                                61.3 原始交易日期 N4
                                61.4原交易授权方式 N2
                                61.5原交易授权机构代码 N11
             */
            tStrCpy(sBuf1, sBuf, 6);
            SET_STR_KEY(pstJson, "o_batch_no", sBuf1);
            tStrCpy(sBuf1, sBuf + 6, 6);
            SET_STR_KEY(pstJson, "o_trace_no", sBuf1);
            tStrCpy(sBuf1, sBuf + 6 + 6, 4);
            SET_STR_KEY(pstJson, "o_trans_date", sBuf1);
            break;
        case 62: /* 自定义域 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /* 用法1: IC卡状态上送 */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            tLog(DEBUG, "trans_code=%s", sTransCode);
            if (!memcmp(sTransCode, "027800", 6)
                    || !memcmp(sTransCode, "027900", 6))
            {
                SET_STR_KEY(pstJson, "62_req_data", sBuf);
            }/* 用法2: IC卡公钥、参数、黑名单下载 */
            else if (!memcmp(sTransCode, "028100", 6)
                    || !memcmp(sTransCode, "028200", 6)
                    || !memcmp(sTransCode, "028300", 6)
                    || !memcmp(sTransCode, "028400", 6)
                    || !memcmp(sTransCode, "028500", 6)
                    || !memcmp(sTransCode, "028600", 6)
                    || !memcmp(sTransCode, "029500", 6))
            {
                SET_INT_KEY(pstJson, "62_req_len", iRet);
                tBcd2Asc((UCHAR*) sBuf1, (UCHAR*) sBuf, iRet << 1, LEFT_ALIGN);
                SET_STR_KEY(pstJson, "62_req_data", sBuf1);
            } else if (!memcmp(sTransCode, "02A000", 6))
            {
                tStrCpy(sCtype, sBuf, 2);
                if (!memcmp(sCtype, "01", 2))
                {
                    tStrCpy(sBuf2, sBuf + 2, 18);
                    SET_STR_KEY(pstJson, "cer_no", sBuf2);
                    tStrCpy(sBuf2, sBuf + 20, 11);
                    tTrim(sBuf2);
                    SET_STR_KEY(pstJson, "mobile", sBuf2);
                    SET_STR_KEY(pstJson, "account_name", sBuf + 31);
                } else
                {
                    return -1;
                }
            } else if (!memcmp(sTransCode, "MM", 1))
            {
                SET_STR_KEY(pstJson, "merch_order_no", sBuf);
            } else
            {
                SET_STR_KEY(pstJson, "private_data", sBuf);
            }
            break;
        case 63: /* 自定义域，用法1: 操作员代码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "teller_no", sBuf);
            break;
        case 64: /* MAC */

            memcpy(caMac, g_pcMsg + g_iMsgLen - 8, 8);
            //tLog(DEBUG, "caMac[%s]", caMac);
            /* 组原报文的MAC Buffer */
            memcpy(caMsgBuf, g_pcMsg, g_iMsgLen - 8);
            iLen = g_iMsgLen - 8;
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);

            if (sTak[0] == '\0' && GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
            {
                tLog(ERROR, sErr, "获取终端[%s:%s]的密钥出错.", sMerchId, sTermId);
                return ( MAC_CHK_ERR);
            }
            tLog(DEBUG, "TDK[%s]", sTdk);
            tLog(DEBUG, "TAK[%s]", sTak);
            //if ((iRet = HSM_GenMac(sTak, (UCHAR*) caMsgBuf, iLen, (UCHAR*) caTmp)) < 0) {
            if ((iRet = tHsm_Gen_Pos_Mac(caTmp, sTak, caMsgBuf, iLen)) < 0)
            {
                tLog(ERROR, "调用加密机出错[%d].", iRet);
                return -1;
            }
            tDumpHex("上送", caMac, 8);
            tDumpHex("计算", caTmp, 8);
            if (memcmp(caTmp, caMac, 8))
            {
                SET_STR_KEY(pstJson, "resp_code", "A0");
                {
                    /*打印mac值,好检查*/
                }
                tLog(ERROR, "校验MAC出错.");
                return -1;
            }
            break;
        default:
            tLog(ERROR, "拆解的报文域[%d]不存在", iBitNo);

            break;
    }
    return ( 0);
}

/* pack */
int SetFld(int iBitNo, cJSON *pstJson) {
    char caMsgBuf[MSG_MAX_LEN], caMac[8], sTmp[512] = {0};
    char sBuf[1024] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    char sTak[48 + 1] = {0}, sTdk[48 + 1] = {0}, sTpk[48 + 1] = {0}, sTmk[48 + 1] = {0}, sRespCode[3] = {0};
    char sTransCode[7] = {0}, sAmtType[3] = {0}, sAmtType2[3] = {0}, sAcctType[3] = {0};
    char sCcyCode[4] = {0}, sBalanceSign[2] = {0}, sUsableLimit[13] = {0}, sTotalLimit[12] = {0}, sBalancel[12] = {0};
    char sTmkFlag[1 + 1] = {0};
    double dAmount = 0.0;
    int iLen, iRet;

    switch (iBitNo)
    {
        case 2: /* 主账号 */
            GET_STR_KEY(pstJson, "card_no", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 4: /* 交易金额 */
            GET_DOU_KEY(pstJson, "amount", dAmount);
            snprintf(sTmp, sizeof (sTmp), "%012.f", dAmount);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, 12));
        case 5: /* 订单号url */
            GET_STR_KEY(pstJson, "code_url", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 6: /* 订单号 */
            GET_STR_KEY(pstJson, "qr_order_no", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 11: /* 终端流水号 */
            GET_STR_KEY(pstJson, "trace_no", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 12: /* 交易时间 */
            GET_STR_KEY(pstJson, "trans_time", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 13: /* 交易日期 */
            GET_STR_KEY(pstJson, "trans_date", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf + 4, 4));
        case 14: /* 卡有效期 */
            GET_STR_KEY(pstJson, "card_exp_date", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 15: /* 清算日期 */
            GET_STR_KEY(pstJson, "settle_date", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf + 4, 4));
        case 23: /* 卡片序列号 */
            GET_STR_KEY(pstJson, "card_sn", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 25: /* 服务点条件码 */
            GET_STR_KEY(pstJson, "cond_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 32: /* 收单机构标识 */
            /*GET_STR_KEY(pstJson, "agent_id", sBuf); motofy by gjq 32域为必返域
             * 如果 agent_id  为NULL 则取“acq_id” 的值返回
             */
            GET_STR_KEY(pstJson, "agent_id", sBuf);
            if (NULL == sBuf || sBuf[0] == '\0')
            {
                GET_STR_KEY(pstJson, "acq_id", sBuf);
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 37: /* 用法1: 收单系统流水号 */
            GET_STR_KEY(pstJson, "rrn", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 38: /* 授权码 */
            GET_STR_KEY(pstJson, "auth_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 39: /* 应答码 */
            GET_STR_KEY(pstJson, "resp_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 41: /* 终端编号 */
            GET_STR_KEY(pstJson, "term_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 42: /* 商户编号 */
            GET_STR_KEY(pstJson, "merch_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 44: /* 接收/收单结构 */
            GET_STR_KEY(pstJson, "iss_id", sBuf);
            snprintf(sTmp, sizeof (sTmp), "%-11s%-11s", sBuf, "49000000");
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, strlen(sTmp)));
        case 49: /* 交易货币代码 */
            GET_STR_KEY(pstJson, "ccy_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 53: /* 安全控制信息 */
            GET_STR_KEY(pstJson, "secure_ctrl", sBuf);
            sBuf[2] = '0';
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 54: /* 附加金额 */
            /*
             * 用法一：账户可用余额
            LENGTH n3 数据元长度
            AVAILABLE - BALANCE - AMOUNT 账户上的可用余额，记录内容为：
                    ACCOUNT - TYPE n2 账户类型
                    AMOUNT - TYPE n2 数量类型
                    CURRENCY - CODE an3 货币代码
                    AMOUNT SIGN an1 余额符号
                    AMOUNT n12 余额
             * 用法二：累计授权金额
             * 用法三：日结额度
             * 2组数据，第一组可用余额，第二组总额度
             * 每组数据元
             * ACCOUNT - TYPE n2 账户类型
                AMOUNT - TYPE n2 数量类型
                CURRENCY - CODE an3 货币代码
                AMOUNT SIGN an1 余额符号
                AMOUNT n12 余额
             */

            GET_STR_KEY(pstJson, "resp_code", sBuf);
            if (NULL == sBuf)
            {
                return 0;
            }
            if (memcmp(sBuf, "00", 2))
                return 0;
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            GET_STR_KEY(pstJson, "amt_type", sAmtType);
            GET_STR_KEY(pstJson, "acct_type", sAcctType);
            GET_STR_KEY(pstJson, "ccy_code", sCcyCode);
            GET_STR_KEY(pstJson, "balance_sign", sBalanceSign);
            GET_STR_KEY(pstJson, "balance1", sBalancel);
            /* t0额度查询 */
            if (!memcmp(sTransCode, "02A100", 6))
            {
                GET_STR_KEY(pstJson, "amt_type2", sAmtType2);
                GET_STR_KEY(pstJson, "usable_limit", sUsableLimit);
                GET_STR_KEY(pstJson, "total_limit", sTotalLimit);
                sprintf(sBuf, "%2s%2s%3s%s%12s", sAmtType, sAcctType, \
			  sCcyCode, sBalanceSign, sUsableLimit);
                iLen = strlen(sBuf);
                sprintf(sBuf + iLen, "%2s%2s%3s%s%12s", sAmtType2, sAcctType,  \
				sCcyCode, sBalanceSign, sTotalLimit);
            } else
            {
                sprintf(sBuf, "%2s%2s%3s%s%12s", sAcctType, sAmtType, \
                        sCcyCode, sBalanceSign, sBalancel);
            }


            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));

        case 55: /* IC卡数据域 */
            GET_STR_KEY(pstJson, "ic_data", sBuf);
            if ('\0' == sBuf[0])
                return 0;
            iLen = strlen(sBuf);
            tAsc2Bcd((UCHAR*) sTmp, (UCHAR*) sBuf, iLen, LEFT_ALIGN);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, iLen / 2));
        case 59: /* 自定义域，用法2: 卡名称 */
        {
            /* 后继处理 */
        }
            return 0;
        case 60: /* 自定义域60: 消息类型码、批次号、网络管理信息码 */
            /* 在外层ModulePack里处理 */
            return 0;
        case 62: /* 自定义域 */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            tLog(DEBUG, "trans_code[%s]", sTransCode);
            /* 用法1: 终端密钥 */
            if (!memcmp(sTransCode, "029000", 6))
            {
                GET_STR_KEY(pstJson, "resp_code", sRespCode);
                if (0 != memcmp(sRespCode, "00", 2))
                    return ( 0);
                GET_STR_KEY(pstJson, "tpk_key", sTpk);
                GET_STR_KEY(pstJson, "tak_key", sTak);
                GET_STR_KEY(pstJson, "tdk_key", sTdk);
                GET_STR_KEY(pstJson, "tmk_key", sTmk);
                GET_STR_KEY(pstJson, "tmk_flag", sTmkFlag);

                tLog(DEBUG, "tak[%s]tpk[%s]tdk[%s]tmk[%s]", sTak, sTpk, sTdk, sTmk);
                tStrCpy(sBuf, "01", 2);
                tStrCpy(sBuf + 2, sTak, 48);
                tStrCpy(sBuf + 50, "02", 2);
                tStrCpy(sBuf + 52, sTpk, 48);
                tStrCpy(sBuf + 100, "03", 2);
                tStrCpy(sBuf + 102, sTdk, 48);
                if (sTmkFlag[0] == '1')
                {
                    tStrCpy(sBuf + 150, "00", 2);
                    tStrCpy(sBuf + 152, sTmk, 48);
                }
                tLog(ERROR, "sKey[%s]", sBuf);
                iLen = strlen(sBuf);
                tAsc2Bcd((UCHAR*) caMsgBuf, (UCHAR*) sBuf, iLen, LEFT_ALIGN);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, caMsgBuf, (iLen + 1) / 2));
                return 0;
            }/* 用法2: IC卡状态上送 */
            else if (!memcmp(sTransCode, "027800", 6)
                    || !memcmp(sTransCode, "027900", 6)
                    || !memcmp(sTransCode, "028100", 6)
                    || !memcmp(sTransCode, "028200", 6)
                    || !memcmp(sTransCode, "028300", 6)
                    )
            {
                GET_INT_KEY(pstJson, "62_len", iLen);
                GET_STR_KEY(pstJson, "62_data", sBuf);
                tLog(DEBUG, "[%d][%s].", iLen, sBuf);
                tAsc2Bcd((UCHAR*) sTmp, (UCHAR*) sBuf, iLen, LEFT_ALIGN);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, iLen / 2));
            } else if (!memcmp(sTransCode, "028500", 6)
                    || !memcmp(sTransCode, "028600", 6)
                    || !memcmp(sTransCode, "028400", 6))
            {
                GET_INT_KEY(pstJson, "62_len", iLen);
                GET_STR_KEY(pstJson, "62_data", sBuf);
                tLog(DEBUG, "[%d][%s].", iLen, sBuf);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, iLen));
            } else if (!memcmp(sTransCode, "029400", 6))
            {
                /* 参数传递-菜单下载 
                编码 含义 格式 取值
                        11 支持的交易类型 N32 1010 0000 0000 0000 0000 0000 0000 0000
                        12 商户名称 ANS40
                        13 广告语 ANS120
                 */
                GET_STR_KEY(pstJson, "bitmap", sTmp);
                snprintf(sBuf, sizeof (sBuf), "11%-32s", sTmp);
                iLen = 2 + 32;
                MEMSET(sTmp);
                GET_STR_KEY(pstJson, "merch_p_name", sTmp);
                snprintf(sBuf + iLen, sizeof (sBuf), "12%-40s", sTmp);
                iLen += 2 + 40;
                MEMSET(sTmp);
                GET_STR_KEY(pstJson, "advert", sTmp);
                snprintf(sBuf + iLen, sizeof (sBuf), "13%-120s", sTmp);
                iLen += 2 + 120;
                sBuf[iLen] = '\0';
                tLog(DEBUG, "029400,len[%d]", iLen);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, iLen));
            } else
                return 0;
        case 63: /* 自定义域，用法2: 国际信用卡公司代码 */
            GET_STR_KEY(pstJson, "resp_desc", sBuf);
            tStrCpy(sTmp, "   ", 3);
            tStrCpy(sTmp + 3, sBuf, strlen(sBuf));
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, strlen(sTmp)));
        case 64: /* MAC */
            /* 组报文的MAC Buffer */
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);
            tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8);
            if ((iLen = tIso2Str(&g_stIsoData, (UCHAR*) caMsgBuf, sizeof (caMsgBuf))) < 0)
                return ( -1);
            if (GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
            {
                tLog(ERROR, "获取终端[%s:%s]的密钥出错,放弃交易.", sMerchId, sTermId);
                return ( -1);
            }
            //if ((iRet = HSM_GenMac(sTak, (UCHAR*) caMsgBuf, iLen - 8, (UCHAR*) caMac)) < 0) {
            if ((iRet = tHsm_Gen_Pos_Mac(caMac, sTak, caMsgBuf, iLen - 8)) < 0)
            {
                tLog(ERROR, "生成MAC出错[%d].", iRet);
                return ( -1);
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8));

        default:
            tLog(ERROR, "组装的报文域[%d]不存在", iBitNo);
    }
    return 0;
}

int DecpcMsg(char *pcClearTrack, char *pcDecTrack, int iDecTrackLen, char *pcTdkuLmk) {
    int iRet = -1;
    char sBlock[2048] = {0}, sTmp[1024] = {0};
    tLog(DEBUG, "Track Data[%d][%s]", iDecTrackLen, pcDecTrack);
    tLog(DEBUG, "Track Len[%d]", iDecTrackLen);
    //memcpy(sBlock, pcDecTrack, iDecTrackLen);
    /*
    for (i = 0; i < iDecTrackLen; i++) {
        snprintf(sBlock + i * 2, sizeof (sBlock), "%02X", (unsigned char) pcDecTrack[i]);
    }
     * */
    tBcd2Asc((unsigned char *) sBlock, (unsigned char *) pcDecTrack, iDecTrackLen << 1, LEFT_ALIGN);
    //iRet = HSM_Dec_Pcmsg(sTmp, sBlock, iDecTrackLen, pcTdkuLmk);
    iRet = tHsm_Dec_Data(sTmp, pcTdkuLmk, sBlock);
    if (iRet < 0)
        return -1;
    tLog(DEBUG, "Track Clear[%s]", sTmp);
    tAsc2Bcd((unsigned char *) pcClearTrack, (unsigned char *) sTmp, iDecTrackLen << 1, LEFT_ALIGN);

    return 0;
}

/* 处理请求，拆包 */
int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {
    int i, iRet = -1, iHeadLen = 0;
    char sMsgId[5] = {0}, sProcCode[9] = {0}, sNmiCode[4] = {0}, sMsgType[3] = {0};
    char sHead[12 + 1] = {0};
    char sBindInfo[16 + 1] = {0}, sMsgData[17] = {0}, sSweepType[3 + 1] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sBatchNo[BATCH_NO_LEN + 1] = {0}, sTpdu[10 + 1] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0}, sTrace[TRACE_NO_LEN + 1] = {0}, sLogicDate[8 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sTak[33] = {0}, sTdk[33] = {0}, sBuf[2048] = {0}, sBuf1[2048] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0};
    char sTmpMsg[MSG_MAX_LEN] = {0}, sTmp[128] = {0};
    UCHAR *pcBitMap = NULL;
    UCHAR caLriHead[5] = {0x4c, 0x52, 0x49, 0x00, 0x1c};
    UCHAR caLriData[5]; /* 网控器LRI报文头      */
    UCHAR caLriHead2[3] = {0xAA, 0xAA, 0xAA}; /* 全包文加密      */
    UCHAR caLriData2[3]; /*    全包文加密   */
    //TransCode stTransCode;

    GET_STR_KEY(pstDataJson, "msg", sTmpMsg);
    int iMsgLen = strlen(sTmpMsg);

    tAsc2Bcd(sMsg, (UCHAR*) sTmpMsg, iMsgLen, LEFT_ALIGN);
    memcpy(caLriData, (sMsg + 5), 5);
    tDumpHex("网", (char *) caLriData, 5);
    memcpy(caLriData2, (sMsg + 8), 3);
    tDumpHex("加密", (char *) caLriData2, 3);
    /* tpdu */
    memcpy(sTmp, sMsg, 5);
    tBcd2Asc((UCHAR*) sTpdu, (UCHAR*) sTmp, 10, LEFT_ALIGN);
    SET_STR_KEY(pstReqJson, "tpdu", sTpdu);

    /* == 有线POS报文结构(走网控器)  == */
    /* 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25   **
     **       [-- TPDU 5B--] [--  LRI报文头      33B --                              **
     **                      [ LRI首标志5B] [ TEL1 8B 主叫号码    ] [ TEL2 8B 被叫   **
     **                      4c 52 49 00 1c                                          **
     **                                                                              **
     ** 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50   **
     **       -- LRI报文头         33B --          ]                                 **
     ** TEL2 8B] 'X'[L3 ] 'I'[L4 ] [ NODE 6B       ] [ ISO8583 MSG                   **
     **          58 00 09 49 00 06                                                   **
     **                                                                              */
    if (!memcmp(caLriData, caLriHead, 5))
    {
        iHeadLen = 44; /* 5+6+33 */
        g_pcMsg = (char*) sMsg + iHeadLen;
        g_iMsgLen = iMsgLen / 2 - iHeadLen;

        /* 报文头 */
        MEMSET(sTmp);
        memcpy(sTmp, sMsg + 5 + 33, 6);
        tBcd2Asc((UCHAR*) sHead, (UCHAR*) sTmp, 12, LEFT_ALIGN);
        SET_STR_KEY(pstReqJson, "head", sHead);

        /* 获取终端绑定信息 */
        memcpy(sTmp, sMsg + 12, 8);
        tBcd2Asc((UCHAR*) sBindInfo, (UCHAR*) sTmp, 16, LEFT_ALIGN);
        SET_STR_KEY(pstReqJson, "bind_info", sBindInfo);
    }/* == 无线POS报文结构(不走网控器) == */
    else if (!memcmp(caLriData2, caLriHead2, 3))
    {
        iHeadLen = 5 + 3 + 3 + 15 + 8; /* 5+3(头)+3(软件版本号,放AAAAAA)+15+8 */
        g_pcMsg = (char*) sMsg + iHeadLen;
        g_iMsgLen = iMsgLen / 2 - iHeadLen;
        memcpy(sMerchId, sMsg + 5 + 3 + 3, 15);
        memcpy(sTermId, sMsg + 5 + 3 + 3 + 15, 8);
        tLog(INFO, "全报文加密:merchId[%s]termId[%s].", sMerchId, sTermId);
        if (GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
        {
            tLog(ERROR, "获取终端[%s:%s]的密钥出错.", sMerchId, sTermId);
            return ( -1);
        }
        tLog(DEBUG, "tdk[%s],tak[%s].", sTdk, sTak);
        memset(sBuf, 0, sizeof (sBuf));
        memset(sBuf1, 0, sizeof (sBuf1));
        memcpy(sBuf, g_pcMsg, g_iMsgLen);
        DecpcMsg(sBuf1, sBuf, g_iMsgLen, sTdk);
        tDumpHex("全报文解密", sBuf1, g_iMsgLen);
        g_pcMsg = sBuf1;
        /* 报文头 */
        MEMSET(sTmp);
        memcpy(sTmp, sMsg + 5, 3);
        tBcd2Asc((UCHAR*) sHead, (UCHAR*) sTmp, 6, LEFT_ALIGN);
        strcat(sHead + 6, "000000");
        tLog(DEBUG, "head:%s", sHead);
        SET_STR_KEY(pstReqJson, "head", sHead);
    } else
    {
        iHeadLen = 11; /* 5+6 */
        g_pcMsg = (char*) sMsg + iHeadLen;
        g_iMsgLen = iMsgLen / 2 - iHeadLen;
        /* 报文头 */
        MEMSET(sTmp);
        memcpy(sTmp, sMsg + 5, 6);
        tBcd2Asc((UCHAR*) sHead, (UCHAR*) sTmp, 12, LEFT_ALIGN);
        SET_STR_KEY(pstReqJson, "head", sHead);
    }
    tClearIso(&g_stIsoData);
    if ((iRet = tStr2Iso(&g_stIsoData, (UCHAR*) g_pcMsg, g_iMsgLen)) < 0)
    {
        tLog(ERROR, "解析8583报文数据错误[%d].", iRet);
        return ( -1);
    }

    tDumpIso(&g_stIsoData, "请求报文");

    tIsoGetBit(&g_stIsoData, 1, sMsgId, sizeof (sMsgId));
    tIsoGetBit(&g_stIsoData, 3, sProcCode, sizeof (sProcCode));
    tTrim(sProcCode);

    /* 解析自定义域60: 消息类型码、批次号、网络管理信息码 */
    tIsoGetBit(&g_stIsoData, 60, sMsgData, sizeof (sMsgData));
    tStrCpy(sMsgType, sMsgData, 2);
    tStrCpy(sBatchNo, sMsgData + 2, 6);
    SET_STR_KEY(pstReqJson, "batch_no", sBatchNo);
    tStrCpy(sNmiCode, sMsgData + 8, 3);
    SET_STR_KEY(pstReqJson, "60_data", sMsgData)

    if (sMsgData[12] == '2')
    {
        SET_STR_KEY(pstReqJson, "invoice_no", "2");
        tLog(ERROR, "fallback标志, invoice_no=2)!");
    }
    /* 使用44域判断二维码通道类型，只有二维码交易的请求交易才使用44域，所以特殊处理 */
    tIsoGetBit(&g_stIsoData, 44, sSweepType, sizeof (sSweepType));
    /* 解析出内部交易类型 */
    if (GetTranType(pstReqJson, sMsgId, sProcCode, sMsgType, sNmiCode, sSweepType) < 0)
    {
        tLog(ERROR, "无法解析的交易类型(MsgId=%s,ProcCode=%s,MsgType=%s, NmiCode=%s,SweepType[%s])!"
                , sMsgId, sProcCode, sMsgType, sNmiCode, sSweepType);
        return TRANS_ERR;
    }
    GET_STR_KEY(pstReqJson, "trans_code", sTransCode);
    tLog(ERROR, "交易类型[%s](MsgId=%s,MsgType=%s,ProcCode=%s,NmiCode=%s,SweepType=%s)!"
            , sTransCode, sMsgId, sProcCode, sNmiCode, sMsgType, sSweepType);

    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++)
    {
        if ((!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6))
                &&(!memcmp(g_staBitMap[i].sMsgType, sMsgType, strlen(sMsgType))))
        {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }
    if (g_staBitMap[i].sTransCode[0] == '\0')
    {
        tLog(ERROR, "交易[%s][%s]未定义解包位图.", sTransCode, sMsgType);
        return TRANS_ERR;
    }
    {
        /* 获取交易日期，时间，rrn，每一个交易都是从manager入和出的 */
        tGetDate(sDate, "", -1);
        tGetTime(sTime, "", -1);
        snprintf(sTmp, sizeof (sTmp), "%s%s", sDate + 5, sTime);
        SET_STR_KEY(pstReqJson, "trans_date", sDate);
        SET_STR_KEY(pstReqJson, "trans_time", sTime);
        sprintf(sTmp, "%s%s", sDate + 4, sTime);
        SET_STR_KEY(pstReqJson, "transmit_time", sTmp);
        if (GetSysTrace(sTrace) < 0)
        {
            tLog(ERROR, "获取系统流水号失败,交易放弃.");
            return -1;
        }
        sprintf(sTmp, "%s%s", sTime, sTrace);
        SET_STR_KEY(pstReqJson, "rrn", sTmp);
        SET_STR_KEY(pstReqJson, "sys_trace", sTrace);
        if (FindValueByKey(sLogicDate, "LOGIC_DATE") < 0)
        {
            tLog(ERROR, "获取逻辑日期失败.");
            return -1;
        }
        SET_STR_KEY(pstReqJson, "logic_date", sLogicDate);
    }
    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++)
    {
        tLog(DEBUG, "GetBit域[%d].", pcBitMap[i]);
        iRet = GetFld(pcBitMap[i], pstReqJson);
        if (iRet < 0)
        {
            tLog(ERROR, "拆解交易[%s][%s]报文域[%d]出错[%d]!"
                    , sTransCode, sMsgType, pcBitMap[i], iRet);
            return ( iRet);
        }
    }
    /* 收单机构初始化赋值 */
#if 0
    GET_STR_KEY(pstReqJson, "acq_id", sAcqInstId);
    if (NULL == sAcqInstId || sAcqInstId[0] == '\0')
    {

        SET_STR_KEY(pstReqJson, "acq_id", "00000000");
        tLog(DEBUG, "机构未上送32域,赋默认值[%s]", "00000000");
    }

    //DUMP_JSON(pstReqJson);
    /* 添加流水 交易码第三位是字母的使用INLINE_TRANS_DETAIL，数字的使用POS_TRANS_DETAIL*/
    if (FindTransCode(&stTransCode, sTransCode) < 0)
    {
        tLog(ERROR, "交易码未定义,交易放弃处理.");
        return -1;
    }
    if (TRUE_FLAG == stTransCode.sLogFlag[0])
    {
        if (!strcmp(stTransCode.sGroupCode, POS_GROUP))
        {
            if (AddPosTransLs(pstReqJson) < 0)
            {
                tLog(ERROR, "流水记录失败,交易放弃处理.");
                return -1;
            }
        } else if (!strcmp(stTransCode.sGroupCode, INLINE_GROUP))
        {
            if (AddInlineTransLs(pstReqJson) < 0)
            {
                tLog(ERROR, "流水记录失败,交易放弃处理.");

                return -1;
            }
        }
    }
#endif
    //DUMP_JSON(pstReqJson);
    return 0;
}

//int ModulePack(char *pcMsg, int *piMsgLen, void *pvJson) {

int ResponseMsg(cJSON *pstRepJson, cJSON *pstDataJson) {
    int i, iRet = -1;
    char sMsgId[5] = {0}, sProcCode[9] = {0}, sNmiCode[4] = {0}, sMsgType[3] = {0}, sTmp[2048] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0};
    char sHead[12 + 1] = {0};
    UCHAR *pcBitMap = NULL;
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sReqFlag[1 + 1] = {0}, sBatchNo[BATCH_NO_LEN + 1] = {0}, sTpdu[10 + 1] = {0};
    //TransCode stTransCode;
    char sIstRespCode[2 + 1] = {0}, sRespCode[2 + 1] = {0};

    //DUMP_JSON(pstDataJson);
    GET_STR_KEY(pstDataJson, "istresp_code", sIstRespCode);
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    /* 如果交易异常返回，无应答码可能会替换resp_code */
    if (sRespCode[0] == '\0')
    {
        tLog(ERROR, "无应答码,交易放弃返回.");
        return -1;
    }


    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
#if 0 
    /* 添加流水 交易码第三位是字母的使用INLINE_TRANS_DETAIL，数字的使用POS_TRANS_DETAIL*/
    if (FindTransCode(&stTransCode, sTransCode) < 0)
    {
        tLog(ERROR, "交易码未定义,交易放弃处理.");
        return TRANS_ERR;
    }
    /* 更新流水 交易码第三位是字母的使用INLINE_TRANS_DETAIL，数字的使用POS_TRANS_DETAIL*/
    if (TRUE_FLAG == stTransCode.sLogFlag[0])
    {
        if (!strcmp(stTransCode.sGroupCode, POS_GROUP))
        {
            if (UpdPosTransLs(pstDataJson) < 0)
            {
                tLog(ERROR, "更新POS交易流水结果失败,放弃回送.");
                return -1;
            }
        } else if (!strcmp(stTransCode.sGroupCode, INLINE_GROUP))
        {
            if (UpdInlineTransLs(pstDataJson) < 0)
            {
                tLog(ERROR, "更新INLINE交易流水结果失败,放弃回送.");
                return -1;
            }
        }
    }
#endif
    if (memcmp(sIstRespCode, "00", 2) == 0 && 0 != memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "银联返回成功，本系统返回失败,交易放弃处理.");
        return TRANS_ERR;
    }

    tClearIso(&g_stIsoData);
    if (GetMsgData(sTransCode, NULL, sMsgId, sProcCode, sMsgType, sNmiCode) < 0)
    {
        tLog(ERROR, "无法组装的交易类型(TransCode=[%s])!", sTransCode);
        return TRANS_ERR;
    }
    tLog(ERROR, "交易类型[%s](MsgId=%s,MsgType=%s,ProcCode=%s,NmiCode=%s)!"
            , sTransCode, sMsgId, sProcCode, sNmiCode, sMsgType);

    /* 应答消息标记 */
    sMsgId[2]++;
    tIsoSetBit(&g_stIsoData, 1, sMsgId, 4);
    if (sProcCode[0])
    {
        tIsoSetBit(&g_stIsoData, 3, sProcCode, 6);
    }

    /* 组包自定义域60: 消息类型码、批次号、网络管理信息码 */
    GET_STR_KEY(pstDataJson, "batch_no", sBatchNo);
    snprintf(sTmp, sizeof (sTmp), "%2s%6s%3s", sMsgType, sBatchNo, sNmiCode);
    tIsoSetBit(&g_stIsoData, 60, sTmp, 11);

    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++)
    {
        if (!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)
                &&!memcmp(g_staBitMap[i].sMsgType, sMsgType, strlen(sMsgType)))
        {
            pcBitMap = g_staBitMap[i].pcPackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0')
    {
        tLog(ERROR, "交易[%s][%s]未定义组包位图.", sTransCode, sMsgType);
        return TRANS_ERR;
    }

    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++)
    {
        tLog(DEBUG, "SetBit域[%d].", pcBitMap[i]);
        iRet = SetFld(pcBitMap[i], pstDataJson);
        if (iRet < 0)
        {
            tLog(ERROR, "组装交易[%s][%s]报文域[%d]出错[%d]!"
                    , sTransCode, sMsgType, pcBitMap[i], iRet);
            return ( iRet);
        }
    }
    /* 预留2位长度+5位tpdu+6报文头 */
    if ((iRet = tIso2Str(&g_stIsoData, (UCHAR*) sTmp, sizeof (sTmp))) < 0)
    {
        tLog(ERROR, "构建8583报文错误[%d]", iRet);
        return ( -1);
    }

    tBcd2Asc((UCHAR*) sMsg + 22, (UCHAR*) sTmp, iRet << 1, LEFT_ALIGN);

    /* TPDU源地址目的地址转换 */
    GET_STR_KEY(pstDataJson, "tpdu", sTpdu);
    tStrCpy(sTmp, sTpdu + 2, 4);
    memcpy(sTpdu + 2, sTpdu + 6, 4);
    memcpy(sTpdu + 6, sTmp, 4);

    tDumpIso(&g_stIsoData, "应答报文");

    /* 签到应答填写处理要求 */
    GET_STR_KEY(pstDataJson, "req_flag", sReqFlag);
    {
        /* 处理要求先不做 */
    }

    memcpy(sMsg, sTpdu, 10);
    GET_STR_KEY(pstDataJson, "head", sHead);
    memcpy(sMsg + 10, sHead, 12);
    SET_STR_KEY(pstRepJson, "msg", (const char*) sMsg);
    return 0;
}


