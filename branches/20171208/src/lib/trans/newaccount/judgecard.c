#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "card.h"

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


    snprintf(sSqlStr, sizeof (sSqlStr), "select  CARD_BIN,CARD_TYPE, CARD_NAME, ISS_BANK_ID, CARD_BIN, CARD_BIN_LEN, CARD_NO_LEN from S_CARD_BIN where CARD_NO_LEN ='%s' AND INSTR( '%s', CARD_BIN, 1, 1 )=1 ORDER BY CARD_BIN_LEN DESC, CARD_NO_LEN DESC ", sCardlen, pcCard);
    if (tExecute(&pstRes, sSqlStr) < 0) {
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

/******************************************************************************/
/*      函数名:     JudgeCard()                                           */
/*      功能说明:   解析预付卡类型                                              */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int JudgeCard(cJSON *pstTransJson, int *piFlag) {

    CardBin stCardBin;
    char sCardNo[20] = {0}, sTrack2[37 + 1] = {0}, sTrack3[104 + 1] = {0};

    memset(&stCardBin, 0, sizeof (CardBin));

    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_STR_KEY(pstTransJson, "track2", sTrack2);
    GET_STR_KEY(pstTransJson, "track3", sTrack3);
    if (FindCardBinEx(&stCardBin, sCardNo, sTrack2, sTrack3) != 0) {
        ErrHanding(pstTransJson, "ZB", "解析支付卡[%s]卡Bin失败.", sCardNo);
        return -1;
    }
    tLog(DEBUG, "stCardBin.sCardId,%s", stCardBin.sCardId);
    tLog(DEBUG, "stCardBin.sCardType,%s", stCardBin.sCardType);

    SET_STR_KEY(pstTransJson, "card_bin", stCardBin.sCardId);
    SET_STR_KEY(pstTransJson, "card_type", stCardBin.sCardType);
    SET_STR_KEY(pstTransJson, "iss_id", stCardBin.sIssBankId);
    /*
        tStrCpy( pstNetTran->sExpireDate, sTrack + stCardBin.lExpPos - 1, 4 );
     */
    SET_STR_KEY(pstTransJson, "card_name", stCardBin.sCardName);
#if 0
    /* 不受理  */
    if (stCardBin.cAcqFlag == '0') {
        tErrLog(ERR, "发卡机构[%s]不受理此卡Bin[%s].", pstNetTran->sIssInstId, stCardBin.sCardBin);
        SetRespCode(pstNetTran, "C9", pstTranData->stSysParam.sInstId);
        return -1;
    }
    sprintf(sSql, "select count(1) from cjzf_agent.black_card_no where card_no=%s", pstNetTran->sCard1);
    iRet = tQueryCount(sSql);
    if (iRet > 0) {
        tErrLog(ERR, "发卡机构[%s]不受理此卡[%s].", pstNetTran->sIssInstId, pstNetTran->sCard1);
        SetRespCode(pstNetTran, "C9", pstTranData->stSysParam.sInstId);
        return -1;
    }
#endif
    tLog(INFO, "解析发卡机构[%s]卡Bin[%s]成功[%s].", stCardBin.sIssBankId, stCardBin.sCardBin, stCardBin.sCardType[0] == '1' ? "信用卡" : "借记卡");
    return 0;
}
