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

    snprintf(sSqlStr, sizeof (sSqlStr), "merge into (SELECT * from b_epos_trans_detail where chk_flag='N') e \
         using (select * from B_Inline_Tarns_Detail_his where check_flag='N' and trans_code in('0AQ000') ) h \
         on (e.rrn=h.rrn and e.trans_date=h.trans_date ) \
         when matched then \
         update set e.CHK_FLAG='Y'  where e.CHK_FLAG='N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tLog(ERROR, "epos对账流水表[对平]记录数[%d].", tGetAffectedRows(pstRes));
    tCommit();
    tReleaseRes(pstRes);
    snprintf(sSqlStr1, sizeof (sSqlStr1), "merge into (select * from B_Inline_Tarns_Detail_his \
             where  trans_code in('0AQ000') and CHECK_FLAG in ('N','M') ) h \
             using b_epos_trans_detail e \
            on (e.rrn=h.rrn and  e.trans_date=h.trans_date ) \
             when matched then \
            update set  h.CHECK_FLAG='Y',h.channel_fee=e.NET_SERVE_FEE+e.BRAND_FEE+e.S_CHARGE+e.S_PAY,h.CHANNEL_SETTLE_DATE=e.SETTLE_DATE");
    if (tExecute(&pstRes, sSqlStr1) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tLog(ERROR, "本地流水表[对平]记录数[%d].", tGetAffectedRows(pstRes));
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
    and chk_flag='N' ");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tLog(ERROR, "本地流水表[长款]记录数[%d].", tGetAffectedRows(pstRes));
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "epos长款,处理完成.");
    return 0;

}

