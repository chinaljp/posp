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
#include "t_dlink.h"


#define aStrEql( str1, str2 ) \
    ( ( strlen(str1) == strlen(str2) ) && ( ! strcmp( str1, str2 ) ) )

/* dbop.pc */
int GetTermKey(char *pcKeyAsc, char *pcMerchId, char *pcTermId, char *pcKeyName) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select a.%s  from B_TERM a, B_MERCH b "
            " where a.USER_CODE = b.USER_CODE AND A.TERM_ID='%s' AND B.MERCH_ID='%s' ", pcKeyName, pcTermId, pcMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, pcKeyAsc);
    }
    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return ( 0);
}

/* term.pc */
int GetChannelTermKey(char *pcKeyAsc, char *pcKeyName) {
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT KEY_VALUE FROM S_PARAM WHERE KEY='%s'", pcKeyName);
    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, pcKeyAsc);
    }
    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
}

int pos2cup(char * pstPosInfo, char *pstCupInfo) {
    char sPosInfo[119 + 1] = {0}, sCInfo[119 + 1] = {0}, sCupInfo[119] = {0}, sLen[3 + 1] = {0}, sPInfo[119 + 1] = {0};
    char sTag[2 + 1] = {0}, sTagL[3 + 1] = {0}, sTagV[99 + 1] = {0}, sTagLV[99 + 1] = {0};
    int i = 0, iCup = 0, w = 0;
    strcpy(sPosInfo, pstPosInfo);
    tStrCpy(sLen, sPosInfo + 2, 3);
    tStrCpy(sPInfo, sPosInfo + 2 + 3, atoi(sLen));
    while (!(i == strlen(sPInfo)))
    {
        /*取tag*/
        tStrCpy(sTag, sPInfo + i, 2);
        /*取tag-L*/
        tStrCpy(sTagL, sPInfo + i + 2, 3);
        /*取tag-V*/
        tStrCpy(sTagV, sPInfo + i + 2 + 3, atoi(sTagL));
        /*组银联tag-v-l*/
        //   sprintf(sTagLV, "%x%x%s", atoi(sTag), atoi(sTagL), sTagV);

        //判断截取的字符串是不是数字字符串
        {
            for (w = 0; sTag[w]; w++) {
                if (!(sTag[w] >= '0' && sTag[w] <= '9')) {
                    tLog(ERROR, "上送终端数据不正确");
                    return -1;
                }
            }

            for (w = 0; sTagL[w]; w++) {
                if (!(sTagL[w] >= '0' && sTagL[w] <= '9')) {
                    tLog(ERROR, "上送终端数据不正确");
                    return -1;
                }
            }
        }

        sTagLV[0] = atoi(sTag);
        sTagLV[1] = atoi(sTagL);
        strcat(sTagLV, sTagV);

        tLog(INFO, "Tag[%s],TagL[%d],TagLv[%s]", sTag, atoi(sTagL), sTagLV);
        strcat(sCInfo, sTagLV);
        i += (5 + atoi(sTagL));

        memset(sTag, 0x00, sizeof (sTag));
        memset(sTagL, 0x00, sizeof (sTagL));
        memset(sTagV, 0x00, sizeof (sTagV));
        memset(sTagLV, 0x00, sizeof (sTagLV));

        iCup++;
        /*当数据出现问题时，该循环可能无限打印，添加以下代码防止死循环*/
        if (iCup == 10)
        {
            break;
        }

    }
    iCup = strlen(sCInfo);
    strcpy(sCupInfo, "PI");
    sprintf(sCupInfo + 2, "%03d%s", iCup, sCInfo);
    //  tLog(INFO, "pos2cup[%s]", sCupInfo);
    strcpy(pstCupInfo, sCupInfo);

    return 0;
}

int Assemblecup(char * pstVisualSn, char *pstCupInfo) {
    char sPosInfo[119 + 1] = {0}, sCInfo[119 + 1] = {0}, sCupInfo[119] = {0}, sLen[3 + 1] = {0}, sPInfo[119 + 1] = {0};
    char sTag[2 + 1] = {0}, sTagL[3 + 1] = {0}, sTagV[99 + 1] = {}, sTagLV[99 + 1] = {0}, sTagLV1[99 + 1] = {0}, sVisualSn[30 + 1] = {0};
    int i = 0, iCup = 0;

    strcpy(sVisualSn, pstVisualSn);
    /*组装104域 tag 04 终端类型*/
    sTagLV[0] = 4;
    sTagLV[1] = 2;
    strcat(sTagLV, "02");
    tLog(INFO, "Tag[04],TagL[2],TagLv[%s]", sTagLV);
    /*组装104域 tag 05 终端序列号*/
    i = strlen(sVisualSn);
    sTagLV1[0] = 5;
    sTagLV1[1] = i;
    strcat(sTagLV1, sVisualSn);
    tLog(INFO, "Tag[05],TagL[%d],TagLv1[%s]", i, sTagLV1);
    strcpy(sCInfo, sTagLV);
    strcat(sCInfo, sTagLV1);
    iCup = strlen(sCInfo);
    strcpy(sCupInfo, "PI");
    sprintf(sCupInfo + 2, "%03d%s", iCup, sCInfo);
    //  tLog(INFO, "pos2cup[%s]", sCupInfo);
    strcpy(pstCupInfo, sCupInfo);

    return 0;
}

int ChkTermType(char * pstTermId, char * pstMerchId, char *pstTermSn) {
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;
    char sVisualSn[50 + 1] = {0}, sDevType[2] = {0}, sDevSn[50 + 1] = {0};

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT B_DEV_TERM_SN,B_DEV_VISUAL_SN,B_DEV_TYPE FROM V_TERM \
      WHERE B_DEV_TERM_TERM_ID='%s' AND B_MERCH_MERCH_ID='%s'", pstTermId, pstMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, sDevSn);
        STRV(pstRes, 2, sVisualSn);
        STRV(pstRes, 3, sDevType);
    }
    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (sDevType[0] == '0')
    {
        tLog(INFO, "终端为存量终端，上送真实sn号");
        strcpy(pstTermSn, sDevSn);
        return 1;
    } else if (sDevType[0] == '1')
    {
        tLog(INFO, "终端为自备机，上送虚拟sn号");
        strcpy(pstTermSn, sVisualSn);
        return 2;
    } else if (sDevType[0] == '2')
    {
        tLog(INFO, "终端为新增终端，上送真实sn号");
        return 3;
    }

}

int GetVisualMerchTermSn(char * pstChannelMerchId, char * pstChannelTermId, char * sSn) {
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;
    char sVisualSn[50 + 1] = {0};

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT VISUAL_SN FROM B_LIFE_MERCH \
      WHERE MERCH_ID='%s' AND TERM_ID='%s'", pstChannelMerchId, pstChannelTermId);
    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, sVisualSn);
    }
    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    strcpy(sSn, sVisualSn);

    return 0;
}

int ChkMerchNosign(char * pstMerchId) {
    char sSqlStr[512];
    int iNum = 0;
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_MERCH_DOUBLE_FREE \
      WHERE USER_CODE = ( SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID='%s') AND STATUS='1'", pstMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes)
    {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes))
    {
        INTV(pstRes, 1, iNum);
    }
    if (0 == iNum)
    {
        tLog(INFO, "商户未开通双免业务");
        tReleaseRes(pstRes);
        return 1;
    } else if (1 == iNum)
    {
        tLog(INFO, "商户已开通双免业务");
        tReleaseRes(pstRes);
        return 0;
    }
}

extern IsoData g_stIsoData;
char *g_pcMsg;
int g_iMsgLen;


TranTbl g_staTranTbl[] = {
    /* 消息ID  处理码    消息类型码  NMI   交易代码  原交易代码  */
    /* 一般类交易 */
    { "0200", "300000", "00", "", "021000", "", ""}, /* POS查余额            */
    { "0200", "000000", "00", "", "020000", "", ""}, /* POS消费              */
    { "0420", "000000", "00", "", "020003", "", ""}, /* POS消费冲正          */
    { "0200", "200000", "00", "", "020002", "", ""}, /* POS消费撤销          */
    { "0420", "200000", "00", "", "020023", "", ""}, /* POS消费撤销冲正      */
    { "0220", "200000", "00", "", "020001", "", ""}, /* POS退货   */
    /* 预授权交易 */
    { "0100", "030000", "06", "", "024000", "", ""}, /* POS预授权            */
    { "0420", "030000", "06", "", "024003", "", ""}, /* POS预授权冲正        */
    { "0100", "200000", "06", "", "024002", "", ""}, /* POS预授权撤销        */
    { "0420", "200000", "06", "", "024023", "", ""}, /* POS预授权撤销冲正    */
    { "0200", "000000", "06", "", "024100", "", ""}, /* POS预授权完成        */
    { "0420", "000000", "06", "", "024103", "", ""}, /* POS预授权完成冲正    */
    { "0200", "200000", "06", "", "024102", "", ""}, /* POS预授权完成撤销    */
    { "0420", "200000", "06", "", "024123", "", ""}, /* POS预授权完成撤销冲正*/
    /* T0交易 */
    { "0200", "000000", "00", "", "T20000", "", ""}, /* T0消费              */
    { "0420", "000000", "00", "", "T20003", "", ""}, /* T0消费冲正		   */
    /* 脱机类交易 */
    { "0220", "000000", "00", "", "U20000", "", ""}, /* POS脱机消费              */
    { "0220", "200000", "69", "", "U20001", "", ""}, /* POS脱机消费退货*/
    /* 订单类交易 */
    { "0200", "000000", "00", "", "M20000", "", ""}, /* @@@订单消费              */
    { "0420", "000000", "00", "", "M20003", "", ""}, /*@@@ 订单消费冲正          */
    { "0200", "200000", "00", "", "M20002", "", ""}, /*@@@ 订单消费撤销          */
    { "0420", "200000", "00", "", "M20023", "", ""}, /* @@@订单消费撤销冲正      */

    { "0620", "300000", "00", "951", "020400", "021000", ""}, /* POS查余额脚本通知*/
    { "0620", "000000", "00", "951", "020400", "020000", ""}, /* POS消费脚本通知*/
    { "0620", "030000", "06", "951", "020400", "024000", ""}, /* POS预授权脚本通知*/
    { "0620", "600000", "91", "951", "020400", "023000", "100000"}, /* 电子现金圈存脚本通知*/
    { "0620", "630000", "91", "951", "020400", "023000", "200000"}, /* 电子现金圈存脚本通知*/
    { "0620", "650000", "91", "951", "020400", "023000", "300000"}, /* 电子现金圈存脚本通知*/
    { "0620", "170000", "91", "951", "020400", "020200", ""}, /* 电子现金充值撤销脚本通知*/

    //{ "0820", "", "", "001", "03H000", "", ""}, /* 平台签到          */
    { "0820", "", "", "001", "009000", "", ""}, /* 平台签到(CUPS)     */
    //{ "0820", "", "", "002", "039004", "", ""}, /* 平台签退          */
    { "0820", "", "", "002", "009100", "", ""}, /* 平台签退(CUPS)       */
    { "0820", "", "", "301", "009200", "", ""}, /* 平台线路测试          */
    { "0820", "", "", "301", "0G9200", "", ""}, /* 平台线路测试cups          */
    { "0820", "", "", "201", "0G9600", "", ""}, /* 银联日切开始cups          */
    { "0820", "", "", "202", "0G9700", "", ""}, /* 银联日切结束cups          */
    { "0820", "", "", "101", "009800", "", ""}, /* 密钥重置申请交易          */
    { "0800", "", "", "101", "0G9900", "", ""}, /* 密钥重置交易（CUPS)     */
    { "", "", "", "", "", "", ""},
};


/*************
 **  交易类  **
 *************/

/* 银联查余额 */
static unsigned char g_caInqueryUnpackMap[] = {\
2, 3, 7, 11, 12, 13, 14, 15, 18, 19, 23, 25, 32, 33, 37, 39, 41, 42, 49, 54, 55, 57, 60, 100, 121, 128, 255};
static unsigned char g_caInqueryPackMap[] = {\
2, 3, 7, 11, 12, 13, 14, 18, 22, 23, 25, 26, 32, 33, 35, 36, 37, 41, 42, 43, 49, 52, 53, 55, 60, 128, 255};

/* 银联消费 */
static unsigned char g_caPurchaseUnpackMap[] = {\
2, 3, 4, 6, 7, 10, 11, 12, 13, 14, 15, 18, 19, 23, 25, 32, 33, 37, 38, 39, 41, 42, 49, 51, 55, 57, 60, 100, 121, 128, 255};
static unsigned char g_caPurchasePackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 14, 18, 22, 23, 25, 26, 32, 33, 35, 36, 37, 41, 42, 43, 49, 52, 53, 55, 60, 104, 128, 255};

/* 非指定账户圈存 */
static unsigned char g_caPurchaseFZDUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 14, 15, 18, 23, 25, 32, 33, 37, 38, 39, 41, 42, 49, 55, 60, 100, 102, 103, 121, 128, 255};
static unsigned char g_caPurchaseFZDPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 14, 18, 22, 23, 25, 26, 32, 33, 35, 36, 37, 41, 42, 43, 48, 49, 52, 53, 55, 60, 102, 103, 128, 255};

/* POS脱机消费 */
static unsigned char g_caTjxfUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 14, 15, 18, 23, 25, 32, 33, 37, 39, 41, 42, 49, 55, 60, 100, 121, 128, 255};
static unsigned char g_caTjxfPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 14, 18, 22, 23, 25, 32, 33, 35, 36, 37, 41, 42, 43, 49, 55, 60, 128, 255};

/* 银联消费撤销, 预授权完成撤销 */
static unsigned char g_caUndoPurchaseUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 15, 18, 23, 25, 32, 33, 37, 39, 41, 42, 49, 55, 57, 60, 90, 100, 121, 128, 255};
static unsigned char g_caUndoPurchasePackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 18, 22, 23, 25, 26, 32, 33, 35, 36, 37, 38, 41, 42, 43, 49, 52, 53, 55, 60, 90, 128, 255};

/* 银联退货 */
static unsigned char g_caRefundReqUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 15, 18, 23, 25, 32, 33, 37, 39, 41, 42, 43, 49, 55, 57, 60, 100, 128, 255};
static unsigned char g_caRefundReqPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 18, 22, 23, 25, 32, 33, 35, 36, 37, 38, 41, 42, 43, 48, 49, 55, 60, 90, 128, 255};

/* 银联退货(CUPS发出) */
static unsigned char g_caRefundUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 15, 18, 23, 25, 32, 33, 37, 41, 42, 43, 49, 60, 90, 100, 128, 255};
static unsigned char g_caRefundPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 15, 18, 22, 23, 25, 32, 33, 37, 38, 39, 41, 42, 49, 60, 100, 128, 255};

/* 银联预授权 */
static unsigned char g_caPreAuthUnpackMap[] = {\
2, 3, 4, 6, 7, 10, 11, 12, 13, 14, 15, 18, 19, 23, 25, 32, 33, 37, 38, 39, 41, 42, 49, 51, 55, 57, 60, 100, 128, 255};
static unsigned char g_caPreAuthPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 14, 18, 22, 23, 25, 26, 32, 33, 35, 36, 37, 41, 42, 43, 49, 52, 53, 55, 60, 104, 128, 255};

/* 银联预授权撤销 */
static unsigned char g_caUndoPreAuthUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 15, 18, 23, 25, 32, 33, 37, 39, 41, 42, 49, 55, 57, 60, 100, 121, 128, 255};
static unsigned char g_caUndoPreAuthPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 18, 22, 23, 25, 26, 32, 33, 35, 36, 37, 38, 41, 42, 43, 49, 52, 53, 55, 60, 128, 255};

/* 银联预授权完成 */
static unsigned char g_caPreAuthCompUnpackMap[] = {\
2, 3, 4, 6, 7, 10, 11, 12, 13, 15, 18, 19, 23, 25, 32, 33, 37, 39, 41, 42, 49, 51, 55, 57, 60, 100, 121, 128, 255};
static unsigned char g_caPreAuthCompPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 18, 22, 23, 25, 26, 32, 33, 35, 36, 37, 38, 41, 42, 43, 49, 52, 53, 55, 60, 128, 255};

/* 银联冲正 */
static unsigned char g_caPurchaseRevUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 15, 18, 23, 25, 32, 33, 37, 39, 41, 42, 49, 55, 57, 60, 100, 121, 128, 255};
static unsigned char g_caPurchaseRevPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 18, 22, 23, 25, 32, 33, 37, 38, 41, 42, 43, 49, 53, 55, 60, 90, 128, 255};


/* 银联预授权冲正 */
static unsigned char g_caPurchaseAuthRevUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 15, 18, 23, 25, 32, 33, 37, 39, 41, 42, 49, 55, 57, 60, 100, 121, 128, 255};
static unsigned char g_caPurchaseAuthRevPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 18, 22, 23, 25, 32, 33, 37, 38, 41, 42, 43, 49, 53, 60, 90, 128, 255};


/* 银联撤销类冲正 */
static unsigned char g_caUndoPurchaseRevUnpackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 15, 18, 23, 25, 32, 33, 37, 39, 41, 42, 49, 55, 57, 60, 100, 128, 255};
static unsigned char g_caUndoPurchaseRevPackMap[] = {\
2, 3, 4, 7, 11, 12, 13, 18, 22, 23, 25, 32, 33, 37, 38, 41, 42, 43, 49, 53, 55, 60, 90, 128, 255};

/* POS其他冲正 */
static unsigned char g_caCorrectUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 55, 57, 60, 100, 121, 128, 255};
static unsigned char g_caCorrectPackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 38, 39, 41, 42, 49, 53, 55, 60, 90, 128, 255};

/* POS脚本通知 */
static unsigned char g_caJbtzUnpackMap[] = {\
2, 7, 11, 12, 13, 15, 23, 25, 32, 33, 38, 39, 41, 42, 49, 55, 57, 60, 70, 100, 255};
static unsigned char g_caJbtzPackMap[] = {\
2, 4, 7, 11, 12, 13, 18, 22, 23, 32, 33, 37, 39, 41, 42, 43, 49, 55, 60, 70, 90, 255};

/*************
 **  管理类  **
 *************/

/* 平台签到 */
static unsigned char g_caLoginReqUnpackMap[] = {\
7, 11, 33, 39, 70, 255};
static unsigned char g_caLoginReqPackMap[] = {\
7, 11, 33, 70, 255};

/* 平台签到(CUPS发出) */
static unsigned char g_caLoginUnpackMap[] = {\
//7, 11, 70, 100, 255};
    7, 11, 33, 39, 70, 255
};
static unsigned char g_caLoginPackMap[] = {\
//7, 11, 39, 70, 100, 255};
    7, 11, 33, 70, 255
};

/* 平台日切(CUPS发出) */
static unsigned char g_carqUnpackMap[] = {\
7, 11, 15, 70, 100, 255};
static unsigned char g_carqPackMap[] = {\
7, 11, 15, 39, 70, 100, 255};

/* 银联密钥重置申请 */
static unsigned char g_caResetKeyReqUnpackMap[] = {\
7, 11, 33, 39, 53, 70, 255};
static unsigned char g_caResetKeyReqPackMap[] = {\
7, 11, 33, 53, 70, 255};

/* 银联密钥重置 */
static unsigned char g_caResetKeyUnpackMap[] = {\
7, 11, 48, 53, 70, 96, 100, 255};
static unsigned char g_caResetKeyPackMap[] = {\
7, 11, 39, 53, 70, 100, 128, 255};

/*************
 **  Mac Buf **
 *************/
static unsigned char g_caMacBufMap[] = {\
1, 2, 3, 4, 7, 11, 18, 25, 28, 32, 33, 38, 39, 41, 42, 90, 102, 103, 255};
static unsigned char g_caMackeyBufMap[] = {\
1, 2, 3, 4, 7, 11, 18, 25, 28, 32, 33, 38, 39, 41, 42, 53, 70, 90, 100, 255};

TranBitMap g_staBitMap[] = {
    /* 交易代码   原交易代码  原原交易码 拆包域位图                       组包域位图                      */
    { "021000", "", "", g_caInqueryUnpackMap, g_caInqueryPackMap}, /* 查余额            */
    { "020000", "", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* 消费              */
    { "T20000", "", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* T0消费              */
    { "020003", "", "", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* 消费冲正          */
    { "T20003", "", "", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* T0消费冲正          */
    { "020002", "", "", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* 消费撤销          */
    { "020023", "", "", g_caUndoPurchaseRevUnpackMap, g_caUndoPurchaseRevPackMap}, /* 消费撤销冲正      */
    { "M20000", "", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* 订单消费              */
    { "M20003", "", "", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* 订单消费冲正          */
    { "M20002", "", "", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* 订单消费撤销          */
    { "M20023", "", "", g_caUndoPurchaseRevUnpackMap, g_caUndoPurchaseRevPackMap}, /* 订单消费撤销冲正      */
    { "020001", "", "", g_caRefundReqUnpackMap, g_caRefundReqPackMap}, /* 退货              */
    { "02A100", "020000", "", g_caRefundUnpackMap, g_caRefundPackMap}, /* 退货通知(入)      */
    { "024000", "", "", g_caPreAuthUnpackMap, g_caPreAuthPackMap}, /* 预授权            */
    { "024003", "", "", g_caPurchaseAuthRevUnpackMap, g_caPurchaseAuthRevPackMap}, /* 预授权冲正        */
    { "024002", "", "", g_caUndoPreAuthUnpackMap, g_caUndoPreAuthPackMap}, /* 预授权撤销        */
    { "024023", "", "", g_caUndoPurchaseRevUnpackMap, g_caUndoPurchaseRevPackMap}, /* 预授权撤销冲正    */
    { "024100", "", "", g_caPreAuthCompUnpackMap, g_caPreAuthCompPackMap}, /* 预授权完成        */
    { "024103", "", "", g_caUndoPurchaseRevUnpackMap, g_caUndoPurchaseRevPackMap}, /* 预授权完成冲正    */
    { "024102", "", "", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* 预授权完成撤销    */
    { "024123", "", "", g_caUndoPurchaseRevUnpackMap, g_caUndoPurchaseRevPackMap}, /* 预授权完成撤销冲正*/

    { "023000", "100000", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* 电子现金圈存              */
    { "023000", "200000", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* 电子现金现金圈存              */
    { "023000", "300000", "", g_caPurchaseFZDUnpackMap, g_caPurchaseFZDPackMap}, /* 电子现金非指定账户圈存              */
    { "U20000", "", "", g_caTjxfUnpackMap, g_caTjxfPackMap}, /* POS脱机消费              */
    { "020100", "022000", "", g_caRefundReqUnpackMap, g_caRefundReqPackMap}, /* POS脱机消费退货              */

    { "020400", "020000", "", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS消费脚本通知          */
    { "020400", "024000", "", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS预授权脚本通知          */
    { "020400", "021000", "", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS查余脚本通知          */
    { "020400", "023000", "100000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS脚本通知          */
    { "020400", "023000", "200000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS脚本通知          */
    { "020400", "023000", "300000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS脚本通知          */

    //{ "03H000", "", "", g_caLoginReqUnpackMap, g_caLoginReqPackMap}, /* 平台签到          */
    { "009000", "", "", g_caLoginUnpackMap, g_caLoginPackMap}, /* 平台签到      */
    { "009100", "", "", g_caLoginUnpackMap, g_caLoginPackMap}, /* 平台签退(cups)      */
    //{ "039004", "", "", g_caLoginReqUnpackMap, g_caLoginReqPackMap}, /* 平台签退      */
    { "009200", "", "", g_caLoginReqUnpackMap, g_caLoginReqPackMap}, /* 线路测试      */
    //{ "039006", "", "", g_caLoginUnpackMap, g_caLoginPackMap}, /* 平台签到(入)      */
    { "0G9600", "", "", g_carqUnpackMap, g_carqPackMap}, /* 平台日切开始(入)      */
    { "0G9700", "", "", g_carqUnpackMap, g_carqPackMap}, /* 平台日切结束(入)      */
    { "009800", "", "", g_caResetKeyReqUnpackMap, g_caResetKeyReqPackMap}, /* 密钥重置申请      */
    { "0G9900", "", "", g_caResetKeyUnpackMap, g_caResetKeyPackMap}, /* 密钥重置交易(入)  */
    { "", "", "", NULL, NULL}
};

/*
 * 根据POS报文的消息类型码、处理码、扩展处理码、网络管理信息码，在定义表中判断查找交易类型
 */
int GetTranType(char *pcTransCode, char *pcMsgType, char *pcProcCode, char *pcProcCodeEx, char *pcNmiCode) {
    int i;

    /* 查取当前交易类型(根据从发卡机构报文解出的消息ID、处理码、消息类型码、网络管理码) */
    for (i = 0; g_staTranTbl[i].sMsgType[0] != '\0'; i++)
    {
        if ((!memcmp(g_staTranTbl[i].sMsgType, pcMsgType, strlen(g_staTranTbl[i].sMsgType)))
                && (!memcmp(g_staTranTbl[i].sProcCode, pcProcCode, (g_staTranTbl[i].sProcCode[0] ? 2 : 0))) /* 非空比较前2位 */
                && (!memcmp(g_staTranTbl[i].sProcCodeEx, pcProcCodeEx, strlen(g_staTranTbl[i].sProcCodeEx)))
                && (!memcmp(g_staTranTbl[i].sNmiCode, pcNmiCode, strlen(g_staTranTbl[i].sNmiCode))))
        {

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

    for (i = 0; g_staTranTbl[i].sMsgType[0] != '\0'; i++)
    {
        if (!memcmp(g_staTranTbl[i].sTransCode, pcTransCode, 6)&&!memcmp(g_staTranTbl[i].sOldTransCode, pcOldTransCode, strlen(g_staTranTbl[i].sOldTransCode)))
        {
            strcpy(pcMsgType, g_staTranTbl[i].sMsgType);
            strcpy(pcProcCode, g_staTranTbl[i].sProcCode);
            strcpy(pcProcCodeEx, g_staTranTbl[i].sProcCodeEx);
            strcpy(pcNmiCode, g_staTranTbl[i].sNmiCode);
            return ( 0);
        }
    }
    return ( -1);
}

/* 根据指定域组装MAC BUFFER */
void GetMacBuf(IsoData *pstIso, UCHAR *pcBitMap, UCHAR *pcMacBuf, int *piLen) {
    int i, iFldNo, iLen;
    char sLen[5], sBuf[1024], sMacBuf[1024] = {0};
    UCHAR cType;
    IsoTable *pstIsoTable = pstIso->pstIsoTable;

    for (i = 0; (iFldNo = pcBitMap[i]) != 255; i++)
    {
        memset(sBuf, 0, sizeof (sBuf));
        memset(sLen, 0, sizeof (sLen));
        iLen = 1024;
        if (1 == tGetIsoFld(pstIso, iFldNo, sBuf, &iLen))
        {
            cType = pstIsoTable[iFldNo - 1].cType & 0xC0;
            if (cType == LLLVAR)
            {
                sprintf(sLen, "%03d", iLen);
            } else if (cType == LLVAR)
            {
                sprintf(sLen, "%02d", iLen);
            }

            //if (53 == iFldNo)
            //strcpy(sBuf, "1000000000000000"); /* PIN KEY */
            /* 90域:原始信息, 只取前20位 */
            if (90 == iFldNo)
                sBuf[20] = '\0';
            strcat(sMacBuf, sLen);
            strcat(sMacBuf, sBuf);
            strcat(sMacBuf, " ");

            tLog(DEBUG, "GetFldNo[%d], cType=[%02x], FldLen[%d], Block[%s], MacBuf[%s]"
                    , iFldNo, cType, iLen, sBuf, sMacBuf);
        }
    }
    iLen = strlen(sMacBuf) - 1; /* 剔除结尾的空格 */
    sMacBuf[iLen] = '\0';
    tLog(DEBUG, "GetMacBuf(ASC) : [%s]", sMacBuf);
    *piLen = iLen;
    tStrCpy(pcMacBuf, sMacBuf, iLen);
}
/*
void tUpper(const char* pcStr, char* pcOutStr) {
    char sTemp[1024] = {0};
    int i, iLen = strlen(pcStr);
    for (i = 0; i < iLen; i++) {
        sTemp[i] = (char) (toupper(pcStr[i]));
    }
    tStrCpy(pcOutStr, sTemp, iLen);
}
 */

/* unpack */
int GetFld(int iBitNo, cJSON *pstJson) {
    char sNumBuf[3] = {0}, sBuf[1024] = {0}, sTak[33] = {0}, caTmp[8] = {0};
    char sAcctType[3] = {0}, sAmtType[3] = {0}, sBalCcyCode[4] = {0}, sBalance1[13] = {0}, sBalanceSign[2] = {0}, sIc[255] = {0};
    UCHAR caMacBuf[1024] = {0}, caMac[8] = {0}, sTransDate[7] = {0};
    char sErr[128] = {0}, sBuf1[128] = {0}, sOldRrn[13] = {0}, sRespDesc[41] = {0};
    char sSecureCtrl[16 + 1] = {0}, sMacKey[16 + 1] = {0};
    int iLen = 0, iRet = -1;

    memset(sNumBuf, 0, sizeof (sNumBuf));
    memset(sBuf, 0, sizeof (sBuf));
    memset(caTmp, 0, sizeof (caTmp));
    switch (iBitNo)
    {
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
        case 70: /* 网络管理信息码 */
            break;
        case 96: /* 报文安全码(单倍长新密钥) */
            GET_STR_KEY(pstJson, "secure_ctrl", sSecureCtrl);
            if ('1' == sSecureCtrl[0]) /* TPK解析48域, 96域不处理 */
                break;
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tBcd2Asc(sMacKey, sBuf, iRet << 1, LEFT_ALIGN);
            SET_STR_KEY(pstJson, "secure_data", sMacKey);
            break;
        case 100: /* 接收机构标识 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            tTrim(sBuf);
            SET_STR_KEY(pstJson, "resp_id", sBuf);
            SET_STR_KEY(pstJson, "iss_id", sBuf);
            break;
        case 121: /* CUPS保留信息: 应答/应答原因码 */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tStrCpy(sRespDesc, sBuf, 1);
            SET_STR_KEY(pstJson, "cups_desc", sRespDesc);
            tLog(DEBUG, "域121--CUPS保留信息[%s]", sRespDesc);
            break;
        case 128: /* MAC */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, caMac, sizeof (caMac));
            if (iRet < 0)
                return ( iRet);
            /* 组原报文的MAC Buffer */
            GetMacBuf(&g_stIsoData, g_caMacBufMap, caMacBuf, &iLen);
            /* 从库表中取TAK校验MAC.( 重置密钥交易不校验MAC ) */
            if (GetChannelTermKey(sTak, "ZAK") < 0)
            {
                tLog(ERROR, "获取渠道mac_key密钥出错.");
                SET_STR_KEY(pstJson, "resp_code", "A0");
                return ( -3);
            }
            tLog(DEBUG, "MacBuf Len[%d]", iLen);
            tLog(DEBUG, "sTak[%s]", sTak);
            //if ((iRet = HSM_GenStdMac(sTak, caMacBuf, iLen, caTmp)) < 0) {
            if ((iRet = tHsm_Gen_Cups_Mac(caTmp, sTak, caMacBuf, iLen)) < 0)
            {
                sprintf(sErr, "调用加密机出错[%d].", iRet);
                tLog(ERROR, sErr);
                SET_STR_KEY(pstJson, "resp_code", "A0");
                return ( -3);
            }
            /* 算出MAC后, 取前4字节转ASCII */
            /*************wuhm
            memcpy(caMacBcd, caTmp, 8);
            tBcd2Asc(caTmp, caMacBcd, 8, 0);
           tBufDump(DEBUG, pGetLogFile, caTmp, 8, "[Unpack] Mac of Asc");
             ******************/
            tDumpHex("返回", caMac, 8);
            tLog(DEBUG, "返回mac:%s", caMac);
            tDumpHex("计算", caTmp, 8);
            tLog(DEBUG, "计算mac:%s", caTmp);
            if (memcmp(caTmp, caMac, 8))
            {
                SET_STR_KEY(pstJson, "resp_code", "A0");
                sprintf(sErr, "校验MAC出错.");
                tLog(ERROR, sErr);
                return ( -3);
            }
            break;
        default:
            tLog(ERROR, "拆解的报文域[%d]不存在", iBitNo);
            break;
    }
    return ( 0);
}

int EncTrack(char *pcDecTrack, char *pcClearTrack, int iClearTrackLen, char *pcTdkuLmk) {
    int iRet = -1, iLen = 0;
    char sBlock[17] = {0}, sTmp[17] = {0};

    tLog(DEBUG, "Enc Pre Track Data[%d][%s]", iClearTrackLen, pcClearTrack);
    tStrCpy(pcDecTrack, pcClearTrack, iClearTrackLen);
    /* 整除是偶数，奇数+1*/
    iLen = iClearTrackLen % 2 ? iClearTrackLen + 1 : iClearTrackLen;
    /* 从后数第二位向前取8字节*/
    tLog(DEBUG, "Track Len[%d]", iLen);
    memcpy(sBlock, pcClearTrack + iLen - 18, 16);
    iRet = HSM_Enc_Track(sTmp, sBlock, 16, pcTdkuLmk);
    if (iRet < 0)
        return -1;

    tLog(DEBUG, "Enc Track Data[%s]", sTmp);
    memcpy(pcDecTrack + iLen - 18, sTmp, 16);
    tLog(DEBUG, "Enc End Track Data[%s]", pcDecTrack);
    return 0;
}

/* pack */
int SetFld(int iBitNo, cJSON *pstJson) {
    char sBuf[1024] = {0}, sTmp[1024] = {0}, sTak[33] = {0}, sTpk[33] = {0}, sChannelTpk[33] = {0};
    UCHAR caMacBuf[1024] = {0}, cOutBuf[1024] = {0}, caMac[8] = {0}, caTmp[8] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    char sCardNo[CARD_NO_LEN + 1] = {0}, sPin[PIN_LEN + 1] = {0}, sVillageFlag[1 + 1] = {0};
    char sInputMode[INPUT_MODE_LEN + 1] = {0}, sInvoiceNo[6 + 1] = {0}, sRespId[INST_ID_LEN + 1] = {0};
    char sSecureCtrl[SECURECTRL_LEN + 1] = {0}, sPosInfo[119 + 1] = {0}, sCupInfo[119 + 1] = {0}, sVisualSn[50 + 1] = {0};
    char sChannelMerchId[MERCH_ID_LEN + 1] = {0}, sChannelTerm[TERM_ID_LEN + 1] = {0}, sMerchType[2] = {0}, sChannelMerchType[2] = {0};
    double dTranAmt = 0.0, dNfcAmount = 0.0;
    int iLen = 0, iRet = -1;
    int i = 0;
    double dAmount = 0.0;

    switch (iBitNo)
    {
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
        case 7: /* 交易传输时间 */
            GET_STR_KEY(pstJson, "transmit_time", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 11: /* 系统跟踪号 */
            GET_STR_KEY(pstJson, "sys_trace", sBuf);
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
        case 18: /* 商户类型 */
            GET_STR_KEY(pstJson, "channel_merch_id", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf + 7, 4);
        case 22: /*  服务点输入方式*/
            GET_STR_KEY(pstJson, "input_mode", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 23: /* 卡片序列号 */
            GET_STR_KEY(pstJson, "card_sn", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 25: /* 服务点条件码 */
            GET_STR_KEY(pstJson, "cond_code", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
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
        case 33: /* 发送机构标识 */
            //GET_STR_KEY(pstJson, "channel_id", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, "49000000", 8);
        case 35: /* 2磁 */
            GET_STR_KEY(pstJson, "track2", sBuf);
            sprintf(sTmp, "%.2s", sBuf);
            tLog(DEBUG, "tmk[%s]", sTmp);
            iLen = strlen(sBuf);
            tLog(DEBUG, "[%d]track2[%s]", iLen, sBuf);
            for (i = 0; i < iLen; i++)
            {
                if (sBuf[i] == 'D')
                {
                    sBuf[i] = '=';
                    break;
                }
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 36: /* 3磁 */
            GET_STR_KEY(pstJson, "track3", sBuf);
            //sprintf(sTmp, "%.3s", sBuf);
            iLen = strlen(sTmp);
            for (i = 0; i < iLen; i++)
            {
                if (sBuf[i] == 'D')
                {
                    sBuf[i] = '=';
                }
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 37: /* RRN */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            if (sTransCode[5] == '3')
            {
                GET_STR_KEY(pstJson, "o_rrn", sBuf);
            } else GET_STR_KEY(pstJson, "rrn", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 38: /* 授权码 */
            GET_STR_KEY(pstJson, "auth_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 39: /* 应答码 */
            GET_STR_KEY(pstJson, "resp_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 41: /* 终端编号 */
            GET_STR_KEY(pstJson, "channel_term_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 42: /* 商户编号 */
            GET_STR_KEY(pstJson, "channel_merch_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 43: /* 商户名称和地址 */
            GET_STR_KEY(pstJson, "channel_merch_name", sBuf);
            if (sBuf[0] == '\0')
            {
                GET_STR_KEY(pstJson, "merch_p_name", sBuf);
            }
            //memset(sBuf, 0, sizeof (sBuf));
            //tLog(DEBUG, "43 sBuf Name[%s][%d]", sBuf, strlen(sBuf));
            tTrim(sBuf);
            //sprintf(sTmp, "%-40s", "黄山赢客缘商贸有限公司");
            sprintf(sTmp, "%-40s", sBuf);
            tLog(DEBUG, "log[%s]", sTmp);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, strlen(sTmp)));
        case 48: /* 附加数据域 */
            /*GET_STR_KEY(pstJson, "48_data", sBuf); */
            /*退货改造 F48：使用AS+OA用法，OA=原始交易金额*/
            dAmount = 0.00;
            GET_DOU_KEY(pstJson, "o_amount", dAmount);
            tLog(DEBUG, "%f", dAmount);
            sprintf(sBuf, "ASOA012%012.f", dAmount * 100);
            tLog(DEBUG, "%s", sBuf);

            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 49: /* 交易货币代码 */
            GET_STR_KEY(pstJson, "ccy_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 52: /* Pin */
            GET_STR_KEY(pstJson, "input_mode", sBuf);
            if (sBuf[2] == '2')
                return 0;

            if (GetChannelTermKey(sChannelTpk, "ZPK") < 0)
            {
                tLog(ERROR, "获取渠道PIN_KEY密钥出错.");
                SET_STR_KEY(pstJson, "resp_code", "A0");
                return (-2);
            }
            tLog(DEBUG, "sTpk channel[%s]", sChannelTpk);

            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);

            if (GetTermKey(sTpk, sMerchId, sTermId, "PIN_KEY") < 0)
            {

                tLog(ERROR, "获取终端[%s:%s]的密钥出错.", sMerchId, sTermId);
                return ( -2);
            }
            tLog(DEBUG, "sTpk term[%s]", sTpk);

            GET_STR_KEY(pstJson, "card_no", sCardNo);
            GET_STR_KEY(pstJson, "pin", sPin);
            tLog(DEBUG, "card_no[%s],pin[%s]", sCardNo, sPin);
            //if (HSM_TranPin_Tpk2Zpk(sCardNo, sPin, sTpk, sChannelTpk, sBuf) < 0) {
            if (tHsm_TranPin(sBuf, sCardNo, sPin, sTpk, sChannelTpk) < 0)
            {
                tLog(ERROR, "密码信息转换失败.");
                return -2;
            }
            tLog(DEBUG, "new sPin[%s]", sBuf);
            tAsc2Bcd(caTmp, sBuf, 16, 0);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, caTmp, 8));
        case 53: /* 安全控制信息 */
            /* 重置密钥: 2.重置MAC KEY 1.重置PIN KEY */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            if (strstr("009800,0G9900", sTransCode) != NULL)
            {
                GET_STR_KEY(pstJson, "secure_ctrl", sBuf);
            } else
            {
                GET_STR_KEY(pstJson, "input_mode", sBuf);
                if (sBuf[2] != '1')
                {
                    return 0;
                }
                strcpy(sBuf, "2600000000000000");
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, SECURECTRL_LEN));
        case 55: /* ic卡数据域 */
            GET_INT_KEY(pstJson, "ic_len", iLen);
            GET_STR_KEY(pstJson, "ic_data", sBuf);
            tAsc2Bcd(sTmp, sBuf, iLen << 1, LEFT_ALIGN);
            return tIsoSetBit(&g_stIsoData, iBitNo, sTmp, iLen);
        case 60: /* 自定义域 */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            GET_STR_KEY(pstJson, "input_mode", sInputMode);
            GET_DOU_KEY(pstJson, "amount", dTranAmt);
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "channel_merch_type", sChannelMerchType);
            GET_STR_KEY(pstJson, "merch_type", sMerchType);
            /* 原因码 */
            if ('3' == sTransCode[5])
                /* 终端引发冲正( 全 额 ) */
                tStrCpy(sBuf, "4351", 4);
            else
                tStrCpy(sBuf, "0000", 4);
            iLen = 4;
            /* 账户所有人类型 :个人卡帐户*/
            tStrCpy(sBuf + iLen, "0", 1);
            iLen += 1;
            /*终端读取能力*/
            if (sInputMode[1] == '2')
            {
                tStrCpy(sBuf + iLen, "2", 1);
            } else if (sInputMode[1] == '7')
            {
                tStrCpy(sBuf + iLen, "6", 1);
            } else if (sInputMode[1] == '5')
            {
                tStrCpy(sBuf + iLen, "6", 1);
            } else
                tStrCpy(sBuf + iLen, "6", 1);
            iLen += 1;
            /*IC 卡条件代码,产品业务标识*/
            tStrCpy(sBuf + iLen, "00", 2);
            iLen += 2;
            /*终端类型*/
            tStrCpy(sBuf + iLen, "03", 2);
            iLen += 2;
            /*境外受理免验密码网络标志及跨境汇款 到账币种要求*/

            /* && 商户开启免密判断 */
            iRet = ChkMerchNosign(sMerchId);
            if (0 == iRet)
            {
                if (FindValueByKey(sTmp, "yc.2free.amount") < 0)
                {
                    tLog(ERROR, "查找key[yc.2free.amount]云闪付nfc限额,失败.");
                    return -1;
                }
                dNfcAmount = atof(sTmp);
                tLog(INFO, "InputMode[%s]", sInputMode);
                /* 撤销类交易的 */
                if ('2' == sTransCode[5] ||
                        /* 撤销冲正类交易的 */
                        !memcmp(sTransCode + 4, "23", 2) ||
                        /* 预授权类不包括预授权和预授权冲正类交易的 */
                        (!memcmp(sTransCode + 1, "24", 2) && memcmp(sTransCode, "024000", 6) && memcmp(sTransCode, "024003", 6)) ||
                        /*退货类不开启免密免签*/
                        !memcmp(sTransCode, "020001", 6))
                {
                    tLog(INFO, "[撤销类交易][撤销冲正类交易][预授权类不包括预授权和预授权冲正类交易][退货]不开启免密");
                    tStrCpy(sBuf + iLen, "0", 1);
                } else
                {
                    if (sInputMode[1] == '7')
                    {
                        tLog(INFO, "AMOUNT[%lf]", dTranAmt);
                        if (sInputMode[2] == '1')
                        {/* 071说明有密码输入，不开启双免 */
                            tLog(INFO, "免密不开启 F22=071，F60.2.6=0");
                            tStrCpy(sBuf + iLen, "0", 1);
                        } else if ('3' == sTransCode[5] && (DBL_CMP(dTranAmt / 100, dNfcAmount)))
                        {/*  冲正类交易 */
                            tLog(INFO, "冲正类交易 金额大于[%f] 免密不开启 F22=072，F60.2.6=0", dNfcAmount);
                            tStrCpy(sBuf + iLen, "0", 1);
                        } else if ('3' == sTransCode[5] && !(DBL_CMP(dTranAmt / 100, dNfcAmount)))
                        {
                            tStrCpy(sBuf + iLen, "1", 1);
                            tLog(INFO, "冲正类交易 金额小于等于[%f] 属于免密商户 免密开启 F22=072，F60.2.6=1", dNfcAmount);
                        }/* 普通消费和预授权类交易 */
                        else if (sInputMode[2] == '2' && (DBL_CMP(dTranAmt / 100, dNfcAmount)))
                        {
                            tLog(INFO, "金额大于[%f] 免密不开启 F22=072，F60.2.6=0", dNfcAmount);
                            tStrCpy(sBuf + iLen, "0", 1);
                        } else if (sInputMode[2] == '2' && !(DBL_CMP(dTranAmt / 100, dNfcAmount)))
                        {
                            tStrCpy(sBuf + iLen, "1", 1);
                            tLog(INFO, "金额小于[%f] 属于免密商户 免密开启 F22=072，F60.2.6=1", dNfcAmount);
                        } else
                        {
                            tLog(INFO, "免密不开启 F22=071，F60.2.6=0");
                            tStrCpy(sBuf + iLen, "0", 1);
                        }
                    } else
                    {
                        tLog(INFO, "非闪付交易 免密不开启 F22 != 072，F60.2.6=0");
                        tStrCpy(sBuf + iLen, "0", 1);
                    }
                }

            } else
            {
                tStrCpy(sBuf + iLen, "0", 1);
            }

            iLen += 1;

            /*IC 卡验证可靠性标志*/
            tStrCpy(sBuf + iLen, "0", 1);
            iLen += 1;
            /*电子商务标志*/
            tStrCpy(sBuf + iLen, "00", 2);
            iLen += 2;
            /*交互方式标志*/
            tStrCpy(sBuf + iLen, "0", 1);
            iLen += 1;
            /*
             * F60.3.1 : 特殊计费类型 16 ans2
             * F60.3.2 : 特殊计费档次 18 ans1
             * F60.3.3 : 保留使用 19 ans3 
             * F60.3.4 : 支持部分承兑和返回余额标志 22 ans1 
             * F60.3.5 : 交易发起方式 23 ans1
             */

            if ('1' == sChannelMerchType[0])
            {
                tStrCpy(sBuf + iLen, "03000001", 8);
            }//渠道商户类型不存在并且商户类型为优惠类时
            else if (0 == sChannelMerchType[0] && '1' == sMerchType[0])
            {
                tStrCpy(sBuf + iLen, "03000001", 8);
            } else
                tStrCpy(sBuf + iLen, "00000001", 8);
            iLen += 8;
            /*
             * F60.3.6 : 交易介质 24 ans1
             * F60.3.7 : IC 卡的应用类型 25 ans1
             * F60.3.8 : 账户结算类型 26 ans2
             * F60.3.9 : 卡账户等级 28 ans1
             * F60.3.10 : 卡产品 29 ans2
             * */
            //tStrCpy(sBuf + iLen, "00000000", 7);
            //iLen += 7;
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
#if 0
            GET_STR_KEY(pstJson, "village_flag", sVillageFlag);
            GET_STR_KEY(pstJson, "invoice_no", sInvoiceNo);
            GET_DOU_KEY(pstJson, "amount", dTranAmt);

            if (!memcmp(sTransCode, "020300", 6) ||
                    !memcmp(sTransCode, "M20300", 6))
            {
                if (
                        (!memcmp(sVillageFlag, "1", 1)) ||
                        (!memcmp(sVillageFlag, "3", 1))
                        )
                {
                    strcpy(sBuf, "435105000300000040000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else if (
                        (!memcmp(sVillageFlag, "2", 1)) ||
                        (!memcmp(sVillageFlag, "4", 1))
                        )
                {
                    strcpy(sBuf, "435105000300000041000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else
                {
                    if (sInputMode[1] == '7')
                    {
                        if ((0 == memcmp(sTransCode, "020300", 6)) ||
                                (0 == memcmp(sTransCode, "024003", 6)))
                        {
                            if (sInputMode[2] == '1')
                            {
                                sprintf(sBuf, "0000060003%d000000000001", 0);
                            } else
                            {
                                sprintf(sBuf, "0000060003%d000000000001", 1);
                            }
                        } else
                        {
                            strcpy(sBuf, "43510600030000000000001");
                        }
                    } else
                    {
                        strcpy(sBuf, "43510500030000000000001");
                    }
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                }
            } else if (!memcmp(sTransCode, "020400", 6))
            {
                if (sInputMode[1] == '2')
                {
                    if (
                            (!memcmp(sVillageFlag, "1", 1)) ||
                            (!memcmp(sVillageFlag, "3", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000040000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else if (
                            (!memcmp(sVillageFlag, "2", 1)) ||
                            (!memcmp(sVillageFlag, "4", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000041000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else
                    {
                        tLog(ERROR, "village的值为[%s].", sVillageFlag);
                        strcpy(sBuf, "000005000300000000000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    }
                } else
                {
                    if (
                            (!memcmp(sVillageFlag, "1", 1)) ||
                            (!memcmp(sVillageFlag, "3", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000040000000000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else if (
                            (!memcmp(sVillageFlag, "2", 1)) ||
                            (!memcmp(sVillageFlag, "4", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000041000000000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else
                    {
                        strcpy(sBuf, "000005000300000000000000000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    }

                }
            } else if (sInputMode[1] == '7')
            {
                if (
                        (!memcmp(sVillageFlag, "1", 1)) ||
                        (!memcmp(sVillageFlag, "3", 1))
                        )
                {
                    strcpy(sBuf, "000006000300000040000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else if (
                        (!memcmp(sVillageFlag, "2", 1)) ||
                        (!memcmp(sVillageFlag, "4", 1))
                        )
                {
                    strcpy(sBuf, "000006000300000041000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else if (
                        (0 == memcmp(sTransCode, "020000", 6)) ||
                        (0 == memcmp(sTransCode, "024000", 6))
                        )
                {
                    if ((sInputMode[2] == '1') || (DBL_CMP(dTranAmt / 100, 300)))
                    {
                        sprintf(sBuf, "0000060003%d000000000001", 0);
                    } else
                    {
                        sprintf(sBuf, "0000060003%d000000000001", 1);
                    }
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else
                {
                    strcpy(sBuf, "00000600030000000000001");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                }
            } else if (sInputMode[1] == '8')
            {
                if (
                        (!memcmp(sVillageFlag, "1", 1)) ||
                        (!memcmp(sVillageFlag, "3", 1))
                        )
                {
                    strcpy(sBuf, "000006000300000040000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else if (
                        (!memcmp(sVillageFlag, "2", 1)) ||
                        (!memcmp(sVillageFlag, "4", 1))
                        )
                {
                    strcpy(sBuf, "000006000300000041000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else
                {
                    strcpy(sBuf, "000006000300000000000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                }
            } else
            {
                if (!memcmp(sInvoiceNo, "2", 1))
                {
                    if (
                            (!memcmp(sVillageFlag, "1", 1)) ||
                            (!memcmp(sVillageFlag, "3", 1))
                            )
                    {
                        strcpy(sBuf, "000005200300000040000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else if (
                            (!memcmp(sVillageFlag, "2", 1)) ||
                            (!memcmp(sVillageFlag, "4", 1))
                            )
                    {
                        strcpy(sBuf, "000005200300000041000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else
                    {
                        strcpy(sBuf, "000005200300000000000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    }
                } else
                {
                    if (
                            (!memcmp(sVillageFlag, "1", 1)) ||
                            (!memcmp(sVillageFlag, "3", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000040000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else if (
                            (!memcmp(sVillageFlag, "2", 1)) ||
                            (!memcmp(sVillageFlag, "4", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000041000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else
                    {
                        strcpy(sBuf, "00000500030000000000001");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    }

                }
            }
            if (!memcmp(sTransCode, "020300", 6) ||
                    !memcmp(sTransCode, "M20300", 6))
            {
                if (
                        (!memcmp(sVillageFlag, "1", 1)) ||
                        (!memcmp(sVillageFlag, "3", 1))
                        )
                {
                    strcpy(sBuf, "435105000300000040000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else if (
                        (!memcmp(sVillageFlag, "2", 1)) ||
                        (!memcmp(sVillageFlag, "4", 1))
                        )
                {
                    strcpy(sBuf, "435105000300000041000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else
                {
                    if (sInputMode[1] == '7')
                    {
                        if ((0 == memcmp(sTransCode, "020300", 6)) ||
                                (0 == memcmp(sTransCode, "024003", 6)))
                        {
                            if (sInputMode[2] == '1')
                            {
                                sprintf(sBuf, "0000060003%d000000000001", 0);
                            } else
                            {
                                sprintf(sBuf, "0000060003%d000000000001", 1);
                            }
                        } else
                        {
                            strcpy(sBuf, "43510600030000000000001");
                        }
                    } else
                        strcpy(sBuf, "43510500030000000000001");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                }
            } else if (!memcmp(sTransCode, "020400", 6))
            {
                if (sInputMode[1] == '2')
                {
                    if (
                            (!memcmp(sVillageFlag, "1", 1)) ||
                            (!memcmp(sVillageFlag, "3", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000040000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else if (
                            (!memcmp(sVillageFlag, "2", 1)) ||
                            (!memcmp(sVillageFlag, "4", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000041000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else
                    {
                        tLog(ERROR, "village的值为[%s].", sVillageFlag);
                        strcpy(sBuf, "000005000300000000000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    }
                } else
                {
                    if (
                            (!memcmp(sVillageFlag, "1", 1)) ||
                            (!memcmp(sVillageFlag, "3", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000040000000000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else if (
                            (!memcmp(sVillageFlag, "2", 1)) ||
                            (!memcmp(sVillageFlag, "4", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000041000000000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else
                    {
                        strcpy(sBuf, "000005000300000000000000000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    }

                }
            } else if (sInputMode[1] == '7')
            {
                if (
                        (!memcmp(sVillageFlag, "1", 1)) ||
                        (!memcmp(sVillageFlag, "3", 1))
                        )
                {
                    strcpy(sBuf, "000006000300000040000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else if (
                        (!memcmp(sVillageFlag, "2", 1)) ||
                        (!memcmp(sVillageFlag, "4", 1))
                        )
                {
                    strcpy(sBuf, "000006000300000041000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else if (
                        (0 == memcmp(sTransCode, "020000", 6)) ||
                        (0 == memcmp(sTransCode, "024000", 6))
                        )
                {
                    if ((sInputMode[2] == '1') || (DBL_CMP(dTranAmt / 100, 300)))
                    {
                        sprintf(sBuf, "0000060003%d000000000001", 0);
                    } else
                    {
                        sprintf(sBuf, "0000060003%d000000000001", 1);
                    }
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else
                {
                    strcpy(sBuf, "00000600030000000000001");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                }
            } else if (sInputMode[1] == '8')
            {
                if (
                        (!memcmp(sVillageFlag, "1", 1)) ||
                        (!memcmp(sVillageFlag, "3", 1))
                        )
                {
                    strcpy(sBuf, "000006000300000040000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else if (
                        (!memcmp(sVillageFlag, "2", 1)) ||
                        (!memcmp(sVillageFlag, "4", 1))
                        )
                {
                    strcpy(sBuf, "000006000300000041000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                } else
                {
                    strcpy(sBuf, "000006000300000000000010000000");
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                }
            } else
            {
                if (!memcmp(sInvoiceNo, "2", 1))
                {
                    if (
                            (!memcmp(sVillageFlag, "1", 1)) ||
                            (!memcmp(sVillageFlag, "3", 1))
                            )
                    {
                        strcpy(sBuf, "000005200300000040000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else if (
                            (!memcmp(sVillageFlag, "2", 1)) ||
                            (!memcmp(sVillageFlag, "4", 1))
                            )
                    {
                        strcpy(sBuf, "000005200300000041000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else
                    {
                        strcpy(sBuf, "000005200300000000000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    }
                } else
                {
                    if (
                            (!memcmp(sVillageFlag, "1", 1)) ||
                            (!memcmp(sVillageFlag, "3", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000040000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else if (
                            (!memcmp(sVillageFlag, "2", 1)) ||
                            (!memcmp(sVillageFlag, "4", 1))
                            )
                    {
                        strcpy(sBuf, "000005000300000041000010000000");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    } else
                    {
                        strcpy(sBuf, "00000500030000000000001");
                        return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
                    }

                }
            }
#endif
        case 70: /* 网络管理信息码 */
            return 0;

        case 90: /* 原始交易信息: 报文类型、系统跟踪号、系统日期时间、发卡机构标识、收单机构标识 */
            GET_STR_KEY(pstJson, "90_data", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));

        case 100: /* 接收机构标识 */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            GET_STR_KEY(pstJson, "resp_id", sRespId);
            if ((!memcmp(sTransCode, "009000", 6)))
            {
                strcpy(sBuf, "49000000");
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
            } else if ((!memcmp(sTransCode, "039006", 6)))
            {
                strcpy(sBuf, "49000000");
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
            } else if ((!memcmp(sTransCode, "039007", 6)))
            {
                strcpy(sBuf, "49000000");
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
            } else if ((!memcmp(sTransCode, "039008", 6)))
            {
                strcpy(sBuf, "49000000");
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
            } else if ((!memcmp(sTransCode, "039009", 6)))
            {
                strcpy(sBuf, "49000000");
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
            } else if ((!memcmp(sTransCode, "039010", 6)))
            {
                strcpy(sBuf, "49000000");
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
            } else
            {
                return tIsoSetBit(&g_stIsoData, iBitNo, sRespId, strlen(sRespId));
            }

        case 102: /* 转出账号标示 */
            /* 电子现金圈存交易，暂不处理 */
            break;
        case 103: /* 转入账号标识 */
            /* 电子现金圈存交易，暂不处理 */
            break;
        case 104: /* 终端上送硬件序列号及密文数据 */
            /*
                  终端入网认证编号 03 
                  设备类型 04 
                  终端序列号 05 
                  加密随机因子 06 
                  硬件序列号密文数据 07 
                  应用程序版本号 08              
             */
            /*判断终端类型*/
            GET_STR_KEY(pstJson, "term_id", sTermId);
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
            /*判断是否为生活圈商户*/
            if (memcmp(sChannelMerchId, sMerchId, 15))
            {
                /*生活圈商户获取虚拟sn*/
                GET_STR_KEY(pstJson, "channel_term_id", sChannelTerm);
                tLog(INFO, "商户为生活圈商户[%s],终端号[%s]", sChannelMerchId, sChannelTerm);
                GetVisualMerchTermSn(sChannelMerchId, sChannelTerm, sVisualSn);
                Assemblecup(sVisualSn, sCupInfo);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sCupInfo, strlen(sCupInfo)));
            } else
            {
                iRet = ChkTermType(sTermId, sMerchId, sVisualSn);
                if (iRet == 1 || iRet == 2)
                {
                    Assemblecup(sVisualSn, sCupInfo);
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sCupInfo, strlen(sCupInfo)));
                } else if (iRet == 3)
                {

                    GET_STR_KEY(pstJson, "pos_info", sPosInfo);
                    /* 银联要求标签tag为PI */
                    tLog(INFO, "终端上送硬件序列号及密文数据[%s]", sPosInfo);
                    pos2cup(sPosInfo, sCupInfo);
                    tLog(INFO, "上送银联硬件序列号及密文数据[%s]", sCupInfo);
                    return ( tIsoSetBit(&g_stIsoData, iBitNo, sCupInfo, strlen(sCupInfo)));
                }
            }

        case 128: /* MAC */
            /* 组报文的MAC Buffer */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            GET_STR_KEY(pstJson, "secure_ctrl", sSecureCtrl);

            if ((!memcmp(sTransCode, "0G9900", 6))
                    || (!memcmp(sTransCode, "00H000", 6)))
            {
                GetMacBuf(&g_stIsoData, g_caMackeyBufMap, caMacBuf, &iLen);
            } else
            {
                GetMacBuf(&g_stIsoData, g_caMacBufMap, caMacBuf, &iLen);
            }
            tLog(DEBUG, "macbuf[%s][%d]", caMacBuf, iLen);
            tLog(DEBUG, "securectrl [%c]", sSecureCtrl[0]);

            if ((!memcmp(sTransCode, "0G9900", 6)) && (sSecureCtrl[0] == '1'))
            {
                /*
                if (GetChannelTermKey(sTak, "ZAK") < 0) {
                    tLog(ERROR, "获取PAK出错.");
                    return ( -2);
                }
                 */
                tLog(DEBUG, "MacBuf Len[%d]", iLen);
                tLog(DEBUG, "PAK[%s]", sTak);
                //if ((iRet = HSM_GenStdMac3Des(sTak, caMacBuf, iLen, caMac)) < 0) {
                if ((iRet = tHsm_Gen_Cups_Mac(caMac, "pos.cups3.zak", caMacBuf, iLen)) < 0)
                {
                    tLog(ERROR, "生成MAC出错[%d].", iRet);
                    return ( -2);
                }
            } else
            {
                if (GetChannelTermKey(sTak, "ZAK") < 0)
                {
                    tLog(ERROR, "获取渠道mac_key密钥出错.");
                    return ( -2);
                }
                tLog(DEBUG, "MacBuf Len[%d]", iLen);
                tLog(DEBUG, "sTak[%s]", sTak);
                /*将小写转成大写*/
                //    tUpper(caMacBuf, cOutBuf);
                //if ((iRet = HSM_GenStdMac(sTak, caMacBuf, iLen, caMac)) < 0) {
                if ((iRet = tHsm_Gen_Cups_Mac(caMac, sTak, caMacBuf, iLen)) < 0)
                {
                    tLog(ERROR, "生成MAC出错[%d].", iRet);
                    return ( -2);
                }
                tDumpHex("计算", caMac, 8);
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8));
        default:
            tLog(ERROR, "组装的报文域[%d]不存在", iBitNo);
    }
    return ( 0);
}
#if 0

/* 打印交易位图 */
void BitMapChk(UCHAR *pcBitMap, char *pcMsg) {
    char sBuf[128] = {0}, sTmp[4];
    int i;

    for (i = 0; pcBitMap[i] != 255; i++)
    {
        sprintf(sTmp, "%d,", pcBitMap[i]);
        strcat(sBuf, sTmp);
    }

    tLog(DEBUG, "%s:\n\t %s", pcMsg, sBuf);
}

/* 将g_staBitMap中定义的pcBitMap, 转换为报文头中的pcaBitMap */
void SetBitMap(UCHAR *pcBitMap, char *pcaBitMap) {
    int i;
    int iMap, iMapi, iMapj;
    const int BYTE_BITS = 8;

    memset(pcaBitMap, 0x00, BITMAP_BYTE);

    /* 位图比特位填充 */
    for (i = 0; pcBitMap[i] != 255; i++)
    {
        iMap = pcBitMap[i] - 1;
        iMapi = iMap / BYTE_BITS;
        iMapj = iMap % BYTE_BITS;
        BIT_SET(pcaBitMap[iMapi], (0x01 << (7 - iMapj)));
    }

    /* 报文中包含有域66到域128中的域, BitMap第一位填1 */
    if (pcBitMap[i - 1] > 65)
    {
        BIT_SET(pcaBitMap[0], 0x80);
    }
}

/* 将报文头中的pcaBitMap, 转换为g_staBitMap中定义的pcBitMap */
void GetBitMap(char *pcaBitMap, UCHAR *pcBitMap) {
    int iMap, iMapi, iMapj;
    UCHAR *pcBit = pcBitMap;
    const int BYTE_BITS = 8;

    /* 位图比特位解析 */
    for (iMapi = 0, iMapj = 0; iMapi < BITMAP_BYTE; iMapj++)
    {
        if (BIT_TEST(pcaBitMap[iMapi], (0x01 << (7 - iMapj))))
        {
            iMap = iMapi * BYTE_BITS + iMapj + 1;

            /* Bit1,Bit65特殊含义, 不作处理 */
            if (iMap != 1 && iMap != 65)
            {
                *pcBit++ = (UCHAR) iMap;
            }
        }

        if (iMapj == 7)
        {
            iMapj = -1;
            iMapi++;
        }
    }

    *pcBit = 255;
}
#endif
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
    char sMsgType[5] = {0}, sProcCode[7] = {0}, sProcCodeEx[3] = {0}, sNmiCode[4] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0}, sTmp[MSG_MAX_LEN] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sDenialCode[5 + 1] = {0}, sRespCode[2 + 1] = {0};
    UCHAR *pcBitMap = NULL;
    char sDirecType[1 + 1] = {0}; // 0:人口方向,1:出口方向
    int iDenialCode = B_FALSE; /* 请求是否被银联拒绝 */

    GET_STR_KEY(pstDataJson, "msg", sTmp);
    int iMsgLen = strlen(sTmp);

    tAsc2Bcd(sMsg, (UCHAR*) sTmp, iMsgLen, LEFT_ALIGN);

    /** 拆包 **/
    g_pcMsg = sMsg + 46;
    g_iMsgLen = iMsgLen / 2 - 46;

    {
        /* 判断拒绝码是否正常,00000正常，否者包文是被打回的，拆包会失败 */
        tStrCpy(sDenialCode, sMsg + 41, 5);
        if (memcmp(sDenialCode, "00000", 5))
        {
            g_pcMsg = sMsg + 46 + 46;
            g_iMsgLen = iMsgLen / 2 - 46 - 46;
            iDenialCode = B_TRUE;
        }
    }


    tClearIso(&g_stIsoData);
    tLog(DEBUG, "tStr2Iso");
    if ((iRet = tStr2Iso(&g_stIsoData, g_pcMsg, g_iMsgLen)) < 0)
    {
        tLog(ERROR, "解析8583报文数据错误[%d].", iRet);
        return ( -1);
    }
    /* 如果是被银拒绝了，请求里有原交易的应答码，可能是成功的，直接解析，会伪造交易的 */
    if (B_TRUE == iDenialCode)
    {
        tIsoSetBit(&g_stIsoData, 39, "30", 2);
    }
    tDumpIso(&g_stIsoData, "应答报文");

    /* 解析出消息类型码 : 1 域  */
    tIsoGetBit(&g_stIsoData, 1, sMsgType, sizeof (sMsgType));
    if ((sMsgType[2] - '0') & 0x01)
    {
        /* 内部匹配用, 忽略请求/应答标识 */
        sMsgType[2]--;
        sDirecType[0] = '1';
    } else
    {
        if (B_TRUE == iDenialCode)
            sDirecType[0] = '1';
        else
            sDirecType[0] = '0';
    }
    /*设置入口方向 or 出口方向*/
    SET_STR_KEY(pstRepJson, "direct_type", sDirecType);
    /* 解析出处理码     : 3 域  */
    iRet = tIsoGetBit(&g_stIsoData, 3, sProcCode, sizeof (sProcCode));
    tTrim(sProcCode);

    /* 解析出扩展处理码 : 25域  */
    iRet = tIsoGetBit(&g_stIsoData, 25, sProcCodeEx, sizeof (sProcCodeEx));
    tTrim(sProcCodeEx);
    SET_STR_KEY(pstRepJson, "cond_code", sProcCodeEx);

    /* 解析出网络管理码 : 70域  */
    iRet = tIsoGetBit(&g_stIsoData, 70, sNmiCode, sizeof (sNmiCode));
    tTrim(sNmiCode);

    /* 解析出内部交易类型 */
    if (GetTranType(sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode) < 0)
    {
        tLog(ERROR, "无法解析的交易类型(MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
                , sMsgType, sProcCode, sProcCodeEx, sNmiCode);
        return -3;
    }
    //GET_STR_KEY(pstRepJson, "trans_code", sTransCode);
    tLog(ERROR, "交易类型[%s](MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
            , sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode);

    /*入口方向取交易码*/
    //   if (sDirecType[0] == '0') {
    SET_STR_KEY(pstRepJson, "trans_code", sTransCode);
    // }
    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++)
    {
        if ((!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)))
        {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0')
    {
        tLog(ERROR, "交易[%s][ProcCode:%s][ProcCodeEx:%s]未定义解包位图."
                , sTransCode, sProcCode, sProcCodeEx);
        return -3;
    }

    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++)
    {
        tLog(INFO, "GetBit域[%d].", pcBitMap[i]);
        iRet = GetFld(pcBitMap[i], pstRepJson);
        if (iRet < 0)
        {
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
    UCHAR sMsg[MSG_MAX_LEN] = {0}, sTmp[2048] = {0}, sBuf[16 + 1] = {0}, sLen[8 + 1] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sOTransCode[TRANS_CODE_LEN + 1] = {0};
    UCHAR *pcBitMap = NULL;
    char sDirecType[1 + 1] = {0}; // 0:人口方向,1:出口方向

    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "o_trans_code", sOTransCode);
    tTrim(sTransCode);

    /* 组装出外部交易类型 */
    if (GetMsgData(sTransCode, sOTransCode, NULL, sMsgType, sProcCode, sProcCodeEx, sNmiCode) < 0)
    {
        tLog(ERROR, "无法组装的交易类型(TransCode=[%s])!", sTransCode);
        return -3;
    }

    tLog(ERROR, "交易类型[%s](MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
            , sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode);
    /** 组包 **/
    tClearIso(&g_stIsoData);
    {
        /* 测试使用 */
        /*
                if (sTransCode[5] == '3') {
                    SET_STR_KEY(pstReqJson, "resp_code", "A0");
                    tLog(ERROR, "测试,冲正直接返回.");
                    return MAC_GEN_ERR;
                }
         */
    }
    /*判断出口方向 or 入口方向*/
    GET_STR_KEY(pstDataJson, "direct_type", sDirecType);
    if (sDirecType[0] == '0')
    {
        sMsgType[2]++;
    }
    /* 组包消息类型码   : 1 域  */
    tIsoSetBit(&g_stIsoData, 1, sMsgType, sizeof (sMsgType));

    /* 组包交易处理码   : 3 域  */
    tIsoSetBit(&g_stIsoData, 3, sProcCode, strlen(sProcCode));

    /* 组包扩展处理码   : 25域  */
    if (sProcCodeEx[0])
    {
        tIsoSetBit(&g_stIsoData, 25, sProcCodeEx, strlen(sProcCodeEx));
    }

    /* 组包网络管理码   : 70域  */
    if (sNmiCode[0])
    {
        tIsoSetBit(&g_stIsoData, 70, sNmiCode, strlen(sNmiCode));
    }
#if 0
    /* 组包原始信息域   : 90域  */
    {
        char sOrgMsgType[5], sOrgProcCode[7], sOrgProcCodeEx[3], sOrgNmiCode[4];
        if (GetMsgData(pcOldTransCode, pcOOldTransCode, "", sOrgMsgType, sOrgProcCode, sOrgProcCodeEx, sOrgNmiCode) >= 0)
        {
            tLog(DEBUG, "获取组装原始交易(TransCode=[%s], OldTransCode=[%s])的交易类型 => MsgType[%s]!"
                    , pcOldTransCode, pcOOldTransCode, sOrgMsgType);
            GET_STR_KEY(pstDataJson, "data1", pcdata1);
            strcpy(pcdata1 + 512, sOrgMsgType);
            SET_STR_KEY(pstDataJson, "data1", pcdata1);
        }
    }
#endif
    /* 取该交易的Bit Map标准定义 */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++)
    {
        if ((!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)))
        {
            pcBitMap = g_staBitMap[i].pcPackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0')
    {
        tLog(ERROR, "交易[%s][ProcCode:%s][ProcEx:%s]未定义组包位图."
                , sTransCode, sProcCode, sProcCodeEx);
        return -3;
    }

    /* 根据Bit Map标准定义解包各数据域 */
    for (i = 0; pcBitMap[i] != 255; i++)
    {
        tLog(INFO, "SetBit域[%d].", pcBitMap[i]);
        iRet = SetFld(pcBitMap[i], pstDataJson);
        if (iRet < 0)
        {
            tLog(ERROR, "组装交易[%s][MsgType:%s][ProcEx:%s]报文域[%d]出错[%d]!"
                    , sTransCode, sMsgType, sProcCodeEx, pcBitMap[i], iRet);
            return ( iRet);
        }
    }

    if ((iRet = tIso2Str(&g_stIsoData, sTmp, sizeof (sTmp))) < 0)
    {
        tLog(ERROR, "构建8583报文错误[%d]", iRet);
        return ( -1);
    }
    sprintf(sBuf, "%04d", (iRet + 46));
    tBcd2Asc((UCHAR*) sLen, (UCHAR*) sBuf, 8, LEFT_ALIGN);
    tDumpIso(&g_stIsoData, "请求报文");
    /* 数据报文     */
    tBcd2Asc((UCHAR*) sMsg + 92, (UCHAR*) sTmp, iRet << 1, LEFT_ALIGN);
    /* 报文头  */
    memcpy(sMsg, "2e020000000030303031303030302020203439303030303030202020000000003030303030303030003030303030", 92);
    tLog(DEBUG, "baowen len[%s]", sLen);
    memcpy(sMsg + 4, sLen, 8);
    SET_STR_KEY(pstReqJson, "msg", (const char*) sMsg);
    return ( 0);
}



