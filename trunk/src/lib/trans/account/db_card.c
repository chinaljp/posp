#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "card.h"
#include "t_cjson.h"
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
int GetCardLen(char *pcTrack) {
    int i = 0;
    char *p = pcTrack;
    while (*(p++) != 'D')
        i++;
    return i;
}

int FindCardBinEx(CardBin *pstCardBin, char *pcCard, char *pcTrack2, char *pcTrack3) {
    char sTrack[105] = {0};
    CardBin stCardBin;
    char cTrackNo;
    char sCardlen[2 + 1] = {0};
    char sSqlStr[256];
    OCI_Resultset *pstRes = NULL;

    if (pcTrack2[0] != '\0') {
        strcpy(sTrack, pcTrack2);
        cTrackNo = '2';
        sprintf(sCardlen, "%d", GetCardLen(sTrack));
        tStrCpy(pcCard, sTrack, atoi(sCardlen));
    } else if (pcTrack3[0] != '\0') {
        strcpy(sTrack, pcTrack3);
        cTrackNo = '3';
        sprintf(sCardlen, "%d", GetCardLen(sTrack) - 2);
        tStrCpy(pcCard, sTrack + 2, atoi(sCardlen));
    } else {
        strcpy(sTrack, pcCard);
        cTrackNo = '2';
        sprintf(sCardlen, "%d", strlen(sTrack));
    }
    tLog(DEBUG, "CARD_NO_LEN: %s, TrackNo: %c, Track: %s"
            , sCardlen, cTrackNo, sTrack);


    snprintf(sSqlStr, sizeof (sSqlStr), "select  CARD_BIN,CARD_TYPE, CARD_NAME, ISS_BANK_ID, CARD_BIN, CARD_BIN_LEN, CARD_NO_LEN from S_CARD_BIN where CARD_NO_LEN = :card_no_len AND INSTR( :card_no, CARD_BIN, 1, 1 )=1 ORDER BY CARD_BIN_LEN DESC, CARD_NO_LEN DESC ");
    
    if (tExecutePre(&pstRes, sSqlStr, 0, 0, 2, sCardlen, ":card_no_len", pcCard, ":card_no") < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }


    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, pstCardBin->sCardId);
        STRV(pstRes, 2, pstCardBin->sCardType);
        STRV(pstRes, 3, pstCardBin->sCardName);
        STRV(pstRes, 4, pstCardBin->sIssBankId);
        STRV(pstRes, 5, pstCardBin->sCardBin);
        INTV(pstRes, 6, pstCardBin->lCardBinLen);
        INTV(pstRes, 7, pstCardBin->lCardNoLen);


        tTrim(pstCardBin->sCardId);
        tTrim(pstCardBin->sCardName);
        tTrim(pstCardBin->sIssBankId);
        tTrim(pstCardBin->sCardBin);

    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "检索到卡BIN信息.");

    return 0;
}
