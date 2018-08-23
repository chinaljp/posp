#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "card.h"
#include "t_cjson.h"

int GenUrl(cJSON *pstTransJson) {
    char sSqlStr[256] = {0};
    char sUrl[200] = {0};
    char sSql[256] = {0}, sMerchId[19] = {0}, sTermId[9] = {0},sRrn[13]={0};
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "o_rrn", sRrn);

    snprintf(sSqlStr, sizeof (sSqlStr), "select (select key_value from s_param where key='92')||(select id from b_pos_trans_detail  where rrn='%s' and term_id='%s' and merch_id='%s')\
    from dual", sRrn, sTermId, sMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        STRV(pstRes, 1, sUrl);
        tTrim(sUrl);

    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    SET_STR_KEY(pstTransJson, "code_url", sUrl);
    tLog(INFO, "%s", sUrl);
    return 0;
}

