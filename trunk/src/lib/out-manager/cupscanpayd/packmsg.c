/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "t_extiso.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "trans_type_tbl.h"
#include "t_cjson.h"





/* 交易位图 */

static unsigned char g_UnOrderJsonpackMap[] = {\
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 24, 255};
static unsigned char g_OrderJsonpackMap[] = {\
1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 16, 17, 18, 20, 21, 22, 26, 255};
static unsigned char g_UnconJsonpackMap[] = {\
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 24, 255};
static unsigned char g_conJsonpackMap[] = {\
1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 16, 17, 18, 20, 21, 22, 26, 255};
static unsigned char g_UnAuthenpackMap[] = {\
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 23, 24, 25, 255};
static unsigned char g_AuthenpackMap[] = {\
1, 5, 10, 11, 16, 17, 18, 20, 25, 255};
static unsigned char g_UnQuotapackMap[] = {\
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 255};
static unsigned char g_QuotapackMap[] = {\
1, 2, 5, 10, 11, 12, 13, 14, 16, 17, 18, 255};
static unsigned char g_UnRquotapackMap[] = {\
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 255};
static unsigned char g_RquotapackMap[] = {\
1, 10, 11, 12, 16, 17, 18, 255};
static unsigned char g_UnRevokepackMap[] = {\
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 24, 255};
static unsigned char g_RevokepackMap[] = {\
1, 2, 5, 10, 11, 13, 15, 16, 17, 18, 19, 20, 23, 24, 255};


TranBitMap g_staBitMap[] = {
    /* 交易代码   原交易代码  原原交易码 拆包域位图         组包域位图          */
    { "100", "", "", g_UnOrderJsonpackMap, g_OrderJsonpackMap}, /* 主扫模式 */
    { "200", "", "", g_UnconJsonpackMap, g_conJsonpackMap}, /* 被扫模式 */
    { "300", "", "", g_UnAuthenpackMap, g_AuthenpackMap}, /* 支付查询 */
    { "400", "", "", g_UnQuotapackMap, g_QuotapackMap}, /* 退款     */
    { "500", "", "", g_UnRquotapackMap, g_RquotapackMap}, /* 退款查询 */
    { "600", "", "", g_UnRevokepackMap, g_RevokepackMap}, /* 撤销   */

    { "", "", "", NULL, NULL}
};

/* unpack */
int GetFld(int iBitNo, cJSON *pstQrcode, cJSON *jsonStr) {

    char sBuf[200] = {0};
    double dAmount = 0.0;

    switch (iBitNo) {

        case 1: /* 交易金额 */

            GET_STR_KEY(jsonStr, "amount", sBuf);
            dAmount = (atof(sBuf))*100;
            SET_DOU_KEY(pstQrcode, "amount", dAmount);
            return iBitNo;

        case 2: /* 支付用户名称，京东支付宝返回 */

            GET_STR_KEY(jsonStr, "buyer_logon_id", sBuf);
            SET_STR_KEY(pstQrcode, "buyer_logon_id", sBuf);

            return iBitNo;

        case 3: /* 二维码图片 */

            GET_STR_KEY(jsonStr, "code_img_url", sBuf);
            SET_STR_KEY(pstQrcode, "code_img_url", sBuf);
            return iBitNo;
        case 4: /* 二维码链接 */

            GET_STR_KEY(jsonStr, "code_url", sBuf);

            SET_STR_KEY(pstQrcode, "code_url", sBuf);
            return iBitNo;

        case 5: /*	错误信息 */


            GET_STR_KEY(jsonStr, "err_msg", sBuf);
            SET_STR_KEY(pstQrcode, "err_msg", sBuf);
            return iBitNo;

        case 6:/*货币种类，微信支付宝返回*/

            GET_STR_KEY(jsonStr, "fee_type", sBuf);
            SET_STR_KEY(pstQrcode, "fee_type", sBuf);
            return iBitNo;

        case 7: /* 说明 */

            GET_STR_KEY(jsonStr, "note", sBuf);
            SET_STR_KEY(pstQrcode, "note", sBuf);
            return iBitNo;

        case 8: /* 交易类型 */

            GET_STR_KEY(jsonStr, "op_type", sBuf);
            SET_STR_KEY(pstQrcode, "op_type", sBuf);
            return iBitNo;

        case 9: /*用户在服务商appid下的唯一标识*/

            GET_STR_KEY(jsonStr, "openid", sBuf);
            SET_STR_KEY(pstQrcode, "op_type", sBuf);
            return iBitNo;

        case 10: /*商户订单号*/

            GET_STR_KEY(jsonStr, "out_trade_no", sBuf);
            SET_STR_KEY(pstQrcode, "out_trade_no", sBuf);
            tLog(ERROR, "返回订单号[%s]", sBuf);
            return iBitNo;

        case 11: /* 交易渠道 */

            GET_STR_KEY(jsonStr, "sweep_type", sBuf);
            SET_STR_KEY(pstQrcode, "sweep_type", sBuf);
            return iBitNo;

        case 12: /* 终端号*/

            GET_STR_KEY(jsonStr, "term_no", sBuf);
            SET_STR_KEY(pstQrcode, "term_id", sBuf);
            return iBitNo;

        case 13: /* 业务结果   */

            GET_STR_KEY(jsonStr, "result_code", sBuf);
            SET_STR_KEY(pstQrcode, "result_code", sBuf);
            return iBitNo;


        case 14: /* 交易流水号 */

            GET_STR_KEY(jsonStr, "rrn", sBuf);
            SET_STR_KEY(pstQrcode, "rrn", sBuf);
            return iBitNo;

        case 15: /* 支付完成时间 */

            GET_STR_KEY(jsonStr, "time_end", sBuf);
            SET_STR_KEY(pstQrcode, "time_end", sBuf);
            return iBitNo;

        case 16:/* 返回状态 */

            GET_STR_KEY(jsonStr, "trade_state", sBuf);
            SET_STR_KEY(pstQrcode, "trade_state", sBuf);
            return iBitNo;

        case 17: /* 交易流水号 */

            GET_STR_KEY(jsonStr, "transaction_id", sBuf);
            SET_STR_KEY(pstQrcode, "transaction_id", sBuf);
            return iBitNo;
        case 18: /* 付款银行，微信返回*/
            GET_STR_KEY(jsonStr, "bank_type", sBuf);
            SET_STR_KEY(pstQrcode, "bank_type", sBuf);
            return iBitNo;
        case 19: /* 退款金额 */

            GET_STR_KEY(jsonStr, "refund_fee", sBuf);
            SET_STR_KEY(pstQrcode, "refund_fee", sBuf);
            return iBitNo;

        case 20: /* 商户退款单号 */

            GET_STR_KEY(jsonStr, "out_refund_no", sBuf);
            SET_STR_KEY(pstQrcode, "out_refund_no", sBuf);
            return iBitNo;

        case 21: /* 退款时间,京东返回 */

            GET_STR_KEY(jsonStr, "refund_time", sBuf);
            SET_STR_KEY(pstQrcode, "refund_time", sBuf);
            return iBitNo;

        case 22: /* 中信银行退款单号，微信支付宝返回 */

            GET_STR_KEY(jsonStr, "refund_id", sBuf);
            SET_STR_KEY(pstQrcode, "refund_id", sBuf);
            return iBitNo;

        case 23: /* 交易码返回 */

            GET_STR_KEY(jsonStr, "trans_code", sBuf);
            SET_STR_KEY(pstQrcode, "trans_code", sBuf);
            return iBitNo;
        case 24: /*银联二维码清算主键settleKey*/
            
            GET_STR_KEY(jsonStr, "settleKey", sBuf);
            SET_STR_KEY(pstQrcode, "settleKey", sBuf);
            return iBitNo;
        case 25: /*银联二维码卡属性 01 - 借记卡 02 - 贷记卡*/    
            
            GET_STR_KEY(jsonStr, "cardAttr", sBuf);
            SET_STR_KEY(pstQrcode, "cardAttr", sBuf);
            return iBitNo;
            
        default:
            tLog(ERROR, "拆解的报文域[%d]不存在", iBitNo);
            break;
    }

    return ( 0);
}

/* pack */
int SetFld(int iBitNo, cJSON *pstQrcode, cJSON *pstOutJson) {
    char sBuf[31] = {0}, sBuf1[31] = {0};
    int iAmount = 0;
    char sAmount[20] = {0};

    switch (iBitNo) {

        case 1: /* 订单号 */
            GET_STR_KEY(pstQrcode, "qr_order_no", sBuf);
            SET_STR_KEY(pstOutJson, "order_no", sBuf);
            return iBitNo;
        case 2: /* 付款金额 */
            GET_INT_KEY(pstQrcode, "amount", iAmount);
            snprintf(sAmount, sizeof (sAmount), "%.2lf", (double) iAmount / 100);
            tTrim(sAmount);
            SET_STR_KEY(pstOutJson, "amount", sAmount);
            return iBitNo;
        case 3: /* 二维码有效时间 */
            GET_STR_KEY(pstQrcode, "expire", sBuf);
            SET_STR_KEY(pstOutJson, "expire_", sBuf);

            return iBitNo;
        case 4: /* 用户身份字符串 */
            GET_STR_KEY(pstQrcode, "seed", sBuf);
            SET_STR_KEY(pstOutJson, "seed", sBuf);
            return iBitNo;
        case 5: /* 商户号或渠道商户号(华夏使用) */
            GET_STR_KEY(pstQrcode, "channel_merch_id", sBuf);
            SET_STR_KEY(pstOutJson, "sub_mer", sBuf);
            return iBitNo;
        case 6: /* 终端号 */
            //GET_STR_KEY(pstQrcode, "term_id", sBuf);
            GET_STR_KEY(pstQrcode, "channel_term_id", sBuf);
            SET_STR_KEY(pstOutJson, "term_no", sBuf);
            return iBitNo;
        case 7: /* 交易标题*/
            GET_STR_KEY(pstQrcode, "order_header", sBuf);
            SET_STR_KEY(pstOutJson, "trade_name", sBuf);
            return iBitNo;
        case 8: /* 交易描述 */
            GET_STR_KEY(pstQrcode, "private_data", sBuf);
            SET_STR_KEY(pstOutJson, "trade_describle", sBuf);
            return iBitNo;
        case 9: /* 扩展信息 */
            GET_STR_KEY(pstQrcode, "extra_info", sBuf);
            SET_STR_KEY(pstOutJson, "extra_info", sBuf);
            return iBitNo;
        case 10: /* 支付渠道标示 */
            GET_STR_KEY(pstQrcode, "sweep_type", sBuf);
            SET_STR_KEY(pstOutJson, "sweep_type", sBuf);
            return iBitNo;
        case 11: /* 渠道类型 */
            GET_STR_KEY(pstQrcode, "op_type", sBuf);
            SET_STR_KEY(pstOutJson, "op_type", sBuf);
            return iBitNo;
        case 12: /* 退款申请码 */
            GET_STR_KEY(pstQrcode, "refund_no", sBuf);
            SET_STR_KEY(pstOutJson, "refund_no", sBuf);
            return iBitNo;
        case 13: /* 退款说明 */
            GET_STR_KEY(pstQrcode, "note", sBuf);
            SET_STR_KEY(pstOutJson, "note", sBuf);
            return iBitNo;
        case 14: /* 退款金额 */
            GET_STR_KEY(pstQrcode, "refund_fee", sBuf);
            SET_STR_KEY(pstOutJson, "refund_fee", sBuf);
            return iBitNo;
        case 15: /* 撤销号 */
            GET_STR_KEY(pstQrcode, "cancel_no", sBuf);
            SET_STR_KEY(pstOutJson, "cancel_no", sBuf);
            return iBitNo;
        case 16: /* 收单系统参考号 */
            GET_STR_KEY(pstQrcode, "rrn", sBuf);
            SET_STR_KEY(pstOutJson, "rrn", sBuf);
            return iBitNo;
        case 17: /*交易码*/
            GET_STR_KEY(pstQrcode, "trans_code", sBuf);
            SET_STR_KEY(pstOutJson, "trans_code", sBuf);
            return iBitNo;
        case 18: /*密钥 appkey*/
            GET_STR_KEY(pstQrcode, "appkey", sBuf);
            SET_STR_KEY(pstOutJson, "appkey", sBuf);
            return iBitNo;
        case 19: /*原交易订单号 origOrderNo*/
            GET_STR_KEY(pstQrcode, "orig_order_no", sBuf);
            SET_STR_KEY(pstOutJson, "origOrderNo", sBuf);
            return iBitNo;
        case 20: /*订单时间 YYYYMMDDHHMMSS*/
            GET_STR_KEY(pstQrcode, "trans_date", sBuf);
            GET_STR_KEY(pstQrcode, "trans_time", sBuf1);
            strcat(sBuf, sBuf1);
            SET_STR_KEY(pstOutJson, "orderTime", sBuf);
            return iBitNo;
        case 21: /*银联商户号*/
            //GET_STR_KEY(pstQrcode, "merch_id", sBuf);channel_merch_id
            GET_STR_KEY(pstQrcode, "channel_merch_id", sBuf);
            SET_STR_KEY(pstOutJson, "merchantNo", sBuf);
            return iBitNo;
        case 22: /*银联商户名称*/
            //GET_STR_KEY(pstQrcode, "merch_name", sBuf);
            GET_STR_KEY(pstQrcode, "channel_merch_name", sBuf);
            SET_STR_KEY(pstOutJson, "merchantName", sBuf);
            return iBitNo;
        case 23: /*订单时间 YYYYMMDDHHMMSS*/
            GET_STR_KEY(pstQrcode, "trans_date", sBuf);
            GET_STR_KEY(pstQrcode, "o_rrn", sBuf1);
            sBuf1[6] = 0;
            strcat(sBuf, sBuf1);
            SET_STR_KEY(pstOutJson, "origOrderTime", sBuf);
            return iBitNo;
        case 24: /* 二维码链接 */
            GET_STR_KEY(pstQrcode, "code_url", sBuf);
            SET_STR_KEY(pstOutJson, "code_url", sBuf);
            return iBitNo;
        case 25: /* 支付类型：主被扫 */
            GET_STR_KEY(pstQrcode, "scan_type", sBuf);
            SET_STR_KEY(pstOutJson, "scan_type", sBuf);
            return iBitNo;
        case 26: /* 银联二维码交易 商户类型 merch_type */
            GET_STR_KEY(pstQrcode, "merch_type", sBuf);
            SET_STR_KEY(pstOutJson, "merch_type", sBuf);
            return iBitNo;
            
        default:
            tLog(ERROR, "组装的报文域[%d]不存在", iBitNo);
    }

    return ( 0);
}

int ResponseMsg(cJSON *pstRepDataJson, cJSON *pstDataJson) {

    int i, iRet;
    char sCode[4] = {0};
    char sTransCode[7] = {0};
    UCHAR *pcBitMap = NULL;
    cJSON *pstJson = NULL;
    char sType[2 + 1] = {0};

    char sJsMsg[4096] = {0}, sOutMsg[4096] = {0};

    /* 打印原始报文 */
    GET_STR_KEY(pstDataJson, "msg", sJsMsg);
    iRet = strlen(sJsMsg);
    tAsc2Bcd((UCHAR*) sOutMsg, (UCHAR*) sJsMsg, iRet, LEFT_ALIGN);
    pstJson = cJSON_Parse(sOutMsg);

    /* 确定交易类型 */
    GET_STR_KEY(pstJson, "op_type", sType);
    if (0 == memcmp(sType, TYPE_A, 2)) {
        strcpy(sCode, "100");
    } else if (0 == memcmp(sType, TYPE_P, 2)) {
        strcpy(sCode, "200");
    } else if (0 == memcmp(sType, TYPE_Q, 2)) {
        strcpy(sCode, "300");
    } else if (0 == memcmp(sType, TYPE_RE, 2)) {
        strcpy(sCode, "600");
    } else {
        tLog(INFO,"sType[%s],无效的交易类型！！！",sType);
    }
#if 0
else if (0 == memcmp(stype, TYPE_R, 2)) {
        strcpy(sCode, "S00001");

    } else if (0 == memcmp(stype, TYPE_RQ, 2)) {
        strcpy(sCode, "S00001");

    } else if (0 == memcmp(stype, TYPE_RE, 2)) {
        strcpy(sCode, "S00001");
    }
#endif

    /* 解析出内部交易类型 */


    tLog(INFO, "交易码[%s]", sCode);
    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if (0 == memcmp(g_staBitMap[i].sTransCode, sCode, 3)) {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "交易[%s]未定义解包位图.", sCode);
        return -1;
    }

    /* 根据Bit Map标准定义解包各数据域 */
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    for (i = 0; pcBitMap[i] != 255; i++) {
        iRet = GetFld(pcBitMap[i], pstRepDataJson, pstJson);
        if (iRet < 0) {
            tLog(ERROR, "拆解交易[%s]报文域[%d]出错[%d]", sTransCode, pcBitMap[i], iRet);
            return ( iRet);
        }
        tLog(INFO, "拆解交易[%s]报文域[%d][%d]", sTransCode, pcBitMap[i], iRet);
    }
    return 0;
}

/* 出口方向: 打包请求报文; 入口方向: 打包应答报文. */
int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {
    int i, iRet;
    UCHAR *pcBitMap;
    char sJsMsg[4096] = {0}, sOutMsg[4096] = {0}, *pcMsg = NULL;

    cJSON * pstOutMsg = NULL;
    char sTransCode[7] = {0}, sOrderNo[31] = {0};

    pstOutMsg = cJSON_CreateObject();
    if (NULL == pstOutMsg) {
        tLog(ERROR, "创建发送Json失败.");
        cJSON_Delete(pstOutMsg);
        return -1;
    }

    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "qr_order_no", sOrderNo);

    /* 确定支付渠道 */
    if (0 == memcmp(sTransCode + 2, "W", 1)) {
        SET_STR_KEY(pstDataJson, "sweep_type", WEIXIN);
    } else if (0 == memcmp(sTransCode + 2, "B", 1)) {
        SET_STR_KEY(pstDataJson, "sweep_type", ZHIFUBAO);
    }

    //    tLog(INFO, "交易码[%s],支付渠道为[%s]", pcTransCode, pcInputMode);
    /* 确定交易类型 */


    if ( 0 == memcmp(sTransCode + 2, "Y100", 4) ) {
        SET_STR_KEY(pstDataJson, "op_type", TYPE_A);
        tLog(INFO, "订单号[%s]主扫模式", sOrderNo);
    } else if ( 0 == memcmp(sTransCode + 2, "Y200", 4) ) {
        SET_STR_KEY(pstDataJson, "op_type", TYPE_P);
        tLog(INFO, "订单号[%s]被扫模式", sOrderNo);
    } else if ( 0 == memcmp(sTransCode + 2, "Y300", 4) ) {
        SET_STR_KEY(pstDataJson, "op_type", TYPE_Q);
        tLog(INFO, "订单号[%s]查询", sOrderNo);
    } else if ( 0 == memcmp(sTransCode + 2, "Y600", 4) ) {
        SET_STR_KEY(pstDataJson, "op_type", TYPE_RE);
        tLog(INFO, "订单号[%s]撤销", sOrderNo);
    }

    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if (0 == memcmp(g_staBitMap[i].sTransCode, sTransCode + 3, 3)) {
            pcBitMap = g_staBitMap[i].pcPackBitmap;
            break;
        }
    }
    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "交易[%s]未定义组包位图.", sTransCode);
        cJSON_Delete(pstOutMsg);
        return -1;
    }

    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++) {
        iRet = SetFld(pcBitMap[i], pstDataJson, pstOutMsg);
        if (iRet < 0) {
            tLog(ERROR, "组装交易[%s]报文域[%d]出错[%d]!", sTransCode, pcBitMap[i], iRet);
            cJSON_Delete(pstOutMsg);
            return ( iRet);
        }
        tLog(INFO, "组装交易[%s]报文域[%d][%d]", sTransCode, pcBitMap[i], iRet);
    }
    /* 将json格式转成字符串 */
    pcMsg = cJSON_PrintUnformatted(pstOutMsg);
    strcpy(sJsMsg, pcMsg);
    if (pcMsg)
        free(pcMsg);
    iRet = strlen(sJsMsg);
    tBcd2Asc((UCHAR*) sOutMsg, (UCHAR*) sJsMsg, iRet << 1, LEFT_ALIGN);
    SET_STR_KEY(pstReqJson, "msg", sOutMsg);
    cJSON_Delete(pstOutMsg);
    return 0;
}