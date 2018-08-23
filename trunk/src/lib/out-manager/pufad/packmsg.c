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
#include "secapi.h"         /* �ӽ���   */
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcKeyAsc);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
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
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcKeyAsc);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
}

extern IsoData g_stIsoData;
char *g_pcMsg;
int g_iMsgLen;


TranTbl g_staTranTbl[] = {
    /* ��ϢID  ������    ��Ϣ������  NMI   ���״���  ԭ���״���  */
    /* һ���ཻ�� */
    { "0200", "300000", "01", "", "021000", "", ""}, /* POS�����            */
    { "0200", "190000", "22", "", "020000", "", ""}, /* POS����              */
    { "0200", "190000", "22", "", "T20000", "", ""}, /* POS����              */
    { "0400", "190000", "22", "", "020003", "", ""}, /* POS���ѳ���          */
    { "0200", "280000", "23", "", "020002", "", ""}, /* POS���ѳ���          */
    { "0400", "280000", "23", "", "020023", "", ""}, /* POS���ѳ�������      */
    { "0800", "", "00", "003", "029000", "", ""}, /* POSǩ��            */

    { "0620", "300000", "00", "951", "020400", "021000", ""}, /* POS�����ű�֪ͨ*/
    { "0620", "000000", "00", "951", "020400", "020000", ""}, /* POS���ѽű�֪ͨ*/
    { "", "", "", "", "", "", ""},
};


/*************
 **  ������  **
 *************/

/* POS����� */
static unsigned char g_caInqueryUnpackMap[] = {\
2, 11, 12, 13, 14, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 54, 55, 60, 63, 64, 255};
static unsigned char g_caInqueryPackMap[] = {\
2, 11, 22, 23, 25, 26, 41, 42, 35, 36, 49, 52, 53, 55, 60, 64, 255};

/* POS���� */
static unsigned char g_caPurchaseUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55, 60, 63, 64, 255};
static unsigned char g_caPurchasePackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 41, 42, 35, 36, 49, 52, 53, 55, 60, 64, 255};

/* POS���ѳ��� */
static unsigned char g_caUndoPurchaseUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 59, 55, 60, 63, 64, 255};
static unsigned char g_caUndoPurchasePackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 64, 255};


/* POS�������� */
static unsigned char g_caCorrectUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 55, 60, 64, 255};
static unsigned char g_caCorrectPackMap[] = {\
4, 11, 14, 22, 23, 25, 38, 39, 41, 42, 49, 53, 55, 60, 61, 64, 255};

/* POS���� */
static unsigned char g_caPurchaseRevUnpackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 55, 60, 64, 255};
static unsigned char g_caPurchaseRevPackMap[] = {\
4, 11, 14, 22, 23, 25, 38, 39, 41, 42, 49, 53, 55, 60, 61, 64, 255};


/* POS�ű�֪ͨ */
static unsigned char g_caJbtzUnpackMap[] = {\
2, 7, 11, 12, 13, 15, 23, 25, 32, 33, 38, 39, 41, 42, 49, 55, 57, 60, 255};
static unsigned char g_caJbtzPackMap[] = {\
2, 4, 7, 11, 12, 13, 18, 22, 23, 32, 33, 37, 39, 41, 42, 43, 49, 55, 60, 255};

/*************
 **  ������  **
 *************/

/* �ַ�ǩ�� */
static unsigned char g_caLoginUnpackMap[] = {\
11, 12, 13, 32, 37, 39, 41, 42, 60, 62, 255};
static unsigned char g_caLoginPackMap[] = {\
11, 41, 42, 60, 63, 255};



TranBitMap g_staBitMap[] = {
    /* ���״���   ԭ���״���  ԭԭ������ �����λͼ                       �����λͼ                      */
    { "021000", "", "", g_caInqueryUnpackMap, g_caInqueryPackMap}, /* �����            */
    { "020000", "", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* ����              */
    { "T20000", "", "", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* T0����              */
    { "020003", "", "", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* ���ѳ���          */
    { "020002", "", "", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* ���ѳ���          */
    { "020023", "", "", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* ���ѳ�������      */
    { "029000", "", "", g_caLoginUnpackMap, g_caLoginPackMap}, /* �ַ�ǩ��    */

    { "020400", "020000", "", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS���ѽű�֪ͨ          */
    { "020400", "021000", "", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS����ű�֪ͨ          */
    { "020400", "023000", "100000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS�ű�֪ͨ          */
    { "020400", "023000", "200000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS�ű�֪ͨ          */
    { "020400", "023000", "300000", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* POS�ű�֪ͨ          */
    { "", "", "", NULL, NULL}
};

/*
 * ����POS���ĵ���Ϣ�����롢�����롢��չ�����롢���������Ϣ�룬�ڶ�������жϲ��ҽ�������
 */
int GetTranType(char *pcTransCode, char *pcMsgType, char *pcProcCode, char *pcProcCodeEx, char *pcNmiCode) {
    int i;

    /* ��ȡ��ǰ��������(���ݴӷ����������Ľ������ϢID�������롢��Ϣ�����롢���������) */
    for (i = 0; g_staTranTbl[i].sMsgType[0] != '\0'; i++) {
        if ((!memcmp(g_staTranTbl[i].sMsgType, pcMsgType, strlen(g_staTranTbl[i].sMsgType)))
                && (!memcmp(g_staTranTbl[i].sProcCode, pcProcCode, (g_staTranTbl[i].sProcCode[0] ? 2 : 0))) /* �ǿձȽ�ǰ2λ */
                //  && (!memcmp(g_staTranTbl[i].sProcCodeEx, pcProcCodeEx, strlen(g_staTranTbl[i].sProcCodeEx)))
                //  && (!memcmp(g_staTranTbl[i].sNmiCode, pcNmiCode, strlen(g_staTranTbl[i].sNmiCode)))
                ) {

            strcpy(pcTransCode, g_staTranTbl[i].sTransCode);
            tLog(DEBUG, "������[%s] ԭ������[%s] ԭԭ������[%s]"
                    , g_staTranTbl[i].sTransCode, g_staTranTbl[i].sOldTransCode, g_staTranTbl[i].sOOldTransCode);

            return ( 0);
        }
    }

    return ( -1);
}

/*
 * �����ڲ����ĵĽ��״��롢ԭ���״��룬�ڶ�����в�����Ϣ�����롢�����롢��չ�����롢���������
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
        case 2: /* ���˺� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_no", sBuf);
            break;
        case 4: /* ���׽�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "amount", sBuf);
            break;
        case 7: /* ���״���ʱ�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "transmit_time", sBuf);
            break;
        case 11: /* ϵͳ���ٺ� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "sys_trace", sBuf);
            break;
        case 12: /* ����ʱ�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "trans_time", sBuf);
            break;
        case 13: /* �������� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /* ��������ֻ����4λMMDD����������ʱ��Ҫ-1�� */
            tGetDate(sTransDate, "", -1);
            if (memcmp(sTransDate + 4, sBuf, 4))
                tAddDay(sTransDate, -1);
            tStrCpy(sTransDate + 4, sBuf, 4);
            SET_STR_KEY(pstJson, "trans_date", sBuf);
            break;
        case 14: /* ����Ч�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_exp_date", sBuf);
            break;
        case 15: /* �������� */
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
        case 23: /* ��Ƭ���к� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_sn", sBuf);
            break;
        case 25: /* ����������� */
            /* ��ModuleUnpack�д���, =���״�����2 */
            return 0;
        case 32: /* ���������ʶ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "acq_id", sBuf);
            break;
        case 33: /* ���ͻ�����ʶ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "fwd_inst_id", sBuf);
            break;
        case 35: /* 2�ŵ����� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "track2", sBuf);
            break;
        case 36: /* 3�ŵ����� */
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
        case 38: /* ��Ȩ�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "auth_code", sBuf);
            break;
        case 39: /* Ӧ���� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "istresp_code", sBuf);
            tLog(DEBUG, "istresp_code=[%s]", sBuf);
            break;
        case 41: /* �ն˱�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "channel_term_id", sBuf);
            break;
        case 42: /* �̻���� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "channel_merch_id", sBuf);
            break;
        case 48: /* ��������.�÷�10: ����Կ(2����,3����) */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            strncpy(sBuf1, sBuf + 2, 16);
            tBcd2Asc(sBuf, sBuf1, (iRet - 2) << 1, LEFT_ALIGN);
            SET_STR_KEY(pstJson, "data1", sBuf);
            break;
        case 49: /* ���׻��Ҵ��� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, 3);
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "ccy_code", sBuf);
            break;
        case 53: /* ��ȫ������Ϣ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, 16);
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "secure_ctrl", sBuf);
        case 54: /* �˻���� */
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
        case 55: /* ic�������� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tBcd2Asc(sIc, sBuf, iRet << 1, LEFT_ALIGN);
            SET_INT_KEY(pstJson, "ic_len", iRet);
            SET_STR_KEY(pstJson, "ic_data", sIc);
            break;
        case 57:/*������*/
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "57_data", sBuf);
            break;
        case 60: /* �Զ����� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "60_data", sBuf);
            break;
        case 61: /* ԭʼ������Ϣ: �������͡�ϵͳ���ٺš�ϵͳ����ʱ�䡢����������ʶ���յ�������ʶ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tStrCpy(sOldRrn, sBuf + 4, 6);
            SET_STR_KEY(pstJson, "o_rrn", sOldRrn);
            break;

        case 62: /* ������������ �ն���Կ��Ϣ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "pufa_inst_key", sBuf);
            break;

        case 64: /* MAC */
            memcpy(caMac, g_pcMsg + g_iMsgLen - 8, 8);
            //tLog(DEBUG, "caMac[%s]", caMac);
            /* ��ԭ���ĵ�MAC Buffer */
            memcpy(caMsgBuf, g_pcMsg, g_iMsgLen - 8);
            iLen = g_iMsgLen - 8;

            GET_STR_KEY(pstJson, "channel_merch_id", sMerchId);
            GET_STR_KEY(pstJson, "channel_term_id", sTermId);

            if (GetChannelTermKey(sTak, "ZAK", sMerchId, sTermId) < 0) {
                tLog(ERROR, "��ȡ�ն�[%s:%s]����Կ����,��������.", sMerchId, sTermId);
                return ( -1);
            }
            tLog(DEBUG, "ZAK[%s]", sTak);

            if ((iRet = tHsm_Gen_Pufa_Mac(caTmp, sTak, caMsgBuf, iLen)) < 0) {
                tLog(ERROR, "����MAC����[%d].", iRet);
                return ( -2);
            }
            tDumpHex("�ַ��·�", caMac, 8);
            tDumpHex("����", caTmp, 8);
            if (memcmp(caTmp, caMac, 8)) {
                SET_STR_KEY(pstJson, "resp_code", "A0");
                {
                    /*��ӡmacֵ,�ü��*/
                }
                tLog(ERROR, "У��MAC����.");
                return -1;
            }
            break;
        default:
            tLog(ERROR, "���ı�����[%d]������", iBitNo);
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
        case 2: /* ���˺� */
            GET_STR_KEY(pstJson, "card_no", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 3:
            break;
        case 4: /* ���׽�� */
            GET_DOU_KEY(pstJson, "amount", dAmount);
            tLog(DEBUG, "%f", dAmount);
            if (fabs(dAmount) < 0.00001)
                return 0;
            sprintf(sBuf, "%012.f", dAmount);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, AMOUNT_LEN);
        case 11: /* ϵͳ���ٺ� */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            if (sTransCode[5] == '3') {
                GET_STR_KEY(pstJson, "o_sys_trace", sBuf);
            } else {
                GET_STR_KEY(pstJson, "sys_trace", sBuf);
            }
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 12: /* ����ʱ�� */
            GET_STR_KEY(pstJson, "trans_time", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 13: /* �������� */
            GET_STR_KEY(pstJson, "trans_date", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf + 4, 4);
        case 14: /* ����Ч�� */
            GET_STR_KEY(pstJson, "card_exp_date", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 15: /* �������� */
            GET_STR_KEY(pstJson, "cups_settle_data", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 22: /*  ��������뷽ʽ*/
            GET_STR_KEY(pstJson, "input_mode", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 23: /* ��Ƭ���к� */
            GET_STR_KEY(pstJson, "card_sn", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 25: /* ����������� */
            GET_STR_KEY(pstJson, "cond_code", sBuf);
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            if (!memcmp(sTransCode, "021000", 6)) {
                return tIsoSetBit(&g_stIsoData, iBitNo, "00", 2);
            }
            return tIsoSetBit(&g_stIsoData, iBitNo, "82", 2);
        case 26: /* �����PIN��ȡ�� */
            GET_STR_KEY(pstJson, "input_mode", sInputMode);
            if (sInputMode[2] == '2')
                return 0;
            GET_STR_KEY(pstJson, "pin_code", sBuf);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));
        case 32: /* ���������ʶ */
            GET_STR_KEY(pstJson, "channel_merch_id", sTmp);
            memset(sBuf, 0, sizeof (sBuf));
            memcpy(sBuf, "4900", 4);
            tStrCpy(sBuf + 4, sTmp + 3, 4);
            return tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf));

        case 35: /* 2�� */
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
        case 36: /* 3�� */
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
        case 38: /* ��Ȩ�� */
            GET_STR_KEY(pstJson, "auth_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 39: /* Ӧ���� */
            /* �ַ�����������06*/
            //     GET_STR_KEY(pstJson, "resp_code", sBuf);
            strncpy(sBuf, "06", 2);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 41: /* �ն˱�� */
            GET_STR_KEY(pstJson, "channel_term_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, 8));
        case 42: /* �̻���� */
            GET_STR_KEY(pstJson, "channel_merch_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, 15));

        case 48: /* ���������� */
            GET_STR_KEY(pstJson, "48_data", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 49: /* ���׻��Ҵ��� */
            GET_STR_KEY(pstJson, "ccy_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 52: /* Pin */
            GET_STR_KEY(pstJson, "input_mode", sBuf);
            if (sBuf[2] == '2')
                return 0;
            GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
            GET_STR_KEY(pstJson, "channel_term_id", sChannelTermId);

            if (GetChannelTermKey(sChannelTpk, "ZPK", sChannelMerchId, sChannelTermId) < 0) {
                tLog(ERROR, "��ȡ����PIN_KEY��Կ����.");
                SET_STR_KEY(pstJson, "resp_code", "A0");
                return (-2);
            }
            tLog(INFO, "sTpk channel[%s]", sChannelTpk);

            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);

            if (GetTermKey(sTpk, sMerchId, sTermId, "PIN_KEY") < 0) {

                tLog(ERROR, "��ȡ�ն�[%s:%s]����Կ����.", sMerchId, sTermId);
                return ( -2);
            }
            tLog(INFO, "sTpk term[%s]", sTpk);

            GET_STR_KEY(pstJson, "card_no", sCardNo);
            GET_STR_KEY(pstJson, "pin", sPin);
            tLog(INFO, "card_no[%s],pin[%s]", sCardNo, sPin);
            //if (HSM_TranPin_Tpk2Zpk(sCardNo, sPin, sTpk, sChannelTpk, sBuf) < 0) {
            if (tHsm_TranPin(sBuf, sCardNo, sPin, sTpk, sChannelTpk) < 0) {
                tLog(ERROR, "������Ϣת��ʧ��.");
                return -2;
            }
            tLog(DEBUG, "new sPin[%s]", sBuf);
            tAsc2Bcd(caTmp, sBuf, 16, 0);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, caTmp, 8));
        case 53: /* ��ȫ������Ϣ */
            /* ������Կ: 2.����MAC KEY 1.����PIN KEY */
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
        case 55: /* ic�������� */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            GET_STR_KEY(pstJson, "input_mode", sInputMode);
            //            && (sInputMode[1] == '5' || sInputMode[1] == '7')
            //                tLog(INFO, "����ic����9F06");
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
        case 60: /* �Զ�����60: ��Ϣ�����롢���κš����������Ϣ�� */
            /* �����ModuleUnpack�ﴦ�� */
            /*
             * ����Ԫ���� N3
             * 60.1 ��Ϣ������ N2
             * 60.2 ���κ� N6
             * 60.3 ���������Ϣ�� N3
             * 60.4 �ն˶�ȡ���� N1
             * 60.5 ����PBOC�� / ���Ǳ�׼��IC���������� N1
             */
            break;
        case 61:
            /*
             * 
            ����������������÷���
            1���ڳ��������������У�������дԭʼ������Ϣ��
            2��������������������д�ֿ��������֤��Ϣ��
             */
            GET_STR_KEY(pstJson, "trans_code", sBuf);
            if (!memcmp(sBuf, "020000", 6) || !memcmp(sBuf, "021000", 6)) {
                return 0;
            }
            /*
            ����Ԫ���� N3
                    61.1 ԭʼ�������κ� N6
                    61.2 ԭʼ����POS��ˮ�� N6
                    61.3 ԭʼ�������� N4
                    61.4ԭ������Ȩ��ʽ N2
                    61.5ԭ������Ȩ�������� N11
             */
            GET_STR_KEY(pstJson, "o_batch_no", sBuf);
            tStrCpy(sBuf1, sBuf, 6);
            GET_STR_KEY(pstJson, "o_sys_trace", sBuf);
            tStrCpy(sBuf1 + 6, sBuf, 6);
            GET_STR_KEY(pstJson, "o_trans_date", sBuf);
            tStrCpy(sBuf1 + 6 + 6, sBuf + 4, 4);
            tStrCpy(sBuf1 + 6 + 6 + 4, "0000000000000", 13);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf1, strlen(sBuf1)));

        case 63:/* �Զ������÷�1: ����Ա���� */
            GET_STR_KEY(pstJson, "teller_no", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));

        case 64: /* MAC */
            /* �鱨�ĵ�MAC Buffer */
            GET_STR_KEY(pstJson, "channel_merch_id", sMerchId);
            GET_STR_KEY(pstJson, "channel_term_id", sTermId);

            tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8);

            if ((iLen = tIso2Str(&g_stIsoData, (UCHAR*) caMacBuf, sizeof (caMacBuf))) < 0)
                return ( -1);

            if (GetChannelTermKey(sTak, "ZAK", sMerchId, sTermId) < 0) {
                tLog(ERROR, "��ȡ�ն�[%s:%s]����Կ����,��������.", sMerchId, sTermId);
                return ( -1);
            }

            if ((iRet = tHsm_Gen_Pufa_Mac(caMac, sTak, caMacBuf, iLen - 8)) < 0) {
                tLog(ERROR, "����MAC����[%d].", iRet);
                return ( -2);
            }
            tDumpHex("����", caMac, 8);

            return ( tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8));

        default:
            tLog(ERROR, "��װ�ı�����[%d]������", iBitNo);
    }
    return ( 0);
}

/* == CUPS���Ľṹ == */

/* Field1   ͷ����          1   **
 ** Field2   ͷ��ʶ�Ͱ汾��  1   **
 ** Field3   �������ĳ���    4   **
 ** Field4   Ŀ��ID          11  **
 ** Field5   ԴID            11  **
 ** Field6   ����ʹ��        3   **
 ** Field7   ���κ�          1   **
 ** Field8   ������Ϣ        8   **
 ** Field9   �û���Ϣ        1   **
 ** Field10  �ܾ���          5   */

/* 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25   **
 ** [���ĳ��� ] [1][2][--3����--] [--      4  Ŀ��ID   11B     --] [-- 5 Դ��ַ  **
 **                               "00010000   "                    "30022900   " **
 **             46 01             30 30 30 31 30 30 30 30 20 20 20 33 30 30 32   **
 **                                                                              **
 ** 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50   **
 ** 5  Դ��ַ   11B  --] [������] [7][-- 8 ������Ϣ 8B   --] [9][--10 �ܾ���5B]  **
 **                                                                              **
 ** 32 39 30 30 20 20 20 00 00 00 00 30 30 30 30 30 30 30 30 00 30 30 30 30 30   **
 **                                                                              **
 ** 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75   **
 ** [�������� ][--       λͼ1 8B   --] [--      λͼ2 8B   --] [--  ������  --  **
 **  4B  ASC                                                                     **
 **                                                                              */

/* ���ڷ���: ���Ӧ����; ��ڷ���: ���������. */
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
    /** ��� **/

    g_pcMsg = sMsg + 11;
    g_iMsgLen = iMsgLen / 2 - 11;
    tClearIso(&g_stIsoData);
    tLog(DEBUG, "tStr2Iso");
    if ((iRet = tStr2Iso(&g_stIsoData, g_pcMsg, g_iMsgLen)) < 0) {
        tLog(ERROR, "����8583�������ݴ���[%d].", iRet);
        return ( -1);
    }
    tDumpIso(&g_stIsoData, "Ӧ����");

    /* ��������Ϣ������ : 1 ��  */
    tIsoGetBit(&g_stIsoData, 1, sMsgType, sizeof (sMsgType));
    /* ����Ӧ����Ϣ�������һ */
    sMsgType[2]--;

    /* ������������     : 3 ��  */
    iRet = tIsoGetBit(&g_stIsoData, 3, sProcCode, sizeof (sProcCode));
    tTrim(sProcCode);

    /* �����Զ�����60: ��Ϣ�����롢���κš����������Ϣ�� */
    tIsoGetBit(&g_stIsoData, 60, sMsgData, sizeof (sMsgData));
    /* ��Ϣ������ */
    tStrCpy(sProcCodeEx, sMsgData, 2);
    tTrim(sProcCodeEx);
    SET_STR_KEY(pstRepJson, "cond_code", sProcCodeEx);
    /* ����Ҫ���κ�ֱ������ +6 */
    /* ��ȡ��������� */
    tStrCpy(sNmiCode, sMsgData + 8, 3);


    /* �������ڲ��������� */
    if (GetTranType(sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode) < 0) {
        tLog(ERROR, "�޷������Ľ�������(MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
                , sMsgType, sProcCode, sProcCodeEx, sNmiCode);
        return -3;
    }
    //GET_STR_KEY(pstRepJson, "trans_code", sTransCode);
    tLog(ERROR, "��������[%s](MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
            , sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode);

    SET_STR_KEY(pstRepJson, "trans_code", sTransCode);

    /* ȡ�ý��׵�Bit Map��׼���� */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if ((!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6))) {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "����[%s][ProcCode:%s][ProcCodeEx:%s]δ������λͼ."
                , sTransCode, sProcCode, sProcCodeEx);
        return -3;
    }

    /* ����Bit Map��׼�������������� */
    for (i = 0; pcBitMap[i] != 255; i++) {
        tLog(INFO, "GetBit��[%d].", pcBitMap[i]);
        iRet = GetFld(pcBitMap[i], pstRepJson);
        if (iRet < 0) {
            tLog(ERROR, "��⽻��[%s][MsgType:%s][ProcEx:%s]������[%d]����[%d]!"
                    , sTransCode, sMsgType, sProcCodeEx, pcBitMap[i], iRet);
            return ( iRet);
        }
    }
    return 0;
}

/* ���ڷ���: ���������; ��ڷ���: ���Ӧ����. */
int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {

    int i = 0, iRet = -1;
    char sMsgType[5] = {0}, sProcCode[7] = {0}, sProcCodeEx[3] = {0}, sNmiCode[4] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0}, sTmp[2048] = {0}, sBuf[16 + 1] = {0}, sLen[8 + 1] = {0}, sMsgData[17] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sOTransCode[TRANS_CODE_LEN + 1] = {0}, sBatchNo[BATCH_NO_LEN + 1] = {0};
    UCHAR *pcBitMap = NULL;

    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "o_trans_code", sOTransCode);
    tTrim(sTransCode);

    /* ��װ���ⲿ�������� */
    if (GetMsgData(sTransCode, sOTransCode, NULL, sMsgType, sProcCode, sProcCodeEx, sNmiCode) < 0) {
        tLog(ERROR, "�޷���װ�Ľ�������(TransCode=[%s])!", sTransCode);
        return -3;
    }

    tLog(ERROR, "��������[%s](MsgType=%s,ProcCode=%s,ProcCodeEx=%s,NmiCode=%s)!"
            , sTransCode, sMsgType, sProcCode, sProcCodeEx, sNmiCode);
    /** ��� **/
    tClearIso(&g_stIsoData);

    /* �����Ϣ������   : 1 ��  */
    tIsoSetBit(&g_stIsoData, 1, sMsgType, sizeof (sMsgType));

    /* ������״�����   : 3 ��  */
    tIsoSetBit(&g_stIsoData, 3, sProcCode, strlen(sProcCode));

    /*��ȡ���κ�*/
    GET_STR_KEY(pstDataJson, "batch_no", sBatchNo);

    /* ����Զ�����60: ��Ϣ�����롢���κš����������Ϣ�� */
    /*60.4�� ����ʹ��
         �ն���ʱ���ã�����ں���������Ĭ���0����
     60.5�� ����ʹ��
         �ն���ʱ���ã�����ں���������Ĭ���00����
     60.6��  �ն˶�ȡ����
      
     */
    sprintf(sMsgData, "%2s%6s%3s%s", sProcCodeEx, sBatchNo, sNmiCode, "00060");
    tLog(INFO, "sMsgData[%s]", sMsgData);
    tLog(INFO, "sMsgLen[%d]", strlen(sMsgData));
    tIsoSetBit(&g_stIsoData, 60, sMsgData, strlen(sMsgData));

    /* ȡ�ý��׵�Bit Map��׼���� */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if ((!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6))) {
            pcBitMap = g_staBitMap[i].pcPackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "����[%s][ProcCode:%s][ProcEx:%s]δ�������λͼ."
                , sTransCode, sProcCode, sProcCodeEx);
        return -3;
    }

    /* ����Bit Map��׼�������������� */
    for (i = 0; pcBitMap[i] != 255; i++) {
        tLog(INFO, "SetBit��[%d].", pcBitMap[i]);
        iRet = SetFld(pcBitMap[i], pstDataJson);
        if (iRet < 0) {
            tLog(ERROR, "��װ����[%s][MsgType:%s][ProcEx:%s]������[%d]����[%d]!"
                    , sTransCode, sMsgType, sProcCodeEx, pcBitMap[i], iRet);
            return ( iRet);
        }
    }

    if ((iRet = tIso2Str(&g_stIsoData, sTmp, sizeof (sTmp))) < 0) {
        tLog(ERROR, "����8583���Ĵ���[%d]", iRet);
        return ( -1);
    }

    tDumpIso(&g_stIsoData, "������");
    /* ���ݱ���     */
    tBcd2Asc((UCHAR*) sMsg + 22, (UCHAR*) sTmp, iRet << 1, LEFT_ALIGN);
    /* ����ͷ  */
    memcpy(sMsg, "600005000060310018110B", 22);

    SET_STR_KEY(pstReqJson, "msg", (const char*) sMsg);
    return ( 0);
}



