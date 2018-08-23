/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   merchproduct.c
 * Author: Administrator
 *
 * Created on 2017年7月14日, 下午2:42
 * Update on 2017年9月4日, 下午5:16
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_db.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "t_app_conf.h"
#include "trace.h"
#include "t_daemon.h"

//int RepFromHX();
int ApplyOpenMerchProd();
int Process();
int RepFromHX();
char g_sMerchId[15+1];

int g_iQutLoop;
/*add by  gjq at 20170904 begin */
/*出入参数 支付宝PID 微信PID*/
static char g_sBbPid[16+1];
static char g_sWxPid[8+1];
/*add by  gjq at 20170904 end */

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

/***********************
 * 发起商户开通产品申请  *
 ***********************/
int main(int argc, char* pcArgv[]) {
    /*add by  gjq at 20170904 begin */
    //printf("argc : %d\n",argc);
    if(argc != 5) {
        printf("输入参考： merchproduct 日志名称 支付宝PID 微信PID 开通个数\n");
        return (0);
    }
    int i= 0; /*用于限制商户开通产品数量*/
    /*add by  gjq at 20170904 end  */
    
    int iRet = 0; 
    g_iQutLoop = 1; 
    tDaemon();
    
     /* 屏蔽信号，只保留推出信号 */
    SigProc();
    sigset(SIGUSR1, SigQuit);
    
    if (tInitLog(pcArgv[1]) < 0) {
        fprintf(stderr, "初始化[%s]日志失败.\n", pcArgv[1]);
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
    MEMSET(g_sBbPid);
    MEMSET(g_sWxPid);
    /*add by  gjq at 20170904 begin */
    /*支付宝PID*/
    tStrCpy(g_sBbPid,pcArgv[2],strlen(pcArgv[2]));
    /*微信PID*/
    tStrCpy(g_sWxPid,pcArgv[3],strlen(pcArgv[3]));
    tLog(DEBUG,"支付宝PID：[%s],微信PID: [%s]",g_sBbPid,g_sWxPid);
    /*add by  gjq at 20170904 end */
    
    while(g_iQutLoop) {
        iRet = Process();
        if(iRet < 0) {
            tLog(ERROR, "商户[%s]开通产品申请失败！",g_sMerchId);
            sleep(1);
        }
        else if(iRet == 1){
            tLog(INFO, "无商户需要开通产品！",g_sMerchId);
            sleep(3);
        }
        else{
            /*tLog(INFO, "已发起商户[%s]开通产品申请！",g_sMerchId);*/
            tLog(INFO, "商户[%s]开通产品申请成功！",g_sMerchId);
            i++;
            tLog(DEBUG,"开通产品申请成功[%d]个,需要开通[%s]个",i,pcArgv[4]);
            if( atoi(pcArgv[4]) == i ) {
                g_iQutLoop = 0;
            }
            
        }    
    }
    
    tCloseRedis();
    tDoneLog();
    tCloseDb();
    return (EXIT_SUCCESS);
}
 /*商户开通产品处理*/
int Process(){
    char sRowid[18+1];
    char sSqlStr[1024];
    int iCount = 0;
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    MEMSET(sRowid);
    MEMSET(sSqlStr);
    
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "select rowid,chnmerch_id from b_merch_send_detail \
                where merchenter_flag = '1' and prod_open_flag = 'n' and rownum = 1 for update nowait");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return ( -1 );
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sRowid);
        STRV(pstRes, 2, g_sMerchId);
    }
    iCount = OCI_GetRowCount(pstRes);
    if (0 == iCount) {
        tLog(ERROR, "未找到记录.");
        /*是否rollback 接触锁表*/
        tReleaseRes(pstRes);
        return ( 1 );
    }
    tReleaseRes(pstRes);
    
    /*更新商户开通状态为z-申请开通未返回结果*/
    MEMSET(sSqlStr);
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "update b_merch_send_detail set prod_open_flag = 'z', \
                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                where rowid = '%s'",sRowid);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "更新商户[%s]开通产品申请状态失败.", g_sMerchId);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tCommit();
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    
    /*申请开通产品*/
    if( ApplyOpenMerchProd() < 0 ) {
        tLog(ERROR, "商户[%s]开通产品失败.", g_sMerchId);
        return ( -1 );
    }
    sleep(2);
    /*接收应答并处理*/
    if(RepFromHX() < 0){
        tLog(INFO, "处理应答失败.");
        /*接收应答失败，暂时 不再次申请*/       
        return ( -1 );
    }
    
    return ( 0 );
}

/*申请开通产品*/
int ApplyOpenMerchProd(){
    
    char *pcMsg = NULL;
    cJSON * pstJson = NULL;
    cJSON * pstDataJson = NULL;
    cJSON * pstMerchProdJson = NULL;
    cJSON * pstWxProdJson = NULL;
    cJSON * pstAlipayProdJson = NULL;
    
    char sSysTrace[6 + 1]; /*系统流水号，用于组key*/
    char sKey[64];
    
    MEMSET(sSysTrace);
    MEMSET(sKey);
    
    /*组装JSON*/  
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "创建Json失败.");
        return -1;
    }
    pstMerchProdJson = cJSON_CreateObject();
    if (NULL == pstMerchProdJson) {
        tLog(ERROR, "创建Json失败.");
        return -1;
    }
    pstWxProdJson = cJSON_CreateObject();
    if (NULL == pstWxProdJson) {
        tLog(ERROR, "创建Json失败.");
        return -1;
    }
    pstAlipayProdJson = cJSON_CreateObject();
    if (NULL == pstAlipayProdJson) {
        tLog(ERROR, "创建Json失败.");
        return -1;
    }
    
    if (GetSysTrace(sSysTrace) != 0) {
        tLog(ERROR, "生成系统流水号失败.");
    }
    strcat(sKey, "OPENPROD_");
    strcat(sKey, sSysTrace);
    
    SET_STR_KEY(pstJson, "svrid", "HUAXIA_CHN");
    SET_STR_KEY(pstJson, "key", sKey);
    /*********************data******************************/
    SET_STR_KEY(pstDataJson,"transcode","ydzf202");
    SET_STR_KEY(pstDataJson,"agentNo","201707280000266");
    SET_STR_KEY(pstDataJson,"merchantNo",g_sMerchId);
    /*微信支付*/
    SET_STR_KEY(pstWxProdJson,"onlineRate","0.0038");
    SET_STR_KEY(pstWxProdJson,"tradeCode","50554");
    SET_STR_KEY(pstWxProdJson,"bizCateGory","204");
    
    /*机构在微信的PID  motify by gjq  at 20170904 begin */
    //SET_STR_KEY(pstWxProdJson,"source","33957971"); 
    SET_STR_KEY(pstWxProdJson,"source",g_sWxPid);
    /*motify by gjq  at 20170904 end */
    
    /*支付宝支付*/
    SET_STR_KEY(pstAlipayProdJson,"onlineRate","0.0038");
    SET_STR_KEY(pstAlipayProdJson,"bizCategory","2015050700000000");/*未知？？！！*/
    
    /*机构在支付宝的PID  motify by gjq  at 20170904 begin */
    //SET_STR_KEY(pstAlipayProdJson,"source","2088721946133655");
    SET_STR_KEY(pstAlipayProdJson,"source",g_sBbPid);
    /*motify by gjq  at 20170904 end */
    
    
    /*子分支*/
    SET_JSON_KEY(pstMerchProdJson,"wxProduct",pstWxProdJson);
    SET_JSON_KEY(pstMerchProdJson,"alipayProduct",pstAlipayProdJson);
    SET_JSON_KEY(pstDataJson,"merchantProduct",pstMerchProdJson);
    /*主分支*/
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    
    pcMsg = cJSON_PrintUnformatted(pstJson);
    /*发送请求*/
    if (tSendMsg("HUAXIA_CHN", pcMsg, strlen(pcMsg)) < 0) {
        tLog(ERROR, "发送消息到SvrId[HUAXIA_CHN]失败,data[%s].", pcMsg);
        return ( -1 );
    }
    free(pcMsg);
    cJSON_Delete(pstJson);
    
    
    return ( 0 );
}

/*接受应答信息并处理*/
int RepFromHX() {
    int i = 0;
    int iLen,iRet;
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    char sSqlStr[1024];
    char sMsg[MSG_MAX_LEN];
    char sRespCode[10 + 1];
    char sRespMsg[1024 + 1];
    char sWxRespCode[10 + 1];
    char sWxRespMsg[1024 + 1];
    char sAliRespCode[10 + 1];
    char sAliRespMsg[1024 + 1];
    char sWRespCode[10 + 1] = {0},sBRespCode[10 + 1] = {0};
    
    cJSON *pstJson = NULL;
    cJSON *pstDataJson = NULL;
    
    
    while(1) {
        MEMSET(sMsg);
        iRet = tRecvMsgEX("YDZF202", sMsg, &iLen, TIMEOUT);
        if(iRet == MSG_ERR) {
                tLog(ERROR, "获取消息失败.");
                return ( -1 );//再次发起开通产品申请
        }
        if(iRet == MSG_TIMEOUT) {
            /*超时则一直等待接收*/
            tLog(DEBUG, "[%s]等待消息超时[%d].", "YDZF202", TIMEOUT);
            sleep(3);
            i++;
            if(i == 3) {
                tLog(ERROR, "超时次数[%d]次，退出等待！",i);
                return ( -1 );
            }            
        }
        if(iRet == MSG_SUCC) {
                pstJson = cJSON_Parse(sMsg);
                if (NULL == pstJson) {
                    tLog(ERROR, "获取消息失败,放弃处理.");
                    return ( -1 ); //？？？？是否一直等待？再次发起开通产品申请
                }
                else {
                    MEMSET(sRespCode);
                    MEMSET(sRespMsg);
                    MEMSET(sWxRespCode);
                    MEMSET(sWxRespMsg);
                    MEMSET(sAliRespCode);
                    MEMSET(sAliRespMsg);
                    MEMSET(sSqlStr);
                    
                    pstDataJson = GET_JSON_KEY(pstJson,"data");
                    
                    GET_STR_KEY(pstDataJson,"respCode",sRespCode);
                    GET_STR_KEY(pstDataJson,"respMsg",sRespMsg);
                    GET_STR_KEY(pstDataJson,"wxRespCode",sWxRespCode);
                    GET_STR_KEY(pstDataJson,"wxRespMsg",sWxRespMsg); 
                    GET_STR_KEY(pstDataJson,"aliRespCode",sAliRespCode);
                    GET_STR_KEY(pstDataJson,"aliRespMsg",sAliRespMsg); 
                    
                    cJSON_Delete(pstJson);

                 /* 由于中文乱码，先不对返回信息做更新  
                  * snprintf(sSqlStr, sizeof (sSqlStr), 
                        "update b_merch_send_detail set wxrespcode = '%s', \
                            wxrespmsg = '%s', alirespcode = '%s', alirespmsg = '%s', \
                            last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                        where chnmerch_id = '%s'",sWxRespCode,sWxRespMsg,sAliRespCode,sAliRespMsg,g_sMerchId);*/
                   /* snprintf(sSqlStr, sizeof (sSqlStr), 
                        "update b_merch_send_detail set wxrespcode = '%s', alirespcode = '%s', \
                            last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                        where chnmerch_id = '%s'",sWxRespCode,sAliRespCode,g_sMerchId);*/
                    
                    /*已开通成功的产品（第二次申请开通时，已开通成功的产品，会返错，不更新） 不更新*/
                    /*查询表中渠道商户对应的产品开通返回码 */
                    snprintf(sSqlStr, sizeof (sSqlStr), "select wxrespcode,alirespcode  from b_merch_send_detail \
                        where chnmerch_id = '%s'",g_sMerchId);
                    if (tExecute(&pstRes, sSqlStr) < 0) {
                        tLog(ERROR, "sql[%s] err!", sSqlStr);
                        return ( -1 );
                    }
                    if (NULL == pstRes) {
                        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
                        return ( -1 );
                    }
                    while (OCI_FetchNext(pstRes)) {
                        STRV(pstRes, 1, sWRespCode);
                        STRV(pstRes, 2, sBRespCode);
                        
                        tTrim(sWRespCode);
                        tTrim(sBRespCode);
                    }
                    if (0 == OCI_GetRowCount(pstRes)) {
                        tLog(ERROR, "未找到记录.");
                        tReleaseRes(pstRes);
                        return ( -1 );
                    }
                    tReleaseRes(pstRes);
                    
                    MEMSET(sSqlStr);
                    tLog(DEBUG,"sWRespCode=[%s],sBRespCode=[%s]",sWRespCode,sBRespCode);
                    if(!memcmp(sWRespCode,"000000",6)) {
                        tLog(DEBUG,"库中商户[%s]已开通微信支付");
                        snprintf(sSqlStr, sizeof (sSqlStr), 
                            "update b_merch_send_detail set  alirespcode = '%s', \
                                last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                            where chnmerch_id = '%s'",sAliRespCode,g_sMerchId);
                    } 
                    else if(!memcmp(sBRespCode,"000000",6)) {
                        tLog(DEBUG,"库中商户[%s]已开通支付宝支付");
                        snprintf(sSqlStr, sizeof (sSqlStr), 
                            "update b_merch_send_detail set  wxrespcode = '%s', \
                                last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                            where chnmerch_id = '%s'",sWxRespCode,g_sMerchId);
                    }
                    else {
                        tLog(DEBUG,"库中商户[%s]微信支付，支付宝支付 都未开通");
                        snprintf(sSqlStr, sizeof (sSqlStr), 
                            "update b_merch_send_detail set wxrespcode = '%s', alirespcode = '%s', \
                                last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                            where chnmerch_id = '%s'",sWxRespCode,sAliRespCode,g_sMerchId);
                    }
                    pstState = tExecuteEx(&pstRes, sSqlStr);
                    if (pstState == NULL || tGetAffectedRows() <= 0) {
                        tLog(DEBUG, "sql[%s]", sSqlStr);
                        tLog(ERROR, "更新返回商户[%s]信息失败.", g_sMerchId);
                    }
                    tCommit();
                    tReleaseResEx(pstState);
                    OCI_StatementFree(pstState);
                    if( !memcmp( sRespCode,"000000",6 ) ) {
                        /*更新开通情况有3中，y-微信、支付宝都已开通,b-支付宝开通，w-微信开通 */
                        if( !memcmp(sWxRespCode,"000000",6) && !memcmp(sAliRespCode,"000000",6) ) {
                            /*y-微信、支付宝都已开通*/
                            MEMSET(sSqlStr);
                            snprintf(sSqlStr, sizeof (sSqlStr), 
                                "update b_merch_send_detail set prod_open_flag = 'y', \
                                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                                where chnmerch_id = '%s'",g_sMerchId);
                        }
                        else if ( !memcmp(sWxRespCode,"000000",6) ) {
                            /*w-微信开通*/
                            MEMSET(sSqlStr);
                            snprintf(sSqlStr, sizeof (sSqlStr), 
                                "update b_merch_send_detail set prod_open_flag = 'w', \
                                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                                where chnmerch_id = '%s'",g_sMerchId);
                        }
                        else if ( !memcmp(sAliRespCode,"000000",6) ) {
                            /*b-支付宝开通*/
                            MEMSET(sSqlStr);
                            snprintf(sSqlStr, sizeof (sSqlStr), 
                                "update b_merch_send_detail set prod_open_flag = 'b', \
                                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                                where chnmerch_id = '%s'",g_sMerchId);
                        }
                        else {
                            /*微信 支付宝 皆开通失败*/
                            tLog(INFO, "商户[%s]开通商品失败.", g_sMerchId);
                            return ( -1 );
                        }
                        pstState = tExecuteEx(&pstRes, sSqlStr);
                        if (pstState == NULL || tGetAffectedRows() <= 0) {
                           // tLog(DEBUG, "sql[%s]", sSqlStr);
                            tLog(ERROR, "更新商户[%s]开通商品标志失败,华夏已返回[%s].", g_sMerchId,sRespCode);
                            tReleaseResEx(pstState);
                            OCI_StatementFree(pstState);
                            return ( -1 );
                        }
                        tCommit();  
                        tReleaseResEx(pstState);
                        OCI_StatementFree(pstState);

                        tLog(INFO, "商户[%s]开通商品成功.", g_sMerchId);
                    } 
                    else {
                        tLog(INFO, "商户[%s]开通商品失败.", g_sMerchId);
                        return ( -1 );
                    }
                }
                break;
        }
    }
    return ( 0 );
}