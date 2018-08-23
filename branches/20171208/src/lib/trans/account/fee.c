/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "merch.h"
#include "t_macro.h"

int ChkMerchNosign(char * pstMerchId) {
    char sSqlStr[512];
    int iNum = 0;
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT COUNT(1) FROM B_MERCH_DOUBLE_FREE \
      WHERE USER_CODE = ( SELECT USER_CODE FROM B_MERCH WHERE MERCH_ID='%s') AND STATUS='1'", pstMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iNum);
    }
    if (0 == iNum) {
        tLog(INFO, "�̻�δ��ͨ˫��ҵ��");
        tReleaseRes(pstRes);
        return 1;
    } else if (1 == iNum) {
        tLog(INFO, "�̻��ѿ�ͨ˫��ҵ��");
        tReleaseRes(pstRes);
        return 0;
    }
}

int IsSaleMcc(char *pcMcc) {
    char sSqlStr[512] = {0}, sValue[128] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select consume_type from s_mcc where mcc='%s'", pcMcc);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "����mcc[%s]�Ƿ��ճ�����mcc.", pcMcc);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sValue);
        tTrim(sValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "��Mcc[%s]�ļ�¼.", pcMcc);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return atoi(sValue);
}

/* �����ַ��� */
int GetTagValue(char *pcTag, char *pcValue, char *pcBuf) {
    int iLen = 0, iOffset = 0, iTagLen = 0;
    char sTag[3] = {0}, sTagLen[3] = {0}, sTmp[16] = {0};
    char *pcTmp = NULL;

    iLen = strlen(pcTag);
    pcTmp = strstr(pcBuf, pcTag);
    if (pcTmp == NULL) {
        return 0;
    }
    tStrCpy(sTagLen, pcTmp + iLen, 2);
    tAsc2Bcd(sTmp, sTagLen, 2, LEFT_ALIGN);
    iTagLen = sTmp[0] *2;
    tStrCpy(pcValue, pcTmp + iLen + 2, iTagLen);
    tLog(DEBUG, "Tag[%s] Len[%d] Value[%s]", pcTag, iTagLen, pcValue);
    return iTagLen;
}

/* ������˫�� */
int CalcMerchFee2Free(cJSON *pstTransJson) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sCardType[2] = {0}, sMcc[4 + 1] = {0};
    char sTmp[128] = {0}, sValue55[1024] = {0}, sInputMode[3 + 1] = {0}, sCardBin[32 + 1] = {0};
    char sChannelMerchId[15 + 1] = {0};
    int i55Len = 0;
    char sMsgData[256] = {0};
    double dAmount = 0.0, dNfcAmount = 0.0;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChannelMerchId);
    tStrCpy(sMcc, sChannelMerchId + 7, 4);
    GET_STR_KEY(pstTransJson, "ic_data", sValue55);
    GET_INT_KEY(pstTransJson, "ic_len", i55Len);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_bin", sCardBin);
    GET_STR_KEY(pstTransJson, "60_data", sMsgData)
    dAmount = dAmount / 100;

    if (FindValueByKey(sTmp, "yc.2free.amount") < 0) {
        ErrHanding(pstTransJson, "96", "����key[yc.2free.amount]������nfc�޶�,ʧ��.");
        return -1;
    }
    dNfcAmount = atof(sTmp);
    if (DBL_CMP(dAmount, dNfcAmount)) {
        tLog(ERROR, "˫���޶�[%f],��ǰ���׶�[%f].", dNfcAmount, dAmount);
        return 0;
    }
    if (memcmp(sInputMode, "072", 3)) {
        tLog(ERROR, "˫�����뷽ʽ������[072],��ǰ��ʽ[%s].", sInputMode);
        return 0;
    }
    if (0 == IsSaleMcc(sMcc)) {
        tLog(ERROR, "��ǰ����MCC[%s]���ճ�������.", sMcc);
        return 0;
    }
    /* 60.2.6��1 */
#if 0 /* pos��60����û�������ܱ�ʶ����������������ȡ���ܿ�����ʶ */
    if (sMsgData[2 + 6 + 3] != '1') {
        tLog(ERROR, "60������[%s]�����ܱ�־.", sMsgData);
        return 0;
    }
#endif 
    if (ChkMerchNosign(sMerchId) != 0) {
        tLog(ERROR, "�̻�δ��ͨ˫��");
        return 0;
    }
    
    return 1;
}

/* ���������� */
int CalcMerchFeeNfc(cJSON *pstTransJson) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sCardType[2] = {0}, sMcc[4 + 1] = {0};
    char sTmp[128] = {0}, sValue55[1024] = {0}, sInputMode[3 + 1] = {0}, sCardBin[32 + 1] = {0};
    char s55Tag[1024] = {0}, ca9F10Tag[1024] = {0};
    int i55Len = 0, i9F10Len = 0;
    char sChannelMerchId[15 + 1] = {0};
    char sValue9F10[256] = {0};
    double dAmount = 0.0, dNfcAmount = 0.0;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "channel_merch_id", sChannelMerchId);
    tStrCpy(sMcc, sChannelMerchId + 7, 4);
    GET_STR_KEY(pstTransJson, "ic_data", sValue55);
    GET_INT_KEY(pstTransJson, "ic_len", i55Len);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_bin", sCardBin);
    dAmount = dAmount / 100;
    /* 
     * ����������
     *  NFC�ƶ�֧��
     */

    /* �ж����~ */
    if (FindValueByKey(sTmp, "yc.nfc.amount") < 0) {
        ErrHanding(pstTransJson, "96", "����key[yc.nfc.amount]������nfc�޶�,ʧ��.");
        return -1;
    }
    dNfcAmount = atof(sTmp);
    /* �ճ�����mcc */
    /* ���׽�� */
    if (DBL_CMP(dNfcAmount, dAmount) && 1 == IsSaleMcc(sMcc)) {
        /* ���⿨bin  ������*/
        if (sInputMode[1] == '2') {
            tLog(ERROR, "����IC��[%s].", sInputMode);
            if (NULL != strstr("621475,621757,625910", sCardBin)) {
                tLog(ERROR, "��bin[%s]�����⿨bin.", sCardBin);
                return 1;
            } else {
                tLog(ERROR, "��bin[%s]�������⿨bin.", sCardBin);
                return 0;
            }
        }

        /* �ж�55�� */
        tLog(DEBUG, "55[%d][%s]", i55Len, sValue55);
        /*
        if (GetTagValue("9F10", sValue9F10, sValue55) == 0) {
            tLog(ERROR, "����û������9F10.");
            return 0;
        }
         */
        tAsc2Bcd(s55Tag, sValue55, i55Len*2, LEFT_ALIGN);
        if (tFindTag(0x9F10, ca9F10Tag, (short *) &i9F10Len, s55Tag, i55Len) < 0) {
            tLog(ERROR, "����û������9F10.");
            return 0;
        }
        tBcd2Asc(sValue9F10, ca9F10Tag, i9F10Len*2, LEFT_ALIGN);
        tLog(DEBUG, "9F10=[%d][%s].", i9F10Len*2, sValue9F10);
        /* applePay 
         * 10�ֽ�07 11�ֽ�01 17�ֽ�80 18�ֽ�B*
         */
        if (!memcmp(sValue9F10 + 18, "07", 2)
                && !memcmp(sValue9F10 + 20, "01", 2)
                && !memcmp(sValue9F10 + 32, "80", 2)
                && !memcmp(sValue9F10 + 34, "B", 1)) {
            tLog(ERROR, "����ʹ��ApplePay֧����ʽ.");
            return 1;
        } else if (!memcmp(sValue9F10 + 18, "A1", 2)) {
            tLog(ERROR, "����ʹ��HCE֧����ʽ.");
            return 1;
        } else if (!memcmp(sValue9F10 + 18, "07", 2)
                && !memcmp(sValue9F10 + 20, "02", 2)
                && !memcmp(sValue9F10 + 32, "80", 2)
                && !memcmp(sValue9F10 + 34, "30", 2)
                && !memcmp(sValue9F10 + 36, "43", 2)) {
            tLog(ERROR, "����ʹ��SamSungPay-IC֧����ʽ.");
            return 1;
        } else if (!memcmp(sValue9F10 + 18, "07", 2)
                && !memcmp(sValue9F10 + 20, "02", 2)
                && !memcmp(sValue9F10 + 32, "80", 2)
                && !memcmp(sValue9F10 + 34, "30", 2)
                && !memcmp(sValue9F10 + 36, "41", 2)) {
            tLog(ERROR, "����ʹ�û�ΪPay֧����ʽ.");
            return 1;
        } else if (!memcmp(sValue9F10 + 18, "07", 2)
                && !memcmp(sValue9F10 + 20, "02", 2)
                && !memcmp(sValue9F10 + 32, "80", 2)
                && !memcmp(sValue9F10 + 34, "30", 2)
                && !memcmp(sValue9F10 + 36, "42", 2)) {
            tLog(ERROR, "����ʹ��С��Pay֧����ʽ.");
            return 1;
        }
    }
    return 0;
}

/* �ж����������� */
int ChkMerchFeeType(cJSON *pstJson, int *piFlag) {
    char sMerchType[2 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    /* nfc */
    if (CalcMerchFeeNfc(pstTransJson) > 0) {
        SET_STR_KEY(pstTransJson, "fee_type", "YN");
        return 0;
        /* ˫�� */
    }
    if (CalcMerchFee2Free(pstTransJson) > 0) {
        SET_STR_KEY(pstTransJson, "fee_type", "YM");
        return 0;
    }
    GET_STR_KEY(pstTransJson, "merch_type", sMerchType);
    /* MERCH_TYPE �̻�����: 0:��׼��1:�Żݣ�2:���� */
    if (sMerchType[0] == '1') {
        SET_STR_KEY(pstTransJson, "fee_type", "Y");
    } else if (sMerchType[0] == '2') {
        SET_STR_KEY(pstTransJson, "fee_type", "M");
    } else {
        SET_STR_KEY(pstTransJson, "fee_type", "B");
    }
    return 0;
}

/* �������п������� */
int MerchCardFee(cJSON *pstJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sCardType[2] = {0}, sFeeDesc[255 + 1] = {0}, sMcc[4 + 1] = {0};
    char sTmp[128] = {0}, sValue55[1024] = {0}, sInputMode[3 + 1] = {0}, sCardBin[32 + 1] = {0};
    char sFeeType[2 + 1] = {0};
    MerchFee stMerchFee;
    double dAmount = 0.0, dFee = 0.0, dRate = 0.0L, dNfcAmount = 0.0, dMax = 0;
    int iTrue = 0;
    cJSON * pstTransJson = NULL, *pstAgentJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "mcc", sMcc);
    GET_STR_KEY(pstTransJson, "ic_data", sValue55);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_bin", sCardBin);
    GET_STR_KEY(pstTransJson, "fee_type", sFeeType);

    dAmount = dAmount / 100;
    if (FindMerchFee(&stMerchFee, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]�ķ�������.", sMerchId);
        return -1;
    }
    if (!memcmp(sFeeType, "YN", 2)) {
        tLog(ERROR, "����ʹ��������NFC����.");
        /* 
         * ����������
         *  NFC�ƶ�֧��
         */
        if (sCardType[0] == '0') {
            tLog(DEBUG, "��ǿ�[%f].", stMerchFee.dYcNfcDRate);
            dRate = stMerchFee.dYcNfcDRate / 100;
            sprintf(sFeeDesc, "%f", dRate);
        } else {
            tLog(DEBUG, "���ÿ�[%f].", stMerchFee.dYcNfcCRate);
            dRate = stMerchFee.dYcNfcCRate / 100;
            sprintf(sFeeDesc, "%f", dRate);
        }
    } else if (!memcmp(sFeeType, "YM", 2)) {
        tLog(ERROR, "����ʹ��������˫�����.");
        if (sCardType[0] == '0') {
            tLog(DEBUG, "��ǿ�[%f].", stMerchFee.dYc2FeeDRate);
            dRate = stMerchFee.dYc2FeeDRate / 100;
            sprintf(sFeeDesc, "%f", dRate);
        } else {
            tLog(DEBUG, "���ÿ�[%f].", stMerchFee.dYc2FeeCRate);
            dRate = stMerchFee.dYc2FeeCRate / 100;
            sprintf(sFeeDesc, "%f", dRate);
        }
    } else {
        tLog(ERROR, "����ʹ��һ�����[%s]����.", sFeeType);
        /* һ����ʼ��� */
        if (sCardType[0] == '0') {
            tLog(DEBUG, "��ǿ�[%f,%f].", stMerchFee.dDRate, stMerchFee.dDMax);
            dRate = stMerchFee.dDRate / 100;
            dMax = stMerchFee.dDMax;
            sprintf(sFeeDesc, "%f,%.0f", dRate, dMax);
        } else {
            tLog(DEBUG, "���ÿ�[%f,%f].", stMerchFee.dCRate, stMerchFee.dCMax);
            dRate = stMerchFee.dCRate / 100;
            dMax = stMerchFee.dCMax;
            sprintf(sFeeDesc, "%f,%f", dRate, dMax);
        }
    }
    dFee = dAmount * dRate;
    if (DBL_CMP(dMax, 0)
            && DBL_CMP(dFee, dMax)) {
        dFee = dMax;
    }
    tMake(&dFee, 2, 'u');
    DEL_KEY(pstTransJson, "fee");
    DEL_KEY(pstTransJson, "fee_flag");
    DEL_KEY(pstTransJson, "fee_desc");
    SET_DOU_KEY(pstTransJson, "fee", dFee);
    SET_STR_KEY(pstTransJson, "fee_flag", "M");
    SET_STR_KEY(pstTransJson, "fee_desc", sFeeDesc);
    tLog(INFO, "�̻�[%s:%s:%s]������[%f][%s]", sMerchId, sCardType, sFeeType, dFee, sFeeDesc);
    return 0;
}