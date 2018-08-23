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
#include "t_redis.h"
#include "t_cjson.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "param.h"
#include "qrpursebook.h"


int DealRespMsg(char *pcRespMsg) {
    
    char sMerchId[15 + 1]         = {0}; //商户号
    char sOrderNo[32 + 1]         = {0}; //商户订单号 
    //char sPayMethod[10+1] = {0};
    char sRespCode[10 + 1]        = {0}; //支付结果
    char sTransCode[6 + 1]        = {0};
    char sSettleKey[38 + 1]       = {0}; //清算主键
    char sBuf[16 + 1]             = {0};
    char sSettleSysTrace[6 + 1]   = {0}; //银联二维码系统跟踪号
    char sSettleTransTime[10 + 1] = {0}; //银联二维码交易传输时间
    char sAmount[12 + 1] = {0}, sCardAttr[2 + 1] = {0}, sCardType[1 + 1] = {0};
    
    char sSqlStr[1024]      = {0};
    char sValidFlag[1 + 1]  = {0};
    char sSettleFlag[1 + 1] = {0};
    OCI_Resultset *pstRes   = NULL;
    OCI_Statement *pstState = NULL;
    
    cJSON *pstJson = NULL;

    pstJson = cJSON_Parse(pcRespMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "获取消息失败,放弃处理.");
        return -1;
    } 
    
    GET_STR_KEY(pstJson,"orderNo",sOrderNo);
    //GET_STR_KEY(pstJson,"payMethod",sPayMethod);
    GET_STR_KEY(pstJson,"respCode",sRespCode);
    GET_STR_KEY(pstJson,"trans_code",sTransCode);
   
    /*获取本笔交易订单的商户号*/
    if( FindMerchId( sMerchId,sOrderNo) < 0 ) {
        tLog(ERROR,"获取本次订单[%s]的商户号失败！",sOrderNo);
        return ( -1 );
    }
    
    tLog(INFO,"商户[%s]的订单号[%s],支付结果[%s]", sMerchId,sOrderNo,sRespCode);
    
    /*add by GJQ at 20171218,  银联二维码异步通知：支付成功更新流水 */
    if( !memcmp(sRespCode,"00",2) && !memcmp(sTransCode,"CUPS00",6) ) {
        GET_STR_KEY(pstJson,"settleKey",sSettleKey);
        GET_STR_KEY(pstJson,"txnAmt",sAmount);
        GET_STR_KEY(pstJson,"cardAttr",sCardAttr);//sCardAttr： 01 - 借记卡， 02 - 贷记卡
        if( sCardAttr[1] == '1' ) {
            strcpy(sCardType,"0");
        } 
        else if( sCardAttr[1] == '2'  ) {
            strcpy(sCardType,"1");
        }
        memcpy(sBuf, sSettleKey+22, sizeof(sBuf)); //截取系统跟踪号 交易传输时间
        memcpy(sSettleSysTrace,sBuf,6);            //截取系统跟踪号
        sSettleSysTrace[7] = '\0';
        memcpy(sSettleTransTime,sBuf+strlen(sSettleSysTrace),10);//截取交易传输时间
        tLog(INFO,"sSettleKey[%s],sSettleSysTrace[%s],sSettleTransTime[%s]",sSettleKey,sSettleSysTrace,sSettleTransTime);   
        
        if( UpCupsSettleKey(sCardType,sSettleSysTrace,sSettleTransTime,sOrderNo) < 0 ) {
            tLog(ERROR, "更新商户[%s]的订单[%s]的card_type,settle_sys_trace、settle_trans_time失败.",sMerchId,sOrderNo);
            return ( -1 );
        }
        /* 计算银联二维码交易的手续费并更新到流水表中 */ 
        tLog(INFO,"#######################################银联二维码异步通知中计算手续费开始！#######################################");
        if( MerchUpFee( sMerchId,sOrderNo,sCardType,sAmount ) < 0 ) {
            tLog(ERROR,"计算商户[%s]的订单[%s]的手续费失败!",sMerchId,sOrderNo);
            return ( -1 );
        }
        tLog(INFO,"#######################################银联二维码异步通知中计算手续费结束！#######################################");
    }
      
    /*查询是否valid_flag = '0' 支付成功状态 若是，则直接返回success*/
    snprintf(sSqlStr,sizeof(sSqlStr),"select valid_flag,settle_flag from b_inline_tarns_detail "
                        "where merch_order_no = '%s' and trans_code not in ('02B300','02W300','02Y300','0AW300','0AB300')",sOrderNo);
    
    tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
       tLog(ERROR, "sql[%s] err!", sSqlStr);
       return ( -1 );
    }

    if (NULL == pstRes) {
       tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
       return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
       STRV(pstRes, 1, sValidFlag);
       STRV(pstRes, 2, sSettleFlag);
       tTrim(sValidFlag);
       tTrim(sSettleFlag);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
       tLog(ERROR, "未找到记录.");
       tReleaseRes(pstRes);
       return ( -1 );
    }
    tReleaseRes(pstRes);
    tLog(DEBUG,"valid_flag[%s]:[%s],settle_flag[%s]:[%s]",sValidFlag,sValidFlag[0] == '0' ? "已支付":"未支付",
            sSettleFlag,sSettleFlag == 'M'?"已入账":"未入账");
    
    /*已支付并且未入账则进行钱包充值， valid_flag ：0 - 已支付状态*/
    if(sValidFlag[0] == '0') {
        /*已支付成功且未入账则充值钱包*/
        if( sSettleFlag[0] == 'N' ) {
            tLog(INFO,"订单[%s]已支付未入账，进行充值",sOrderNo);
            if( AddQrWallet(sOrderNo) ) {
                tLog(ERROR,"订单[%s]充值钱包失败",sOrderNo);
                /*钱包充值失败 继续交易， 后期对账进行补录*/
                //return ( -1 );
            }
        }
        else {
            tLog(INFO,"订单[%s]已支付且已入账",sOrderNo);
            return ( 0 );
        }
    }
    /*sRespCode：00-支付成功, valid_flag ：4 - 未支付状态*/
    else if( sValidFlag[0] == '4' && !memcmp(sRespCode,"00",2) ) {
        /*支付成功并且未入账则充值钱包*/
        if( sSettleFlag[0] == 'N' ) {
            tLog(INFO,"订单[%s]支付成功且未入账，进行充值",sOrderNo);
            if( AddQrWallet(sOrderNo) ) {
                tLog(ERROR,"订单[%s]充值钱包失败",sOrderNo);
                /*钱包充值失败 继续交易， 后期对账进行补录*/
                //return ( -1 );
            }
        }
        else {
            tLog(INFO,"订单[%s]支付成功且已入账",sOrderNo);
        }
        /*更新valid_flag*/
        if( UpValidFlag( sOrderNo ) < 0 ) {
            tLog(ERROR, "更新商户[%s]的订单[%s]的valid_flag标志失败.",sMerchId,sOrderNo);
            return ( -1 );
        }
    } 
    else {
        tLog(INFO,"订单[%s]支付结果[%s],未支付或支付失败",sOrderNo,sRespCode);
        return ( -1 );
    }
    
    return ( 0 );
}

int GoBackMsg(char *pcBackFlag) {
    char *sMsg = NULL;
    char sRepSvrId[32] = {0};
    cJSON *pstJson = NULL;
    
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_REP", GetSvrId());
    tLog(INFO,"SvrId : [%s]",sRepSvrId);
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建发送Json失败.");
        return ( -1 );
    }
    SET_STR_KEY(pstJson,sRepSvrId,pcBackFlag);
    sMsg = cJSON_PrintUnformatted(pstJson);
    
    /*发送请求*/
    if (tSendMsg(sRepSvrId, sMsg, strlen(sMsg)) < 0) {
        tLog(ERROR, "发送消息到SvrId[%s]失败,data[%s].", sRepSvrId,sMsg);
        free(sMsg);
        cJSON_Delete(pstJson);
        return ( -1 );
    }
    free(sMsg);
    cJSON_Delete(pstJson);
    
    return ( 0 );
}

/* 充值钱包 */
int AddQrWallet(char *pcMerchOrderNo) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    
    
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }
    if ( Net2Wallet( pcMerchOrderNo,pstNetJson ) ) {
        tLog(ERROR,"订单[%s]组钱包充值报文出错！",pcMerchOrderNo);
        return ( -1 );
    }
    /*获取pstNetJson 中的rrn 用来组sKey*/
    GET_STR_KEY(pstNetJson, "rrn", sRrn);
    
    SET_STR_KEY(pstNetJson, "walletType", "200"); //钱包类型   
    SET_STR_KEY(pstNetJson, "postType", "1200");//二维码被扫
    SET_STR_KEY(pstNetJson, "trans_code", "00T600");
    
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T600%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        tLog(DEBUG, "二维码交易[%s]钱包入账失败.", sRrn);
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
        if (memcmp(sRespCode, "00", 2)) {
            tLog(ERROR, "钱包入账失败[%s:%s].", sRespCode, sResvDesc);
            /*钱包入账失败 继续交易， 后期对账进行补录*/
        } else {
            tLog(DEBUG, "交易[%s]钱包入账成功.", sRrn);
        }
    }
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);  
    
    return ( 0 );
}

int Net2Wallet(char *pcMerchOrderNo,cJSON *pstNetJson) {
    double dAmount = 0.0, dFee = 0.0;
    char sMerchId[15 + 1] = {0}, sSettleDate[8 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sSysTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0};
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select amount,fee,merch_id,settle_date,rrn,sys_trace,trans_date,trans_time from "
                "b_inline_tarns_detail where merch_order_no = '%s' and trans_code not in ('02Y300','02W300','02B300','0AW300','0AB300')",pcMerchOrderNo);
    
    if (tExecute(&pstRes, sSqlStr) < 0) {
       tLog(ERROR, "sql[%s] err!", sSqlStr);
       return ( -1 );
    }

    if (NULL == pstRes) {
       tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
       return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
        DOUV(pstRes, 1, dAmount);
        DOUV(pstRes, 2, dFee);
        STRV(pstRes, 3, sMerchId);
        STRV(pstRes, 4, sSettleDate);
        STRV(pstRes, 5, sRrn);
        STRV(pstRes, 6, sSysTrace);
        STRV(pstRes, 7, sTransDate);
        STRV(pstRes, 8, sTransTime);
       
       
       tTrim(sMerchId);
       tTrim(sSettleDate);
       tTrim(sRrn);
       tTrim(sSysTrace);
       tTrim(sTransDate);
       tTrim(sTransTime);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
       tLog(ERROR, "未找到记录.");
       tReleaseRes(pstRes);
       return ( -1 );
    }
    tReleaseRes(pstRes);
    
    SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
    SET_STR_KEY(pstNetJson, "trans_type", "INLINE"); //用于区分银行卡还是二维码的交易
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_DOU_KEY(pstNetJson, "transAmount", dAmount);
    SET_DOU_KEY(pstNetJson, "settleAmount", dAmount - dFee);
    SET_STR_KEY(pstNetJson, "sys_trace", sSysTrace);
    SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
    SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
    SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);
    
    return ( 0 );
}
