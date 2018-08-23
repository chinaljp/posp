/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"

/*获取订单的商户号 */
int FindMerchId( char *pcMerchId,char *pcChannelMerchId,char *pcOrderNo ) {
    char sSqlStr[1024]      = {0};
    char sMerchId[15 + 1]   = {0};
    char sChannelMerchId[15 + 1]  = {0};
    OCI_Resultset *pstRes   = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select merch_id,channel_merch_id from b_inline_tarns_detail "
                        "where merch_order_no = '%s' and trans_code not in ('02B300','02W300','02Y300','0AY300','0AW300','0AB300','0AQ300')",pcOrderNo);
    
    tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
       tLog(ERROR, "sql[%s] err!", sSqlStr);
       return ( -1 );
    }

    if (NULL == pstRes) {
       tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
       return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
       STRV(pstRes, 1, sMerchId);
       STRV(pstRes, 2, sChannelMerchId);
       tTrim(sMerchId);
       tTrim(sChannelMerchId);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
       tLog(ERROR, "未找到记录.");
       tReleaseRes(pstRes);
       return ( -1 );
    }
    tReleaseRes(pstRes);
    strcpy(pcMerchId,sMerchId);
    strcpy(pcChannelMerchId,sChannelMerchId);
    
    return ( 0 );
}

int FindMerchOrderNo( char *pcOrderNo, char *pcRrn ) {
    char sSqlStr[1024]      = {0};
    OCI_Resultset *pstRes   = NULL;
    
     snprintf(sSqlStr,sizeof(sSqlStr),"select merch_order_no from b_inline_tarns_detail "
                        "where rrn = '%s'",pcRrn);
    
    tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
       tLog(ERROR, "sql[%s] err!", sSqlStr);
       return ( -1 );
    }

    if (NULL == pstRes) {
       tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
       return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
       STRV(pstRes, 1, pcOrderNo);
       tTrim(pcOrderNo);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
       tLog(ERROR, "未找到记录.");
       tReleaseRes(pstRes);
       return ( -1 );
    }
    tReleaseRes(pstRes);

    return ( 0 );
 }
/*获取商户类型、MCC*/
int FindMerchMsg( char *pcMerchId,char *pcMerchType,char *pcMcc ) {
    char sSqlStr[1024]      = {0};
    char sMerchType[2 + 1]  = {0};
    char sMcc[4 + 1]        = {0};
    OCI_Resultset *pstRes   = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select merch_type,mcc from b_merch where merch_id = '%s'",pcMerchId);
    
    tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
       tLog(ERROR, "sql[%s] err!", sSqlStr);
       return ( -1 );
    }

    if (NULL == pstRes) {
       tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
       return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
       STRV(pstRes, 1, sMerchType);
       STRV(pstRes, 2, sMcc);
       tTrim(sMerchType);
       tTrim(sMcc);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
       tLog(ERROR, "未找到记录.");
       tReleaseRes(pstRes);
       return ( -1 );
    }
    tReleaseRes(pstRes);
    strcpy(pcMerchType,sMerchType);
    strcpy(pcMcc,sMcc);
    return ( 0 );
}

/*更新本次银联二维码交易订单的手续费*/
int UpFeeMsg(char *pcOrderNo,char *pcFeeType,double dFee,char *pcFeeDesc) {
    char sSqlStr[1024]      = {0};
    OCI_Resultset *pstRes   = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set fee_type = '%s',fee = %.02f,fee_desc = '%s',fee_flag = 'M' "
                    "where merch_order_no = '%s' and trans_code not in ('02Y300','0AY300')",
            pcFeeType,dFee,pcFeeDesc,pcOrderNo);

    if ( tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0 ) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tReleaseRes(pstRes);
        return ( -1 );
    }
    tLog(DEBUG,"更新了[%d]条",tGetAffectedRows());
    tCommit();
    tReleaseRes(pstRes);
    return ( 0 );
}

/*更新银联二维码交易*/
int UpCupsSettleKey(char *pcCardType, char *pcSettleSysTrace, char *pcSettleTransTime, char *pcOrderNo) { 
    int iNum = 0;
    char sSqlStr[1024]      = {0};
    OCI_Resultset *pstRes   = NULL;
    
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set card_type = '%s',settle_sys_trace = '%s',settle_trans_time = '%s' "
                    "where merch_order_no = '%s' and trans_code not in ('02Y300','0AY300') and "
            "not exists (select 1 from b_inline_tarns_detail where settle_sys_trace = '%s' and settle_trans_time = '%s')",
            pcCardType,pcSettleSysTrace,pcSettleTransTime,pcOrderNo,pcSettleSysTrace,pcSettleTransTime );
    tLog(DEBUG,"sql = [%s]",sSqlStr);
    if (  tExecute(&pstRes, sSqlStr) < 0 ) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tReleaseRes(pstRes);
        return ( -1 );
    }

    iNum =  tGetAffectedRows();
    tLog(INFO,"更新了[%d]条,%s,商户订单号[%s]",iNum,
        iNum == 0? "查询交易中可能已更新，若本次交易异常请检查交易流水记录":"更新成功",pcOrderNo); //若查询交易中已经更新则此处更新0条
    tCommit();
    tReleaseRes(pstRes);
    return ( 0 );
}

/*更新银联二维码交易金额*/
int UpCupsAmt(double pcAmount, double pcOrderAmount, char *pcOrderNo) {
    char sSqlStr[1024] = {0};
    int iResCnt = 0;
    
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set amount = %.02f,order_amt = %.02f "
                    "where merch_order_no = '%s' and trans_code not in ('02Y300','0AY300')", pcAmount,pcOrderAmount,pcOrderNo );
    tLog(DEBUG,"sql = [%s]",sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tCommit();
    tLog(DEBUG, "影响记录数[%d]", iResCnt);
    return ( 0 );
}

/*更新VALID_FLAG*/
int UpValidFlag( char *pcOrderNo ) {
    char sSqlStr[1024]      = {0};
    OCI_Resultset *pstRes   = NULL;
    
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set valid_flag='0',last_mod_time = sysdate "
                    "where valid_flag = '4' and merch_order_no = '%s' and trans_code not in ('02B300','02W300','02Y300','0AY300','0AW300','0AB300','0AQ300')",pcOrderNo );

    if ( tExecute(&pstRes, sSqlStr) < 0 ) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tReleaseRes(pstRes);
        return ( -1 );
    }
    tLog(INFO,"更新了[%d]条",tGetAffectedRows());
    tCommit();
    tReleaseRes(pstRes);
 
    return ( 0 );
}
int UpRespDesc( char *pcRespCode, char *pcRespDesc, char *pcRrn ) {
    char sSqlStr[1024] = {0};
    int iResCnt = 0;
    
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set resp_code = '%s', istresp_code = '%s', resp_desc ='%s', notice_flag = '2',"
                " last_mod_time = sysdate where rrn = '%s' and notice_flag != '1'",pcRespCode,pcRespCode,pcRespDesc,pcRrn );
    
    tLog(DEBUG,"sql = [%s]",sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tCommit();
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
    tCommit();
    tLog(DEBUG, "影响记录数[%d]", iResCnt);
    return ( 0 );
}