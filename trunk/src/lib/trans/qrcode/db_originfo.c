#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"
#include "qrcodetransdetail.h"

/*二维码  查询交易 从交易流水表中获取渠道商户号*/
int FindOriTransMsg(char *pcOrderNo, char *pcTransCode, char *pcChannelMerchId,char *pcValidFlag, double * pdAmount) {
//int FindOriTransMsg(char *pcOrderNo, char *pcTransCode, char *pcChannelMerchId,char *pcValidFlag) {
    OCI_Resultset *pstRes = NULL;
    double dAmount = 0.00;
    char sSqlStr[1024];
    MEMSET(sSqlStr);
    
    tTrim(pcOrderNo);
    snprintf(sSqlStr,sizeof(sSqlStr),"select trans_code,channel_merch_id,valid_flag,amount "
                    "from b_inline_tarns_detail where merch_order_no = '%s' "
                    "and trans_code in (select trans_code from s_trans_code where settle_flag = '1'and group_code = 'INLINE')",pcOrderNo);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    
     while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcTransCode);
        STRV(pstRes, 2, pcChannelMerchId);
        STRV(pstRes, 3, pcValidFlag);
        DOUV(pstRes, 4, dAmount);
        
        tTrim(pcTransCode);
        tTrim(pcChannelMerchId);
        tTrim(pcValidFlag);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);    
    *pdAmount = dAmount;
    return ( 0 );
}

/*二维码 撤销交易检查原始流水*/
int FindOriTransDetail(char *pcOrrn, QrcodeTransDetail *pstQrcodeTransDetail) {
    QrcodeTransDetail stQrcodeTransDetail;
    OCI_Resultset *pstRes = NULL;
    char sSqlStr[1024];
    
    MEMSET(sSqlStr);
    memset(&stQrcodeTransDetail, 0x00, sizeof (QrcodeTransDetail));
    
    tTrim(pcOrrn);
    snprintf(sSqlStr, sizeof(sSqlStr), "select "
            "id, trans_code, amount, settle_date, rrn, mcc, agent_id, channel_merch_id, valid_flag, "
            "check_flag, settle_flag, batch_no, fee, fee_flag, fee_desc, fee_type, merch_order_no, order_url,user_code "
            "from b_inline_tarns_detail where rrn = '%s'",pcOrrn);
    
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
				
	STRV(pstRes, 1, stQrcodeTransDetail.sId);
        STRV(pstRes, 2, stQrcodeTransDetail.sTransCode);
        DOUV(pstRes, 3, stQrcodeTransDetail.dAmount);
        STRV(pstRes, 4, stQrcodeTransDetail.sSettleDate);
        STRV(pstRes, 5, stQrcodeTransDetail.sRrn);
        STRV(pstRes, 6, stQrcodeTransDetail.sMcc);
        STRV(pstRes, 7, stQrcodeTransDetail.sAgentId);
        STRV(pstRes, 8, stQrcodeTransDetail.sChannelMerchId);
        STRV(pstRes, 9, stQrcodeTransDetail.sValidFlag);
        STRV(pstRes, 10, stQrcodeTransDetail.sCheckFlag);
        STRV(pstRes, 11, stQrcodeTransDetail.sSettleFlag);
        STRV(pstRes, 12, stQrcodeTransDetail.sBatchNo);
        DOUV(pstRes, 13, stQrcodeTransDetail.dFee);
        STRV(pstRes, 14, stQrcodeTransDetail.sFeeFlag);
        STRV(pstRes, 15, stQrcodeTransDetail.sFeeDesc);
        STRV(pstRes, 16, stQrcodeTransDetail.sFeeType);
        STRV(pstRes, 17, stQrcodeTransDetail.sMerchOrderNo);
        STRV(pstRes, 18, stQrcodeTransDetail.sOrderUrl); 
        STRV(pstRes, 19, stQrcodeTransDetail.sUserCode);
                
        tTrim(stQrcodeTransDetail.sTransCode);
        tTrim(stQrcodeTransDetail.sSettleDate);
        tTrim(stQrcodeTransDetail.sRrn);
        tTrim(stQrcodeTransDetail.sMcc);
        tTrim(stQrcodeTransDetail.sAgentId);
        tTrim(stQrcodeTransDetail.sChannelMerchId);
        tTrim(stQrcodeTransDetail.sValidFlag);
        tTrim(stQrcodeTransDetail.sCheckFlag);
        tTrim(stQrcodeTransDetail.sSettleFlag);
        tTrim(stQrcodeTransDetail.sBatchNo);
        tTrim(stQrcodeTransDetail.sFeeFlag);
        tTrim(stQrcodeTransDetail.sFeeDesc);
        tTrim(stQrcodeTransDetail.sFeeType);
        tTrim(stQrcodeTransDetail.sMerchOrderNo);
        tTrim(stQrcodeTransDetail.sOrderUrl);
        tTrim(stQrcodeTransDetail.sUserCode);
    }
    
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    memcpy(pstQrcodeTransDetail, &stQrcodeTransDetail, sizeof (QrcodeTransDetail));
    tLog(DEBUG,"检索到原交易信息");
    return ( 0 );
}

