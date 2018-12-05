/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"

/*只适用于查询交易使用，更新被扫交易流水中的valid_flag*/
int UpValidflag(char * pstOrderNo) {
    char sOrderNo[30 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sOrderNo, pstOrderNo);
    tTrim(sOrderNo);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET VALID_FLAG='0' WHERE MERCH_ORDER_NO='%s' AND "
                          "TRANS_CODE NOT IN ('02B300','02W300','02Y300','0AY300','0AB300','0AW300')", sOrderNo);
    
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
        tLog(ERROR, "更新订单号[%s]结算标志失败",sOrderNo);
        return -1;
    }

    tReleaseRes(pstRes);
    return 0;
}

/*银联二维码查询结果 支付成功时 更新settleKey中的信息到原交易流水中*/
int UpCupsSettleMessage(char * pcOrderNo, char *pcCardType, char * pcSettleSysTrace, char * pcSettleTransTime, char *pcChannelSettleDate,char * pcSettleDate) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    int iNum = 0;
    
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set card_type = '%s',settle_sys_trace = '%s',settle_trans_time = '%s',channel_settle_date = '%s',settle_date = '%s'  "
                    "where merch_order_no = '%s' and trans_code not in ('02Y300','0AY300') and "
            "not exists (select 1 from b_inline_tarns_detail where settle_sys_trace = '%s' and settle_trans_time = '%s')",
            pcCardType,pcSettleSysTrace,pcSettleTransTime,pcChannelSettleDate,pcSettleDate,pcOrderNo,pcSettleSysTrace,pcSettleTransTime );
        
    if (tExecute(&pstRes, sSqlStr) < 0 ) {
        tLog(ERROR,"sSqlStr = [%s]",sSqlStr);
        tLog(ERROR, "更新订单[%s]的 settle_sys_trace、settle_trans_time 失败.",pcOrderNo);
        return -1;
    }
    iNum = tGetAffectedRows();
    tLog(INFO,"更新了[%d]条,%s,商户订单号[%s]",iNum,
            iNum == 0? "异步通知中可能已更新，若本次交易异常请检查交易流水记录":"更新成功",pcOrderNo);//若异步通知处理中已经更新则此处更新0条
    tReleaseRes(pstRes);
    return ( 0 );
}

int UpCupsSettleKey(char * pcOrderNo, char * pcSettleSysTrace, char * pcSettleTransTime) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    int iNum = 0;
    
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set settle_sys_trace = '%s',settle_trans_time = '%s' "
                    "where merch_order_no = '%s' and trans_code not in ('02Y300','0AY300') and "
            "not exists (select 1 from b_inline_tarns_detail where settle_sys_trace = '%s' and settle_trans_time = '%s')",
            pcSettleSysTrace,pcSettleTransTime,pcOrderNo,pcSettleSysTrace,pcSettleTransTime );
        
    if (tExecute(&pstRes, sSqlStr) < 0 ) {
        tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
        tLog(ERROR, "更新订单[%s]的 settle_sys_trace、settle_trans_time 失败.",pcOrderNo);
        return -1;
    }
    iNum = tGetAffectedRows();
    tLog(INFO,"更新了[%d]条,%s,商户订单号[%s]",iNum,
            iNum == 0? "异步通知中可能已更新，若本次交易异常请检查交易流水记录":"更新成功",pcOrderNo);//若异步通知处理中已经更新则此处更新0条
    tReleaseRes(pstRes);
    return ( 0 );
}

/*二维码主扫交易 更新交易流水中的valid_flag, 二维码撤销交易，更新二维码撤销交易流水中的valid_flag  注意更新二维码主扫交易流水中的valid_flag  不在使用 作废20180129 ***/
int UpValidflagInLine(char * pcSysTrace,char * pcTransDate) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    tTrim(pcSysTrace);
    tTrim(pcTransDate);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET VALID_FLAG='0' WHERE SYS_TRACE='%s' AND TRANS_DATE = '%s' AND \
                          TRANS_CODE IN ('02W100','02B100','0AW100','0AB100','02W600','02B600','0AW600','0AB600','02Y600')",pcSysTrace,pcTransDate);
    
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新流水[%s]有效标志失败,trans_date= [%s]",pcSysTrace,pcTransDate);
        return -1;
    }

    tReleaseRes(pstRes);

    return 0;
}

/*查询渠道商户appkey*/
int FindMerchApk(char *pcChannelMerchId, char *pcAppKey) {
    OCI_Resultset *pstRes = NULL;
    char sSqlStr[1024];
    
    tTrim(pcChannelMerchId);
    snprintf(sSqlStr,sizeof(sSqlStr),"select app_key \
                            from b_merch_send_detail where chnmerch_id = '%s'",pcChannelMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcAppKey);
        
        tTrim(pcAppKey);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    
    return ( 0 );
}

/* 更新valid_flag标志 */
int UpdTransDetail(char *pcTable, char *pcDate, char *pcRrn, char *pcValidFlag) {
    char sSqlStr[1024];
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE %s SET "
            " VALID_FLAG= '%s' "
            " WHERE trans_date = '%s'"
            " AND rrn = '%s' "
            , pcTable, pcValidFlag, pcDate, pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新原交易[%s:%s]valid_flag[%s]失败.", pcDate, pcRrn, pcValidFlag);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "更新原交易[%s:%s]valid_flag[%s]成功[%d].", pcDate, pcRrn, pcValidFlag, tGetAffectedRows(pstRes));
    return 0;
}

int FindOldOrderNo(char * pstOrrn, char * pstOldOrderNo) {

    char sOrderNo[30 + 1] = {0}, sORrn[12 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sORrn, pstOrrn);
    tTrim(sORrn);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT ORDER_NO FROM B_INLINE_TARNS_DETAIL WHERE RRN='%s'", sORrn);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找原交易RRN=[%s]订单号失败", sORrn);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sOrderNo);
    }
    strcpy(pstOldOrderNo, sOrderNo);
    tReleaseRes(pstRes);
    return 0;
}

/*更新清算日期、商户订单号 add by gjq at 20171212*/
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

/*更新计算后的 手续费（微信、支付宝 主被扫交易使用）*/
int UpFeeMsg(char *pcOrderNo,char *pcFeeType,double dFee,char *pcFeeDesc) {
    char sSqlStr[1024]      = {0};
    OCI_Resultset *pstRes   = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set fee_type = '%s',fee = %.02f,fee_desc = '%s',fee_flag = 'M' "
                    "where merch_order_no = '%s' and trans_code not in ('02B300','02W300','0AB300','0AW300')",
            pcFeeType,dFee,pcFeeDesc,pcOrderNo);

    if ( tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0 ) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tReleaseRes(pstRes);
        return ( -1 );
    }
    tLog(DEBUG,"更新了[%d]条",tGetAffectedRows());
    tReleaseRes(pstRes);
    return ( 0 );
}
