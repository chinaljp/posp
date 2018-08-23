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
#include "t_redis.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "t_log.h"
#include "param.h"
#include "t_db.h"



int EposNoSettleProc() {
    char sSqlStr[256] = {0};
    OCI_Resultset *pstRes = NULL;   
    tLog(DEBUG, "不结算,处理完成.");
    return 0;

}


int EposEqualProc() {
    char sSqlStr[2048] = {0}, sSqlStr1[2048] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_epos_trans_detail set CHK_FLAG='Y' \
    where (rrn,trans_date) in ( select rrn,trans_date from B_Inline_Tarns_Detail_his \
    where b_epos_trans_detail.rrn=B_Inline_Tarns_Detail_his.rrn AND  \
    b_epos_trans_detail.trans_date=B_Inline_Tarns_Detail_his.trans_date AND \
    B_Inline_Tarns_Detail_his.trans_code in('0AQ000') ) AND CHK_FLAG= 'N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    snprintf(sSqlStr1, sizeof (sSqlStr1), "UPDATE B_Inline_Tarns_Detail_his  set CHECK_FLAG='Y',channel_fee= \
    (select NET_SERVE_FEE+BRAND_FEE+S_CHARGE+S_PAY from b_epos_trans_detail where b_epos_trans_detail.rrn=B_Inline_Tarns_Detail_his.rrn \
            AND  b_epos_trans_detail.trans_date=B_Inline_Tarns_Detail_his.trans_date) \
	where (rrn,trans_date) in (select rrn,trans_date from b_epos_trans_detail \
    where b_epos_trans_detail.rrn=B_Inline_Tarns_Detail_his.rrn  and b_epos_trans_detail.trans_date=B_Inline_Tarns_Detail_his.trans_date \
    AND B_Inline_Tarns_Detail_his.trans_code in('0AQ000') ) and CHECK_FLAG in ('N','M') \
    and trans_code in('0AQ000')");
    if (tExecute(&pstRes, sSqlStr1) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "epos对账平,处理完成.");
    return 0;

}

int EposLongProc() {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_epos_trans_detail  set CHK_FLAG='L' \
    where (rrn,trans_date) not in (select rrn,trans_date from B_Inline_Tarns_Detail_his \
    where b_epos_trans_detail.rrn=B_Inline_Tarns_Detail_his.rrn  and b_epos_trans_detail.trans_date=B_Inline_Tarns_Detail_his.trans_date \
    AND B_Inline_Tarns_Detail_his.trans_code in('0AQ000') \
    union select rrn,trans_date from B_Inline_Tarns_Detail \
    where b_epos_trans_detail.rrn=B_Inline_Tarns_Detail.rrn and  b_epos_trans_detail.trans_date=B_Inline_Tarns_Detail.trans_date \
    AND B_Inline_Tarns_Detail.trans_code in('0AQ000')) \
    and chk_flag='N' " );
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "epos长款,处理完成.");
    return 0;

}

