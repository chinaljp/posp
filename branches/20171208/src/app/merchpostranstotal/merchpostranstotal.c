/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   merchpostranstotal.c
 * Author: Administrator
 *
 * Created on 2017年7月17日, 上午11:01
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_db.h"
#include "t_tools.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "trace.h"
#include "t_log.h"
#include "t_app_conf.h"
#include "t_daemon.h"

/*
 * 
 */

int UpdateRefund(char *pcMerchId, char *pcStatus);
int GetRrn(char *pcRrn,char *pcSysTrace);
int addWalletToAgent(char *pcAgentId, char *pcAmount);
int UnFrozenWallet(char *pcMerchId, char *SerialNum, char *pcAmount, char *pcFrozenStatus);
int BackMoney();
int g_iQutLoop;

void SigQuit(int iSigNo) {
    int i;
    tLog(DEBUG, "收到[%d]退出信号!", iSigNo);
    g_iQutLoop = 0;
}

/* 屏蔽信号，只保留推出信号 */
void SigProc() {
    int i;
    for (i = 0; i < NSIG; i++) {
        if (i != SIGSEGV)
            sigset(i, SIG_IGN);
    }
}

int main(int argc, char* pcArgv[]) {
    
    int iCnt = 0;
    OCI_Resultset *pstRes = NULL;
    char sCurTime[8+1];
    char sTime[2+1];
    char sSqlStr[1024];
    
    MEMSET(sCurTime);
    MEMSET(sTime);
    MEMSET(sSqlStr);
    g_iQutLoop = 1;
    
    tDaemon();
    
    /* 屏蔽信号，只保留推出信号 */
    SigProc();
    sigset(SIGUSR1, SigQuit);
    
    if (tInitLog(pcArgv[0]) < 0) {
        fprintf(stderr, "初始化[%s]日志失败.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }
    
    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis连接失败.");
        tDoneLog();
        return -1;
    }
    
    if (tOpenDb() < 0) {
        tLog(ERROR, "数据库连接失败.");
        tDoneLog();
        tCloseRedis();
        return -1;
    }
    
    while(g_iQutLoop) {
        tGetTime(sCurTime, ":", -1);
        tLog(INFO, "当前时间[%s]",sCurTime);
        /*取当前时间整点*/
        tStrCpy( sTime, sCurTime, 2 );
        if(atoi(sTime) == 6) {
            /*助贷通累计交易额 只累计贷记卡交易额*/
            snprintf(sSqlStr,sizeof(sSqlStr),"merge into b_open_merchant o \
                                using (select user_code,sum(h.amount) as amount from b_pos_trans_detail_his h \
                                join b_cups_trans_detail c on c.channel_rrn=h.rrn and c.trans_date=h.trans_date and c.chk_flag='Y' and h.card_type = '1'\
                                where c.settle_date=to_char(sysdate-1,'YYYYMMDD') group by user_code ) p \
                                on (p.user_code=o.user_code) \
                                when matched then \
                                update set o.trans_amt =o.trans_amt+p.amount");
            if(tExecute(&pstRes, sSqlStr) < 0) {
                tLog(DEBUG, "sql[%s]", sSqlStr);
                tLog(ERROR, "============POS交易，商户累计交易额更新失败=============");
                tRollback();
            }
            else {
                iCnt = tGetAffectedRows();
                tLog(ERROR, "============POS交易，商户累计交易额更新成功,更新商户数[%d]=============",iCnt);
                tCommit();
            }
            tReleaseRes(pstRes);
            
            /*检查助贷通商户在规定周期内的交易额是否满足规定，满足则返回押金商户，不满足则将押金返给代理商*/
            if( BackMoney() < 0) {
                tLog(ERROR,"助贷通商户押金返还操作失败，注意查看原因！！！");
            }
        }
        
        sleep(3600);
    }
    tDoneLog();
    tCloseRedis();
    tCloseDb();
    return (EXIT_SUCCESS);
}

/* 助贷通 交易金额累计 符合条件，返还押金*/
int BackMoney(){
    char sSqlStr[1024] = {0};
    char sUserCode[15+1] = {0};
    char sMerchId[15+1] = {0}, sAgentId[32 + 1] = {0}, sSerialNum[32+1] = {0}, sRefundStat[2+1] = {0};
    char sCycleDate[8+1] = {0}, sCurrentDate[8+1] = {0};
    double dFreezeAmt = 0.00, dFullAmt = 0.00, dTransAmt = 0.00;
    char sAmount[20+1] = {0};/*商户押金*/
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select merch_id,agent_id,serial_num,freeze_amount,full_amount,trans_amt,"
                        "cycle_date,substr(to_char(sysdate,'yyyymmddhh24miss'),1,8),refund_status"
                        " from b_open_merchant where refund_status in ('0','X')");
    
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while ( OCI_FetchNext(pstRes) ) { 
     tLog(DEBUG,"OCI_GetRowCount(pstRes)==[%d]",OCI_GetRowCount(pstRes));   
        MEMSET(sMerchId);
        MEMSET(sAgentId);
        MEMSET(sSerialNum);
        MEMSET(sCycleDate);
        MEMSET(sCurrentDate);
        MEMSET(sAmount);
        MEMSET(sRefundStat);
        dFreezeAmt = 0.00; 
        dFullAmt = 0.00; 
        dTransAmt = 0.00;
        
        STRV(pstRes, 1, sMerchId);
        STRV(pstRes, 2, sAgentId);
        STRV(pstRes, 3, sSerialNum);
        DOUV(pstRes, 4, dFreezeAmt);
        DOUV(pstRes, 5, dFullAmt);
        DOUV(pstRes, 6, dTransAmt);
        STRV(pstRes, 7, sCycleDate);
        STRV(pstRes, 8, sCurrentDate);
        STRV(pstRes, 9, sRefundStat);
        
        tTrim(sMerchId);
        tTrim(sAgentId);
        tTrim(sSerialNum);
        tTrim(sCycleDate);
        tTrim(sCurrentDate);
        
        tLog(DEBUG,"押金金额[%.02f]",dFreezeAmt);
        /*押金金额dFreezeAmt 转为字符串类型*/ 
        sprintf(sAmount,"%.02f",dFreezeAmt);
        tLog(DEBUG,"当前日期sCurrentDate[%s]",sCurrentDate);
        /*若当前日期大于等于规定日期 并且 助贷通商户累计交易额少于规定交易额 则 将押金转给代理商（钱包充值操作）*/
        /*或者 商户押金已经被扣除的 则 必定符合以上将押金转给代理商 的条件 */
        if( (memcmp(sCurrentDate,sCycleDate,8)>= 0 && dTransAmt < dFullAmt) || sRefundStat[0] == 'X' ) {
            tLog(INFO,"超出或达到规定日期[%s],助贷通商户[%s]累计交易额为[%.02f],小于规定交易额[%.02f],押金转给代理商[%s]",
                     sCycleDate,sMerchId,dTransAmt,dFullAmt,sAgentId); 
            tLog(INFO,"或者超出规定日期[%s],助贷通商户[%s]累计交易额为[%.02f],大于等于规定交易额[%.02f],返还状态sRefundStat：X-押金已扣除，押金转给代理商[%s]",
                     sCycleDate,sMerchId,dTransAmt,dFullAmt,sAgentId); 
            /*押金转给代理商，如果商户押金未扣除 则进行扣除商户押金操作 */
            if(sRefundStat[0] != 'X') {
                if( UnFrozenWallet(sMerchId,sSerialNum,sAmount,"6")<0 ) {
                    tLog(ERROR,"扣除商户[%s]押金失败",sMerchId);
                    continue;
                } 
                /*将商户押金转给代理商，扣除商户押金 更新返还标志refund_status 为 X-已扣除商户押金*/
                if( UpdateRefund(sMerchId, "X") < 0 ) {
                    tLog(ERROR,"扣除商户[%s]押金成功,押金返回状态更新失败，一定要即时处理啊啊啊啊啊啊",sMerchId);
                    continue;
                }
            } 
            
            /*钱包充值操作*/
            if( addWalletToAgent(sAgentId,sAmount) <0 ){
                tLog(ERROR,"代理商[%s]入账失败！",sAgentId);
                continue;
            }
            /*入账成功更新押金返还状态refund_status 为: 2-返还代理商*/
            if( UpdateRefund(sMerchId, "2") < 0 ) {
                tLog(ERROR,"代理商[%s]入账成功,押金返回状态更新失败，一定要即时处理啊啊啊啊啊啊",sAgentId);
                continue;
            }
            tLog(INFO,"代理商[%s]入账成功,入账金额[%.2f]",sAgentId,dFreezeAmt);
        }
        /*若当前日期小于等于规定日期 并且 助贷通商户累计交易额大于等于规定交易额 则 将押金返还给助贷通商户（资金解冻操作）*/
        else if( memcmp(sCurrentDate,sCycleDate,8)<= 0 && dTransAmt >= dFullAmt ) {
            tLog(INFO,"规定日期[%s]内,助贷通商户[%s]累计交易额为[%.02f],大于等于规定交易额[%.02f],押金返还",
                    sCycleDate,sMerchId,dTransAmt,dFullAmt);
            /*解冻操作*/
            if( UnFrozenWallet(sMerchId,sSerialNum,sAmount,"5")<0 ) {
                tLog(ERROR,"商户[%s]押金返还失败",sMerchId);
                continue;
            }
            /*解冻成功更新押金返还状态refund_status : 1-返还商户*/
            if( UpdateRefund(sMerchId, "1") < 0 ) {
                tLog(ERROR,"商户[%s]押金解冻成功,押金返回状态更新失败，一定要即时处理啊啊啊啊啊啊",sMerchId);
                continue;
            }
            tLog(INFO,"商户[%s]押金返还成功,押金金额[%.2f]",sMerchId,dFreezeAmt);
        }
        else {
            tLog(INFO,"规定日期[%s]内,助贷通商户[%s]累计交易额为[%.02f],小于规定交易额[%.02f],不作操作",
                    sCycleDate,sMerchId,dTransAmt,dFullAmt);
        }
    }
    
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无未返还的助贷通商户信息");
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    
    tLog(ERROR, "返还满足条件的助贷通商户的押金处理完毕");
    return ( 0 );
}

/*代理商入账*/
int addWalletToAgent(char *pcAgentId, char *pcAmount) {
    cJSON *pstTransJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    
    pstTransJson = cJSON_CreateObject();
    if (NULL == pstTransJson) {
        tLog(ERROR, "创建发送Json失败.");
        return ( -1 );
    }
    SET_STR_KEY(pstTransJson, "agentNo", pcAgentId);
     /*生成rrn,获取系统流水号*/
    if( GetRrn(sRrn,sSysTrace) < 0 ) {
        tLog(ERROR,"生成rrn失败");
        return ( -1 );
    }
    tLog(DEBUG,"生成rrn[%s]，获取系统流水号[%s]",sRrn,sSysTrace);
    SET_STR_KEY(pstTransJson, "sys_trace", sSysTrace);
    SET_STR_KEY(pstTransJson, "rrn", sRrn);
    SET_STR_KEY(pstTransJson, "amount", pcAmount);
    SET_STR_KEY(pstTransJson, "debitType", "1");
    SET_STR_KEY(pstTransJson, "postType", "1");
    SET_STR_KEY(pstTransJson, "remark", "助代通入账");
    SET_STR_KEY(pstTransJson, "trans_code", "00A600");
    
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00A600%s", sRrn);
    stQMsgData.pstDataJson = pstTransJson;
    iRet = tSvcCall("00A600_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            tLog(ERROR, "交易[%s]失败,代理商入账超时.", sRrn);
        } else {
            tLog(INFO, "交易[%s]失败,代理商入账失败.", sRrn);
        }
        return ( -1 );
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
        MEMSET(sRrn);
        GET_STR_KEY(pstRecvDataJson, "rrn", sRrn);
        if (memcmp(sRespCode, "00", 2)) {
            tLog(ERROR, "交易[%s]失败,代理商入账失败[%s:%s].", sRrn, sRespCode, sResvDesc);
            return ( -1 );
        } else {
            tLog(INFO, "交易[%s]成功,代理商入账成功.", sRrn);
        }
    }
    cJSON_Delete(pstTransJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);

    return ( 0 );
}
/*资金解冻or扣除*/
int UnFrozenWallet(char *pcMerchId, char *SerialNum, char *pcAmount, char *pcFrozenStatus) {
    cJSON * pstTransJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    
    pstTransJson = cJSON_CreateObject();
    if (NULL == pstTransJson) {
        tLog(ERROR, "创建发送Json失败.");
        return ( -1 );
    }
    SET_STR_KEY(pstTransJson, "merchantNo", pcMerchId);
    SET_STR_KEY(pstTransJson, "optNo", SerialNum);
    /*生成rrn,获取系统流水号*/
    if( GetRrn(sRrn,sSysTrace) < 0 ) {
        tLog(ERROR,"生成rrn失败");
        return ( -1 );
    }
    tLog(DEBUG,"生成rrn[%s]，获取系统流水号[%s]",sRrn,sSysTrace);
    SET_STR_KEY(pstTransJson, "sys_trace", sSysTrace);
    SET_STR_KEY(pstTransJson, "rrn", sRrn);
    SET_STR_KEY(pstTransJson, "amount", pcAmount);
    SET_STR_KEY(pstTransJson, "walletType", "0");
    SET_STR_KEY(pstTransJson, "frozenBusinessType", "2");
    SET_STR_KEY(pstTransJson, "effectiveMode", "2");
    tLog(DEBUG,"冻结状态 pcFrozenStatus[%s]",pcFrozenStatus);
    SET_STR_KEY(pstTransJson, "frozenStatus", pcFrozenStatus);/*frozenStatus 冻结状态：1-冻结，4-部分解冻，5-解冻，6-扣除*/
    SET_STR_KEY(pstTransJson, "frozenRemark", "助代通押金");
    SET_STR_KEY(pstTransJson, "trans_code", "00TD00");
    
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00TD00%s", sRrn);
    stQMsgData.pstDataJson = pstTransJson;
    iRet = tSvcCall("00TD00_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    tLog(DEBUG,"iRet[%d]",iRet);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            if(pcFrozenStatus[0] = '5') {
                tLog(ERROR, "交易[%s]失败,返还商户押金超时.", sRrn);
            } 
            else {
                tLog(ERROR, "交易[%s]失败,扣除商户押金超时.", sRrn);
            }
        } else {
            if(pcFrozenStatus[0] = '5') {
                tLog(INFO, "交易[%s]失败,返还商户押金失败.", sRrn);
            }
            else {
                tLog(INFO, "交易[%s]失败,扣除商户押金失败.", sRrn);
            }
        }
        return ( -1 );
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
        MEMSET(sRrn);
        GET_STR_KEY(pstRecvDataJson, "rrn", sRrn);
        if (memcmp(sRespCode, "00", 2)) {
            if(pcFrozenStatus[0] == '5') {
                tLog(ERROR, "交易[%s]失败,返还商户押金失败[%s:%s].", sRrn, sRespCode, sResvDesc);
            }
            else {
                tLog(ERROR, "交易[%s]失败,扣除商户押金失败[%s:%s].", sRrn, sRespCode, sResvDesc);
            }
            return ( -1 );
        } else {
            if(pcFrozenStatus[0] == '5') {
                tLog(INFO, "交易[%s]成功,返还商户押金成功.", sRrn);
            }
            else {
                tLog(INFO, "交易[%s]成功,扣除商户押金成功.", sRrn);
            }
        }
    }
    cJSON_Delete(pstTransJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);
 
    return ( 0 );
}

int GetRrn(char *pcRrn,char *pcSysTrace) {
    char sTime[6+1] = {0}, sTrace[6+1] = {0}, sTmp[15+1] = {0};
    /*当前交易时间与系统流水号组成参考号rrn*/
    tGetTime(sTime, "", -1);
    if (GetSysTrace(sTrace) < 0) {
        tLog(ERROR, "获取系统流水号失败,交易放弃.");
        return -1;
    }
    sprintf(sTmp, "%s%s", sTime, sTrace);
    tTrim(sTmp);
    tStrCpy(pcRrn,sTmp,12);
    tStrCpy(pcSysTrace,sTrace,6);
    
    return ( 0 );
}

int UpdateRefund(char *pcMerchId, char *pcStatus) {
    char sSqlStr[1024] = {0};
    OCI_Statement *pstState = NULL;
    OCI_Resultset *pstRes = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"update b_open_merchant set refund_status = '%s',last_mod_time = sysdate"
                        " where merch_id = '%s'",pcStatus,pcMerchId);
    
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (  pstState ==  NULL || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "更新助贷通商户[%s]的押金返还状态[%s]失败,1-返还商户,2-返还代理商,X-商户押金已扣除（此状态在代理商入账失败时会出现）",pcMerchId,pcStatus);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return ( -1 );
    }
    tLog(DEBUG,"更新了[%d]条",tGetAffectedRows());
    tCommit();
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    
    return ( 0 );
}