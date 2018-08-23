#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"



/******************************************************************************/
/*      函数名:     GetUrl()                                           */
/*      功能说明:   解析预付卡类型                                              */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/

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

int GetUrl(cJSON *pstTransJson, int *piFlag) {

    char sErr[128] = {0};
    char sSql[2048];

    if (GenUrl(pstTransJson) != 0)
    {
        ErrHanding(pstTransJson, "96", "取得RRN对应的URL地址失败");
        return -1;
    }
    return 0;
}

