/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"
#include "postransdetail.h"
#include "qrcodetransdetail.h"
#include <curl/curl.h>

//extern char* GetNoticeCnt();

int UpdNoticeStat(cJSON *pstJson, char *pcTableName);
int UpdNoticeCnt(cJSON *pstJson, char *pcTableName);
//long lWeight[] = {1, 10, 20, 100, 200, 300};

/*去掉全部的\r,\n,\t,''*/
char *tTrimAll(char *pcStr) {
    char *pcTmp = NULL, *pcTmp1 = NULL;

    for (pcTmp = pcStr; *pcTmp != '\0'; pcTmp++) {
        if (*pcTmp != ' ' && *pcTmp != '\t' &&
                *pcTmp != '\n' && *pcTmp != '\r')
            ;
        else {
            memmove(pcTmp, pcTmp + 1, strlen(pcTmp + 1) + 1);
            pcTmp--;
        }
    }
    return ( pcStr);
}

size_t HttpResponseHandler(char *ptr, size_t size, size_t nmemb, void *userdata) {
    //tTrimAll(ptr);
    memcpy(userdata, ptr, size * nmemb);
    return (size * nmemb);
}

int HttpRequest(char *pcPostUrl, char *pcJsonData, char *pcHttpReponse) {
    CURL *curl;
    CURLcode res;
    //    char sHttpResponse[2048] = {0};
    struct curl_slist *plist = NULL;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, pcPostUrl);
    // 设置http发送的内容类型为JSON  
    plist = curl_slist_append(plist, "Content-Type: application/json;charset=GBK");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
    //打印发送报文所有内容
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    // 设置要POST的JSON数据  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pcJsonData);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpResponseHandler);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pcHttpReponse);
    // Perform the request, res will get the return code   
    res = curl_easy_perform(curl);
    // Check for errors  
    if (res != CURLE_OK) {
        tLog(ERROR, "curl_easy_perform() failed:%s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }
    tLog(ERROR, "HttpReponse[%s]", pcHttpReponse);
    
    // always cleanup  
    curl_easy_cleanup(curl);
    return 0;
}
/*
int JudgeTime(char *pcTransmitTime, int iQueryCount) {
    char sTransTime[8 + 1] = {0};
    time_t tCTime, tCntTime;
    time_t tTransTime;
    struct tm stm;
    int i;
    //获得当前时间
    time(&tCTime);
    strptime(pcTransmitTime, "%Y%m%d%H%M%S", &stm);
    tTransTime = mktime(&stm);
    tCntTime = tCTime - tTransTime;
    tLog(ERROR, "下单时间[%s]", pcTransmitTime);
    tLog(ERROR, "当前查询次数[%d],当前时间戳[%ld],下单时间戳[%ld],间隔时间[%ld]", iQueryCount, tCTime, tTransTime, tCntTime);
    for (i = 0; i < 6; i++) {
        tLog(ERROR, "设定查询间隔时间[%ld]", lWeight[iQueryCount]);
        if (abs(tCntTime - lWeight[iQueryCount]) <= 5)
            return 0;
        else
            return -1;
    }
    return -1;
}
*/
/*通知推送成功，更新通知推送状态*/
int UpdNoticeStat(cJSON *pstJson, char *pcTableName) {
    char sSqlStr[512] = {0};
    char sDate[9] = {0}, sTime[7] = {0};
    char sRrn[12 + 1] = {0};
    int iCnt = 0, iResCnt = 0;
    char sTransTime[14 + 1] = {0}, sTransDate[8 + 1] = {0};


    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "tranTime", sTransTime);
    memcpy(sTransDate,sTransTime,8);
    tLog(DEBUG,"sTransDate[%s]",sTransDate);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE %s SET notice_status = '1',notice_count = 1 WHERE rrn = '%s' and trans_date = '%s'", pcTableName,sRrn,sTransDate);
    tLog(INFO, "sql[%s]", sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tCommit();
    tLog(DEBUG, "影响记录数[%d]", iResCnt);
    return 0;
}
/*通知推送失败，更新通知推送次数*/
int UpdNoticeCnt(cJSON *pstJson, char *pcTableName) {
    char sSqlStr[512] = {0};
    char sRrn[12 + 1] = {0};
    int iCnt = 0, iResCnt = 0;
    char sTransTime[14 + 1] = {0}, sTransDate[8 + 1] = {0};
    
    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "tranTime", sTransTime);
    memcpy(sTransDate,sTransTime,8);
    tLog(DEBUG,"sTransDate[%s]",sTransDate);
    
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE %s SET notice_count = notice_count + 1 WHERE rrn = '%s' and trans_date = '%s'", pcTableName,sRrn,sTransDate);
    tLog(INFO, "sql[%s]", sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tCommit();
    tLog(DEBUG, "影响记录数[%d]", iResCnt);
    return 0;
}

int FindPosOtransMessage(PosTransDetail *pstPosTransDetail,char *pcTableName) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    snprintf(sSqlStr, sizeof (sSqlStr), "select trace_no,batch_no,auth_code from %s where rrn = '%s' and trans_date = '%s'",
        pcTableName,pstPosTransDetail->sORrn,pstPosTransDetail->sOTransDate);
    tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
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
        STRV(pstRes, 1, pstPosTransDetail->sOTraceNo);
        STRV(pstRes, 2, pstPosTransDetail->sOBatchNo);
        STRV(pstRes, 3, pstPosTransDetail->sOAuthCode);
    }
    
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "获取原交易流水[%d]条",OCI_GetRowCount(pstRes));
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);

    return ( 0 );
}

int FindInlineOtransMessage(QrcodeTransDetail *pstQrcodeTransDetail) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    snprintf(sSqlStr, sizeof (sSqlStr), "select trace_no,batch_no from b_inline_tarns_detail where rrn = '%s' and trans_date = '%s'",
        pstQrcodeTransDetail->sORrn,pstQrcodeTransDetail->sTransDate);
    
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstQrcodeTransDetail->sOTraceNo);
        STRV(pstRes, 2, pstQrcodeTransDetail->sOBatchNo);
    }
    
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    return ( 0 );
}

int ScanPosTrans() {
    char sSqlStr[2048];
    char sHttpResponse[2048 + 1] = {0};
    char sAmount[14 + 1] = {0},sTransTime[14+1] = {0};
    double dAMount = 0.00;
    char sUrl[100 + 1] = {0};
    char *pcMsg = NULL;
    PosTransDetail stPosTransDetail;
    char sSettleAmount[14+1] = {0}, sSign[32+1] = {0};
    char sRespCode[32 + 1] = {0}, sRespDesc[256] = {0};
    
    cJSON *pstJson = NULL, *pstRecvJson = NULL;
    OCI_Resultset *pstRes = NULL;
    //tStrCpy(sUrl, pcUrl, strlen(pcUrl));
    /*
    snprintf( sSqlStr, sizeof (sSqlStr), "select \
                        a.trans_code,a.acq_id,a.trans_date,a.trans_time,a.card_no, \
                        a.trace_no,a.sys_trace,a.rrn,a.auth_code,a.batch_no,a.merch_order_no, \
                        a.input_mode,a.card_type,a.merch_id,a.term_id, \
                        b.merch_name,a.amount,a.fee,a.settle_date, \
                        a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param \
                    from b_pos_trans_detail a join b_merch b on a.merch_id = b.merch_id join b_agent_function c on b.agent_id = c.agent_id \
                    where c.notice_flag = '1' and a.merch_id = b.merch_id and a.trans_code in (select trans_code from s_trans_code where settle_flag = '1') \
                    and a.notice_status ='0' and a.notice_count < 5 and resp_code != 'XX'" );
    */
    snprintf( sSqlStr, sizeof (sSqlStr), "select a.trans_code,a.acq_id,a.trans_date,a.trans_time,a.card_no,"
                        " a.trace_no,a.sys_trace,a.rrn,a.auth_code,a.batch_no,a.merch_order_no,"
                        " a.input_mode,a.card_type,a.merch_id,a.term_id,"
                        " b.merch_name,a.amount,a.fee,a.settle_date,"
                        " a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param,a.fee_type"
                    " from b_pos_trans_detail a join b_merch b on a.merch_id = b.merch_id join b_agent_function c on b.agent_id = c.agent_id"
                    " where a.notice_count < :notice_count and c.notice_flag = :notice_flag and a.merch_id = b.merch_id and a.trans_code in (select trans_code from s_trans_code where settle_flag = :settle_flag)"
                    " and a.notice_status = :notice_status and  resp_code != :resp_code" );
    
    if (tExecutePre(&pstRes, sSqlStr, 1, 0, 4, 5,":notice_count","1",":notice_flag","1",":settle_flag","0",":notice_status","XX",":resp_code") < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
	tReleaseRes(pstRes);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        MEMSET_ST(stPosTransDetail);
        
        pstJson = cJSON_CreateObject();
        if (NULL == pstJson) {
            tLog(ERROR, "创建发送Json失败.");
            return 0;
        }

        STRV(pstRes, 1, stPosTransDetail.sTransCode);
        STRV(pstRes, 2, stPosTransDetail.sAcqId);
        STRV(pstRes, 3, stPosTransDetail.sTransDate);
        STRV(pstRes, 4, stPosTransDetail.sTransTime);
        STRV(pstRes, 5, stPosTransDetail.sCardNo);
        STRV(pstRes, 6, stPosTransDetail.sTraceNo);
        STRV(pstRes, 7, stPosTransDetail.sSysTrace);
        STRV(pstRes, 8, stPosTransDetail.sRrn);
        STRV(pstRes, 9, stPosTransDetail.sAuthCode);
        STRV(pstRes, 10, stPosTransDetail.sBatchNo);
        STRV(pstRes, 11, stPosTransDetail.sMerchOrderNo);
        STRV(pstRes, 12, stPosTransDetail.sInputMode);
        STRV(pstRes, 13, stPosTransDetail.sCardType);
        STRV(pstRes, 14, stPosTransDetail.sMerchId);
        STRV(pstRes, 15, stPosTransDetail.sTermId);
        STRV(pstRes, 16, stPosTransDetail.sMerchName);
        DOUV(pstRes, 17, stPosTransDetail.dAmount);
        DOUV(pstRes, 18, stPosTransDetail.dFee);
        STRV(pstRes, 19, stPosTransDetail.sSettleDate);
        STRV(pstRes, 20, stPosTransDetail.sRespCode);
        STRV(pstRes, 21, stPosTransDetail.sRespDesc);
        STRV(pstRes, 22, stPosTransDetail.sOTransDate);
        STRV(pstRes, 23, stPosTransDetail.sORrn);
        STRV(pstRes, 24, sUrl);
        STRV(pstRes, 25, stPosTransDetail.sFeeType);
        
        SET_STR_KEY(pstJson, "tranCode", stPosTransDetail.sTransCode);
        SET_STR_KEY(pstJson, "platCode", stPosTransDetail.sAcqId);
        
        sprintf(sTransTime,"%s%s",stPosTransDetail.sTransDate,stPosTransDetail.sTransTime);
        SET_STR_KEY(pstJson, "tranTime", sTransTime);
        
        SET_STR_KEY(pstJson, "cardNo", stPosTransDetail.sCardNo);
        SET_STR_KEY(pstJson, "traceNo", stPosTransDetail.sTraceNo);
        SET_STR_KEY(pstJson, "sysTraceNo", stPosTransDetail.sSysTrace);
        SET_STR_KEY(pstJson, "rrn", stPosTransDetail.sRrn);
        SET_STR_KEY(pstJson, "authCode", stPosTransDetail.sAuthCode);
        SET_STR_KEY(pstJson, "batchNo", stPosTransDetail.sBatchNo);
        
        if(stPosTransDetail.sTransCode[0] == 'M') {
            SET_STR_KEY(pstJson, "orderId", stPosTransDetail.sMerchOrderNo);
        }
        
        SET_STR_KEY(pstJson, "inputMode", stPosTransDetail.sInputMode);
        SET_STR_KEY(pstJson, "cardType", stPosTransDetail.sCardType);
        SET_STR_KEY(pstJson, "merchantId", stPosTransDetail.sMerchId);
        SET_STR_KEY(pstJson, "termId", stPosTransDetail.sTermId);
        SET_STR_KEY(pstJson, "merchantName", stPosTransDetail.sMerchName);
        
        dAMount  = stPosTransDetail.dAmount * 100;
        sprintf(sAmount, "%.f", dAMount);
        SET_STR_KEY(pstJson, "amount", sAmount);
        sprintf(sSettleAmount, "%.f", dAMount - (stPosTransDetail.dFee * 100));
        SET_STR_KEY(pstJson, "settleAmount", sSettleAmount);
        SET_STR_KEY(pstJson, "feeType", stPosTransDetail.sFeeType);
        
        SET_STR_KEY(pstJson, "settleDate", stPosTransDetail.sSettleDate);
        tTrim(stPosTransDetail.sRespCode);
        SET_STR_KEY(pstJson, "sysRespCode", stPosTransDetail.sRespCode);
        tTrimAll(stPosTransDetail.sRespDesc);
        SET_STR_KEY(pstJson, "sysRespDesc", stPosTransDetail.sRespDesc);
        /*撤销 或 退货 交易 组originalTranDate、originalRrn ;
          originaltraceNo、originalbatchNo、originalAuthCode这三个字段需要从原交易流水中获取;
        */
        if( ( stPosTransDetail.sTransCode[5] == '2' && memcmp(stPosTransDetail.sRespCode,"25",2) ) || stPosTransDetail.sTransCode[5] == '1') {
            if( FindPosOtransMessage(&stPosTransDetail,"b_pos_trans_detail") < 0 ){
                if( FindPosOtransMessage(&stPosTransDetail,"b_pos_trans_detail_his") < 0) {
                    tLog(ERROR,"从原交易流水中获取信息失败！");
                    continue;
                } 
            }
            
            SET_STR_KEY(pstJson, "originalTranDate", stPosTransDetail.sOTransDate);
            SET_STR_KEY(pstJson, "originalRrn", stPosTransDetail.sORrn);
            SET_STR_KEY(pstJson, "originaltraceNo", stPosTransDetail.sOTraceNo);
            SET_STR_KEY(pstJson, "originalbatchNo", stPosTransDetail.sOBatchNo);
            SET_STR_KEY(pstJson, "originalAuthCode", stPosTransDetail.sOAuthCode);
        }
            
        tLog(INFO, "检索到pos交易流水信息.");
        
        if (strlen(sUrl) == 0 || !strstr(sUrl, "http")) {
            tLog(ERROR, "交易通知Url为空 或 不合规的URL请求地址，无法发送请求 ");
        } else {
            /*发送前进行 签名计算，并组入json*/
            if( getSign(pstJson,sSign) < 0 ) {
                tLog(ERROR,"计算签名失败,rrn[%s]",stPosTransDetail.sRrn);
                continue;
            }
            SET_STR_KEY(pstJson, "sign", sSign);
            pcMsg = cJSON_PrintUnformatted(pstJson);
            
            tLog(INFO, "向[%s]发送http请求,json参数为[%s]", sUrl, pcMsg);
            if ( HttpRequest(sUrl, pcMsg, sHttpResponse) < 0 ) {
                tLog(ERROR, "http请求失败");
                /*更新交易通知推送次数*/
                if (UpdNoticeCnt(pstJson,"b_pos_trans_detail")< 0) {
                    tLog(ERROR, "更新POS交易通知推送次数失败！");
                }
            } else {
                 if ( strstr(sHttpResponse, "null") != NULL || strstr(sHttpResponse, "html") != NULL ) {
                    tLog(ERROR, "http返回数据里含null 或 html");
                } else {
                    pstRecvJson = cJSON_Parse(sHttpResponse);
                    //此处返回内容待处理
                    GET_STR_KEY(pstRecvJson, "responseCode", sRespCode);
                    GET_STR_KEY(pstRecvJson, "responseDesc", sRespDesc);
                    if (memcmp(sRespCode, "00", 2) == 0) {
                        /*更新交易通知推送状态*/
                        if (UpdNoticeStat(pstRecvJson,"b_pos_trans_detail") < 0) {
                            tLog(ERROR, "更新pos交易通知推送状态失败！");
                        } else {
                            tLog(ERROR, "pos交易通知[%s]推送成功！", stPosTransDetail.sRrn);
                        }
                    } else {
                        tLog(ERROR, "pos交易通知[%s]推送失败,返回码[%s:%s]", stPosTransDetail.sRrn, sRespCode,sRespDesc);
                        SET_STR_KEY(pstRecvJson,"rrn",stPosTransDetail.sRrn);
                        /*更新交易通知推送次数*/
                        if (UpdNoticeCnt(pstRecvJson,"b_pos_trans_detail")< 0) {
                            tLog(ERROR, "更新POS交易通知推送次数失败！");
                        }
                    }
                    cJSON_Delete(pstRecvJson);
                }
            }
        }

        cJSON_Delete(pstJson);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未检索到待推送的交易流水.....");
        tReleaseRes(pstRes);
        return -2;
    }
    tReleaseRes(pstRes);
    return 0;
}


int ScanInlineTrans() {
    char sSqlStr[2048];
    char sHttpResponse[2048 + 1] = {0};
    double dAMount = 0.00;
    char sAmount[14 + 1] = {0},sTransTime[14+1] = {0};
    char sUrl[100 + 1] = {0};
    char *pcMsg = NULL;
    QrcodeTransDetail stQrcodeTransDetail;
    char sSettleAmount[14+1] = {0}, sSign[32+1] = {0};
    char sRespCode[32 + 1] = {0}, sRespDesc[256] = {0};
    
    cJSON *pstJson = NULL, *pstRecvJson = NULL;
    OCI_Resultset *pstRes = NULL;
    
    //tStrCpy(sUrl, pcUrl, strlen(pcUrl));
    
    snprintf(sSqlStr, sizeof (sSqlStr), "select \
                        a.trans_code,a.acq_id,a.trans_date,a.trans_time,a.card_no, \
                              a.trace_no,a.sys_trace,a.rrn,a.batch_no,a.merch_order_no, \
                              a.card_type,a.merch_id,a.term_id, \
                              b.merch_name,a.amount,a.fee,a.settle_date, \
                              a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param,a.fee_type \
                    from b_inline_tarns_detail a join b_merch b on a.merch_id = b.merch_id join b_agent_function c on b.agent_id = c.agent_id \
                    where c.notice_flag = '1' and a.merch_id = b.merch_id and a.trans_code in (select trans_code from s_trans_code where settle_flag = '1') \
                    and a.notice_status ='0' and a.notice_count < 5 and a.valid_flag in('0','1') and resp_code != 'XX'" );
    /*注意 二维码 支付成功标准判别  valid_flag = '0' 或者以settle_flag = 'M' 使用哪个判定支付结果好些？  and a.valid_flag in('0','1')*/
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        MEMSET_ST(stQrcodeTransDetail);
        
        pstJson = cJSON_CreateObject();
        if (NULL == pstJson) {
            return 0; 
        }

        STRV(pstRes, 1, stQrcodeTransDetail.sTransCode);
        STRV(pstRes, 2, stQrcodeTransDetail.sAcqId);
        STRV(pstRes, 3, stQrcodeTransDetail.sTransDate);
        STRV(pstRes, 4, stQrcodeTransDetail.sTransTime);
        STRV(pstRes, 5, stQrcodeTransDetail.sCardNo);
        STRV(pstRes, 6, stQrcodeTransDetail.sTraceNo);
        STRV(pstRes, 7, stQrcodeTransDetail.sSysTrace);
        STRV(pstRes, 8, stQrcodeTransDetail.sRrn);
        STRV(pstRes, 9, stQrcodeTransDetail.sBatchNo);
        STRV(pstRes, 10, stQrcodeTransDetail.sMerchOrderNo);
        STRV(pstRes, 11, stQrcodeTransDetail.sCardType);
        STRV(pstRes, 12, stQrcodeTransDetail.sMerchId);
        STRV(pstRes, 13, stQrcodeTransDetail.sTermId);
        STRV(pstRes, 14, stQrcodeTransDetail.sMerchName);
        DOUV(pstRes, 15, stQrcodeTransDetail.dAmount);
        DOUV(pstRes, 16, stQrcodeTransDetail.dFee);
        STRV(pstRes, 17, stQrcodeTransDetail.sSettleDate);
        STRV(pstRes, 18, stQrcodeTransDetail.sRespCode);
        STRV(pstRes, 19, stQrcodeTransDetail.sRespDesc);
        STRV(pstRes, 20, stQrcodeTransDetail.sOTransDate);
        STRV(pstRes, 21, stQrcodeTransDetail.sORrn);
        STRV(pstRes, 22, sUrl);
        STRV(pstRes, 23, stQrcodeTransDetail.sFeeType);
 
        SET_STR_KEY(pstJson, "tranCode", stQrcodeTransDetail.sTransCode);      
        SET_STR_KEY(pstJson, "platCode", stQrcodeTransDetail.sAcqId);
         
        sprintf(sTransTime,"%s%s",stQrcodeTransDetail.sTransDate,stQrcodeTransDetail.sTransTime);
        SET_STR_KEY(pstJson, "tranTime", sTransTime);       
        
        SET_STR_KEY(pstJson, "cardNo", stQrcodeTransDetail.sCardNo);
        SET_STR_KEY(pstJson, "traceNo", stQrcodeTransDetail.sTraceNo);
        SET_STR_KEY(pstJson, "sysTraceNo", stQrcodeTransDetail.sSysTrace);
        SET_STR_KEY(pstJson, "rrn", stQrcodeTransDetail.sRrn);
        SET_STR_KEY(pstJson, "batchNo", stQrcodeTransDetail.sBatchNo);

        SET_STR_KEY(pstJson, "orderId", stQrcodeTransDetail.sMerchOrderNo);
        SET_STR_KEY(pstJson, "cardType", stQrcodeTransDetail.sCardType);
        SET_STR_KEY(pstJson, "merchantId", stQrcodeTransDetail.sMerchId);
        SET_STR_KEY(pstJson, "termId", stQrcodeTransDetail.sTermId);
        SET_STR_KEY(pstJson, "merchantName", stQrcodeTransDetail.sMerchName);
        
        dAMount  = stQrcodeTransDetail.dAmount * 100;
        sprintf(sAmount, "%.f", dAMount);
        SET_STR_KEY(pstJson, "amount", sAmount);
        sprintf(sSettleAmount, "%.f", dAMount - (stQrcodeTransDetail.dFee * 100));
        SET_STR_KEY(pstJson, "settleAmount", sSettleAmount);
        SET_STR_KEY(pstJson, "feeType", stQrcodeTransDetail.sFeeType);
        
        SET_STR_KEY(pstJson, "settleDate", stQrcodeTransDetail.sSettleDate);
        SET_STR_KEY(pstJson, "sysRespCode", stQrcodeTransDetail.sRespCode);
        tTrimAll(stQrcodeTransDetail.sRespDesc);
        SET_STR_KEY(pstJson, "sysRespDesc", stQrcodeTransDetail.sRespDesc);
        
        
        /*二维码撤销交易 原交易日期 与当前交易日期一直*/
        if( stQrcodeTransDetail.sTransCode[3] == '6' ) {
            
            if( FindInlineOtransMessage(&stQrcodeTransDetail) <0 ){
                tLog(ERROR,"从原交易流水中获取信息失败！");
                continue;
            }
            
            SET_STR_KEY(pstJson, "originalTranDate", stQrcodeTransDetail.sTransDate);
            SET_STR_KEY(pstJson, "originalRrn", stQrcodeTransDetail.sORrn);
            SET_STR_KEY(pstJson, "originaltraceNo", stQrcodeTransDetail.sOTraceNo);
            SET_STR_KEY(pstJson, "originalbatchNo", stQrcodeTransDetail.sOBatchNo); 
        }
        
        if (strlen(sUrl) == 0 || !strstr(sUrl, "http")) {
            tLog(ERROR, "交易通知Url为空 或 不合规的URL请求地址，无法发送请求 ");
        } else {
            /*发送前进行 签名计算，并组入json*/
            if( getSign(pstJson,sSign) < 0 ) {
                tLog(ERROR,"计算签名失败,rrn[%s]",stQrcodeTransDetail.sRrn);
                continue;
            }
            SET_STR_KEY(pstJson, "sign", sSign);
            pcMsg = cJSON_PrintUnformatted(pstJson);
            
            tLog(INFO, "向[%s]发送http请求,json参数为[%s]", sUrl, pcMsg);
            if ( HttpRequest(sUrl, pcMsg, sHttpResponse) < 0 ) {
                tLog(ERROR, "http请求失败");
                /*更新交易通知推送次数*/
                if (UpdNoticeCnt(pstJson,"b_inline_tarns_detail")< 0) {
                    tLog(ERROR, "更新二维码交易通知推送次数失败！");
                }
            } else {
                 if ( strstr(sHttpResponse, "null") != NULL || strstr(sHttpResponse, "HTML") != NULL ) {
                    tLog(ERROR, "http返回数据里含null 或 html");
                } else {
                    pstRecvJson = cJSON_Parse(sHttpResponse);
                    //此处返回内容待处理
                    GET_STR_KEY(pstRecvJson, "responseCode", sRespCode);
                    GET_STR_KEY(pstRecvJson, "responseDesc", sRespDesc);
                    if (memcmp(sRespCode, "00", 2) == 0) {
                        /*更新交易通知推送状态*/
                        if (UpdNoticeStat(pstRecvJson,"b_inline_tarns_detail")< 0) {
                            tLog(ERROR, "更新二维码交易通知推送状态失败！");
                        } 
                        else {
                            tLog(ERROR, "二维码交易通知[%s]推送成功！", stQrcodeTransDetail.sRrn);
                        }
                    } else {
                        tLog(ERROR, "pos交易通知[%s]推送失败,返回码[%s:%s]", stQrcodeTransDetail.sRrn, sRespCode,sRespDesc);
                        /*更新交易通知推送次数*/
                        if (UpdNoticeCnt(pstRecvJson,"b_inline_tarns_detail")< 0) {
                            tLog(ERROR, "更新二维码交易通知推送次数失败！");
                        } 
                    }
                    cJSON_Delete(pstRecvJson);
                }
            }
        }

        cJSON_Delete(pstJson);
        
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未检索到待推送的交易流水.....");
        tReleaseRes(pstRes);
        return -2;
    }
    tReleaseRes(pstRes);
    return 0;
}

/*推送 二维码的失败交易 通知， 30分钟之前的待支付状态的交易 默认交易失败  */
int ScanFailInlineTrans() {
    char sSqlStr[2048];
    char sHttpResponse[2048 + 1] = {0};
    double dAMount = 0.00;
    char sAmount[14 + 1] = {0},sTransTime[14+1] = {0};
    char sUrl[100 + 1] = {0};
    char *pcMsg = NULL;
    QrcodeTransDetail stQrcodeTransDetail;
    char sSettleAmount[14+1] = {0}, sSign[32+1] = {0};
    char sRespCode[32 + 1] = {0}, sRespDesc[256] = {0};
    
    cJSON *pstJson = NULL, *pstRecvJson = NULL;
    OCI_Resultset *pstRes = NULL;
    
    //tStrCpy(sUrl, pcUrl, strlen(pcUrl));
    
    snprintf(sSqlStr, sizeof (sSqlStr), "select \
                        a.trans_code,a.acq_id,a.trans_date,a.trans_time,a.card_no, \
                              a.trace_no,a.sys_trace,a.rrn,a.batch_no,a.merch_order_no, \
                              a.card_type,a.merch_id,a.term_id, \
                              b.merch_name,a.amount,a.fee,a.settle_date, \
                              a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param,a.fee_type \
                    from b_inline_tarns_detail a join b_merch b on a.merch_id = b.merch_id join b_agent_function c on b.agent_id = c.agent_id \
                    where c.notice_flag = '1' and a.merch_id = b.merch_id and a.trans_code in (select trans_code from s_trans_code where settle_flag = '1') \
                    and a.notice_status ='0' and a.notice_count < 5 and a.valid_flag = '4' and resp_code != 'XX' and sysdate-a.create_time>=30/60/24" );
    
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        MEMSET_ST(stQrcodeTransDetail);
        
        pstJson = cJSON_CreateObject();
        if (NULL == pstJson) {
            return 0; 
        }

        STRV(pstRes, 1, stQrcodeTransDetail.sTransCode);
        STRV(pstRes, 2, stQrcodeTransDetail.sAcqId);
        STRV(pstRes, 3, stQrcodeTransDetail.sTransDate);
        STRV(pstRes, 4, stQrcodeTransDetail.sTransTime);
        STRV(pstRes, 5, stQrcodeTransDetail.sCardNo);
        STRV(pstRes, 6, stQrcodeTransDetail.sTraceNo);
        STRV(pstRes, 7, stQrcodeTransDetail.sSysTrace);
        STRV(pstRes, 8, stQrcodeTransDetail.sRrn);
        STRV(pstRes, 9, stQrcodeTransDetail.sBatchNo);
        STRV(pstRes, 10, stQrcodeTransDetail.sMerchOrderNo);
        STRV(pstRes, 11, stQrcodeTransDetail.sCardType);
        STRV(pstRes, 12, stQrcodeTransDetail.sMerchId);
        STRV(pstRes, 13, stQrcodeTransDetail.sTermId);
        STRV(pstRes, 14, stQrcodeTransDetail.sMerchName);
        DOUV(pstRes, 15, stQrcodeTransDetail.dAmount);
        DOUV(pstRes, 16, stQrcodeTransDetail.dFee);
        STRV(pstRes, 17, stQrcodeTransDetail.sSettleDate);
        STRV(pstRes, 18, stQrcodeTransDetail.sRespCode);
        STRV(pstRes, 19, stQrcodeTransDetail.sRespDesc);
        STRV(pstRes, 20, stQrcodeTransDetail.sOTransDate);
        STRV(pstRes, 21, stQrcodeTransDetail.sORrn);
        STRV(pstRes, 22, sUrl);
        STRV(pstRes, 23, stQrcodeTransDetail.sFeeType);
 
        SET_STR_KEY(pstJson, "tranCode", stQrcodeTransDetail.sTransCode);      
        SET_STR_KEY(pstJson, "platCode", stQrcodeTransDetail.sAcqId);
         
        sprintf(sTransTime,"%s%s",stQrcodeTransDetail.sTransDate,stQrcodeTransDetail.sTransTime);
        SET_STR_KEY(pstJson, "tranTime", sTransTime);       
        
        SET_STR_KEY(pstJson, "cardNo", stQrcodeTransDetail.sCardNo);
        SET_STR_KEY(pstJson, "traceNo", stQrcodeTransDetail.sTraceNo);
        SET_STR_KEY(pstJson, "sysTraceNo", stQrcodeTransDetail.sSysTrace);
        SET_STR_KEY(pstJson, "rrn", stQrcodeTransDetail.sRrn);
        SET_STR_KEY(pstJson, "batchNo", stQrcodeTransDetail.sBatchNo);

        SET_STR_KEY(pstJson, "orderId", stQrcodeTransDetail.sMerchOrderNo);
        SET_STR_KEY(pstJson, "cardType", stQrcodeTransDetail.sCardType);
        SET_STR_KEY(pstJson, "merchantId", stQrcodeTransDetail.sMerchId);
        SET_STR_KEY(pstJson, "termId", stQrcodeTransDetail.sTermId);
        SET_STR_KEY(pstJson, "merchantName", stQrcodeTransDetail.sMerchName);
        
        dAMount  = stQrcodeTransDetail.dAmount * 100;
        sprintf(sAmount, "%.f", dAMount);
        SET_STR_KEY(pstJson, "amount", sAmount);
        sprintf(sSettleAmount, "%.f", dAMount - (stQrcodeTransDetail.dFee * 100));
        SET_STR_KEY(pstJson, "settleAmount", sSettleAmount);
        SET_STR_KEY(pstJson, "feeType", stQrcodeTransDetail.sFeeType);
        
        SET_STR_KEY(pstJson, "settleDate", stQrcodeTransDetail.sSettleDate);
        if ( !memcmp(stQrcodeTransDetail.sRespCode,"00",2) ) {
            SET_STR_KEY(pstJson, "sysRespCode", "R3");
            SET_STR_KEY(pstJson, "sysRespDesc", "未支付");
        }
        else {
            SET_STR_KEY(pstJson, "sysRespCode", stQrcodeTransDetail.sRespCode);
            tTrimAll(stQrcodeTransDetail.sRespDesc);
            SET_STR_KEY(pstJson, "sysRespDesc", stQrcodeTransDetail.sRespDesc);
        }
        
        
        /*二维码撤销交易 原交易日期 与当前交易日期一直*/
        if( stQrcodeTransDetail.sTransCode[3] == '6' && memcmp(stQrcodeTransDetail.sRespCode,"25",2) ) {
            
            if( FindInlineOtransMessage(&stQrcodeTransDetail) <0 ){
                tLog(ERROR,"从原交易流水中获取信息失败！");
                continue;
            }
            
            SET_STR_KEY(pstJson, "originalTranDate", stQrcodeTransDetail.sTransDate);
            SET_STR_KEY(pstJson, "originalRrn", stQrcodeTransDetail.sORrn);
            SET_STR_KEY(pstJson, "originaltraceNo", stQrcodeTransDetail.sOTraceNo);
            SET_STR_KEY(pstJson, "originalbatchNo", stQrcodeTransDetail.sOBatchNo); 
        }
        
        if (strlen(sUrl) == 0 || !strstr(sUrl, "http")) {
            tLog(ERROR, "交易通知Url为空 或 不合规的URL请求地址，无法发送请求 ");
        } else {
            /*发送前进行 签名计算，并组入json*/
            if( getSign(pstJson,sSign) < 0 ) {
                tLog(ERROR,"计算签名失败,rrn[%s]",stQrcodeTransDetail.sRrn);
                continue;
            }
            SET_STR_KEY(pstJson, "sign", sSign);
            pcMsg = cJSON_PrintUnformatted(pstJson);
            
            tLog(INFO, "向[%s]发送http请求,json参数为[%s]", sUrl, pcMsg);
            if ( HttpRequest(sUrl, pcMsg, sHttpResponse) < 0 ) {
                tLog(ERROR, "http请求失败");
                /*更新交易通知推送次数*/
                if (UpdNoticeCnt(pstJson,"b_inline_tarns_detail")< 0) {
                    tLog(ERROR, "更新二维码交易通知推送次数失败！");
                }
            } else {
                 if ( strstr(sHttpResponse, "null") != NULL || strstr(sHttpResponse, "HTML") != NULL ) {
                    tLog(ERROR, "http返回数据里含null 或 html");
                } else {
                    pstRecvJson = cJSON_Parse(sHttpResponse);
                    //此处返回内容待处理
                    GET_STR_KEY(pstRecvJson, "responseCode", sRespCode);
                    GET_STR_KEY(pstRecvJson, "responseDesc", sRespDesc);
                    if (memcmp(sRespCode, "00", 2) == 0) {
                        /*更新交易通知推送状态*/
                        if (UpdNoticeStat(pstRecvJson,"b_inline_tarns_detail")< 0) {
                            tLog(ERROR, "更新二维码交易通知推送状态失败！");
                        } 
                        else {
                            tLog(ERROR, "二维码交易通知[%s]推送成功！", stQrcodeTransDetail.sRrn);
                        }
                    } else {
                        tLog(ERROR, "pos交易通知[%s]推送失败,返回码[%s:%s]", stQrcodeTransDetail.sRrn, sRespCode,sRespDesc);
                        /*更新交易通知推送次数*/
                        if (UpdNoticeCnt(pstRecvJson,"b_inline_tarns_detail")< 0) {
                            tLog(ERROR, "更新二维码交易通知推送次数失败！");
                        } 
                    }
                    cJSON_Delete(pstRecvJson);
                }
            }
        }

        cJSON_Delete(pstJson);
        
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未检索到待推送的交易流水.....");
        tReleaseRes(pstRes);
        return -2;
    }
    tReleaseRes(pstRes);
    return 0;
}

int ScanDayFailInlineTrans() {
    char sSqlStr[2048];
    char sHttpResponse[2048 + 1] = {0};
    double dAMount = 0.00;
    char sAmount[14 + 1] = {0},sTransTime[14+1] = {0};
    char sUrl[100 + 1] = {0};
    char *pcMsg = NULL;
    QrcodeTransDetail stQrcodeTransDetail;
    char sSettleAmount[14+1] = {0}, sSign[32+1] = {0};
    char sRespCode[32 + 1] = {0}, sRespDesc[256] = {0};
    
    cJSON *pstJson = NULL, *pstRecvJson = NULL;
    OCI_Resultset *pstRes = NULL;
    
    //tStrCpy(sUrl, pcUrl, strlen(pcUrl));
    
    snprintf(sSqlStr, sizeof (sSqlStr), "select \
                        a.trans_code,a.acq_id,a.trans_date,a.trans_time,a.card_no, \
                              a.trace_no,a.sys_trace,a.rrn,a.batch_no,a.merch_order_no, \
                              a.card_type,a.merch_id,a.term_id, \
                              b.merch_name,a.amount,a.fee,a.settle_date, \
                              a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param,a.fee_type \
                    from b_inline_tarns_detail_his a join b_merch b on a.merch_id = b.merch_id join b_agent_function c on b.agent_id = c.agent_id \
                    where c.notice_flag = '1' and a.merch_id = b.merch_id and a.trans_code in (select trans_code from s_trans_code where settle_flag = '1') \
                    and a.notice_status ='0' and a.notice_count < 5 and a.valid_flag = '4' and resp_code != 'XX' and a.trans_date = TO_CHAR( SYSDATE-1,'YYYYMMDD')" );
    
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        MEMSET_ST(stQrcodeTransDetail);
        
        pstJson = cJSON_CreateObject();
        if (NULL == pstJson) {
            return 0; 
        }

        STRV(pstRes, 1, stQrcodeTransDetail.sTransCode);
        STRV(pstRes, 2, stQrcodeTransDetail.sAcqId);
        STRV(pstRes, 3, stQrcodeTransDetail.sTransDate);
        STRV(pstRes, 4, stQrcodeTransDetail.sTransTime);
        STRV(pstRes, 5, stQrcodeTransDetail.sCardNo);
        STRV(pstRes, 6, stQrcodeTransDetail.sTraceNo);
        STRV(pstRes, 7, stQrcodeTransDetail.sSysTrace);
        STRV(pstRes, 8, stQrcodeTransDetail.sRrn);
        STRV(pstRes, 9, stQrcodeTransDetail.sBatchNo);
        STRV(pstRes, 10, stQrcodeTransDetail.sMerchOrderNo);
        STRV(pstRes, 11, stQrcodeTransDetail.sCardType);
        STRV(pstRes, 12, stQrcodeTransDetail.sMerchId);
        STRV(pstRes, 13, stQrcodeTransDetail.sTermId);
        STRV(pstRes, 14, stQrcodeTransDetail.sMerchName);
        DOUV(pstRes, 15, stQrcodeTransDetail.dAmount);
        DOUV(pstRes, 16, stQrcodeTransDetail.dFee);
        STRV(pstRes, 17, stQrcodeTransDetail.sSettleDate);
        STRV(pstRes, 18, stQrcodeTransDetail.sRespCode);
        STRV(pstRes, 19, stQrcodeTransDetail.sRespDesc);
        STRV(pstRes, 20, stQrcodeTransDetail.sOTransDate);
        STRV(pstRes, 21, stQrcodeTransDetail.sORrn);
        STRV(pstRes, 22, sUrl);
        STRV(pstRes, 23, stQrcodeTransDetail.sFeeType);
 
        SET_STR_KEY(pstJson, "tranCode", stQrcodeTransDetail.sTransCode);      
        SET_STR_KEY(pstJson, "platCode", stQrcodeTransDetail.sAcqId);
         
        sprintf(sTransTime,"%s%s",stQrcodeTransDetail.sTransDate,stQrcodeTransDetail.sTransTime);
        SET_STR_KEY(pstJson, "tranTime", sTransTime);       
        
        SET_STR_KEY(pstJson, "cardNo", stQrcodeTransDetail.sCardNo);
        SET_STR_KEY(pstJson, "traceNo", stQrcodeTransDetail.sTraceNo);
        SET_STR_KEY(pstJson, "sysTraceNo", stQrcodeTransDetail.sSysTrace);
        SET_STR_KEY(pstJson, "rrn", stQrcodeTransDetail.sRrn);
        SET_STR_KEY(pstJson, "batchNo", stQrcodeTransDetail.sBatchNo);

        SET_STR_KEY(pstJson, "orderId", stQrcodeTransDetail.sMerchOrderNo);
        SET_STR_KEY(pstJson, "cardType", stQrcodeTransDetail.sCardType);
        SET_STR_KEY(pstJson, "merchantId", stQrcodeTransDetail.sMerchId);
        SET_STR_KEY(pstJson, "termId", stQrcodeTransDetail.sTermId);
        SET_STR_KEY(pstJson, "merchantName", stQrcodeTransDetail.sMerchName);
        
        dAMount  = stQrcodeTransDetail.dAmount * 100;
        sprintf(sAmount, "%.f", dAMount);
        SET_STR_KEY(pstJson, "amount", sAmount);
        sprintf(sSettleAmount, "%.f", dAMount - (stQrcodeTransDetail.dFee * 100));
        SET_STR_KEY(pstJson, "settleAmount", sSettleAmount);
        SET_STR_KEY(pstJson, "feeType", stQrcodeTransDetail.sFeeType);
        
        SET_STR_KEY(pstJson, "settleDate", stQrcodeTransDetail.sSettleDate);
        if ( !memcmp(stQrcodeTransDetail.sRespCode,"00",2) ) {
            SET_STR_KEY(pstJson, "sysRespCode", "R3");
            SET_STR_KEY(pstJson, "sysRespDesc", "未支付");
        }
        else {
            SET_STR_KEY(pstJson, "sysRespCode", stQrcodeTransDetail.sRespCode);
            tTrimAll(stQrcodeTransDetail.sRespDesc);
            SET_STR_KEY(pstJson, "sysRespDesc", stQrcodeTransDetail.sRespDesc);
        }
        
        /*二维码撤销交易 原交易日期 与当前交易日期一直*/
        if( stQrcodeTransDetail.sTransCode[3] == '6' && memcmp(stQrcodeTransDetail.sRespCode,"25",2) ) {
            
            if( FindInlineOtransMessage(&stQrcodeTransDetail) <0 ){
                tLog(ERROR,"从原交易流水中获取信息失败！");
                continue;
            }
            
            SET_STR_KEY(pstJson, "originalTranDate", stQrcodeTransDetail.sTransDate);
            SET_STR_KEY(pstJson, "originalRrn", stQrcodeTransDetail.sORrn);
            SET_STR_KEY(pstJson, "originaltraceNo", stQrcodeTransDetail.sOTraceNo);
            SET_STR_KEY(pstJson, "originalbatchNo", stQrcodeTransDetail.sOBatchNo); 
        }
        
        if (strlen(sUrl) == 0 || !strstr(sUrl, "http")) {
            tLog(ERROR, "交易通知Url为空 或 不合规的URL请求地址，无法发送请求 ");
        } else {
            /*发送前进行 签名计算，并组入json*/
            if( getSign(pstJson,sSign) < 0 ) {
                tLog(ERROR,"计算签名失败,rrn[%s]",stQrcodeTransDetail.sRrn);
                continue;
            }
            SET_STR_KEY(pstJson, "sign", sSign);
            pcMsg = cJSON_PrintUnformatted(pstJson);
            
            tLog(INFO, "向[%s]发送http请求,json参数为[%s]", sUrl, pcMsg);
            if ( HttpRequest(sUrl, pcMsg, sHttpResponse) < 0 ) {
                tLog(ERROR, "http请求失败");
                /*更新交易通知推送次数*/
                if (UpdNoticeCnt(pstJson,"b_inline_tarns_detail_his")< 0) {
                    tLog(ERROR, "更新二维码交易通知推送次数失败！");
                }
            } else {
                 if ( strstr(sHttpResponse, "null") != NULL || strstr(sHttpResponse, "HTML") != NULL ) {
                    tLog(ERROR, "http返回数据里含null 或 html");
                } else {
                    pstRecvJson = cJSON_Parse(sHttpResponse);
                    //此处返回内容待处理
                    GET_STR_KEY(pstRecvJson, "responseCode", sRespCode);
                    GET_STR_KEY(pstRecvJson, "responseDesc", sRespDesc);
                    if (memcmp(sRespCode, "00", 2) == 0) {
                        /*更新交易通知推送状态*/
                        if (UpdNoticeStat(pstRecvJson,"b_inline_tarns_detail_his")< 0) {
                            tLog(ERROR, "更新二维码交易通知推送状态失败！");
                        } 
                        else {
                            tLog(ERROR, "二维码交易通知[%s]推送成功！", stQrcodeTransDetail.sRrn);
                        }
                    } else {
                        tLog(ERROR, "pos交易通知[%s]推送失败,返回码[%s:%s]", stQrcodeTransDetail.sRrn, sRespCode,sRespDesc);
                        /*更新交易通知推送次数*/
                        if (UpdNoticeCnt(pstRecvJson,"b_inline_tarns_detail_his")< 0) {
                            tLog(ERROR, "更新二维码交易通知推送次数失败！");
                        } 
                    }
                    cJSON_Delete(pstRecvJson);
                }
            }
        }

        cJSON_Delete(pstJson);
        
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未检索到待推送的交易流水.....");
        tReleaseRes(pstRes);
        return -2;
    }
    tReleaseRes(pstRes);
    return 0;
}



int getSign(cJSON *pstJson,char *pcsSign) {
    FILE *fp = NULL;
    char sBuf[4096] = {0}, order[128]  = {0}, sSign[32 + 1] = {0};
    char sTemp[256 + 1] = {0};
    int i = 0;
    char *sKey[] = { "amount","authCode","bankName","batchNo","cardNo",
                    "cardType","extData","feeType","inputMode","merchantId","merchantName",
                    "orderId","originalAuthCode","originalbatchNo","originalRrn",
                    "originaltraceNo","originalTranDate","platCode","rrn","settleAmount",
                    "settleDate","sysRespCode","sysRespDesc","sysTraceNo","termId","traceNo",
                    "tranCode","tranTime","" };

    memcpy(sBuf,"16D0462C164CB0E3AD6EF2B0B9514092",32);
    while(1)
    {
        if(sKey[i][0] == '\0') {
            break;
        }
        tLog(DEBUG,"sKey[%d]:[%s]", i,sKey[i]);
        MEMSET(sTemp);
        GET_STR_KEY(pstJson, sKey[i], sTemp);
        tTrim(sTemp);
        memcpy(sBuf+strlen(sBuf),sTemp,strlen(sTemp));
        i++;
    }
    tTrim(sBuf);
    tTrimAll(sBuf);
    tLog(DEBUG,"sBuf[%s]",sBuf);
 
    sprintf(order,"echo -n %s|md5sum",sBuf);
    fp = popen(order,"r");
    fgets(sSign,sizeof(sSign),fp);
    tLog(INFO,"sSign[%s]",sSign);
    pclose(fp);
    tTrim(sSign);
    tStrCpy( pcsSign, sSign, 32 );
    
    return ( 0 );
}