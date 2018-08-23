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

int tQueryCount(char *pcSqlStr) {
    int iCounts;
    char *pcSql;
    pcSql = pcSqlStr;
    OCI_Resultset *pstRes = NULL;

    if (tExecute(&pstRes, pcSql) < 0) {
        tLog(ERROR, "sql[%s] err!", pcSql);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", pcSql);
        tReleaseRes(pstRes);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iCounts);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        iCounts = 0;
    }
    tReleaseRes(pstRes);
    return ( iCounts);
}

int GetQICParam(char *pcQICParam, int *piLength, char * psMerchFreeFlag) {
    char sFF805D[2] = {0};
    char sFF803A[4] = {0};
    char sFF803C[4] = {0};
    char sFF8058[13] = {0};
    char sFF8054[2] = {0};
    char sFF8055[2] = {0};
    char sFF8056[2] = {0};
    char sFF8057[2] = {0};
    char sFF8059[13] = {0};
    char sFF805A[2] = {0};
    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT FF805D, FF803A, FF803C, FF8058,FF8054, FF8055,  \
     FF8056, FF8057, FF8059, FF805A FROM b_term_qic_param");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sFF805D);
        STRV(pstRes, 2, sFF803A);
        STRV(pstRes, 3, sFF803C);
        STRV(pstRes, 4, sFF8058);
        STRV(pstRes, 5, sFF8054);
        STRV(pstRes, 6, sFF8055);
        STRV(pstRes, 7, sFF8056);
        STRV(pstRes, 8, sFF8057);
        STRV(pstRes, 9, sFF8059);
        STRV(pstRes, 10, sFF805A);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "未配置ic卡非接参数.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    tLog(INFO, "MERCH free flag [%c]", psMerchFreeFlag[0]);

    if (psMerchFreeFlag[0] != '1') {
        tLog(INFO, "商户未开通双免，关闭免密免签标识");
        /** b_merch:  0:未开通双免 1:已开通双免, 2:禁止开通双免, 3:开通错误
         * 商户没有报备双免，
         * 非接快速业务标识 
         * CDCVM标识
         * 免签标识
         * 开关为关闭状态
         */
        sFF8054[0] = '0';
        sFF8057[0] = '0';
        sFF805A[0] = '0';
    }


    /* 非接交易通道开关 */
    tStrCpy(pcQICParam, "FF805D", 6);
    sprintf(pcQICParam + 6, "%03d", strlen(sFF805D));
    tStrCpy(pcQICParam + 9, sFF805D, strlen(sFF805D));
    *piLength += (9 + strlen(sFF805D));


    /* 闪卡当笔重刷处理时间 */
    tStrCpy(pcQICParam + *piLength, "FF803A", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF803A));
    tStrCpy(pcQICParam + *piLength + 9, sFF803A, strlen(sFF803A));
    *piLength += (9 + strlen(sFF803A));

    /* 闪卡记录可处理时间 */
    tStrCpy(pcQICParam + *piLength, "FF803C", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF803C));
    tStrCpy(pcQICParam + *piLength + 9, sFF803C, strlen(sFF803C));
    *piLength += (9 + strlen(sFF803C));

    /* 非接快速业务（QPS）免密限额 */
    tStrCpy(pcQICParam + *piLength, "FF8058", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF8058));
    tStrCpy(pcQICParam + *piLength + 9, sFF8058, strlen(sFF8058));
    *piLength += (9 + strlen(sFF8058));


    /* 非接快速业务标识 */
    tStrCpy(pcQICParam + *piLength, "FF8054", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF8054));
    tStrCpy(pcQICParam + *piLength + 6 + 3, sFF8054, strlen(sFF8054));
    *piLength += (9 + strlen(sFF8054));


    /* BIN表A标识 */
    tStrCpy(pcQICParam + *piLength, "FF8055", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF8055));
    tStrCpy(pcQICParam + *piLength + 6 + 3, sFF8055, strlen(sFF8055));
    *piLength += (9 + strlen(sFF8055));


    /* BIN表B标识 */
    tStrCpy(pcQICParam + *piLength, "FF8056", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF8056));
    tStrCpy(pcQICParam + *piLength + 6 + 3, sFF8056, strlen(sFF8056));
    *piLength += (9 + strlen(sFF8056));


    /* CDCVM标识 */
    tStrCpy(pcQICParam + *piLength, "FF8057", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF8057));
    tStrCpy(pcQICParam + *piLength + 6 + 3, sFF8057, strlen(sFF8057));
    *piLength += (9 + strlen(sFF8057));


    /* 免签限额 */
    tStrCpy(pcQICParam + *piLength, "FF8059", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF8059));
    tStrCpy(pcQICParam + *piLength + 6 + 3, sFF8059, strlen(sFF8059));
    *piLength += (9 + strlen(sFF8059));


    /* 免签标识 */
    tStrCpy(pcQICParam + *piLength, "FF805A", 6);
    sprintf(pcQICParam + *piLength + 6, "%03d", strlen(sFF805A));
    tStrCpy(pcQICParam + *piLength + 6 + 3, sFF805A, strlen(sFF805A));
    *piLength += (9 + strlen(sFF805A));
    return 0;
}

int UpQicFlag(char * psTermId) {
    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_TERM SET FREE_FLAG='1' \
     where TERM_ID='%s'", psTermId);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新[%s]下载参数标志失败.", psTermId);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "更新终端[%s]参数标志成功,商户已下载双免参数", psTermId);
    return 0;

}

int GetMerchFreeFlag(char * psMerchId, char * psTermId, char * psMerchFreeFlag) {
    char sMerchId[15 + 1] = {0};
    char sTermId[8 + 1] = {0};
    char sSqlStr[512] = {0}, sMerchFreeFlag[2] = {0}, sTermFreeFlag[2] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sMerchId, psMerchId);
    strcpy(sTermId, psTermId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select free_flag from b_merch  WHERE merch_id = '%s' and status='1' ", sMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sMerchFreeFlag);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    strcpy(psMerchFreeFlag, sMerchFreeFlag);
    tReleaseRes(pstRes);
    return 0;
}

/* 非接参数 */
int DownQICParam(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sTmp[1024] = {0}, sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sMerchFreeFlag[2] = {0};
    int iLen = 0, iRet = 0;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    if (GetMerchFreeFlag(sMerchId, sTermId, sMerchFreeFlag) < 0) {
        ErrHanding(pstTransJson, "96", "获取商户双免标识失败");
        return -1;
    }

    if (GetQICParam(sTmp, &iLen, sMerchFreeFlag) < 0) {
        ErrHanding(pstTransJson, "96", "非接参数下载失败.");
        return -1;
    }
    SET_INT_KEY(pstTransJson, "62_len", iLen);
    SET_STR_KEY(pstTransJson, "62_data", sTmp);
    tLog(DEBUG, "非接参数下载成功.");
    return 0;
}

int UPQicParamFlag(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sTermId[8 + 1] = {0};
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    if (UpQicFlag(sTermId) < 0) {
        ErrHanding(pstTransJson, "96", "非接参数下载失败.");
        return -1;
    }
    tLog(INFO, "非接参数下载成功");
    return 0;
}

int GetNoPwBin(char *pcNoPwBin, int *piLen, int *piLength) {
    char sBIN[11] = {0};
    char sIndex[4] = {0};
    int iLine = 0;
    int iStart = 0;
    int iEnd = 0;
    int iTotal = 0;
    int i = 0, iLen = 0, iInt = 0;
    char sIssued[4] = {0};
    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    /* 每次下发15组卡bin信息 */
    strncpy(sIssued, pcNoPwBin, 3);
    iStart = atoi(sIssued);
    if (iStart == 0) {
        iStart = 1;
    }
    iEnd = iStart + 15 - 1;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  r,card_bin  FROM ( SELECT rownum r, card_bin,last_upd_date FROM B_NOPWD_CARD_WHITE_LIST ) "
            " WHERE r between %d and %d", iStart, iEnd);
    tLog(DEBUG, "%s", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    tLog(DEBUG, "%d", (*piLen));
    memset(pcNoPwBin, 0x20, (*piLen));
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iLine);
        STRV(pstRes, 2, sBIN);
        tTrim(sBIN);
        iLen = strlen(sBIN);
        memcpy(pcNoPwBin + 1 + 3 + i, sBIN, iLen);
        sprintf(sIndex, "%03d", iLine);
        memcpy(pcNoPwBin + 1, sIndex, 3);
        i = i + 12;
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "没有卡bin信息可更新.");
        tReleaseRes(pstRes);
        strcpy(pcNoPwBin, "0");
        *piLength = 4;
        return 0;
    }
    tLog(DEBUG, "卡bin表B查询完成.");
    tReleaseRes(pstRes);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT count(*) FROM B_NOPWD_CARD_WHITE_LIST");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iTotal);
        if (iTotal > iEnd) {
            memcpy(pcNoPwBin, "2", 1);
        } else {
            memcpy(pcNoPwBin, "1", 1);
        }
    }
    *piLength = 4 + i;
    tReleaseRes(pstRes);
    return 0;
}

/* 免密新增卡bin下载          */
int NoPwBin(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sTmp[1024] = {0}, sTmp1[1024] = {0}, sData[128] = {0};
    int iLen = 0, iTmpLen = 0;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "62_req_data", sData);
    tLog(DEBUG, "sData[%s]", sData);

    tAsc2Bcd(sTmp, (UCHAR*) sData, 6, LEFT_ALIGN);
    tLog(INFO, "sTmp[%s]", sTmp);
    iTmpLen = sizeof (sTmp);
    if (GetNoPwBin(sTmp, &iTmpLen, &iLen) < 0) {
        ErrHanding(pstTransJson, "96", "免密新增卡BIN下载失败.");
        return -1;
    }
    SET_INT_KEY(pstTransJson, "62_len", iLen);
    tStrCpy(sTmp1, sTmp, iLen);
    SET_STR_KEY(pstTransJson, "62_data", sTmp1);
    tLog(DEBUG, "免密新增卡BIN下载成功.");
    return 0;
}

int GetBinBkList(char *pcNoPwBin, int *piLen, int *piLength) {
    char sBIN[11] = {0};
    char sIndex[4] = {0};
    int iLine = 0;
    int iStart = 0;
    int iEnd = 0;
    int iTotal = 0;
    int i = 0, iLen = 0;
    char sIssued[4] = {0};
    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    /* 每次下发15组卡bin信息 */
    strncpy(sIssued, pcNoPwBin, 3);
    tLog(INFO, "sIssued[%s]", sIssued);
    iStart = atoi(sIssued);
    tLog(INFO, "iStart[%d]", iStart);
    if (iStart == 0) {
        iStart = 1;
    }
    iEnd = iStart + 15 - 1;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  r,card_bin  FROM ( SELECT rownum r, card_bin,last_upd_date FROM B_NOPWD_CARD_BLACK_LIST ) "
            " WHERE r between %d and %d", iStart, iEnd);
    tLog(INFO, "sql[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    memset(pcNoPwBin, 0x20, *piLen);
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iLine);
        STRV(pstRes, 2, sBIN);
        tTrim(sBIN);
        iLen = strlen(sBIN);
        memcpy(pcNoPwBin + 1 + 3 + i, sBIN, iLen);
        sprintf(sIndex, "%03d", iLine);
        memcpy(pcNoPwBin + 1, sIndex, 3);
        i = i + 12;
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "没有卡bin信息可更新.");
        tReleaseRes(pstRes);
        strcpy(pcNoPwBin, "0");
        *piLength = 4;
        return 0;
    }
    tReleaseRes(pstRes);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT count(*) FROM B_NOPWD_CARD_BLACK_LIST");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iTotal);
        if (iTotal > iEnd) {
            memcpy(pcNoPwBin, "2", 1);
        } else {
            memcpy(pcNoPwBin, "1", 1);
        }
    }
    *piLength = 4 + i;
    tReleaseRes(pstRes);
    return 0;
}

/* 免密卡bin黑名单下载 */
int BinBkList(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sTmp[1024], sTmp1[1024], sData[128] = {0};
    int iLen = 0, iTmpLen = 0;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "62_req_data", sData);
    tAsc2Bcd(sTmp, (UCHAR*) sData, 6, LEFT_ALIGN);
    iTmpLen = sizeof (sTmp);
    if (GetBinBkList(sTmp, &iTmpLen, &iLen) < 0) {
        ErrHanding(pstTransJson, "96", "免密卡bin黑名单下载失败.");
        return -1;
    }
    SET_INT_KEY(pstTransJson, "62_len", iLen);
    tStrCpy(sTmp1, sTmp, iLen);
    SET_STR_KEY(pstTransJson, "62_data", sTmp1);
    tLog(DEBUG, "免密卡bin黑名单下载成功.");
    return 0;
}

int GetPubKey(char *pcPubKey, int *piLength) {
    char sRID[11] = {0};
    char sIndex[5] = {0};
    char sValidDate[9] = {0};
    char sHashId[3] = {0};
    char sAlgoId[3] = {0};
    char sMod[513] = {0};
    char sE[7] = {0};
    char sCheckValue[513] = {0};

    char bRID[6] = {0};
    char bTag[3] = {0};
    char sLen[7] = {0};
    char bLen[4] = {0};
    char bIndex[2] = {0};
    char bHashId[2] = {0};
    char bAlgoId[2] = {0};
    char bMod[256] = {0};
    char bE[4] = {0};
    char bCheckValue[256] = {0};
    int i = 1;
    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    strncpy(sRID, pcPubKey + 6, 10);
    strncpy(sIndex, pcPubKey + 22, 2);

    snprintf(sSqlStr, sizeof (sSqlStr), "select valid_date, hash_id, algo_id, mod, index_number, check_value "
            " from b_term_pubkey_param where rid = '%s' and index_no = '%s'", sRID, sIndex);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sValidDate);
        STRV(pstRes, 2, sHashId);
        STRV(pstRes, 3, sAlgoId);
        STRV(pstRes, 4, sMod);
        STRV(pstRes, 5, sE);
        STRV(pstRes, 6, sCheckValue);

        tTrim(sValidDate);
        tTrim(sHashId);
        tTrim(sAlgoId);
        tTrim(sMod);
        tTrim(sE);
        tTrim(sCheckValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "没有RID[%s]Index[%s]信息.", sRID, sIndex);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    *pcPubKey = '1';
    tAsc2Bcd(bTag, "9F06", 4, LEFT_ALIGN);
    memcpy(pcPubKey + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sRID) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcPubKey + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bRID, sRID, strlen(sRID), LEFT_ALIGN);
    memcpy(pcPubKey + i, bRID, strlen(sRID) / 2);
    i += strlen(sRID) / 2;

    tAsc2Bcd(bTag, "9F22", 4, LEFT_ALIGN);
    memcpy(pcPubKey + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sIndex) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcPubKey + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bIndex, sIndex, strlen(sIndex), LEFT_ALIGN);
    memcpy(pcPubKey + i, bIndex, strlen(sIndex) / 2);
    i += strlen(sIndex) / 2;

    tAsc2Bcd(bTag, "DF05", 4, LEFT_ALIGN);
    memcpy(pcPubKey + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", 8);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcPubKey + i, bLen, 1);
    i += 1;
    memcpy(pcPubKey + i, sValidDate, 8);
    i += 8;

    tAsc2Bcd(bTag, "DF06", 4, LEFT_ALIGN);
    memcpy(pcPubKey + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sHashId) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcPubKey + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bHashId, sHashId, strlen(sHashId), LEFT_ALIGN);
    memcpy(pcPubKey + i, bHashId, strlen(sHashId) / 2);
    i += strlen(sHashId) / 2;

    tAsc2Bcd(bTag, "DF07", 4, LEFT_ALIGN);
    memcpy(pcPubKey + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sAlgoId) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcPubKey + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bAlgoId, sAlgoId, strlen(sAlgoId), LEFT_ALIGN);
    memcpy(pcPubKey + i, bAlgoId, strlen(sAlgoId) / 2);
    i += strlen(sAlgoId) / 2;

    tAsc2Bcd(bTag, "DF02", 4, LEFT_ALIGN);
    memcpy(pcPubKey + i, bTag, 2);
    i += 2;

    /* 模数长度占一个或两个字节 */
    if ((strlen(sMod) + 1) / 2 > 127) {
        sprintf(sLen, "%04x", (33024 + (strlen(sMod) + 1) / 2));
        tAsc2Bcd(bLen, sLen, 4, LEFT_ALIGN);
        memcpy(pcPubKey + i, bLen, 2);
        i += 2;
    } else {
        sprintf(sLen, "%02x", (strlen(sMod) + 1) / 2);
        tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
        memcpy(pcPubKey + i, bLen, 1);
        i += 1;
    }

    tAsc2Bcd(bMod, sMod, strlen(sMod), LEFT_ALIGN);
    memcpy(pcPubKey + i, bMod, (strlen(sMod) + 1) / 2);
    i += (strlen(sMod) + 1) / 2;

    memset(sLen, 0, sizeof (sLen));
    memset(bLen, 0, sizeof (bLen));

    tAsc2Bcd(bTag, "DF04", 4, LEFT_ALIGN);
    memcpy(pcPubKey + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", (strlen(sE) + 1) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcPubKey + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bE, sE, strlen(sE), LEFT_ALIGN);
    memcpy(pcPubKey + i, bE, (strlen(sE) + 1) / 2);
    i += (strlen(sE) + 1) / 2;

    tAsc2Bcd(bTag, "DF03", 4, LEFT_ALIGN);
    memcpy(pcPubKey + i, bTag, 2);
    i += 2;

    /* 公钥校验值长度占一个或两个字节 */
    if ((strlen(sCheckValue) + 1) / 2 > 127) {
        sprintf(sLen, "%04x", (33024 + (strlen(sCheckValue) + 1) / 2));
        tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
        memcpy(pcPubKey + i, bLen, 2);
        i += 2;
    } else {
        sprintf(sLen, "%02x", (strlen(sCheckValue) + 1) / 2);
        tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
        memcpy(pcPubKey + i, bLen, 1);
        i += 1;
    }

    tAsc2Bcd(bCheckValue, sCheckValue, strlen(sCheckValue), LEFT_ALIGN);
    memcpy(pcPubKey + i, bCheckValue, (strlen(sCheckValue) + 1) / 2);
    i += (strlen(sCheckValue) + 1) / 2;

    *piLength = i;

    return 0;
}

/* IC卡公钥下载 */
int DownPubKey(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sData[200] = {0};
    char sBuf[2048] = {0}, sTmp[1024];
    int iLen = 0;

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_INT_KEY(pstTransJson, "62_req_len", iLen);
    GET_STR_KEY(pstTransJson, "62_req_data", sData);
    strcpy(sTmp, sData);
    tLog(DEBUG, "len=[%d],data=[$s]", iLen, sData);
    if (GetPubKey(sTmp, &iLen) < 0) {
        ErrHanding(pstTransJson, "96", "公钥下载失败.");
        return -1;
    }

    SET_INT_KEY(pstTransJson, "62_len", iLen << 1);
    tBcd2Asc((UCHAR*) sBuf, (UCHAR*) sTmp, iLen << 1, LEFT_ALIGN);
    SET_STR_KEY(pstTransJson, "62_data", sBuf);
    tLog(DEBUG, "公钥下载成功.");
    return 0;
}

int GetICParam(char *pcICParam, int *piLength) {
    char sAID[33] = {0};
    char sAsi[3] = {0};
    char sVersionid[5] = {0};
    char sTac_defualt[11] = {0};
    char sTac_online[11] = {0};
    char sTag_reject[11] = {0};
    char sLamount[9] = {0};
    char sThreshold[9] = {0};
    char sMost_percent[3] = {0};
    char sTag_percent[3] = {0};
    char sDdol[512] = {0};
    char sPin_support[3] = {0};
    char sE_cash_limit_amt[13] = {0};
    char sQ_min_amt[13] = {0};
    char sQ_max_amt[13] = {0};
    char sCvm[13] = {0};

    char bAID[17] = {0};
    char bTag[3] = {0};
    char sLen[7] = {0};
    char bLen[4] = {0};
    char bAsi[2] = {0};
    char bVersionid[3] = {0};
    char bTac_defualt[6] = {0};
    char bTac_online[6] = {0};
    char bLamount[5] = {0};
    char bMost_percent[2] = {0};
    char bDdol[256] = {0};
    char bPin_support[2] = {0};
    char bE_cash_limit_amt[7] = {0};
    int i = 1;
    int iLen = 0;
    char sSqlStr[512];
    OCI_Resultset *pstRes = NULL;

    strncpy(sLen, pcICParam + 4, 2);
    sscanf(sLen, "%02x", &iLen);
    strncpy(sAID, pcICParam + 6, iLen * 2);

    snprintf(sSqlStr, sizeof (sSqlStr), "select asi, versionid, tac_defualt, tac_online,tag_reject, lamount, threshold, most_percent,"
            " tag_percent, ddol, pin_support, e_cash_limit_amt,q_min_amt, q_max_amt, cvm"
            " from b_term_ic_param where aid = '%s'", sAID);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sAsi);
        STRV(pstRes, 2, sVersionid);
        STRV(pstRes, 3, sTac_defualt);
        STRV(pstRes, 4, sTac_online);
        STRV(pstRes, 5, sTag_reject);
        STRV(pstRes, 6, sLamount);
        STRV(pstRes, 7, sThreshold);
        STRV(pstRes, 8, sMost_percent);
        STRV(pstRes, 9, sTag_percent);
        STRV(pstRes, 10, sDdol);
        STRV(pstRes, 11, sPin_support);
        STRV(pstRes, 12, sE_cash_limit_amt);
        STRV(pstRes, 13, sQ_min_amt);
        STRV(pstRes, 14, sQ_max_amt);
        STRV(pstRes, 15, sCvm);

        tTrim(sAsi);
        tTrim(sVersionid);
        tTrim(sTac_defualt);
        tTrim(sTac_online);
        tTrim(sTag_reject);
        tTrim(sLamount);
        tTrim(sThreshold);
        tTrim(sMost_percent);
        tTrim(sTag_percent);
        tTrim(sDdol);
        tTrim(sPin_support);
        tTrim(sE_cash_limit_amt);
        tTrim(sQ_min_amt);
        tTrim(sQ_max_amt);
        tTrim(sCvm);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "没有AID[%s]信息.", sAID);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    *pcICParam = '1';
    tAsc2Bcd(bTag, "9F06", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", (strlen(sAID) + 1) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bAID, sAID, strlen(sAID), LEFT_ALIGN);
    memcpy(pcICParam + i, bAID, (strlen(sAID) + 1) / 2);
    i += (strlen(sAID) + 1) / 2;

    tAsc2Bcd(bTag, "DF01", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sAsi) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bAsi, sAsi, strlen(sAsi), LEFT_ALIGN);
    memcpy(pcICParam + i, bAsi, strlen(sAsi) / 2);
    i += strlen(sAsi) / 2;

    tAsc2Bcd(bTag, "9F08", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sVersionid) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bVersionid, sVersionid, strlen(sVersionid), LEFT_ALIGN);
    memcpy(pcICParam + i, bVersionid, strlen(sVersionid) / 2);
    i += strlen(sVersionid) / 2;

    tAsc2Bcd(bTag, "DF11", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sTac_defualt) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bTac_defualt, sTac_defualt, strlen(sTac_defualt), LEFT_ALIGN);
    memcpy(pcICParam + i, bTac_defualt, strlen(sTac_defualt) / 2);
    i += strlen(sTac_defualt) / 2;

    tAsc2Bcd(bTag, "DF12", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sTac_online) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bTac_online, sTac_online, strlen(sTac_online), LEFT_ALIGN);
    memcpy(pcICParam + i, bTac_online, strlen(sTac_online) / 2);
    i += strlen(sTac_online) / 2;

    tAsc2Bcd(bTag, "DF13", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sTag_reject) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bTac_online, sTag_reject, strlen(sTag_reject), LEFT_ALIGN);
    memcpy(pcICParam + i, bTac_online, strlen(sTag_reject) / 2);
    i += strlen(sTag_reject) / 2;

    tAsc2Bcd(bTag, "9F1B", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sLamount) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bLamount, sLamount, strlen(sLamount), LEFT_ALIGN);
    memcpy(pcICParam + i, bLamount, strlen(sLamount) / 2);
    i += strlen(sLamount) / 2;

    tAsc2Bcd(bTag, "DF15", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sThreshold) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bLamount, sThreshold, strlen(sThreshold), LEFT_ALIGN);
    memcpy(pcICParam + i, bLamount, strlen(sThreshold) / 2);
    i += strlen(sThreshold) / 2;

    tAsc2Bcd(bTag, "DF16", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sMost_percent) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bMost_percent, sMost_percent, strlen(sMost_percent), LEFT_ALIGN);
    memcpy(pcICParam + i, bMost_percent, strlen(sMost_percent) / 2);
    i += strlen(sMost_percent) / 2;

    tAsc2Bcd(bTag, "DF17", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sTag_percent) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bMost_percent, sTag_percent, strlen(sTag_percent), LEFT_ALIGN);
    memcpy(pcICParam + i, bMost_percent, strlen(sTag_percent) / 2);
    i += strlen(sTag_percent) / 2;

    tAsc2Bcd(bTag, "DF14", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;

    /* 缺省DDOL长度占一个或两个字节 */
    if ((strlen(sDdol) + 1) / 2 > 127) {
        sprintf(sLen, "%04x", (33024 + (strlen(sDdol) + 1) / 2));
        tAsc2Bcd(bLen, sLen, 4, LEFT_ALIGN);
        memcpy(pcICParam + i, bLen, 2);
        i += 2;
    } else {
        sprintf(sLen, "%02x", (strlen(sDdol) + 1) / 2);
        tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
        memcpy(pcICParam + i, bLen, 1);
        i += 1;
    }

    tAsc2Bcd(bDdol, sDdol, strlen(sDdol), LEFT_ALIGN);
    memcpy(pcICParam + i, bDdol, (strlen(sDdol) + 1) / 2);
    i += (strlen(sDdol) + 1) / 2;

    memset(sLen, 0, sizeof (sLen));
    memset(bLen, 0, sizeof (bLen));

    tAsc2Bcd(bTag, "DF18", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", (strlen(sPin_support) + 1) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bPin_support, sPin_support, strlen(sPin_support), LEFT_ALIGN);
    memcpy(pcICParam + i, bPin_support, (strlen(sPin_support) + 1) / 2);
    i += (strlen(sPin_support) + 1) / 2;

    tAsc2Bcd(bTag, "9F7B", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sE_cash_limit_amt) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bE_cash_limit_amt, sE_cash_limit_amt, strlen(sE_cash_limit_amt), LEFT_ALIGN);
    memcpy(pcICParam + i, bE_cash_limit_amt, strlen(sE_cash_limit_amt) / 2);
    i += strlen(sE_cash_limit_amt) / 2;

    tAsc2Bcd(bTag, "DF19", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sQ_min_amt) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bE_cash_limit_amt, sQ_min_amt, strlen(sQ_min_amt), LEFT_ALIGN);
    memcpy(pcICParam + i, bE_cash_limit_amt, strlen(sQ_min_amt) / 2);
    i += strlen(sQ_min_amt) / 2;

    tAsc2Bcd(bTag, "DF20", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sQ_max_amt) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bE_cash_limit_amt, sQ_max_amt, strlen(sQ_max_amt), LEFT_ALIGN);
    memcpy(pcICParam + i, bE_cash_limit_amt, strlen(sQ_max_amt) / 2);
    i += strlen(sQ_max_amt) / 2;

    tAsc2Bcd(bTag, "DF21", 4, LEFT_ALIGN);
    memcpy(pcICParam + i, bTag, 2);
    i += 2;
    sprintf(sLen, "%02x", strlen(sCvm) / 2);
    tAsc2Bcd(bLen, sLen, 2, LEFT_ALIGN);
    memcpy(pcICParam + i, bLen, 1);
    i += 1;
    tAsc2Bcd(bE_cash_limit_amt, sCvm, strlen(sCvm), LEFT_ALIGN);
    memcpy(pcICParam + i, bE_cash_limit_amt, strlen(sCvm) / 2);
    i += strlen(sCvm) / 2;

    *piLength = i;

    return 0;
}

/* IC卡参数下载 */
int DownICParam(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sData[200] = {0};
    char sBuf[2048] = {0}, sTmp[1024];
    int iLen = 0;

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_INT_KEY(pstTransJson, "62_req_len", iLen);
    GET_STR_KEY(pstTransJson, "62_req_data", sData);
    strcpy(sTmp, sData);
    tLog(DEBUG, "len=[%d],data=[%s]", iLen, sData);
    if (GetICParam(sTmp, &iLen) < 0) {
        ErrHanding(pstTransJson, "96", "IC卡参数下载失败.");
        return -1;
    }

    SET_INT_KEY(pstTransJson, "62_len", iLen << 1);
    tBcd2Asc((UCHAR*) sBuf, (UCHAR*) sTmp, iLen << 1, LEFT_ALIGN);
    SET_STR_KEY(pstTransJson, "62_data", sBuf);
    tLog(DEBUG, "IC卡参数下载成功.");
    return 0;
}
/* IC卡黑名单下载 */
