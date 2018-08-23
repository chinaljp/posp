/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SendMerchMsg.c
 * Author: Administrator
 *
 * Created on 2017年7月12日, 下午5:57
 * Update on 2017年9月4日, 下午5:11
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

int RepFromHX();
int GetMerchMsg();
int Process();

char g_sMerchId[15+1];
char g_sKey[64]; /*暂用于匹配返回的信息*/
/*add by GJQ */
char g_MerchShortName[128]; /*商户简称*/

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

/*
 * 
 */
int main(int argc, char* pcArgv[]) {
    
    /*add by  gjq at 20170906 begin */
    //printf("argc : %d\n",argc);
    if(argc != 4) {
        printf("输入参考： merchmsgenter 日志名称 商户简称 入驻个数\n");
        return (0);
    }
    int i= 0; /*用于限制商户入驻数量*/
    /*add by  gjq at 20170906 end  */
    
    
    int iRet = 0;
    int inum = 0;
    char sSqlStr[1024];
    OCI_Resultset *pstInsRes = NULL;
    MEMSET(sSqlStr);
    
    g_iQutLoop = 1; 
    tDaemon();
    
     /* 屏蔽信号，只保留推出信号 */
    SigProc();
    sigset(SIGUSR1, SigQuit);
    
    if (tInitLog(pcArgv[0]) < 0) {
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
    MEMSET(g_MerchShortName);
    tStrCpy(g_MerchShortName,pcArgv[2],strlen(pcArgv[2]));
    tLog(DEBUG,"商户简称g_MerchShortName[%s]",g_MerchShortName);
    
    snprintf(sSqlStr,sizeof(sSqlStr),"INSERT INTO B_MERCH_SEND_DETAIL (MERCH_ID,LAST_DATETIME) \
                    SELECT a.MERCH_ID,to_char(systimestamp,'yyyymmddhh24miss') \
                    FROM B_LIFE_MERCH a  WHERE STATUS = '1' \
                    AND NOT EXISTS (SELECT 1 FROM B_MERCH_SEND_DETAIL b WHERE a.MERCH_ID = b.MERCH_ID)");

    while(g_iQutLoop) {
        iRet = Process();
        if(iRet < 0) {
            tLog(ERROR, "商户[%s]入驻失败！",g_sMerchId);
            sleep(1);
        }
        else if(iRet == 1) {
            if(i == 0) {
                tLog(INFO, "无商户可入驻！");
                /*无商户可入驻时检查是否有新商户需要入驻 */
                tLog(INFO, "检查是否有新商户需要入驻！");
                if(tExecute(&pstInsRes, sSqlStr) < 0) {
                    tLog(DEBUG, "sql[%s]", sSqlStr);
                    return(-1);/*test add */
                }
                tCommit();
                tReleaseRes(pstInsRes);
                /*检查3次无新商户入驻，进程退出*/
                inum++;
                if(inum == 3) {
                    tLog(INFO, "暂无新商户需要入驻，此次服务到此结束(^_^)！");
                    return ( 0 );
                }
            }
            else {
                tLog(INFO, "[%d]个商户入驻成功！",i);
                i=0;
            }
        }
        else {
            i++;
            /*商户入驻3000个 进程退出 add by gjq  at 20170904*/
            tLog(DEBUG,"已成功入驻商户[%d]个,需要入驻[%s]",i,pcArgv[3]);
            if(atoi(pcArgv[3]) == i){
                g_iQutLoop = 0;
            }  
        }  
    }
    
    tCloseRedis();
    tDoneLog();
    tCloseDb();
    return (EXIT_SUCCESS);
}
 /*商户入驻处理*/
int Process(){
    char sSqlStr[1024];
    int iCount = 0;
    OCI_Resultset *pstRes = NULL;

    MEMSET(sSqlStr);
    
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "select merch_id from b_merch_send_detail where send_status = '0' and rownum = 1 for update nowait");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return ( -1 );
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, g_sMerchId);
    }
    iCount = OCI_GetRowCount(pstRes);
    if (0 == iCount) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        /*商户入驻完毕或暂无商户入驻*/
        return ( 1 );
    }
    tReleaseRes(pstRes);
    
    /*更新商户信息发送状态为2-发送中*/
    MEMSET(sSqlStr);
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "update b_merch_send_detail set send_status = '2', \
                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                where merch_id = '%s'",g_sMerchId);
    
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "更新商户[%s]信息发送状态失败.", g_sMerchId);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    
    /*获取商户信息并发送*/
    if( GetMerchMsg() < 0 ) {
        tLog(ERROR, "商户[%s]信息发送失败.", g_sMerchId);
        return ( -1 );
    }
    
    /*更新商户信息发送状态为1-已发送*/
    MEMSET(sSqlStr);
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "update b_merch_send_detail set send_status = '1', \
                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                where merch_id = '%s'",g_sMerchId);
    
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "更新商户[%s]信息发送状态失败.", g_sMerchId);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    sleep(2);
    /*接收应答并处理*/
    if(RepFromHX() < 0) {
        tLog(ERROR, "此次接收应答失败！");
        /*接收应答失败（接收应答消息失败、接收超时次数超过3次）的商户，进行重新申请入驻*/
        /*更新商户信息发送状态为0-未发送*/
        MEMSET(sSqlStr);
        snprintf(sSqlStr, sizeof (sSqlStr), 
                    "update b_merch_send_detail set send_status = '0', \
                        last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                    where merch_id = '%s'",g_sMerchId);

        if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
            tLog(DEBUG, "sql[%s]", sSqlStr);
            tLog(ERROR, "更新商户[%s]信息发送状态失败.", g_sMerchId);
            return -1;
        }
        tCommit();
        tReleaseRes(pstRes);
        return ( -1 );
    }
    
    return ( 0 );
}

/*获取商户信息并发送*/
int GetMerchMsg(){
    char sSqlStr[1024];
    int iCount = 0;
    OCI_Resultset *pstRes = NULL;
    
    char *pcMsg = NULL;
    cJSON * pstJson = NULL;
    cJSON * pstDataJson = NULL;
    cJSON * pstMerchInfoJson = NULL;
    cJSON * pstContactInfoJson = NULL;
    cJSON * pstBankCardJson = NULL;
    /*JSON * pstAddrInfoJson = NULL;*/
        
    char sTmp[128];
    char sFullName[60+1];         /*商户名称      */
    char sContactName[64+1];      /*联系人名称    */
    char sContactPhone[20+1];     /*联系人手机号  */
    char sContactPersonType[30+1];/*联系人类型    */
    
    char sSysTrace[6 + 1]; /*系统流水号，用于组key*/
    
    MEMSET(sTmp);
    MEMSET(sFullName);
    MEMSET(sContactName);
    MEMSET(sContactPhone);
    MEMSET(sContactPersonType);
    MEMSET(sSysTrace);
    
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "select merch_name, \
                case when handler_name is not null then handler_name else legal_name end, \
                handler_mobile, \
                case when handler_name is not null then 'AGENT' else 'LEGAL_PERSON' end \
                from b_life_merch where merch_id = '%s'",g_sMerchId);
    
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    
    while (OCI_FetchNext(pstRes)) {
        /*商户全称过长只取前60字节*/
        STRV(pstRes, 1, sTmp);
        snprintf(sFullName,sizeof(sFullName),sTmp);
        
        STRV(pstRes, 2, sContactName);
        STRV(pstRes, 3, sContactPhone);
        STRV(pstRes, 4, sContactPersonType);
        
        tTrim(sFullName);
        tTrim(sContactName);
        tTrim(sContactPhone);
        tTrim(sContactPersonType);
        /*tLog(ERROR, "sTmp = [%s],sFullName = [%s],sContactName = [%s],sContactPhone = [%s],sContactPersonType = [%s]",
                sTmp,sFullName,sContactName,sContactPhone,sContactPersonType);*/
    }
    iCount = OCI_GetRowCount(pstRes);
    if (0 == iCount) {
        tLog(ERROR, "未找到商户[%s]信息,iCount = [%d].",g_sMerchId,iCount);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
   
    /*取值 组装JSON*/  
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
    pstMerchInfoJson = cJSON_CreateObject();
    if (NULL == pstMerchInfoJson) {
        tLog(ERROR, "创建Json失败.");
        return -1;
    }
    pstContactInfoJson = cJSON_CreateObject();
    if (NULL == pstContactInfoJson) {
        tLog(ERROR, "创建Json失败.");
        return -1;
    }
    pstBankCardJson = cJSON_CreateObject();
    if (NULL == pstBankCardJson) {
        tLog(ERROR, "创建Json失败.");
        return -1;
    }
    /*
    pstAddrInfoJson = cJSON_CreateObject();
    if (NULL == pstAddrInfoJson) {
        tLog(ERROR, "创建Json失败.");
        return -1;
    }
    */
    if (GetSysTrace(sSysTrace) != 0) {
        tLog(ERROR, "生成系统流水号失败.");
    }
    
    MEMSET(g_sKey);
    strcat(g_sKey, "ENTER_");
    strcat(g_sKey, sSysTrace);
    
    SET_STR_KEY(pstJson, "svrid", "HUAXIA_CHN");
    SET_STR_KEY(pstJson, "key", g_sKey);
    /*********************data******************************/
    SET_STR_KEY(pstDataJson,"transcode","ydzf201");
    SET_STR_KEY(pstDataJson,"agentNo","201707280000266");
    /**********商户信息************/
    SET_STR_KEY(pstMerchInfoJson,"fullName",sFullName);
    //SET_STR_KEY(pstMerchInfoJson,"shortName","钱客通（畅捷）");/* 商户简称（数据库中此字段过长，此项写定值）*/
    //SET_STR_KEY(pstMerchInfoJson,"shortName","畅捷钱客通");
    SET_STR_KEY(pstMerchInfoJson,"shortName",g_MerchShortName);
    
    SET_STR_KEY(pstMerchInfoJson,"servicePhone","00000");/*客服电话 暂时没有，取定值00000*/
    /*联系人信息*/
    SET_STR_KEY(pstContactInfoJson,"contactName",sContactName);
    SET_STR_KEY(pstContactInfoJson,"contactPhone",sContactPhone);
    SET_STR_KEY(pstContactInfoJson,"contactPersonType",sContactPersonType);
    SET_STR_KEY(pstContactInfoJson,"contactIdCard","110106198906253625"); /*身份证号使用开户身份证*/
    /*结算卡信息*/
    SET_STR_KEY(pstBankCardJson,"bankAccountNo","10238000000156276");
    SET_STR_KEY(pstBankCardJson,"bankAccountName","华夏银行北京分行四道口支行");
    SET_STR_KEY(pstBankCardJson,"idCard","110106198906253625");
    SET_STR_KEY(pstBankCardJson,"bankAccountType","2");
    SET_STR_KEY(pstBankCardJson,"bankAccountLineNo","304100011803");
    SET_STR_KEY(pstBankCardJson,"bankAccountAddress","北京市海淀区四道口2号A座京果商厦一层");
     /*联系人地址 -- 非必填项，不传* /
    SET_STR_KEY(pstAddrInfoJson,"province",);
    SET_STR_KEY(pstAddrInfoJson,"city",);
    SET_STR_KEY(pstAddrInfoJson,"district",);
    SET_STR_KEY(pstAddrInfoJson,"address",);
    */
    SET_JSON_KEY(pstMerchInfoJson,"contactInfo",pstContactInfoJson);
    SET_JSON_KEY(pstMerchInfoJson,"bankCardInfo",pstBankCardJson);
    /*SET_JSON_KEY(pstMerchInfoJson,"addressInfo",pstAddrInfoJson);*/
    SET_JSON_KEY(pstDataJson,"merchantInfo",pstMerchInfoJson);
    
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    
    pcMsg = cJSON_PrintUnformatted(pstJson);
    /*发送请求*/
    if (tSendMsg("HUAXIA_CHN", pcMsg, strlen(pcMsg)) < 0) {
        tLog(ERROR, "发送消息到SvrId[HUAXIA_CHN]失败,data[%s].", pcMsg);
        free(pcMsg);
        cJSON_Delete(pstJson);
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
    char sSqlStr[1024];
    char sMsg[MSG_MAX_LEN];
    char sKey_p[64];
    char sChnMerchId[15+1];
    char sAppKey[32 + 1];
    char sRespCode[10 + 1];
    char sRespMsg[100 + 1];
    cJSON *pstJson = NULL;
    cJSON *pstDataJson = NULL;
    
    while(1) {
        iLen = 0;
        MEMSET(sMsg);
        iRet = tRecvMsgEX("YDZF201", sMsg, &iLen, TIMEOUT);
        if(iRet == MSG_ERR) {
                tLog(ERROR, "获取消息失败.");
                sleep(10);
                return ( -1 );
        }
        if(iRet == MSG_TIMEOUT) {
            tLog(DEBUG, "[%s]等待消息超时[%d].", "YDZF201", TIMEOUT);
            i++;
            sleep(3);/*超时，等待3秒继续接收，超时3次退出此次接收，发起下一次请求*/
            if(i == 3){
                tLog(ERROR, "超时次数[%d]次，退出等待！",i);
                /*return ( -1 ); 对于返回超时的商户信息 不再发起 入驻申请*/
            }
        }
        if(iRet == MSG_SUCC) {
                pstJson = cJSON_Parse(sMsg);
                if (NULL == pstJson) {
                    tLog(ERROR, "获取消息失败,放弃处理.");
                    return ( -1 );
                }
                else {
                    MEMSET(sChnMerchId);
                    MEMSET(sAppKey);
                    MEMSET(sRespCode);
                    MEMSET(sRespMsg);
                    MEMSET(sSqlStr);
                    
                    GET_STR_KEY(pstJson,"key",sKey_p);
                    if( !memcmp( g_sKey, sKey_p, strlen(g_sKey) ) ) {
                        pstDataJson = GET_JSON_KEY(pstJson,"data");
                    }
                    else {
                        tLog(INFO,"此次请求信息key[%s]与接收到的key[%s]不一致！",g_sKey,sKey_p);
                        /*key不一致，抛弃此次应答信息，继续接收下一条应答信息，进行匹配*/
                        continue;
                    }
                    
                    GET_STR_KEY(pstDataJson,"merchantNo",sChnMerchId);
                    GET_STR_KEY(pstDataJson,"appkey",sAppKey);
                    GET_STR_KEY(pstDataJson,"respCode",sRespCode);
                    /*GET_STR_KEY(pstDataJson,"respMsg",sRespMsg); 由于返回信息中文乱码咋不更新此字段内容*/
                    
                    cJSON_Delete(pstJson);
                    
                    snprintf(sSqlStr, sizeof (sSqlStr), 
                        "update b_merch_send_detail set chnmerch_id = '%s', \
                            app_key = '%s', resp_code = '%s', resp_msg = '%s', \
                            last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                        where merch_id = '%s'", sChnMerchId,sAppKey,sRespCode,sRespMsg,g_sMerchId);

                    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
                        /*tLog(DEBUG, "sql[%s]", sSqlStr);*/
                        tLog(ERROR, "更新渠道返回商户[%s]信息失败.", sChnMerchId);
                    }
                    tReleaseRes(pstRes);
                    if(!memcmp( sRespCode,"000000",6 )) {
                        MEMSET(sSqlStr);
                        snprintf(sSqlStr, sizeof (sSqlStr), 
                            "update b_merch_send_detail set merchenter_flag = '1', \
                                last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                            where merch_id = '%s'",g_sMerchId);

                        if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
                            tLog(DEBUG, "sql[%s]", sSqlStr);
                            tLog(ERROR, "更新商户[%s]入驻成功标志失败.", g_sMerchId);
                        }
                        tReleaseRes(pstRes); 
                        tLog(INFO, "商户[%s]入驻成功", g_sMerchId);
                    }
                    tCommit();
                }
                break;
        }
    }
    
    return ( 0 );
}