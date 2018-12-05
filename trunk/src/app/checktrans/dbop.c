/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2016年11月30日, 下午9:00
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_db.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_redis.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "t_log.h"
#include "param.h"
#include "dbop.h"

int NoSettleProc() {
    char sSqlStr[256] = {0};
    OCI_Resultset *pstRes = NULL;


    /* 冲正交易 */
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_CUPS_TRANS_DETAIL  set CHK_FLAG='B' "
            "where  (sys_trace,transmit_time) in (select org_sys_trace,org_transmit_time from B_CUPS_TRANS_DETAIL where trans_code='020300' and CHK_FLAG='N' ) "
            "and CHK_FLAG='N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置冲正交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "银联流水表[冲正]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);

    MEMSET(sSqlStr);
    pstRes = NULL;

    /* 撤销的原交易 */
    /*
         snprintf(sSqlStr, sizeof (sSqlStr), "update B_CUPS_TRANS_DETAIL set CHK_FLAG = 'B' "
            "where channel_rrn in(select substr(ORG_TRANSMIT_TIME, 5, 10) || ORG_SYS_TRACE "
            " from B_CUPS_TRANS_DETAIL where trans_code = '020200' and CHK_FLAG = 'N') and CHK_FLAG = 'N'");
     */

    snprintf(sSqlStr, sizeof (sSqlStr), " update B_CUPS_TRANS_DETAIL set CHK_FLAG = 'B'"
            " where (transmit_time,sys_trace) in (select ORG_TRANSMIT_TIME,ORG_SYS_TRACE "
            " from B_CUPS_TRANS_DETAIL where trans_code = '020200' and CHK_FLAG = 'N') and CHK_FLAG = 'N'");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置撤销的原交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "银联流水表[撤销的原交易]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
   
    MEMSET(sSqlStr);
    pstRes = NULL;

    /* 查询和撤销和冲正交易 */
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_CUPS_TRANS_DETAIL set CHK_FLAG = 'B'"
            " where trans_code in('021000', '020200', '020300') and CHK_FLAG = 'N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置查询和撤销交易为不结算失败.");
        return -1;
    }
    tLog(ERROR, "银联流水表[查询和撤销和冲正]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    
    MEMSET(sSqlStr);
    pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_INLINE_TARNS_DETAIL_HIS set CHECK_FLAG = 'B' "
           "where (settle_trans_time,settle_sys_trace) in (select transmit_time,sys_trace "
           "from B_CUPS_TRANS_DETAIL where CHK_FLAG = 'B') and CHECK_FLAG = 'N'");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置银联二维码流水表不结算失败.");
        return -1;
    }
    tLog(ERROR, "银联二维码流水表不对账记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
 

    tLog(DEBUG, "不结算,处理完成.");
    return 0;

}

int EqualProc() {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
/*
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_trans_detail_HIS set CHECK_FLAG='Y',channel_fee="
            "(select fee from B_CUPS_TRANS_DETAIL where B_CUPS_TRANS_DETAIL.channel_rrn=B_POS_trans_detail_HIS.rrn)"
            " where B_POS_trans_detail_HIS.rrn in ( select B_CUPS_TRANS_DETAIL.channel_rrn from B_CUPS_TRANS_DETAIL"
            " where B_CUPS_TRANS_DETAIL.channel_rrn=B_POS_trans_detail_HIS.rrn and B_CUPS_TRANS_DETAIL.CHK_FLAG='N')"
            "AND check_flag='N'");
 * 
 */ 
    snprintf(sSqlStr, sizeof (sSqlStr), "merge into (select * from B_POS_trans_detail_HIS"
            " where check_flag ='N' and resp_code='00' and fee_flag in('M','Y') ) h"
            " using B_CUPS_TRANS_DETAIL c"
            " on (c.channel_rrn=h.rrn and c.CHK_FLAG='N' and c.trans_date=h.trans_date and c.trans_time=h.trans_time)"
            " when matched then"
            " update set  h.CHECK_FLAG='Y',h.channel_fee=c.fee,h.CHANNEL_SETTLE_DATE=c.SETTLE_DATE");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置对账平失败.");
        return -1;
    }
    tLog(ERROR, "本地流水表[对平]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);

    MEMSET(sSqlStr);
    pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "merge into (select * from B_inline_tarns_detail_HIS "
            "where check_flag ='N' and resp_code='00' and fee_flag in('M','Y') ) h "
            "using B_CUPS_TRANS_DETAIL c "
            "on (c.sys_trace=h.settle_sys_trace and c.CHK_FLAG='N' and c.transmit_time=h.settle_trans_time and h.amount=c.amount) "
            "when matched then "
            "update set  h.CHECK_FLAG='Y',h.channel_fee=c.fee,h.CHANNEL_SETTLE_DATE=c.SETTLE_DATE");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "银联二维码设置对账平失败.");
        return -1;
    }
    tLog(ERROR, "银联二维码本地流水表[对平]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);

    MEMSET(sSqlStr);
    pstRes = NULL;
    /*
        snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_CUPS_TRANS_DETAIL  set CHK_FLAG='Y'"
                "where channel_rrn "
                "in (select B_POS_trans_detail_HIS.rrn  from B_POS_trans_detail_HIS "
                "where B_CUPS_TRANS_DETAIL.channel_rrn=B_POS_trans_detail_HIS.rrn )"
                " and CHK_FLAG='N'");
     */
    snprintf(sSqlStr, sizeof (sSqlStr), "merge into (SELECT * from B_CUPS_TRANS_DETAIL where chk_flag='N') c"
            " using (select * from B_POS_trans_detail_HIS"
            " where check_flag ='Y' and resp_code='00' and fee_flag in ('M','Y')) h"
            " on (c.channel_rrn=h.rrn and c.trans_date=h.trans_date and c.trans_time=h.trans_time)"
            " when matched then"
            "  update set c.CHK_FLAG='Y' where  c.chk_flag='N' ");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置渠道对账平失败.");
        return -1;
    }
    tLog(ERROR, "银联流水表[对平]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);

    MEMSET(sSqlStr);
    pstRes = NULL;
    
    snprintf(sSqlStr, sizeof (sSqlStr), "merge into (SELECT * from B_CUPS_TRANS_DETAIL where chk_flag='N') c "
             "using (select * from B_INLINE_tarns_detail_HIS "
             "where check_flag ='Y' and resp_code='00' and fee_flag in ('M','Y')) h "
             "on (c.sys_trace=h.settle_sys_trace and c.transmit_time=h.settle_trans_time) "
             "when matched then "                                                                                                             
             "update set c.CHK_FLAG='Y', input_mode=replace(input_mode,'05','94') where  c.chk_flag='N' ");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "银联二维码设置渠道对账平失败.");
        return -1;
    }
    tLog(ERROR, "银联二维码流水表[对平]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    
    tLog(DEBUG, "对账平,处理完成.");
}

int LongProc() {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_CUPS_TRANS_DETAIL  set CHK_FLAG='L' where CHK_FLAG='N'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "设置长款失败.");
        return -1;
    }

    tLog(ERROR, "银联流水表[长款]记录数[%d].", tGetAffectedRows(pstRes));
    if (0 != tGetAffectedRows(pstRes)) {
        tCommit();
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "长款,处理完成.");
    return 0;
}



