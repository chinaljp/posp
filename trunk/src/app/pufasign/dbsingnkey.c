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

int GetPufaZmKey(char * psZmkuLmk, char * psChannelId) {

    char sSqlStr[512] = {0}, sZmkuLmk[20 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "select ZMK from B_CHANNEL where CHANNEL_ID = '%s'", psChannelId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sZmkuLmk);
    }
    strcpy(psZmkuLmk, sZmkuLmk);

    tReleaseRes(pstRes);
    return 0;
}

int UpdPufaChannelKey(char * psKeyuLmk, char * psChannelTermId, char * psKeyName) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "update b_channel_term set %s  = '%s' where channel_term ='%s'", psKeyName, psKeyuLmk, psChannelTermId);
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "�����ն�[%s]-[%s]ʧ��.", psChannelTermId, psKeyName);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "�����ն�[%s]-[%s]�ɹ�.", psChannelTermId, psKeyName);
    return 0;
}

int DbPufa029000() {
    char sSqlStr[512] = {0}, sChannelMerchId[15 + 1] = {0}, sChanneTermId[8 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;
            snprintf(sSqlStr, sizeof (sSqlStr), "select a.channel_merch,a.channel_term from B_CHANNEL_TERM a \
             join B_CHANNEL_MERCH b on a.channel_merch = b.channel_merch \
             where b.channel_id='48560000' and a.status='1'");
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "������ˮ���ӱ�.");
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sChannelMerchId);
        STRV(pstRes, 2, sChanneTermId);
        tLog(INFO, "channel_merch[%s],channel_term[%s]", sChannelMerchId, sChanneTermId);
        Pufa029000(sChannelMerchId, sChanneTermId);
        tLog(INFO, "�ַ��̻�[%s]-----ǩ���ɹ�", sChannelMerchId);
        memset(sChannelMerchId, 0, sizeof (sChannelMerchId));
        memset(sChanneTermId, 0, sizeof (sChanneTermId));
    }
    tReleaseRes(pstRes);
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    return 0;

}