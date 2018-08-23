#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"
#include "t_extiso.h"
#include "secapi.h"         /* 加解密   */
#include "trans_type_tbl.h"
#include "t_macro.h"
#include "err.h"
#include "tKms.h"


#define aStrEql( str1, str2 ) \
    ( ( strlen(str1) == strlen(str2) ) && ( ! strcmp( str1, str2 ) ) )
/* dbop.pc */
int GetTermKey(char *pcKeyAsc, char *pcMerchId, char *pcTermId, char *pcKeyName) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select a.%s  from B_TERM a, B_MERCH b "
            " where a.USER_CODE = b.USER_CODE AND A.TERM_ID='%s' AND B.MERCH_ID='%s' ", pcKeyName, pcTermId, pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcKeyAsc);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return ( 0);
}

/* term.pc */
int GetChannelTermKey(char *pcKeyAsc, char *pcKeyName, char * pcChannelMerchId, char * ChannelTermId) {
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT %s FROM B_CHANNEL_TERM WHERE CHANNEL_MERCH='%s' AND CHANNEL_TERM='%s' AND STATUS = '1'", pcKeyName, pcChannelMerchId, ChannelTermId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcKeyAsc);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
}

extern IsoData g_stIsoData;
char *g_pcMsg;
int g_iMsgLen;


TranTbl g_staTranTbl[] = {
    /* 消息ID  处理码    消息类型码  NMI   交易代码  原交易代码  */
    /* 一般类交易 */
    { "0200", "300000", "01", "", "021000", "", ""}, /* POS查余额            */
    { "0200", "190000", "22", "", "020000", "", ""}, /* POS消费              */
    { "0200", "190000", "22", "", "T20000", "", ""}, /* POS消费              */
    { "0400", "190000", "22", "", "020003", "", ""}, /* POS消费冲正          */
    { "0200", "280000", "23", "", "020002", "", ""}, /* POS消费撤销          */
    { "0400", "280000", "23", "", "020023", "", ""}, /* POS消费撤销冲正      */
    { "0800", "", "00", "003", "029000", "", ""}, /* POS签到            */

    { "0620", "300000", "00", "951", "020400", "021000", ""}, /* POS查余额脚本通知*/
    { "0620", "000000", "00", "951", "020400", "020000", ""}, /* POS消费脚本通知*/
    { "", "", "", "", "", "", ""},
};


/*************
 **  交易类  **
 *************/

/* POS查余额 */
static unsigned char g_caInqueryUnpackMap[] = {\
2, 11, 12, 13, 14, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 54, 55, 60, 63, 64, 255};
static unsigned char g_caInqueryPackMap[] = {\
2, 11, 22, 23, 25, 26, 41, 42, 35, 36, 49, 52, 53, 55, 60, 64, 255};

/* POS消费 */
static unsigned char g_caPurchaseUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55, 60, 63, 64, 255};
static unsigned char g_caPurchasePackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 41, 42, 35, 36, 49, 52, 53, 55, 60, 64, 255};

/* POS消费撤销 */
static unsigned char g_caUndoPurchaseUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 59, 55, 60, 63, 64, 255};
static unsigned char g_caUndoPurchasePackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 64, 255};


/* POS撤销冲正 */
static unsigned char g_caCorrectUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 55, 60, 64, 255};
static unsigned char g_caCorrectPackMap[] = {\
4, 11, 14, 22, 23, 25, 38, 39, 41, 42, 49, 53, 55, 60, 61, 64, 255};

/* POS冲正 */
static unsigned char g_caPurchaseRevUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 55, 60, 64, 255};
static unsigned char g_caPurchaseRevPackMap[] = {\
4, 11, 14, 22, 23, 25, 38, 39, 41, 42, 49, 53, 55, 60, 61, 64, 255};


/* POS脚本通知 */
static unsigned char g_caJbtzUnpackMap[] = {\
2, 7, 11, 12, 13, 15, 23, 25, 32, 33, 38, 39, 41, 42, 49, 55, 57, 60, 255};
static unsigned char g_caJbtzPackMap[] = {\
2, 4, 7, 11, 12, 13, 18, 22, 23, 32, 33, 37, 39, 41, 42, 43, 49, 55, 60, 255};

/*************
 **  管理类  **
 *************/

/* 浦发签到 */
static unsigned char g_caLoginUnpackMap[] = {\
11, 12, 13, 32, 37, 39, 41, 42, 60, 62, 255};
static unsigned char g_caLoginPackMap[] = {\
11, 41, 42, 60, 63, 255};



TranBitMap g_staBitMap[] = {
    /* 交易代码   原交易代码  原原交易码 拆包域位图                       组包域位图                      */
    { "021000", "", "", g_caInqueryUnpackMap, g_caInqueryPackMap}, /* 查余额            */
    { "020000", "", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* 消费              */
    { "T20000", "", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* T0消费              */
    { "020003", "", "", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* 消费冲正          */
    { "020002", "", "", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* 消费撤销          */
    { "020023", "", "", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* 消费撤销冲正      */
    { "029000", "", "", g_caLoginUnpackMap, g_caLoginPackMap}, /* 浦发签到    */

    { "020400", "020000", "", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS消费脚本通知          */
    { "020400", "021000", "", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS查余脚本通知          */
    { "020400", "023000", "100000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS脚本通知          */
    { "020400", "023000", "200000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS脚本通知          */
    { "020400", "023000", "300000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS脚本通知          */
    { "", "", "", NULL, NULL}
};

/*
 * 根据POS报文的消息类型码、处理码、扩展处理码、网络管理信息码，在定义表中判断查找交易类型
 */
int GetTranType(char *pcTransCode, char *pcMsgType, char *pcProcCode, char *pcProcCodeEx, char *pcNmiCode) {
    int i;

    /* 查取当前交易类型(根据从发卡机构报文解出的消息ID、处理码、消息类型码、网络管理码) */
    for (i = 0; g_staTranTbl[i].sMsgType[0] != '\0'; i++) {
        if ((!memcmp(g_staTranTbl[i].sMsgType, pcMsgType, strlen(g_staTranTbl[i].sMsgType)))
                && (!memcmp(g_staTranTbl[i].sProcCode, pcProcCode, (g_staTranTbl[i].sProcCode[0] ? 2 : 0))) /* 非空比较前2位 */
                //  && (!memcmp(g_staTranTbl[i].sProcCodeEx, pcProcCodeEx, strlen(g_staTranTbl[i].sProcCodeEx)))
                //  && (!memcmp(g_staTranTbl[i].sNmiCode, pcNmiCode, strlen(g_staTranTbl[i].sNmiCode)))
                ) {

            strcpy(pcTransCode, g_staTranTbl[i].sTransCode);
            tLog(DEBUG, "交易码[%s] 原交易码[%s] 原原交易码[%s]"
                    , g_staTranTbl[i].sTransCode, g_staTranTbl[i].sOldTransCode, g_staTranTbl[i].sOOldTransCode);

            return ( 0);
        }
    }

    return ( -1);
}

/*
 * 根据内部报文的交易代码、原交易代码，在定义表中查找消息类型码、处理码、扩展处理码、网络管理码
 */
int GetMsgData(char *pcTransCode, char *pcOldTransCode, char *pcOOldTransCode,
        char *pcMsgType, char *pcProcCode, char *pcProcCodeEx, char *pcNmiCode) {
    int i;

    for (i = 0; g_staTranTbl[i].sMsgType[0] != '\0'; i++) {
        if (!memcmp(g_staTranTbl[i].sTransCode, pcTransCode, 6)&&!memcmp(g_staTranTbl[i].sOldTransCode, pcOldTransCode, strlen(g_staTranTbl[i].sOldTransCode))) {
            strcpy(pcMsgType, g_staTranTbl[i].sMsgType);
            strcpy(pcProcCode, g_staTranTbl[i].sProcCode);
            strcpy(pcProcCodeEx, g_staTranTbl[i].sProcCodeEx);
            strcpy(pcNmiCode, g_staTranTbl[i].sNmiCode);
            return ( 0);
        }
    }
    return ( -1);
}

/* unpack */
int GetFld(int iBitNo, cJSON *pstJson) {
    char sNumBuf[3] = {0}, sBuf[1024] = {0}, sTak[33] = {0}, caTmp[8] = {0};
    char sAcctType[3] = {0}, sAmtType[3] = {0}, sBalCcyCode[4] = {0}, sBalance1[13] = {0}, sBalanceSign[2] = {0}, sIc[255] = {0};
    UCHAR caMacBuf[1024] = {0}, caMac[8] = {0}, sTransDate[7] = {0}, caMsgBuf[1024] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    char sErr[128] = {0}, sBuf1[128] = {0}, sOldRrn[13] = {0}, sRespDesc[41] = {0};
    char sSecureCtrl[16 + 1] = {0}, sMacKey[16 + 1] = {0};
    int iLen = 0, iRet = -1;

    memset(sNumBuf, 0, sizeof (sNumBuf));
    memset(sBuf, 0, sizeof (sBuf));
    memset(caTmp, 0, sizeof (caTmp));
    switch (iBitNo) {
        case 2: /* 主账号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_no", sBuf);
            break;
        case 4: /* 交易金额 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "amount", sBuf);
            break;
        case 7: /* 交易传输时间 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "transmit_time", sBuf);
            break;
        case 11: /* 系统跟踪号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "sys_trace", sBuf);
            break;
        case 12: /* 交易时间 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "trans_time", sBuf);
            break;
        case 13: /* 交易日期 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /* 交易日期只上送4位MMDD，跨年或跨天时需要-1天 */
            tGetDate(sTransDate, "", -1);
            if (memcmp(sTransDate + 4, sBuf, 4))
                tAddDay(sTransDate, -1);
            tStrCpy(sTransDate + 4, sBuf, 4);
            SET_STR_KEY(pstJson, "trans_date", sBuf);
            break;
        case 14: /* 卡有效期 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_exp_date", sBuf);
            break;
        case 15: /* 清算日期 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "channel_settle_date", sBuf);
            break;
        case 18: /* MCC */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "channel_mcc", sBuf);
            break;
        case 23: /* 卡片序列号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_sn", sBuf);
            break;
        case 25: /* 服务点条件码 */
            /* 在ModuleUnpack中处理, =交易处理码2 */
            return 0;
        case 32: /* 受理机构标识 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "acq_id", sBuf);
            break;
        case 33: /* 发送机构标识 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "fwd_inst_id", sBuf);
            break;
        case 35: /* 2磁道数据 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "track2", sBuf);
            break;
        case 36: /* 3磁道数据 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "track3", sBuf);
            break;
        case 37: /* RRN */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "channel_rrn", sBuf);
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
            SET_STR_KEY(pstJson, "istresp_code", sBuf);
            tLog(DEBUG, "istresp_code=[%s]", sBuf);
            break;
        case 41: /* 终端编号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "channel_term_id", sBuf);
            break;
        case 42: /* 商户编号 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "channel_merch_id", sBuf);
            break;
        case 48: /* 附加数据.用法10: 新密钥(2倍长,3倍长) */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            strncpy(sBuf1, sBuf + 2, 16);
            tBcd2Asc(sBuf, sBuf1, (iRet - 2) << 1, LEFT_ALIGN);
            SET_STR_KEY(pstJson, "data1", sBuf);
            break;
        case 49: /* 交易货币代码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, 3);
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "ccy_code", sBuf);
            break;
        case 53: /* 安全控制信息 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, 16);
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "secure_ctrl", sBuf);
        case 54: /* 账户余额 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tStrCpy(sAcctType, sBuf, 2);
            tStrCpy(sAmtType, sBuf + 2, 2);
            tStrCpy(sBalCcyCode, sBuf + 4, 3);
            sBalanceSign[0] = sBuf[7];
            sBalanceSign[1] = '\0';
            tStrCpy(sBalance1, sBuf + 8, 12);

            SET_STR_KEY(pstJson, "acct_type", sAcctType);
            SET_STR_KEY(pstJson, "amt_type", sAmtType);
            SET_STR_KEY(pstJson, "balccy_code", sBalCcyCode);
            SET_STR_KEY(pstJson, "balance_sign", sBalanceSign);
            SET_STR_KEY(pstJson, "balance", sBalance1);
            break;
        case 55: /* ic卡数据域 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tBcd2Asc(sIc, sBuf, iRet << 1, LEFT_ALIGN);
            SET_INT_KEY(pstJson, "ic_len", iRet);
            SET_STR_KEY(pstJson, "ic_data", sIc);
            break;
        case 57:/*附加域*/
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "57_data", sBuf);
            break;
        case 60: /* 自定义域 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "60_data", sBuf);
            break;
        case 61: /* 原始交易信息: 报文类型、系统跟踪号、系统日期时间、发卡机构标识、收单机构标识 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tStrCpy(sOldRrn, sBuf + 4, 6);
            SET_STR_KEY(pstJson, "o_rrn", sOldRrn);
            break;

        case 62: /* 交换中心数据 终端密钥信息 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "pufa_inst_key", sBuf);
            break;

        case 64: /* MAC */
            memcpy(caMac, g_pcMsg + g_iMsgLen - 8, 8);
            //tLog(DEBUG, "caMac[%s]", caMac);
            /* 组原报文的MAC Buffer */
            memcpy(caMsgBuf, g_pcMsg, g_iMsgLen - 8);
            iLen = g_iMsgLen - 8;

            GET_STR_KEY(pstJson, "channel_merch_id", sMerchId);
            GET_STR_KEY(pstJson, "channel_term_id", sTermId);

            if (GetChannelTermKey(sTak, "ZAK", sMerchId, sTermId) < 0) {
                tLog(ERROR, "获取终端[%s:%s]的密钥出错,放弃交易.", sMerchId, sTermId);
                return ( -1);
            }
            tLog(DEBUG, "ZAK[%s]", sTak);

            if ((iRet = tHsm_Gen_Pufa_Mac(caTmp, sTak, caMsgBuf, iLen)) < 0) {
                tLog(ERROR, "生成MAC出错[%d].", iRet);
                return ( -2);
            }
            tDumpHex("浦发下发", caMac, 8);
            tDumpHex("计算", caTmp, 8);
            if (memcmp(caTmp, caMac, 8)) {
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
    char sBuf[1024] = {0}, sBuf1[1024] = {0}, sTmp[1024] = {0}, sTak[33] = {0}, sTpk[33] = {0}, sChannelTpk[33] = {0};
    UCHAR caMacBuf[4096] = {0}, cOutBuf[1024] = {0}, caMac[8] = {0}, caTmp[8] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    char sCardNo[CARD_NO_LEN + 1] = {0}, sPin[PIN_LEN + 1] = {0}, sVillageFlag[1 + 1] = {0};
    char sInputMode[INPUT_MODE_LEN + 1] = {0}, sInvoiceNo[6 + 1] = {0}, sRespId[INST_ID_LEN + 1] = {0};
    char sSecureCtrl[SECURECTRL_LEN + 1] = {0}, sPosInfo[119 + 1] = {0}, sCupInfo[119 + 1] = {0}, sVisualSn[50 + 1] = {0};
    char sChannelMerchId[MERCH_ID_LEN + 1] = {0}, sChannelTermId[TERM_ID_LEN + 1] = {0};
    double dTranAmt = 0.0, dNfcAmount = 0.0;
    int iLen = 0, iRet = -1;
    int i = 0;
    double dAmount = 0.0;

    switch (iBitNo) {
        case 2: /* 主账号 */
            GET_STR_KEY(pstJson, "card_no", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 3:
            break;
        case 4: /* 交易金额 */
            GET_DOU_KEY(pstJson, "amount", dAmount);
            tLog(DEBUG, "%f", dAmount);
            if (fabs(dAmount) < 0.00001)
                return 0;
            sprintf(sBuf, "%012.f", dAmount);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, AMOUNT_LEN);
        case 11: /* 系统跟踪号 */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            if (sTransCode[5] == '3') {
                GET_STR_KEY(pstJson, "o_sys_trace", sBuf);
            } else {
                GET_STR_KEY(pstJson, "sys_trace", sBuf);
            }
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 12: /* 交易时间 */
            GET_STR_KEY(pstJson, "trans_time", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 13: /* 交易日期 */
            GET_STR_KEY(pstJson, "trans_date", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf + 4, 4);
        case 14: /* 卡有效期 */
            GET_STR_KEY(pstJson, "card_exp_date", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 15: /* 清算日期 */
            GET_STR_KEY(pstJson, "cups_settle_data", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 22: /*  服务点输入方式*/
            GET_STR_KEY(pstJson, "input_mode", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 23: /* 卡片序列号 */
            GET_STR_KEY(pstJson, "card_sn", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 25: /* 服务点条件码 */
            GET_STR_KEY(pstJson, "cond_code", sBuf);
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            if (!memcmp(sTransCode, "021000", 6)) {
                return tIsoSetBit(&g_stIsoData, iBitNo, "00", 2);
            }
            return tIsoSetBit(&g_stIsoData, iBitNo, "82", 2);
        case 26: /* 服务点PIN获取码 */
            GET_STR_KEY(pstJson, "input_mode", sInputMode);
            if (sInputMode[2] == '2')
                return 0;
            GET_STR_KEY(pstJson, "pin_code", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 32: /* 受理机构标识 */
            GET_STR_KEY(pstJson, "channel_merch_id", sTmp);
            memset(sBuf, 0, sizeof (sBuf));
            memcpy(sBuf, "4900", 4);
            tStrCpy(sBuf + 4, sTmp + 3, 4);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));

        case 35: /* 2磁 */
            GET_STR_KEY(pstJson, "track2", sBuf);
            sprintf(sTmp, "%.2s", sBuf);
            tLog(DEBUG, "tmk[%s]", sTmp);
            iLen = strlen(sBuf);
            tLog(DEBUG, "[%d]track2[%s]", iLen, sBuf);
            for (i = 0; i < iLen; i++) {
                if (sBuf[i] == 'D') {
                    sBuf[i] = '=';
                    break;
                }
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 36: /* 3磁 */
            GET_STR_KEY(pstJson, "track3", sBuf);
            //sprintf(sTmp, "%.3s", sBuf);
            iLen = strlen(sTmp);
            for (i = 0; i < iLen; i++) {
                if (sBuf[i] == 'D') {
                    sBuf[i] = '=';
                }
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 37: /* RRN */
            GET_STR_KEY(pstJson, "o_pufa_rrn", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 38: /* 授权码 */
            GET_STR_KEY(pstJson, "auth_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 39: /* 应答码 */
            /* 浦发冲正类上送06*/
            //     GET_STR_KEY(pstJson, "resp_code", sBuf);
            strncpy(sBuf, "06", 2);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 41: /* 终端编号 */
            GET_STR_KEY(pstJson, "channel_term_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, 8));
        case 42: /* 商户编号 */
            GET_STR_KEY(pstJson, "channel_merch_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, 15));

        case 48: /* 附加数据域 */
            GET_STR_KEY(pstJson, "48_data", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 49: /* 交易货币代码 */
            GET_STR_KEY(pstJson, "ccy_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 52: /* Pin */
            GET_STR_KEY(pstJson, "input_mode", sBuf);
            if (sBuf[2] == '2')
                return 0;
            GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
            GET_STR_KEY(pstJson, "channel_term_id", sChannelTermId);

            if (GetChannelTermKey(sChannelTpk, "ZPK", sChannelMerchId, sChannelTermId) < 0) {
                tLog(ERROR, "获取渠道PIN_KEY密钥出错.");
                SET_STR_KEY(pstJson, "resp_code", "A0");
                return (-2);
            }
            tLog(INFO, "sTpk channel[%s]", sChannelTpk);

            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);

            if (GetTermKey(sTpk, sMerchId, sTermId, "PIN_KEY") < 0) {

                tLog(ERROR, "获取终端[%s:%s]的密钥出错.", sMerchId, sTermId);
                return ( -2);
            }
            tLog(INFO, "sTpk term[%s]", sTpk);

            GET_STR_KEY(pstJson, "card_no", sCardNo);
            GET_STR_KEY(pstJson, "pin", sPin);
            tLog(INFO, "card_no[%s],pin[%s]", sCardNo, sPin);
            //if (HSM_TranPin_Tpk2Zpk(sCardNo, sPin, sTpk, sChannelTpk, sBuf) < 0) {
            if (tHsm_TranPin(sBuf, sCardNo, sPin, sTpk, sChannelTpk) < 0) {
                tLog(ERROR, "密码信息转换失败.");
                return -2;
            }
            tLog(DEBUG, "new sPin[%s]", sBuf);
            tAsc2Bcd(caTmp, sBuf, 16, 0);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, caTmp, 8));
        case 53: /* 安全控制信息 */
            /* 重置密钥: 2.重置MAC KEY 1.重置PIN KEY */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            if (strstr("009800,0G9900", sTransCode) != NULL) {
                GET_STR_KEY(pstJson, "secure_ctrl", sBuf);
            } else {
                GET_STR_KEY(pstJson, "input_mode", sBuf);
                if (sBuf[2] != '1') {
                    return 0;
                }
                strcpy(sBuf, "2600000000000000");
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, SECURECTRL_LEN));
        case 55: /* ic卡数据域 */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            GET_STR_KEY(pstJson, "input_mode", sInputMode);
            //            && (sInputMode[1] == '5' || sInputMode[1] == '7')
            //                tLog(INFO, "冲正ic域填9F06");
            //                tStrCpy(sBuf, "9F06", 4);
            if (sTransCode[5] == '3') {
                return 0;
            } else {
                GET_INT_KEY(pstJson, "ic_len", iLen);
                GET_STR_KEY(pstJson, "ic_data", sBuf);
            }
            tLog(INFO, "ic[%s]", sBuf);
            tAsc2Bcd(sTmp, sBuf, iLen << 1, LEFT_ALIGN);
            return tIsoSetBit(&g_stIsoData, iBitNo, sTmp, iLen);
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
        case 61:
            /*
             * 
            本域存在以下两种用法：
            1、在冲正、撤销交易中，用于填写原始交易信息。
            2、在其它交易中用于填写持卡人身份认证信息。
             */
            GET_STR_KEY(pstJson, "trans_code", sBuf);
            if (!memcmp(sBuf, "020000", 6) || !memcmp(sBuf, "021000", 6)) {
                return 0;
            }
            /*
            数据元长度 N3
                    61.1 原始交易批次号 N6
                    61.2 原始交易POS流水号 N6
                    61.3 原始交易日期 N4
                    61.4原交易授权方式 N2
                    61.5原交易授权机构代码 N11
             */
            GET_STR_KEY(pstJson, "o_batch_no", sBuf);
            tStrCpy(sBuf1, sBuf, 6);
            GET_STR_KEY(pstJson, "o_sys_trace", sBuf);
            tStrCpy(sBuf1 + 6, sBuf, 6);
            GET_STR_KEY(pstJson, "o_trans_date", sBuf);
            tStrCpy(sBuf1 + 6 + 6, sBuf + 4, 4);
            tStrCpy(sBuf1 + 6 + 6 + 4, "0000000000000", 13);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf1, strlen(sBuf1)));

        case 63:/* 自定义域，用法1: 操作员代码 */
            GET_STR_KEY(pstJson, "teller_no", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));

        case 64: /* MAC */
            /* 组报文的MAC Buffer */
            GET_STR_KEY(pstJson, "channel_merch_id", sMerchId);
            GET_STR_KEY(pstJson, "channel_term_id", sTermId);

            tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8);

            if ((iLen = tIso2Str(&g_stIsoData, (UCHAR*) caMacBuf, sizeof (caMacBuf))) < 0)
                return ( -1);

            if (GetChannelTermKey(sTak, "ZAK", sMerchId, sTermId) < 0) {
                tLog(ERROR, "获取终端[%s:%s]的密钥出错,放弃交易.", sMerchId, sTermId);
                return ( -1);
            }

            if ((iRet = tHsm_Gen_Pufa_Mac(caMac, sTak, caMacBuf, iLen - 8)) < 0) {
                tLog(ERROR, "生成MAC出错[%d].", iRet);
                return ( -2);
            }
            tDumpHex("计算", caMac, 8);

            return ( tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8));

        default:
            tLog(ERROR, "组装的报文域[%d]不存在", iBitNo);
    }
    return ( 0);
}

/* == CUPS报文结构 == */

/* Field1   头长度          1   **
 ** Field2   头标识和版本号  1   **
 ** Field3   整个报文长度    4   **
 ** Field4   目的ID          11  **
 ** Field5   源ID            11  **
 ** Field6   保留使用        3   **
 ** Field7   批次号          1   **
 ** Field8   交易信息        8   **
 ** Field9   用户信息        1   **
 ** Field10  拒绝码          5   */

/* 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25   **
 ** [报文长度 ] [1][2][--3长度--] [--      4  目的ID   11B     --] [-- 5 源地址  **
 **                               "00010000   "                    "30022900   " **
 **             46 01             30 30 30 31 30 30 30 30 20 20 20 33 30 30 32   **
 **                                                                              **
 ** 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50   **
 ** 5  源地址   11B  --] [保留域] [7][-- 8 交易信息 8B   --] [9][--10 拒绝码5B]  **
 **                                                                              **
 ** 32 39 30 30 20 20 20 00 00 00 00 30 30 30 30 30 30 30 30 00 30 30 30 30 30   **
 **                                                                              **
 ** 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75   **
 ** [报文类型 ][--       位图1 8B   --] [--      位图2 8B   --] [--  报文域  --  **
 **  4B  ASC                                                                     **
 **                                                                              */

/* 出口方向: 拆包应答报文; 入口方向: 拆包请求报文. */
//int ModuleUnpack(void *pvNetTran, char *pcMsg, int iMsgLen, PktType *pePktType, DirecType *peDirecType) {

int ResponseMsg(cJSON *pstRepJson, cJSON *pstDataJson) {

    int i = 0, iRet = -1;
    char sMsgType[5] = {0}, sProcCode[7] = {0}, sProcCodeEx[3] = {0}, sNmiCode[4] = {0}, sMsgData[36] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0}, sTmp[MSG_MAX_LEN] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0};
    UCHAR *pcBitMap = NULL;

    GET_STR_KEY(pstDataJson, "msg", sTmp);
    int iMsgLen = strlen(sTmp);
    tAsc2Bcd(sMsg, (UCHAR*) sTmp, iMsgLen, LEFT_ALIGN);
    /** 拆包 **/

    g_pcMsg = sMsg + 11;
    g_iMsgLen = iMsgLen / 2 - 11;
    tClearIso(&g_stIsoData);
    tLog(DEBUG, "tStr2Iso");
    if ((iRet = tStr2Iso(&g_stIsoData, g_pcMsg, g_iMsgLen)) < 0) {
        tLog(ERROR, "解析8583报文数据错误[%d].", iRet);
        return ( -1);
    }
    tDumpIso(&g_stIsoData, "应答报文");

    /* 解析出消息类型码 : 1 域  */
    tIsoGetBit(&g_stIsoData, 1, sMsgType, sizeof (sMsgType));
    /* 银联应答消息类型码减一 */
    sMsgType[2]--;

    /* 解析出处理码     : 3 域  */
    iRet = tIsoGetBit(&g_stIsoData, 3, sProcCode, sizeof (sProcCode));
    tTrim(sProcCode);

    /* 解析自定义域60: 消息类型码、批次号、网络管理信息码 */
    tIsoGetBit(&g_stIsoData, 60, sMsgData, sizeof (sMsgData));
    /* 消息类型码 */
    tStrCpy(sProcCodeEx, sMsgData, 2);
    tTrim(sProcCodeEx);
    SET_STR_KEY(pstRepJson, "cond_code", sProcCodeEx);
    /* 不需要批次号直接跳过 +6 */
    /* 获取网络管理码 */
    tStrCpy(sNmiCode, sMsgData + 8, 3);


    /* 解析出内部交易类型 */
    if (GetTranType(sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode) < 0) {
        tLog(ERROR, "无法解析的交易类型(MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
                , sMsgType, sProcCode, sProcCodeEx, sNmiCode);
        return -3;
    }
    //GET_STR_KEY(pstRepJson, "trans_code", sTransCode);
    tLog(ERROR, "交易类型[%s](MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
            , sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode);

    SET_STR_KEY(pstRepJson, "trans_code", sTransCode);

    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if ((!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6))) {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "交易[%s][ProcCode:%s][ProcCodeEx:%s]未定义解包位图."
                , sTransCode, sProcCode, sProcCodeEx);
        return -3;
    }

    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++) {
        tLog(INFO, "GetBit域[%d].", pcBitMap[i]);
        iRet = GetFld(pcBitMap[i], pstRepJson);
        if (iRet < 0) {
            tLog(ERROR, "拆解交易[%s][MsgType:%s][ProcEx:%s]报文域[%d]出错[%d]!"
                    , sTransCode, sMsgType, sProcCodeEx, pcBitMap[i], iRet);
            return ( iRet);
        }
    }
    return 0;
}

/* 出口方向: 打包请求报文; 入口方向: 打包应答报文. */
int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {

    int i = 0, iRet = -1;
    char sMsgType[5] = {0}, sProcCode[7] = {0}, sProcCodeEx[3] = {0}, sNmiCode[4] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0}, sTmp[2048] = {0}, sBuf[16 + 1] = {0}, sLen[8 + 1] = {0}, sMsgData[17] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sOTransCode[TRANS_CODE_LEN + 1] = {0}, sBatchNo[BATCH_NO_LEN + 1] = {0};
    UCHAR *pcBitMap = NULL;

    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "o_trans_code", sOTransCode);
    tTrim(sTransCode);

    /* 组装出外部交易类型 */
    if (GetMsgData(sTransCode, sOTransCode, NULL, sMsgType, sProcCode, sProcCodeEx, sNmiCode) < 0) {
        tLog(ERROR, "无法组装的交易类型(TransCode=[%s])!", sTransCode);
        return -3;
    }

    tLog(ERROR, "交易类型[%s](MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
            , sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode);
    /** 组包 **/
    tClearIso(&g_stIsoData);

    /* 组包消息类型码   : 1 域  */
    tIsoSetBit(&g_stIsoData, 1, sMsgType, sizeof (sMsgType));

    /* 组包交易处理码   : 3 域  */
    tIsoSetBit(&g_stIsoData, 3, sProcCode, strlen(sProcCode));

    /*获取批次号*/
    GET_STR_KEY(pstDataJson, "batch_no", sBatchNo);

    /* 组包自定义域60: 消息类型码、批次号、网络管理信息码 */
    /*60.4域 保留使用
         终端暂时不用，如存在后续子域则默认填‘0’；
     60.5域 保留使用
         终端暂时不用，如存在后续子域则默认填‘00’；
     60.6域  终端读取能力
      
     */
    sprintf(sMsgData, "%2s%6s%3s%s", sProcCodeEx, sBatchNo, sNmiCode, "00060");
    tLog(INFO, "sMsgData[%s]", sMsgData);
    tLog(INFO, "sMsgLen[%d]", strlen(sMsgData));
    tIsoSetBit(&g_stIsoData, 60, sMsgData, strlen(sMsgData));

    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if ((!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6))) {
            pcBitMap = g_staBitMap[i].pcPackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "交易[%s][ProcCode:%s][ProcEx:%s]未定义组包位图."
                , sTransCode, sProcCode, sProcCodeEx);
        return -3;
    }

    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++) {
        tLog(INFO, "SetBit域[%d].", pcBitMap[i]);
        iRet = SetFld(pcBitMap[i], pstDataJson);
        if (iRet < 0) {
            tLog(ERROR, "组装交易[%s][MsgType:%s][ProcEx:%s]报文域[%d]出错[%d]!"
                    , sTransCode, sMsgType, sProcCodeEx, pcBitMap[i], iRet);
            return ( iRet);
        }
    }

    if ((iRet = tIso2Str(&g_stIsoData, sTmp, sizeof (sTmp))) < 0) {
        tLog(ERROR, "构建8583报文错误[%d]", iRet);
        return ( -1);
    }

    tDumpIso(&g_stIsoData, "请求报文");
    /* 数据报文     */
    tBcd2Asc((UCHAR*) sMsg + 22, (UCHAR*) sTmp, iRet << 1, LEFT_ALIGN);
    /* 报文头  */
    memcpy(sMsg, "600005000060310018110B", 22);

    SET_STR_KEY(pstReqJson, "msg", (const char*) sMsg);
    return ( 0);
}



