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

int ZxEqualProc() {
    char sSqlStr[2048] = {0}, sSqlStr1[2048] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_zx_settle_trans_detail set CHK_FLAG='Y' \
    where merch_order_no in ( select order_no from B_Inline_Tarns_Detail_his \
    where b_zx_settle_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.order_no AND  \
    B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') ) AND CHK_FLAG= 'N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    snprintf(sSqlStr1, sizeof (sSqlStr1), "UPDATE B_Inline_Tarns_Detail_his  set CHECK_FLAG='Y',channel_fee= \
    (select fee from b_zx_settle_trans_detail where b_zx_settle_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.order_no) \
	where order_no in (select order_no from B_Inline_Tarns_Detail_his,b_zx_settle_trans_detail \
    where b_zx_settle_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.order_no  \
    AND B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') ) and CHECK_FLAG in ('N','M') \
    and trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200')");
    if (tExecute(&pstRes, sSqlStr1) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "中信二维码对账平,处理完成.");
    return 0;

}

int ZxLongProc() {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_zx_settle_trans_detail  set CHK_FLAG='L' \
    where merch_order_no not in (select order_no from B_Inline_Tarns_Detail_his \
    where b_zx_settle_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.order_no  \
    AND B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200')) \
    and chk_flag='N' and  to_char(to_date(trans_time,'yyyy-mm-dd HH24:MI:SS'),'yyyymmddhh24miss') < ( to_char(sysdate-1,'yyyymmdd')||'233000' )");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "中信二维码长款,处理完成.");
    return 0;

}