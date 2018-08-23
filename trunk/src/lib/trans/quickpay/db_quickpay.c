/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"

/*更新清算日期、商户订单号 add by gjq at 20180706*/
int UpMerchOrderNo(char *pcDate,char *pcMerchOrderNo,char *pcMerchId,char *pcRrn,char *pcTransCode) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set settle_date = '%s',merch_order_no='%s' "
                    "where merch_id = '%s' and trans_code = '%s' and rrn = '%s'",pcDate,pcMerchOrderNo,pcMerchId,pcTransCode,pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "更新商户[%s]的订单号[%s]失败.",pcMerchId,pcMerchOrderNo);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return ( 0 );
}
/*更新清算标志*/
int UpValidflag(char * psORrn) {
    char sSqlStr[512] = {0};
    int iResCnt = 0;

    tTrim(psORrn);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET VALID_FLAG='0' WHERE rrn = '%s' ", psORrn);
    
    tLog(DEBUG,"sSqlStr[%s]",sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tLog(DEBUG, "影响记录数[%d]", iResCnt);
    return 0;
}
/*快捷无卡消费支付失败后， 更新支付结果描述;  
 * notice_flag = 1 通知已推送APP，
 * 此时原交易的支付结果 已经通过上游的发过来的异步通知更新到了原交易流水中， 此时不需要此更新 
 * 更新未入账的原交易 的应答信息
 */
int UpRespDesc( char *pcRespCode, char *pcRespDesc, char *pcRrn ) {
    char sSqlStr[1024] = {0};
    int iResCnt = 0;
    
    if ( !memcmp(pcRespCode,"00",2) ) {
        snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set resp_code = '%s', istresp_code = '%s', resp_desc ='%s', last_mod_time = sysdate "
                    "where settle_flag != 'M' and  rrn = '%s' and notice_flag != '1' and resp_code != '00'",pcRespCode,pcRespCode,pcRespDesc,pcRrn);
    } 
    else {
        snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set resp_code = '%s', istresp_code = '%s', resp_desc ='%s', last_mod_time = sysdate "
                    "where settle_flag != 'M' and  rrn = '%s' and notice_flag != '1'",pcRespCode,pcRespCode,pcRespDesc,pcRrn);
    }
    
    tLog(DEBUG,"sql = [%s]",sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    
    tLog(DEBUG, "影响记录数[%d]", iResCnt);
    return ( 0 );
}

/*更新 推送标志 notice_flag = '2' 向APP 推送通知 */
int UpNoticeFlag( char *pcRrn ) {
    char sSqlStr[1024] = {0};
    int iResCnt = 0;
    /*注意： notice_flag = '1' 通知已推送APP, 无需更新 notice_flag = '2' 再次向APP推送通知 */
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set notice_flag = '2' where rrn = '%s' and notice_flag != '1'", pcRrn );
    
    tLog(DEBUG,"sql = [%s]",sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    
    tLog(DEBUG, "影响记录数[%d]", iResCnt);
    return ( 0 );
}