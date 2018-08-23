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

int PubKeyInfo(char *psPubKeyInfo, int *psLength) {
    char sRID[11] = {0};
    char sIndex[3] = {0};
    char sDueDate[9] = {0};
    int iStart = 1;
    int iEnd = 1;
    int iTotal = 0;

    char sIssued[3] = {0};
    char bRID[6] = {0}, sTmp[128] = {0};
    char bIndex[2] = {0};
    int i = 0, iLen = 0;
    char bTag[3] = {0};
    char sLen[2] = {0};
    char bLen[2] = {0};

    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    /* ÿ���·�15�鹫Կ��Ϣ */
    strncpy(sIssued, psPubKeyInfo + 1, 2);
    iStart = atoi(sIssued) + 1;
    iEnd = iStart + 5 - 1;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT rid, index_no, valid_date "
            " FROM (SELECT rownum r, rid, index_no, valid_date FROM b_term_pubkey_param)"
            " WHERE r between %d and %d", iStart, iEnd);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    iLen = 2;
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sRID);
        STRV(pstRes, 2, sIndex);
        STRV(pstRes, 3, sDueDate);

        tTrim(sRID);
        tTrim(sIndex);
        tTrim(sDueDate);

        memcpy(psPubKeyInfo + iLen, "9F06", 4);
        iLen += 4;
        sprintf(sLen, "%02x", 5);
        memcpy(psPubKeyInfo + iLen, sLen, 2);
        iLen += 2;
        memcpy(psPubKeyInfo + iLen, sRID, 10);
        iLen += 10;

        memcpy(psPubKeyInfo + iLen, "9F22", 4);
        iLen += 4;
        sprintf(sLen, "%02x", 1);
        memcpy(psPubKeyInfo + iLen, sLen, 2);
        iLen += 2;
        memcpy(psPubKeyInfo + iLen, sIndex, 2);
        iLen += 2;

        memcpy(psPubKeyInfo + iLen, "DF05", 4);
        iLen += 4;
        sprintf(sLen, "%02x", 8);
        memcpy(psPubKeyInfo + iLen, sLen, 2);
        iLen += 2;
        tBcd2Asc((UCHAR*) sTmp, (UCHAR*) sDueDate, 16, LEFT_ALIGN);
        tStrCpy(psPubKeyInfo + iLen, sTmp, 16);
        iLen += 16;
        tLog(DEBUG, "len[%d],i[%d]", iLen, i);
        i++;
    }
    /* ����0��ʾPOS����û�й�Կ��Ϣ��Ϊ����1��ʾ�����й�Կ��Ϣ
     * ����2��ʾһ�������޷�������еĹ�Կ��Ϣ����ҪPOS�ն�������������
     * ����Կ��Ϣ�����һ��ʱ����λ����Ҫ��д3 */
    if (0 == OCI_GetRowCount(pstRes)) {
        strcpy(psPubKeyInfo, "0");
        tLog(WARN, "û�й�Կ��Ϣ");
        tReleaseRes(pstRes);
        return -1;
    } else {
        tReleaseRes(pstRes);
        snprintf(sSqlStr, sizeof (sSqlStr), "SELECT count(*) INTO :iTotal FROM b_term_pubkey_param");

        if (tExecute(&pstRes, sSqlStr) < 0) {
            return -1;
        }
        while (OCI_FetchNext(pstRes)) {
            INTV(pstRes, 1, iTotal);
        }
        tReleaseRes(pstRes);

        if (iTotal <= iEnd && iStart == 1) {
            memcpy(psPubKeyInfo, "31", 2);
        } else if (iTotal > iEnd) {
            memcpy(psPubKeyInfo, "32", 2);
        } else {
            memcpy(psPubKeyInfo, "33", 2);
        }
    }
    tLog(DEBUG, "iTotal=%d,end=%d,start=%d", iTotal, iEnd, iStart);
    *psLength = iLen;
    return 0;
}

int AIDList(char *psAIDList, int *psLength) {
    char sAID[33] = {0};
    int iStart = 1;
    int iEnd = 1;
    int iTotal = 0;

    char sIssued[3] = {0};
    char bAID[17] = {0};
    int i = 0;
    char bTag[3] = {0};
    char sLen[2] = {0};
    char bLen[2] = {0};

    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    /* ÿ���·�30��AID */
    strncpy(sIssued, psAIDList + 1, 2);
    iStart = atoi(sIssued) + 1;
    iEnd = iStart + 30 - 1;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT aid "
            " FROM (SELECT rownum r, aid FROM b_term_ic_param)"
            " WHERE r between %d and %d", iStart, iEnd);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sAID);
        tTrim(sAID);

        memcpy(psAIDList + 2 + i, "9F06", 4);
        sprintf(sLen, "%02x", strlen(sAID) / 2);
        memcpy(psAIDList + 2 + i + 4, sLen, 2);
        memcpy(psAIDList + 2 + i + 4 + 2, sAID, strlen(sAID));
        i = i + 4 + 2 + strlen(sAID);
    }
    /* ����0��ʾPOS����û�в�����Ϣ��Ϊ����1��ʾ�����в�����Ϣ
     * ����2��ʾһ�������޷�������еĲ�����Ϣ����ҪPOS�ն�������������
     * ��������Ϣ�����һ��ʱ����λ����Ҫ��д3 */
    if (0 == OCI_GetRowCount(pstRes)) {
        strcpy(psAIDList, "0");
        tLog(WARN, "û�в�����Ϣ");
        tReleaseRes(pstRes);
        return -1;
    } else {
        tReleaseRes(pstRes);
        snprintf(sSqlStr, sizeof (sSqlStr), "SELECT count(*) FROM b_term_ic_param");

        if (tExecute(&pstRes, sSqlStr) < 0) {
            return -1;
        }
        while (OCI_FetchNext(pstRes)) {
            INTV(pstRes, 1, iTotal);
        }
        tReleaseRes(pstRes);

        if (iTotal <= iEnd && iStart == 1) {
            memcpy(psAIDList, "31", 2);
        } else if (iTotal > iEnd) {
            memcpy(psAIDList, "32", 2);
        } else {
            memcpy(psAIDList, "33", 2);
        }
    }

    *psLength = 2 + i;
    return 0;
}

/* IC����Կ */
int DownPubKeyRid(cJSON *pstTransJson, int *piFlag) {
    char sLen[16 + 1] = {0};
    char sTmp[1024];
    int iLen = 0;

    GET_STR_KEY(pstTransJson, "62_req_data", sLen);
    tStrCpy(sTmp, sLen, 3);

    if (PubKeyInfo(sTmp, &iLen) < 0) {
        ErrHanding(pstTransJson, "96", "״̬����-��ȡ��ԿRID��Ϣʧ��.");
        return -1;
    }

    SET_INT_KEY(pstTransJson, "62_len", iLen);
    SET_STR_KEY(pstTransJson, "62_data", sTmp);
    tLog(DEBUG, "״̬����-��ȡ��ԿRID��Ϣ�ɹ�.");
    return 0;
}

/* IC������ */
int DownAidList(cJSON *pstTransJson, int *piFlag) {
    char sLen[16 + 1] = {0};
    char sTmp[1024];
    int iLen = 0;

    GET_STR_KEY(pstTransJson, "62_req_data", sLen);
    tStrCpy(sTmp, sLen, 3);

    if (AIDList(sTmp, &iLen) < 0) {
        ErrHanding(pstTransJson, "96", "״̬����-��ȡIC��������Ϣʧ��.");
        return -1;
    }

    SET_INT_KEY(pstTransJson, "62_len", iLen);
    SET_STR_KEY(pstTransJson, "62_data", sTmp);
    tLog(DEBUG, "״̬����-��ȡIC��������Ϣ�ɹ�.");
    return 0;
}
