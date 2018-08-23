#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "card.h"
#include "t_cjson.h"
#include "quickpay.h"
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

int FindCardBinEx(CardBin *pstCardBin, char *pcCard) {
    
    char sCardlen[2 + 1] = {0};
    char sSqlStr[256] = {0};
    OCI_Resultset *pstRes = NULL;
    
    sprintf(sCardlen, "%d", strlen(pcCard));
    tLog(DEBUG, "CARD_NO_LEN: %s", sCardlen);
    
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

int FindCardMsg( CardMsg *pstCardMsg, char *pcId ) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    
    snprintf(sSqlStr, sizeof (sSqlStr), "select card_no_encrypt,card_holder_name_encrypt,mobile_no_encrypt,card_v_no_encrypt,card_exp_date_encrypt,key_name"
                                        " from b_merch_quick_card where id = '%s' and status = '1'",pcId);
     
    tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstCardMsg->sCardNoE);
        STRV(pstRes, 2, pstCardMsg->sCardHolderNameE);
        STRV(pstRes, 3, pstCardMsg->sMobileNoE);
        STRV(pstRes, 4, pstCardMsg->sCardCvnNoE);
        STRV(pstRes, 5, pstCardMsg->sCardExpDateE);
        STRV(pstRes, 6, pstCardMsg->sKey);
        
        tTrim(pstCardMsg->sCardNoE);
        tTrim(pstCardMsg->sCardHolderNameE);
        tTrim(pstCardMsg->sMobileNoE);
        tTrim(pstCardMsg->sCardCvnNoE);
        tTrim(pstCardMsg->sCardExpDateE);
        tTrim(pstCardMsg->sKey);
    }
    
     if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "检索到卡信息.");
    
    return ( 0 );
}
