#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"
#include "quickpay.h"

/*无卡快捷消费查询 检索原交易信息*/
int FindOriTransMsg( QuickPayDetail *pstQuickPayDetail, char *pcRrn ) {
    OCI_Resultset *pstRes = NULL;
    QuickPayDetail stQuickPayDetail;
    char sSqlStr[1024];
    MEMSET(sSqlStr);
    
    tTrim(pcRrn);
    snprintf(sSqlStr,sizeof(sSqlStr),"select amount,trans_time,trans_date,merch_order_no,valid_flag"
                    " from b_inline_tarns_detail where rrn = '%s' "
                    " and trans_code in (select trans_code from s_trans_code where settle_flag = '1'and group_code = 'INLINE')",pcRrn);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    
     while (OCI_FetchNext(pstRes)) {
        DOUV(pstRes, 1, stQuickPayDetail.dAmount);
        STRV(pstRes, 2, stQuickPayDetail.sTransTime);
        STRV(pstRes, 3, stQuickPayDetail.sTransDate);
        STRV(pstRes, 4, stQuickPayDetail.sMerchOrderNo);
        STRV(pstRes, 5, stQuickPayDetail.sValidFlag);
        
        tTrim(stQuickPayDetail.sTransTime);
        tTrim(stQuickPayDetail.sTransDate);
        tTrim(stQuickPayDetail.sMerchOrderNo);
        tTrim(stQuickPayDetail.sValidFlag);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);    
    tLog(INFO, "检索到原交易流水信息.");
    memcpy(pstQuickPayDetail, &stQuickPayDetail, sizeof (stQuickPayDetail));
    
    return ( 0 );
}
