/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_app_conf.h"
#include "t_log.h"
#include "t_db.h"
#include "param.h"

int AddZxTrans(ZxSettleTransDetail *pstChannleTrans) {

    char sSqlStr[4096] = {0};
    int iRet = -1;
    ZxSettleTransDetail stChannleTrans;
    memset(&stChannleTrans, 0x00, sizeof (ZxSettleTransDetail));
    stChannleTrans = *pstChannleTrans;
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_ZX_SETTLE_TRANS_DETAIL(TRANS_TIME, ACCOUNT_ID, TH_MERCH_NO, MERCH_NO, SUB_MERCH_NO,  \
     TERM_ID, WFT_ORDER_NO, TH_ORDER_NO, MERCH_ORDER_NO, USER_ID,  \
     TRANS_TYPE, TRAN_STATUS, PAY_BANK, AMT_TYPE, AMOUNT, REPAK_AMT, \
     REFUND_NO, MERCH_REFUND_NO, REFUND_AMT, REPAK_REFUND_AMT, \
     REFUND_TYPE, REFUND_STATUS, COMM_NAME, MERCH_DATA,  \
     FEE, RATE, TERM_TYPE, CHK_MARK, STORES_NO, MERCH_NAME,MERCH_ID, SUB_MERCH_ID,ACTUAL_AMOUNT, EXTEND1 ) \
    VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%f','%f','%s', \
            '%s','%f','%f','%s','%s','%s','%s','%f','%f','%s','%s','%s','%s','%s','%s','%f','%s')", stChannleTrans.sTansTime,
            stChannleTrans.sAccountId,
            stChannleTrans.sThMerchNo,
            stChannleTrans.sMerchNo,
            stChannleTrans.sSubMerchNo,
            stChannleTrans.sTermId,
            stChannleTrans.sWftOrderNo,
            stChannleTrans.sThOrderNo,
            stChannleTrans.sMerchOrderNo,
            stChannleTrans.sUserId,
            stChannleTrans.sTransType,
            stChannleTrans.sTranStatus,
            stChannleTrans.sPayBank,
            stChannleTrans.sAmtType,
            stChannleTrans.dDmount,
            stChannleTrans.dRepakAmt,
            stChannleTrans.sRefundNo,
            stChannleTrans.sMerchRefundNo,
            stChannleTrans.dRefundAmt,
            stChannleTrans.dRepakRefundAmt,
            stChannleTrans.sRefundType,
            stChannleTrans.sRefundStatus,
            stChannleTrans.sCommName,
            stChannleTrans.sMerchData,
            stChannleTrans.dFee,
            stChannleTrans.dRate,
            stChannleTrans.sTermType,
            stChannleTrans.sChkMark,
            stChannleTrans.sNumber,
            stChannleTrans.sMerchName,
            stChannleTrans.sMerchId,
            stChannleTrans.sSubMerchId,
            stChannleTrans.dActualAmount,
            stChannleTrans.sExtend1);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    return 0;
}