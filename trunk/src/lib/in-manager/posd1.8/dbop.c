#include <stdio.h>
#include <string.h>
#include <time.h>
#include "t_tools.h"
#include "trans_type_tbl.h"
#include "t_log.h"
#include "t_db.h"

int GetTermKey(char *pcMerchId, char *pcTermId, char *pcTak, char *pcTdk) {
    char sMerchId[16], sTermId[9], sTak[33], sTdk[33], sSqlStr[512];
    OCI_Resultset *pstRes = NULL;

    strcpy(sMerchId, pcMerchId);
    strcpy(sTermId, pcTermId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select a.MAC_KEY,a.TRACK_KEY  from B_TERM a, B_MERCH b "
            " where a.USER_CODE = b.USER_CODE AND A.TERM_ID='%s' AND B.MERCH_ID='%s' ", pcTermId, pcMerchId);


    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sTak);
        STRV(pstRes, 2, sTdk);
        tTrim(sTak);
        tTrim(sTdk);
    }
    strcpy(pcTak, sTak);
    strcpy(pcTdk, sTdk);
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return ( 0);
}

