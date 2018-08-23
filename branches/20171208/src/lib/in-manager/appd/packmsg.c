#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include "t_extiso.h"
#include "t_tools.h"
#include "trans_type_tbl.h"


#include "t_redis.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "trans_code.h"



/*************
 **  交易类  **
 *************/

/* 实名认证 */
static unsigned char g_AuthenUnpackMap[] = {\
1, 2, 11, 22, 25, 26, 32, 38, 42, 255};

static unsigned char g_AuthenPackMap[] = {\
11, 12, 13, 37, 39, 63, 255};

/* 日结额度查询 */
static unsigned char g_QuotaUnpackMap[] = {\
1, 11, 42, 255};
static unsigned char g_QuotaPackMap[] = {\
11, 12, 13, 37, 39, 42, 49, 53, 63, 255};

/* 日结消费通知 */
static unsigned char g_caUndoPurchaseUnpackMap[] = {\
1, 11, 37, 41, 42, 60, 255};
static unsigned char g_caUndoPurchasePackMap[] = {\
11, 12, 13, 37, 39, 42, 63, 255};


/* 主扫消费 */
static unsigned char g_UnActivescanpackMap[] = {\
1, 4, 6, 11, 42, 44, 58, 62, 255};
static unsigned char g_ActivescanpackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* 被扫消费 */
static unsigned char g_UnPassivescanpackMap[] = {\
1, 4, 11, 42, 44, 58, 62, 255};
static unsigned char g_PassivescanpackMap[] = {\
4, 5, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* 查询 */
static unsigned char g_UnQueryresultpackMap[] = {\
1, 6, 11, 42, 44, 255};
static unsigned char g_QueryresultpackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* 二维码支付撤销 */
static unsigned char g_UnTdRevokepackMap[] = {\
1, 4, 6, 11, 42, 44, 58, 255};
static unsigned char g_TdRevokepackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* 退款 */
static unsigned char g_UnRefundpackMap[] = {\
1, 4, 6, 11, 42, 44, 58, 255};
static unsigned char g_RefundpackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* 退款查询 */
static unsigned char g_UnQueryRefundpackMap[] = {\
1, 6, 11, 37, 42, 44, 255};
static unsigned char g_QueryRefundpackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* 订单撤销 */
static unsigned char g_UnRevokepackMap[] = {\
1, 4, 6, 11, 42, 44, 58, 255};
static unsigned char g_RevokepackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};



TranBitMap g_staBitMap[] = {
    /* 交易码   消息类型码 拆包域位图                      组包域位图                   */
    { "0AA000", "", g_AuthenUnpackMap, g_AuthenPackMap}, /* 商户实名认证 */
    { "0AA001", "", g_AuthenUnpackMap, g_AuthenPackMap}, /* 交易实名认证 */
    { "0AA002", "", g_AuthenUnpackMap, g_AuthenPackMap}, /* 银行卡实名认证 */
    { "0AA100", "", g_QuotaUnpackMap, g_QuotaPackMap}, /* 日结限额查询  */

    { "0AB100", "", g_UnActivescanpackMap, g_ActivescanpackMap}, /* 支付宝主扫模式 */
    { "0AB200", "", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* 支付宝被扫模式 */
    { "0AB300", "", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* 支付宝支付查询 */
    { "0AB600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap}, /* 支付宝支付撤销 */
    
    //   { "AS0004", "60", g_UnRefundpackMap, g_RefundpackMap}, /* 退款 */
    //   { "AS0005", "60", g_UnQueryRefundpackMap, g_QueryRefundpackMap}, /* 退款查询 */
    //  { "AS0006", "60", g_UnRevokepackMap, g_RevokepackMap}, /* 订单撤销 */

    { "0AW100", "", g_UnActivescanpackMap, g_ActivescanpackMap}, /* 微信主扫模式 */
    { "0AW200", "", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* 微信被扫模式 */
    { "0AW300", "", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* 微信支付查询 */
    { "0AW600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap}, /* 微信支付撤销 */
    
    //    { "AS0004", "60", g_UnRefundpackMap, g_RefundpackMap}, /* 退款 */
    //    { "AS0005", "60", g_UnQueryRefundpackMap, g_QueryRefundpackMap}, /* 退款查询 */
    //    { "AS0006", "60", g_UnRevokepackMap, g_RevokepackMap}, /* 订单撤销 */

    { "TA0010", "", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* 日结消费确认 */
    { "", "", NULL, NULL}
};

int GetFld(int iBitNo, cJSON *pstNetTran, cJSON *jsonStr) {
    char sBuf[1024] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0};
    
    switch (iBitNo) {

        case 1: /* 交易码 */
            GET_STR_KEY(pstNetTran, "transCode", sBuf);
            SET_STR_KEY(jsonStr, "trans_code", sBuf);
            break;

        case 2: /* 鉴权卡号 */
            GET_STR_KEY(pstNetTran, "accountNo", sBuf);
            SET_STR_KEY(jsonStr, "card_no", sBuf);
            break;

        case 4: /* 交易金额 */
            GET_STR_KEY(pstNetTran, "amount", sBuf);
            SET_DOU_KEY(jsonStr, "amount", atol(sBuf));
            break;

            /* 订单号 */
        case 6:/* 订单号 */
            GET_STR_KEY(pstNetTran, "transCode", sTransCode);
            GET_STR_KEY(pstNetTran, "orderNum", sBuf);
            if (!memcmp(sTransCode, "0AW100", 6) || !memcmp(sTransCode, "0AB100", 6)) {
                tLog(DEBUG,"===============seed[%s]",sBuf);
                SET_STR_KEY(jsonStr, "seed", sBuf);
            } else {
                tLog(DEBUG,"===============qr_order_no[%s]",sBuf);
                SET_STR_KEY(jsonStr, "qr_order_no", sBuf);
            }
            break;

        case 11: /* 终端流水号 */
            GET_STR_KEY(pstNetTran, "sellCardRRN", sBuf);
            SET_STR_KEY(jsonStr, "trace_no", sBuf);
            break;

        case 22: /* 卡类型 */
            GET_STR_KEY(pstNetTran, "accountType", sBuf);
            SET_STR_KEY(jsonStr, "card_type", "2");
            break;

        case 25: /* 鉴权预留手机号 */
            GET_STR_KEY(pstNetTran, "idTel", sBuf);
            SET_STR_KEY(jsonStr, "mobile", sBuf);
            break;

        case 26: /* 鉴权身份证号 */
            GET_STR_KEY(pstNetTran, "idNum", sBuf);
            SET_STR_KEY(jsonStr, "cer_no", sBuf);
            break;

        case 32: /* 鉴权姓名 */
            GET_STR_KEY(pstNetTran, "accountName", sBuf);
            SET_STR_KEY(jsonStr, "account_name", sBuf);
            tLog(INFO, "实名姓名[%s]", sBuf);
            break;

        case 37: /* 用法1: 收单系统流水号(原交易流水号) */
            GET_STR_KEY(pstNetTran, "sOldRrn", sBuf);
            SET_STR_KEY(jsonStr, "o_rrn", sBuf);
            break;

        case 38: /* 证件类型 */
            GET_STR_KEY(pstNetTran, "idType", sBuf);
            SET_STR_KEY(jsonStr, "idType", sBuf);
            break;

        case 41: /* 终端编号 */
            GET_STR_KEY(pstNetTran, "termId", sBuf);
            SET_STR_KEY(jsonStr, "term_id", sBuf);
            break;

        case 42: /* 商户编号 */
            GET_STR_KEY(pstNetTran, "cardTermCode", sBuf);
            SET_STR_KEY(jsonStr, "merch_id", sBuf);
            break;

        case 44: /* 支付渠道 001京东 002微信 003支付宝 */
            GET_STR_KEY(pstNetTran, "attachContent", sBuf);
            SET_STR_KEY(jsonStr, "input_module", sBuf);
            break;

        case 58: /* 订单标题 */
            GET_STR_KEY(pstNetTran, "orderTitle", sBuf);
            SET_STR_KEY(jsonStr, "order_header", sBuf);
            tLog(DEBUG, "订单标题[%s].", sBuf);
            break;

        case 60: /* 原交易流水号 */
            GET_STR_KEY(pstNetTran, "oldCardRRN", sBuf);
            SET_STR_KEY(jsonStr, "o_trace_no", sBuf);
            break;

        case 62: /* 交易描述 */
            GET_STR_KEY(pstNetTran, "orderDesc", sBuf);
            SET_STR_KEY(jsonStr, "private_data", sBuf);
            tLog(DEBUG, "62[%s].", sBuf);
            break;

        default:
            tLog(ERROR, "拆解的报文域[%d]不存在", iBitNo);
            break;
    }

    return ( 0);
}

/* pack */
int SetFld(int iBitNo, cJSON *pstNetTran, cJSON *pstOutJson) {
    char sBuf[1024] = {0}, sTmp[512] = {0};
    double dAmount = 0.0;
    switch (iBitNo) {

        case 4: /* 交易金额 */

            GET_DOU_KEY(pstNetTran, "amount", dAmount);
            snprintf(sTmp, sizeof (sTmp), "%012.f", dAmount);
            SET_STR_KEY(pstOutJson, "amount", sTmp);

            return iBitNo;

        case 5: /* 订单号url */
            GET_STR_KEY(pstNetTran, "code_url", sBuf);
            SET_STR_KEY(pstOutJson, "codeurl", sBuf);
            return iBitNo;

        case 6: /* 订单号 */
            GET_STR_KEY(pstNetTran, "qr_order_no", sBuf);
            SET_STR_KEY(pstOutJson, "orderNum", sBuf);
            return iBitNo;

        case 11: /* 终端流水号 */
            GET_STR_KEY(pstNetTran, "trace_no", sBuf);
            SET_STR_KEY(pstOutJson, "sellCardRRN", sBuf);
            return iBitNo;

        case 12: /* 交易时间 */
            GET_STR_KEY(pstNetTran, "trans_time", sBuf);
            SET_STR_KEY(pstOutJson, "sellCardTime", sBuf);
            return iBitNo;

        case 13: /* 交易日期 */
            GET_STR_KEY(pstNetTran, "trans_date", sBuf);
            SET_STR_KEY(pstOutJson, "sellCardDate", sBuf);
            return iBitNo;

        case 37: /* 检索参考号 */
            GET_STR_KEY(pstNetTran, "rrn", sBuf);
            SET_STR_KEY(pstOutJson, "indexRRN", sBuf);
            return iBitNo;

        case 39: /* 应答码 */
            GET_STR_KEY(pstNetTran, "resp_code", sBuf);
            SET_STR_KEY(pstOutJson, "respCode", sBuf);
            return iBitNo;


        case 42: /* 商户编号 */
            GET_STR_KEY(pstNetTran, "merch_id", sBuf);
            SET_STR_KEY(pstOutJson, "cardTermCode", sBuf);
            return iBitNo;

        case 44: /* 支付渠道 001京东 002微信 003支付宝 */
            GET_STR_KEY(pstNetTran, "input_module", sBuf);
            SET_STR_KEY(pstOutJson, "attachContent", sBuf);
            return iBitNo;

        case 49: /* 商户日结额度 */
            GET_STR_KEY(pstNetTran, "total_limit", sBuf);
            SET_STR_KEY(pstOutJson, "maxPosition", sBuf);
            return iBitNo;

        case 53: /* 商户可用额度 */
            GET_STR_KEY(pstNetTran, "usable_limit", sBuf);
            SET_STR_KEY(pstOutJson, "currentPosition", sBuf);
            return iBitNo;

        case 63: /* 应答描述 */
            GET_STR_KEY(pstNetTran, "resp_desc", sBuf);
            SET_STR_KEY(pstOutJson, "respDesc", sBuf);
            return iBitNo;

        default:
            tLog(ERROR, "组装的报文域[%d]不存在", iBitNo);
    }

    return 0;
}

//int ModuleUnpack(char *pcMsg, int iMsgLen, void *pvNetTran) {

int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {
    int i, iRet;
    char sTransCode[7] = {0}, *pcMsg = NULL;
    UCHAR *pcBitMap;
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0}, sTrace[6 + 1] = {0}, sTmp[128] = {0};
    cJSON *pstJson = NULL;
    UCHAR sMsg[MSG_MAX_LEN] = {0};
    TransCode stTransCode;
    GET_STR_KEY(pstDataJson, "msg", sMsg);

    int iMsgLen = strlen((const char *) sMsg);
    pstJson = cJSON_Parse((const char *) sMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "转换消息失败,放弃处理.");
        return -1;
    }

    /* 打印原始报文 */
    DUMP_JSON(pstJson, pcMsg);
    /* 获取jsonStr数据 */

    GET_STR_KEY(pstJson, "transCode", sTransCode);

    tLog(DEBUG, "交易码transcode=[%s]", sTransCode);

    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if (0 == memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)) {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }
    tLog(DEBUG, "TransCode=[%s]", g_staBitMap[i].sTransCode);

    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "交易[%s]未定义解包位图.", sTransCode);
        return -1;
    }

    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++) {
        iRet = GetFld(pcBitMap[i], pstJson, pstReqJson);
        if (iRet < 0) {
            tLog(ERROR, "拆解交易[%s]报文域[%d]出错[%d]!"
                    , sTransCode, pcBitMap[i], iRet);
            return ( iRet);
        }
        tLog(DEBUG, "拆解交易[%s]报文域[%d]", sTransCode, pcBitMap[i]);
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
        if (GetSysTrace(sTrace) < 0) {
            tLog(ERROR, "获取系统流水号失败,交易放弃.");
            return -1;
        }
        sprintf(sTmp, "%s%s", sTime, sTrace);
        SET_STR_KEY(pstReqJson, "rrn", sTmp);
        SET_STR_KEY(pstReqJson, "sys_trace", sTrace);
        SET_STR_KEY(pstReqJson, "logic_date", sDate);
    }

    if (FindTransCode(&stTransCode, sTransCode) < 0) {
        tLog(ERROR, "交易码未定义,交易放弃处理.");
        return -1;
    }

    return 0;
}

//int ModulePack(char *pcMsg, int *piMsgLen, void *pvNetTran) {

int ResponseMsg(cJSON *pstRepDataJson, cJSON *pstDataJson) {
    int i, iRet;
    unsigned char *pcBitMap;
    char sTransCode[7] = {0}, *pcMsg = NULL;
    cJSON * pstOutMsg = NULL;
    TransCode stTransCode;
    pstOutMsg = cJSON_CreateObject();
    if (NULL == pstOutMsg) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);

    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);

    /* 添加流水 交易码第三位是字母的使用INLINE_TRANS_DETAIL，数字的使用POS_TRANS_DETAIL*/
    if (FindTransCode(&stTransCode, sTransCode) < 0) {
        tLog(ERROR, "交易码未定义,交易放弃处理.");
    }
    
    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if (0 == memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)) {
            pcBitMap = g_staBitMap[i].pcPackBitmap;
            break;
        }
    }
    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "交易[%s]未定义组包位图.", sTransCode);
        return -1;
    }
    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++) {
        iRet = SetFld(pcBitMap[i], pstDataJson, pstOutMsg);
        if (iRet < 0) {
            tLog(ERROR, "组装交易[%s]报文域[%d]出错[%d]!", sTransCode, pcBitMap[i], iRet);
            return ( iRet);
        }
        tLog(DEBUG, "组装交易[%s]报文域[%d]", sTransCode, pcBitMap[i]);
    }
    DUMP_JSON(pstOutMsg, pcMsg);
    pcMsg = cJSON_PrintUnformatted(pstOutMsg);
    SET_STR_KEY(pstRepDataJson, "msg", (const char*) pcMsg);
    if (pcMsg)
        free(pcMsg);
    cJSON_Delete(pstOutMsg);
    return ( 0);
}

