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



int BsEqualProc() {
    char sSqlStr[2048] = {0}, sSqlStr1[2048] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_qrbank_trans_detail set CHK_FLAG='Y' \
    where merch_order_no in ( select merch_order_no from B_Inline_Tarns_Detail_his \
    where b_qrbank_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no AND  \
    B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') ) AND CHK_FLAG= 'N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    snprintf(sSqlStr1, sizeof (sSqlStr1), "UPDATE B_Inline_Tarns_Detail_his  set CHECK_FLAG='Y',channel_fee= \
    (select fee from b_qrbank_trans_detail where b_qrbank_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no) \
	where merch_order_no in (select merch_order_no from b_qrbank_trans_detail \
    where b_qrbank_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no  \
    AND B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') ) and CHECK_FLAG in ('N','M') \
    and trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200')");
    if (tExecute(&pstRes, sSqlStr1) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "包商银行二维码对账平,处理完成.");
    return 0;

}

int BsLongProc() {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_qrbank_trans_detail  set CHK_FLAG='L' \
    where merch_order_no not in (select merch_order_no from B_Inline_Tarns_Detail_his \
    where b_qrbank_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no  \
    AND B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') \
    union select merch_order_no from B_Inline_Tarns_Detail \
    where b_qrbank_trans_detail.merch_order_no=B_Inline_Tarns_Detail.merch_order_no  \
    AND B_Inline_Tarns_Detail.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200')) \
    and chk_flag='N' " );
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "包商银行二维码长款,处理完成.");
    return 0;

}

int WbNoSettleProc() {
    char sSqlStr[256] = {0};
    OCI_Resultset *pstRes = NULL;

    /* 撤销的原交易 */
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_WB_TRANS_DETAIL set CHK_FLAG = 'B' "
            "where merch_order_no in(select o_merch_order_no "
            "from B_WB_TRANS_DETAIL where o_merch_order_no is not null and CHK_FLAG = 'N') and CHK_FLAG = 'N'");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置撤销的原交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "微众流水表[撤销的原交易]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    
    MEMSET(sSqlStr);    
    pstRes = NULL;
    
    /* 撤销交易 */
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_WB_TRANS_DETAIL set CHK_FLAG = 'B'"
            " where o_merch_order_no is not null and CHK_FLAG = 'N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置查询和撤销交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "微众流水表[查询和撤销]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    
    //更新流水表撤销和撤销原交易
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_INLINE_TARNS_DETAIL_HIS set CHECK_FLAG = 'B'"
            " where merch_order_no in(select merch_order_no "
            " from B_WB_TRANS_DETAIL where CHK_FLAG = 'B') and CHECK_FLAG in('N','Y') and substr(trans_code,4,1)!='3' ");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置查询和撤销交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "二维码流水表[查询和撤销]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    
    
    tLog(DEBUG, "不结算,处理完成.");
    return 0;

}


int WbEqualProc() {
    char sSqlStr[2048] = {0}, sSqlStr1[2048] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_wb_trans_detail set CHK_FLAG='Y' \
    where merch_order_no in ( select merch_order_no from B_Inline_Tarns_Detail_his \
    where b_wb_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no AND  \
    B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') ) AND CHK_FLAG= 'N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    snprintf(sSqlStr1, sizeof (sSqlStr1), "UPDATE B_Inline_Tarns_Detail_his  set CHECK_FLAG='Y',channel_fee= \
    (select fee from b_wb_trans_detail where b_wb_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no) \
	where merch_order_no in (select merch_order_no from b_wb_trans_detail \
    where b_wb_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no  \
    AND B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') ) and CHECK_FLAG in ('N','M') \
    and trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200')");
    if (tExecute(&pstRes, sSqlStr1) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "微众二维码对账平,处理完成.");
    return 0;

}

int WbLongProc() {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_wb_trans_detail  set CHK_FLAG='L' \
    where merch_order_no not in (select merch_order_no from B_Inline_Tarns_Detail_his \
    where b_wb_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no  \
    AND B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') \
    union select merch_order_no from B_Inline_Tarns_Detail \
    where b_wb_trans_detail.merch_order_no=B_Inline_Tarns_Detail.merch_order_no  \
    AND B_Inline_Tarns_Detail.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200')) \
    and chk_flag='N' " );
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "微众二维码长款,处理完成.");
    return 0;

}


int ZxNoSettleProc() {
    char sSqlStr[256] = {0};
    OCI_Resultset *pstRes = NULL;

    /* 撤销的原交易 */
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_ZX_TRANS_DETAIL set CHK_FLAG = 'B' "
            "where merch_order_no in(select o_merch_order_no "
            "from B_ZX_TRANS_DETAIL where o_merch_order_no is not null and CHK_FLAG = 'N') and CHK_FLAG = 'N'");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置撤销的原交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "微众流水表[撤销的原交易]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    
    MEMSET(sSqlStr);    
    pstRes = NULL;
    
    /* 撤销交易 */
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_WB_TRANS_DETAIL set CHK_FLAG = 'B'"
            " where o_merch_order_no is not null and CHK_FLAG = 'N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置查询和撤销交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "微众流水表[查询和撤销]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    
    //更新流水表撤销和撤销原交易
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_INLINE_TARNS_DETAIL_HIS set CHECK_FLAG = 'B'"
            " where merch_order_no in(select merch_order_no "
            " from B_WB_TRANS_DETAIL where CHK_FLAG = 'B') and CHECK_FLAG in('N','Y') and substr(trans_code,4,1)!='3' ");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置查询和撤销交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "二维码流水表[查询和撤销]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    
    
    tLog(DEBUG, "不结算,处理完成.");
    return 0;

}


int ZxEqualProc() {
    char sSqlStr[2048] = {0}, sSqlStr1[2048] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_zx_trans_detail set CHK_FLAG='Y' \
    where merch_order_no in ( select merch_order_no from B_Inline_Tarns_Detail_his \
    where b_zx_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no AND  \
    B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') ) AND CHK_FLAG= 'N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    snprintf(sSqlStr1, sizeof (sSqlStr1), "UPDATE B_Inline_Tarns_Detail_his  set CHECK_FLAG='Y',channel_fee= \
    (select fee from b_zx_trans_detail where b_zx_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no) \
	where merch_order_no in (select merch_order_no from b_zx_trans_detail \
    where b_zx_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no  \
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
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE b_zx_trans_detail  set CHK_FLAG='L' \
    where merch_order_no not in (select merch_order_no from B_Inline_Tarns_Detail_his \
    where b_zx_trans_detail.merch_order_no=B_Inline_Tarns_Detail_his.merch_order_no  \
    AND B_Inline_Tarns_Detail_his.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200') \
    union select merch_order_no from B_Inline_Tarns_Detail \
    where b_zx_trans_detail.merch_order_no=B_Inline_Tarns_Detail.merch_order_no  \
    AND B_Inline_Tarns_Detail.trans_code in('02B100','02B200','02W100','02W200','0AB100','0AB200','0AW100','0AW200')) \
    and chk_flag='N' " );
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