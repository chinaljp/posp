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
#include "t_db.h"
#include "t_tools.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "trace.h"
#include "t_log.h"

int UpdateRefund(char *pcMerchId, char *pcStatus);
int GetRrn(char *pcRrn,char *pcSysTrace);
int UnFrozenWallet(char *pcMerchId, char *SerialNum, char *pcAmount, char *pcFrozenStatus);
int BackMoney();

int merchpospro() {
    char sDate[8 + 1] = {0},sCurrentDate[8 + 1] = {0},sBeforeDate[8 + 1] = {0},sSettleDate[8 + 1] = {0};
    int i = 0;
    /*累计商户信用卡交易金额*/
    if ( PosCdTotal() < 0 ) {
        tLog(ERROR, "助贷通商户信用卡交易金额累计失败！");
        return ( -1 );
    }
    /*累计商户二维码交易金额*/
    tGetDate(sCurrentDate, "", -1);
    tStrCpy(sDate,sCurrentDate,8);
    
    tAddDay(sDate, -1);
    tStrCpy(sSettleDate,sDate,8);
    
    //若当天为非结算日，则不做二维码交易金额累计
    if ( IsSettleDate(sCurrentDate) == 0 ) {       
        /* 获取上一个结算日，节假日没有太长的，所以只检查15天的 */
        for (i = 0; i < 15; i++) {
            if (IsSettleDate(sDate) == 0) {
                tStrCpy(sBeforeDate, sDate, 8);
                break;
            }
            tAddDay(sDate, -1);
        }
        if(InlineTotal(sBeforeDate,sSettleDate) < 0) {
           tLog(ERROR,"累计商户二维码交易额失败!");
           return ( -1 );
        }
    }
    else {
        tLog(ERROR,"当前日期[%s]为非结算日，不累计二维码交易金额！",sCurrentDate);
    }
    
    /*检查助贷通商户在规定周期内的交易额是否满足规定，满足则将商户服务费返回给商户，不满足则将商户服务费扣除*/
    if( BackMoney() < 0 ) {
        tLog(ERROR,"助贷通商户押金返还操作失败，注意查看原因！！！");
    }
    
    return ( 0 );
}

//累计商户信用卡交易金额 
int PosCdTotal() {
    int iCnt = 0;
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    
    tLog(INFO,"=================累计贷记卡交易额=========================");
    snprintf(sSqlStr,sizeof(sSqlStr),"merge into b_term_activity_merch o \
                                using (select h.merch_id as merch_id,sum(h.amount) as amount from b_pos_trans_detail_his h \
                                join b_cups_trans_detail c on c.channel_rrn=h.rrn and c.trans_date=h.trans_date and c.chk_flag='Y' and h.card_type = '1' \
                                join b_term_activity_merch b on h.merch_id = b.merch_id and b.create_time < h.create_time \
                                where c.settle_date=to_char(sysdate-1,'YYYYMMDD') group by h.merch_id ) p \
                                on (p.merch_id=o.merch_id and o.merch_remission_status = '0' and o.status = '1') \
                                when matched then \
                                update set o.trans_total_amt =o.trans_total_amt+p.amount");
    tLog(DEBUG, "sql[%s]", sSqlStr);
    if(tExecute(&pstRes, sSqlStr) < 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "============商户累计信用卡交易额更新失败=============");
        tRollback();
        return ( -1 );
    }
    else {
        iCnt = tGetAffectedRows();
        tLog(INFO, "============商户累计信用卡交易额更新成功,更新商户数[%d]=============",iCnt);
        tCommit();
    }
    tReleaseRes(pstRes);
    return ( 0 );
}

//累计商户二维码交易金额 
int InlineTotal(char *pcBeforeDate, char *pcSettleDate) {
    int iCnt = 0;
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstQrcodeRes = NULL;
    
    tLog(INFO,"=================累计二维码交易额, [%s] - [%s]=========================",pcBeforeDate,pcSettleDate);
    snprintf(sSqlStr,sizeof(sSqlStr),"merge into b_term_activity_merch o \
                        using (select h.merch_id as merch_id,sum(h.amount) as amount from b_inline_tarns_detail_his h \
                                join b_term_activity_merch b on h.merch_id = b.merch_id and b.create_time < h.create_time \
                                where h.trans_date between '%s' and '%s' and h.check_flag = 'Y' and h.trans_code != '0AQ000' group by h.merch_id) p \
                        on (o.merch_id = p.merch_id and o.merch_remission_status = '0' and o.status = '1') \
                        when matched then \
                                update set o.trans_total_amt =o.trans_total_amt+p.amount",pcBeforeDate,pcSettleDate);
    tLog(DEBUG, "sql[%s]", sSqlStr);
    if(tExecute(&pstQrcodeRes, sSqlStr) < 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "============商户累计二维码交易额更新失败=============");
        tRollback();
        return ( -1 );
    }
    else {
        iCnt = tGetAffectedRows();
        tLog(INFO, "============商户累计二维码交易额更新成功,更新商户数[%d]=============",iCnt);
        tCommit();
    }
    tReleaseRes(pstQrcodeRes);
    return ( 0 );
}

//处理助贷通商户服务费
int BackMoney() {
    char sSqlStr[1024] = {0};
    char sMerchId[15+1] = {0}, sSerialNum[32+1] = {0}, sMerchReStat[2+1] = {0};
    char sEndDate[8+1] = {0}, sCurrentDate[8+1] = {0};
    double dServAmt = 0.00, dTotalAmt = 0.00, dTransTotalAmt = 0.00;
    char sAmount[20+1] = {0};/*商户服务费*/
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select merch_id,serial_num,service_charge,total_amt,trans_total_amt,"
                           "end_date,substr(to_char(sysdate,'yyyymmddhh24miss'),1,8),merch_remission_status"
                        " from b_term_activity_merch where merch_remission_status in ('0','a','b') and status = '1'");
    
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
        MEMSET(sSerialNum);
        MEMSET(sEndDate);
        MEMSET(sCurrentDate);
        MEMSET(sAmount);
        MEMSET(sMerchReStat);
        dServAmt = 0.00; 
        dTotalAmt = 0.00; 
        dTransTotalAmt = 0.00;
        
        STRV(pstRes, 1, sMerchId);
        STRV(pstRes, 2, sSerialNum);
        DOUV(pstRes, 3, dServAmt);
        DOUV(pstRes, 4, dTotalAmt);
        DOUV(pstRes, 5, dTransTotalAmt);
        STRV(pstRes, 6, sEndDate);
        STRV(pstRes, 7, sCurrentDate);
        STRV(pstRes, 8, sMerchReStat);
        
        tTrim(sMerchId);
        tTrim(sSerialNum);
        tTrim(sEndDate);
        tTrim(sCurrentDate);
        tTrim(sMerchReStat);
        
        tLog(DEBUG,"服务费[%.02f]",dServAmt);
        /*押金金额dServAmt 转为字符串类型*/ 
        sprintf(sAmount,"%.02f",dServAmt);
        tLog(DEBUG,"当前日期sCurrentDate[%s]",sCurrentDate);
        
        if( (memcmp(sCurrentDate,sEndDate,8) >= 0 && !DBL_CMP(dTransTotalAmt,dTotalAmt) && !DBL_EQ(dTransTotalAmt,dTotalAmt)) || sMerchReStat[0] == 'b' ) {
            /*扣除*/
            if ( sMerchReStat[0] == 'b' ) {
                tLog(INFO,"返还状态: [%s],押金扣除失败，继续扣除！", sMerchReStat); 
            } 
            else {
                tLog(INFO,"达到规定日期[%s],助贷通商户[%s]累计交易额为[%.02f],小于满返金额[%.02f],押金扣除.",
                     sEndDate,sMerchId,dTransTotalAmt,dTotalAmt); 
                /*对于需要处理的数据初始状态（即：返还状态为初始状态：0），每开始处理一条时，将此一条数据更新成 h-处理中，再进行处理操作*/
                UpdateRefund(sMerchId, "h");
            }
            
            /*扣除操作*/
            if( UnFrozenWallet(sMerchId,sSerialNum,sAmount,"6")<0 ) {
                tLog(ERROR,"扣除商户[%s]服务费失败",sMerchId);
                /*更新状态返还状态*/
                if( sMerchReStat[0] != 'b' ) {
                    if( UpdateRefund(sMerchId, "b") < 0 ) {
                        tLog(ERROR,"商户[%s]服务费扣除失败,扣除失败 状态更新失败!!!",sMerchId);
                    }
                }
                continue;
            }
            if( UpdateRefund(sMerchId, "2") < 0 ) {
                tLog(ERROR,"商户[%s]服务费扣除成功,扣除成功 状态更新失败!!!",sMerchId);
                continue;
            }
            tLog(INFO,"商户[%s]服务费扣除成功,扣除金额[%.2f]",sMerchId,dServAmt);
        }
        else if( (memcmp(sCurrentDate,sEndDate,8) <= 0 && DBL_CMP(dTransTotalAmt,dTotalAmt)) || 
                 (memcmp(sCurrentDate,sEndDate,8) <= 0 && DBL_EQ(dTransTotalAmt,dTotalAmt)) || sMerchReStat[0] == 'a' ) {
            /*返还*/
            if( sMerchReStat[0] == 'a' ) {
                tLog(INFO,"返还状态：[%s]，押金返还失败，继续返还！", sMerchReStat);
            }
            else {
                tLog(INFO,"规定日期[%s]内,助贷通商户[%s]累计交易额为[%.02f],大于等于规定交易额[%.02f],押金返还",
                    sEndDate,sMerchId,dTransTotalAmt,dTotalAmt);
                /*对于需要处理的数据初始状态（即：返还状态为初始状态：0），每开始处理一条时，将此一条数据更新成 h-处理中，再进行处理操作*/
                UpdateRefund(sMerchId, "h");
            }
            
            /*解冻操作*/
            if( UnFrozenWallet(sMerchId,sSerialNum,sAmount,"5")<0 ) {
                tLog(ERROR,"返还商户[%s]服务费失败",sMerchId);
                /*更新状态返还状态*/
                if( sMerchReStat[0] != 'a' ) {
                    if( UpdateRefund(sMerchId, "a") < 0 ) {
                        tLog(ERROR,"商户[%s]服务费返还失败,返还失败 状态更新失败!!!",sMerchId);
                    }
                }
                continue;
            }
            if( UpdateRefund(sMerchId, "1") < 0 ) {
                tLog(ERROR,"商户[%s]服务费返还成功,返还成功 状态更新失败!!!",sMerchId);
                continue;
            }
            tLog(INFO,"商户[%s]服务费返还成功,返还金额[%.2f]",sMerchId,dServAmt);
        }
        else {
            /*不做操作*/
            tLog(INFO,"规定日期[%s]内,助贷通商户[%s]累计交易额为[%.02f],小于规定交易额[%.02f],不做操作",
                    sEndDate,sMerchId,dTransTotalAmt,dTotalAmt);
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
    
    tLog(ERROR, "返还满足条件的助贷通商户的商户服务费处理完毕");
    
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
    int iResCnt = 0;
    
    if (pcStatus[0] == '1' || pcStatus[0] == '2') {
        snprintf(sSqlStr,sizeof(sSqlStr),"update b_term_activity_merch set merch_remission_status = '%s',status = 'X',merch_remission_time = sysdate,last_mod_time = sysdate,"
                            " remark = '%s' where merch_id = '%s'",pcStatus,"正常关闭",pcMerchId);
    }
    else {
        snprintf(sSqlStr,sizeof(sSqlStr),"update b_term_activity_merch set merch_remission_status = '%s',last_mod_time = sysdate"
                            " where merch_id = '%s'",pcStatus,pcMerchId);
    }
    
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        tLog(ERROR, "更新助贷通商户[%s]的押金返还状态[%s]失败,1-已返还; a-返还失败; 2-已扣除; b-扣除失败; ",pcMerchId,pcStatus);
        return ( -1 );
    }
    tCommit();
    tLog(INFO, "影响记录数[%d]", iResCnt);

    return ( 0 );
}