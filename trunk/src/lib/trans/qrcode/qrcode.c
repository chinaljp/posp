/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"

/* 主扫模式 */
int QrcodeActive(cJSON *pstTranData, int *piFlag) {
    /*   扫描付款码二维码获取到的用户身份字符串  */

    /*  主扫模式订单号暂用身份唯一值，pos修改 需要很大更新量，暂且使用身份串
    char sOrder_no[31] = {0};
    char sMerchId[16] = {0}, sRrn[13] = {0};
    cJSON * pstTransJson;
    

    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);

    sprintf(sOrder_no, "%s%s", sRrn, sMerchId);
    tLog(DEBUG, "qr_order_no[%s]", sOrder_no);
    SET_STR_KEY(pstTransJson, "qr_order_no", sOrder_no);
     */
    cJSON * pstTransJson;
    char sSeed[30] = {0};
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "seed", sSeed);
    SET_STR_KEY(pstTransJson, "qr_order_no", sSeed);
    return 0;
}


/*被扫模式*/

/*二维码下单交易*/
int QrcodeOrder(cJSON *pstTranData, int *piFlag) {

    char sOrder_no[32+1] = {0},sTransDate[8+1] = {0}, sTransCode[6 + 1] = {0};
    char sMerchId[16] = {0}, sRrn[13] = {0}, sDate[8 + 1] = {0};
    cJSON * pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "settle_date", sDate);

    sprintf(sOrder_no, "%s%s%s", sTransDate+2,sRrn+6, sMerchId);
    tLog(DEBUG, "qr_order_no[%s]", sOrder_no);
    /*正向交易 请求交易流中将订单号更新到本次交易的流水记录中(二维码主扫 被扫,撤销)*/
    if( UpMerchOrderNo(sDate,sOrder_no,sMerchId,sRrn,sTransCode) < 0 ) {
        ErrHanding(pstTransJson, "96", "更新商户[%s]订单号[%s]失败.", sMerchId,sOrder_no);
        return ( -1 );
    }
    
    SET_STR_KEY(pstTransJson, "qr_order_no", sOrder_no);
    return 0;
}

/*二维码查询交易返回赋值*/
void QrcodeInquiry(cJSON *pstTranData, char *pstRespCode) {
    //  char sRespCode[2+1]={0};
    char sResult_code[3] = {0}, sTrade_state[3] = {0};
    cJSON * pstTransJson = NULL, *pstRepJson = NULL;

    pstRepJson = GET_JSON_KEY(pstTranData, "response");

    GET_STR_KEY(pstRepJson, "result_code", sResult_code);
   // GET_STR_KEY(pstRepJson, "trade_state", sTrade_state);

    if (sResult_code[0] == '0') {
        strcpy(pstRespCode, "00");
    } else if (sResult_code[0] == '1') {
        strcpy(pstRespCode, "R9");
    }
    /* 返回状态   */
    if (!strcmp(sResult_code, "Q1") ||
            !strcmp(sResult_code, "Q2") ||
            !strcmp(sResult_code, "Q3") ||
            !strcmp(sResult_code, "Q4") ||
            !strcmp(sResult_code, "Q5") ||
            !strcmp(sResult_code, "Q6") ||
            !strcmp(sResult_code, "Q7") ||
            !strcmp(sResult_code, "Q8") ||
            !strcmp(sResult_code, "Q9") ||
            !strcmp(sResult_code, "R0") ||
            !strcmp(sResult_code, "R1") ||
            !strcmp(sResult_code, "R2") ||
            !strcmp(sResult_code, "R3") ||
            !strcmp(sResult_code, "R4") ||
            !strcmp(sResult_code, "R5") ||
            !strcmp(sResult_code, "R6") ||
            !strcmp(sResult_code, "R7") ||
            !strcmp(sResult_code, "R8")
            ) {
        strcpy(pstRespCode, sResult_code);

    }
}

/* 下单交易返回 */
int RespInfo(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sRespCode[2 + 1] = {0}, sCodeUrl[100] = {0};
    
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_STR_KEY(pstRepJson, "result_code", sRespCode);

    // GET_STR_KEY(pstRepJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRepJson, "code_url", sCodeUrl);

    // SET_STR_KEY(pstDataJson,"resp_code",sRespCode);
    SET_STR_KEY(pstDataJson, "code_url", sCodeUrl);
    ErrHanding(pstDataJson, sRespCode);
    return 0;
}


/****************************************************************************************/
/*      函数名:     UpOrderValidProc()                  	                                */
/*      功能说明:   更新原交易结算标识						        */
/*      输入参数:   TranData *pstTranData                                                */
/*      输出参数:                                                                        */
/*      返回说明:   0 成功  非 0 不成功                                                   */
/*只适用查询交易使用，更新被扫交易流水中的valid_flag                                        */
/*****************************************************************************************/
int UpOrderValidProc(cJSON *pstJson, int *piFlag) {
    int iRet = 0;
    char sTransCode[6 + 1] = {0}, sRespCode[2 + 1] = {0}, sQrOrderNo[30 + 1] = {0};
    char sCardType[1 + 1]         = {0};
    char sCardAttr[2 + 1]         = {0}; //银联二维码卡属性 01 - 借记卡 02 - 贷记卡
    char sSettleKey[38 + 1]       = {0}; //清算主键
    char sBuf[16 + 1]             = {0};
    char sSettleSysTrace[6 + 1]   = {0}; //银联二维码系统跟踪号
    char sSettleTransTime[10 + 1] = {0}; //银联二维码交易传输时间
    char sChannelSettleDate[4+1]  = {0}; //渠道清算日期
    char sSettleDate[8 + 1]={0}; //系统结算日期
    
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "qr_order_no", sQrOrderNo);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    
    tLog(DEBUG, "TRANS_CODE = [%s],RESP_CODE=[%s]", sTransCode,sRespCode);
    tLog(DEBUG, "ORDER_NO=[%s]", sQrOrderNo);
    if ( !memcmp(sRespCode, "00", 2) ) {

        iRet = UpValidflag(sQrOrderNo);
        if (iRet < 0) {
            tLog(ERROR, "更新二维码valid_flag标志失败，订单号[%s]", sQrOrderNo);
            return -1;
        }
        tLog(INFO, "更新订单号[%s]的清算标志成功.", sQrOrderNo);
        
        /*银联二维码查询支付结果 为 支付成功 需要更新原交易流水*/
        if(sTransCode[2] == 'Y') {
            GET_STR_KEY(pstDataJson, "settleKey", sSettleKey);
            GET_STR_KEY(pstDataJson, "cardAttr", sCardAttr);
            GET_STR_KEY(pstDataJson, "ChannelSettleDate",sChannelSettleDate);
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
            tLog(INFO,"sTransCode = [%s],需要更新清算主键中的系统跟踪号、交易传输时间",sTransCode);
            
            /*获取settle_date(出参)*/
            GetSysSettleDate(sSettleDate,sChannelSettleDate);
                      
            iRet = UpCupsSettleMessage(sQrOrderNo,sCardType,sSettleSysTrace,sSettleTransTime,sChannelSettleDate,sSettleDate);
            if (iRet < 0) {
                tLog(ERROR, "更新银联二维码清算主键内容失败，订单号[%s]", sQrOrderNo);
                return -1;
            }
            tLog(INFO, "更新订单号[%s]的清算主键内容成功.", sQrOrderNo); 
        } 
           
    } else {
        tLog(ERROR,"RESP_CODE=[%s],订单[%s]交易失败", sRespCode,sQrOrderNo);
        ErrHanding(pstDataJson, sRespCode);
        return ( -1 );
    }
    return 0;
}

/****************************************************************************************/
/*      函数名:     UpdSettleKey()                  	                                */
/*      功能说明:   更新原交易结算标识						        */
/*      输入参数:   TranData *pstTranData                                                */
/*      输出参数:                                                                        */
/*      返回说明:   0 成功  非 0 不成功                                                   */
/*****************************************************************************************/
int UpdSettleKey(cJSON *pstJson, int *piFlag) {
    int iRet = 0;
    char sTransCode[6 + 1] = {0}, sRespCode[2 + 1] = {0}, sQrOrderNo[30 + 1] = {0};
    char sCardType[1 + 1]         = {0};
    char sCardAttr[2 + 1]         = {0}; //银联二维码卡属性 01 - 借记卡 02 - 贷记卡
    char sSettleKey[38 + 1]       = {0}; //清算主键
    char sBuf[16 + 1]             = {0};
    char sSettleSysTrace[6 + 1]   = {0}; //银联二维码系统跟踪号
    char sSettleTransTime[10 + 1] = {0}; //银联二维码交易传输时间
    
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "qr_order_no", sQrOrderNo);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    
    tLog(DEBUG, "TRANS_CODE = [%s],RESP_CODE=[%s]", sTransCode,sRespCode);
    tLog(DEBUG, "ORDER_NO=[%s]", sQrOrderNo);
    if ( !memcmp(sRespCode, "00", 2) ) {  
        /*银联二维码查询支付结果 为 支付成功 需要更新原交易流水*/
        if(sTransCode[2] == 'Y') {
            GET_STR_KEY(pstDataJson, "settleKey", sSettleKey);
            memcpy(sBuf, sSettleKey+22, sizeof(sBuf)); //截取系统跟踪号 交易传输时间
            memcpy(sSettleSysTrace,sBuf,6);            //截取系统跟踪号
            sSettleSysTrace[7] = '\0';
            memcpy(sSettleTransTime,sBuf+strlen(sSettleSysTrace),10);//截取交易传输时间
            tLog(INFO,"sSettleKey[%s],sSettleSysTrace[%s],sSettleTransTime[%s]",sSettleKey,sSettleSysTrace,sSettleTransTime);
            tLog(INFO,"sTransCode = [%s],需要更新清算主键中的系统跟踪号、交易传输时间",sTransCode);
            
            iRet = UpCupsSettleKey(sQrOrderNo,sSettleSysTrace,sSettleTransTime);
            if (iRet < 0) {
                tLog(ERROR, "更新银联二维码清算主键内容失败，订单号[%s]", sQrOrderNo);
                return -1;
            }
            tLog(INFO, "更新订单号[%s]的清算主键内容成功.", sQrOrderNo); 
        } 
           
    } else {
        tLog(ERROR,"RESP_CODE=[%s],订单[%s]交易失败", sRespCode,sQrOrderNo);
        ErrHanding(pstDataJson, sRespCode);
        return ( -1 );
    }
    return 0;
}


/*********************************************************************************************/
/*      函数名:     UpInLineValidProc()                  	                             */
/*      功能说明:   更新交易有效标识(交易成功进行更新)					     */
/*      输入参数:   TranData *pstTranData                                                     */
/*      输出参数:                                                                             */
/*      返回说明:   0 成功  非 0 不成功                                                        */
/* 二维码主扫交易 更新交易流水中的valid_flag, 二维码撤销交易，更新二维码撤销交易流水中的valid_flag */
/**********************************************************************************************/
int UpInLineValidProc(cJSON *pstJson, int *piFlag) {
    int iRet = 0;
    char sRespCode[2 + 1] = {0}, sTransDate[8 + 1] = {0}, sSysTrace[6 + 1] = {0};
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstDataJson,"trans_date",sTransDate);

    tLog(INFO, "RESP_CODE=[%s]", sRespCode);
    tLog(INFO, "SYS_TRACE=[%s]", sSysTrace);
    tLog(INFO, "TRANS_DATE=[%s]", sTransDate);
    if (!memcmp(sRespCode, "00", 2)) {

        iRet = UpValidflagInLine(sSysTrace,sTransDate);
        if (iRet < 0) {
            tLog(ERROR, "更新二维码valid_flag标志失败，交易日期[%s]，流水号[%s]", sTransDate,sSysTrace);
            return -1;
        }

        tLog(INFO, "更新流水[%s]的有效标志成功,trans_date = [%s].", sSysTrace,sTransDate);

        return 0;
    } else {
        tLog(ERROR,"RESP_CODE=[%s],交易失败", sRespCode);
        ErrHanding(pstDataJson, sRespCode);
        return ( -1 );
    }

}

/*更新消费撤销原交易*/
int UpdQrTransDetail(cJSON *pstJson, int *piFlag) {
    
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "交易失败[%s],不更新原消费交易的有效标志.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "trans_date", sOTransDate);/*此处sOTransDate 撤销适用 退款交易使用时 注意原交易日期*/
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_INLINE_TARNS_DETAIL", sOTransDate, sORrn, "1") < 0) {
        ErrHanding(pstTransJson, "96", "更新原消费交易[%s:%s]有效标志[1]失败.", sOTransDate, sORrn);
        return -1;
    }
    tLog(DEBUG, "更新原消费交易[%s:%s]有效标志[1]成功.", sOTransDate, sORrn);
    
    return ( 0 );
}


/* 交易返回 */
int QueryReturnInfo(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sSettleKey[38 + 1] = {0}, sCardAttr[2 + 1] = {0}, sChannelSettleDate[4] = {0};
    char sResultCode[2 + 1] = {0}, sRespCode[2 + 1] = {0};
    /*char sCodeUrl[100] = {0}, sTransCode[6 + 1] = {0}, sTradeState[3 + 1] = {0};*/
    double dAmount = 0.0;
    
   //char sSettleKey[38 + 1] = {0};
    /* 转换赋值交易码 */
    //QrcodeInquiry(pstJson, sRespCode);
    /*将返回的内容赋值*/
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_DOU_KEY(pstRepJson, "amount", dAmount);
    SET_DOU_KEY(pstDataJson, "amount", dAmount);
    
    GET_STR_KEY(pstRepJson, "settleKey", sSettleKey);
    SET_STR_KEY(pstDataJson, "settleKey", sSettleKey);
    
    GET_STR_KEY(pstRepJson, "cardAttr", sCardAttr);
    SET_STR_KEY(pstDataJson, "cardAttr", sCardAttr);
    
    GET_STR_KEY(pstRepJson, "ChannelSettleDate",sChannelSettleDate);
    SET_STR_KEY(pstDataJson, "ChannelSettleDate", sChannelSettleDate);
            
    GET_STR_KEY(pstRepJson, "result_code", sResultCode);
    strcpy(sRespCode,sResultCode);
    //GET_DOU_KEY(pstRepJson, "settleKey", sSettleKey);
    //SET_DOU_KEY(pstDataJson, "settleKey", sSettleKey);
    
#if 0
    /*华夏二维码 交易返回码 停用 */
    GET_STR_KEY(pstRepJson, "trans_code", sTransCode);
    if ( !memcmp(sTransCode,"02B300",6) || !memcmp(sTransCode,"02W300",6) || 
         !memcmp(sTransCode,"0AB300",6) || !memcmp(sTransCode,"0AW300",6) ) {
        GET_STR_KEY(pstRepJson, "trade_state", sTradeState);
        if( !memcmp(sTradeState,"100",3) ) {
            strcpy(sRespCode, "Q3");
        } else if ( !memcmp(sTradeState,"101",3) ) {
            strcpy(sRespCode, "R3");
        } else if ( !memcmp(sTradeState,"102",3) ) {
            strcpy(sRespCode, "Q7");
        } else if ( !memcmp(sTradeState,"103",3) ) {
            strcpy(sRespCode, "FD");/*订单处理中,交易未完成*/
        } else if ( !memcmp(sTradeState,"104",3) ) {
            strcpy(sRespCode, "R6");
        } else {
            strcpy(sRespCode, "96");
        }
    }
#endif
    ErrHanding(pstDataJson, sRespCode);
    return 0;
}

int EmptytransFlow(cJSON *pstJson, int *piFlag) {
    tLog(INFO, "空交易流");

    return 0;
}

int RetFalse(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    tLog(INFO, "暂不支持此交易");
    ErrHanding(pstTransJson, "FA", "暂不支持此交易");
    return (-1);
}

int PosVersionCtrl(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sHead[6 + 1] = {0}, sVersion[2 + 1] = {0}, sTransCode[6 + 1] = {0};
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "head", sHead);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    tStrCpy(sVersion, sHead + 2, 2);
    tLog(INFO,"报文头：%s pos版本[%s]", sHead, sVersion);
    if ( (!memcmp(sTransCode,"02B",3) || !memcmp(sTransCode,"02W100",6) )&& memcmp(sVersion,"21",2) ){
          ErrHanding(pstDataJson, "12", "Pos版本号[%s]不支持此交易[%s].", sVersion, sTransCode );
        return -1;
    }
    return 0;
}

int TmpSetSweepType(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sMerchId[15 + 1] = {0}, sTransCode[6 + 1] = {0};
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "merch_id", sMerchId);
    if ( !memcmp(sTransCode,"02B",3) || !memcmp(sTransCode,"0AB",3) ){
        if( FindALiPayChannel(sMerchId)==0 )
        {
            SET_STR_KEY(pstDataJson, "sweep_type", "BNAliPay"); 
        }
        else
            SET_STR_KEY(pstDataJson, "sweep_type", "ZXAliPay"); 
        
    }
    return 0;
}

int FindALiPayChannel(char *pcMerchId) {
//int FindOriTransMsg(char *pcOrderNo, char *pcTransCode, char *pcChannelMerchId,char *pcValidFlag) {
    OCI_Resultset *pstRes = NULL;
    double dAmount = 0.00;
    char sSqlStr[1024]={0},sZfbStatus[2]={0};
    MEMSET(sSqlStr);
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select decode((select key_value from s_param where key='ALIPAY_BIG_MERCH_SWITCH') ,'1','0',(select  zfb_status from WZ_MERCH where merch_id ='%s')) from dual",pcMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    
     while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sZfbStatus);        
        tTrim(sZfbStatus);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);    
    tLog(INFO,"商户%s进件状态：%s ", pcMerchId, sZfbStatus);
    if(sZfbStatus[0]=='1')
        return ( 0 );
    else return -1;
}