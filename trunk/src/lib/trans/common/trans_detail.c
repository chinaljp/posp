/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "trans_code.h"
#include "t_macro.h"


#define POS_TRANS_FIELDS "TRANS_CODE,CARD_NO,CARD_ID,CARD_TYPE,CARD_EXP_DATE,AMOUNT" \
",TRANSMIT_TIME,TRANS_DATE,TRANS_TIME,SYS_TRACE,LOGIC_DATE,MERCH_ID,TERM_ID,TERM_SN" \
",TRACE_NO,RRN,MCC,INPUT_MODE,COND_CODE,PIN_CODE,ACQ_ID,ISS_ID,AGENT_ID,CHANNEL_ID" \
",CHANNEL_MERCH_ID,CHANNEL_TERM_ID,CHANNEL_TERM_SN,CHANNEL_MCC,AUTHO_FLAG,BATCH_NO" \
",AUTH_CODE,RESP_CODE,ISTRESP_CODE,RESP_DESC,RESP_ID,MERCH_ORDER_NO,O_RRN" \
",O_TRANS_DATE,SETTLE_DATE,TRANS_TYPE,VALID_FLAG,CHECK_FLAG,REFUND_AMT,FEE,FEE_FLAG,FEE_DESC,SIGN_FLAG,CARD_NO_ENCRYPT,KEY_NAME"

#define INLINE_TRANS_FIELDS "TRANS_CODE,CARD_NO,CARD_ID,CARD_TYPE,CARD_EXP_DATE,AMOUNT" \
",TRANSMIT_TIME,TRANS_DATE,TRANS_TIME,SYS_TRACE,LOGIC_DATE" \
",MERCH_ID,TERM_ID,TERM_SN,TRACE_NO,RRN,MCC" \
",ACQ_ID,AGENT_ID,CHANNEL_ID ,CHANNEL_MERCH_ID,CHANNEL_TERM_ID,CHANNEL_TERM_SN,CHANNEL_MCC" \
",TRANS_TYPE,VALID_FLAG,CHECK_FLAG,SETTLE_FLAG,RESP_CODE,BATCH_NO,REFUND_AMT" \
",ORDER_NO,ORDER_URL,MOBILE,ACCOUNT_NAME,CERT_NO" \
",MERCH_ORDER_NO,O_RRN,CARD_NO_ENCRYPT,MOBILE_ENCRYPT,CERT_NO_ENCRYPT,KEY_NAME,CALLBACK_URL"

int AddPosTransLs(cJSON *pstJson) {
    char sSqlStr[1024] = {0};
    char sRrn[RRN_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0}, sCardNo[CARD_NO_LEN + 1] = {0};
    char sCardId[CARD_ID_LEN + 1] = {0}, sCardType[CARD_TYPE_LEN + 1] = {0}, sCardExpDate[EXP_DATE_LEN + 1] = {0};
    char sTransmitTime[TRANSMIT_TIME_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN] = {0}, sTransTime[TRANS_TIME_LEN + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0}, sLogicDate[LOGIC_DATE_LEN + 1] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0};
    char sTermId[TERM_ID_LEN + 1] = {0}, sTermSn[SN_LEN + 1] = {0}, sTraceNo[TRACE_NO_LEN + 1] = {0}, sMcc[MCC_LEN + 1] = {0};
    char sInputMode[INPUT_MODE_LEN + 1] = {0}, sCondCode[COND_CODE_LEN + 1] = {0}, sPinCode[PIN_CODE_LEN + 1] = {0};
    char sAcqId[INST_ID_LEN + 1] = {0}, sIssId[INST_ID_LEN + 1] = {0}, sAgentId[INST_ID_LEN + 1] = {0}, sChannelId[INST_ID_LEN + 1] = {0};
    char sChannelMerchId[MERCH_ID_LEN + 1] = {0}, sChannelTermId[TERM_ID_LEN + 1] = {0};
    char sChannelTermSn[SN_LEN + 1] = {0}, sChannelMcc[MCC_LEN + 1] = {0}, sAuthoFlag[AUTHO_FLAG_LEN + 1] = {0};
    char sBatchNo[BATCH_NO_LEN + 1] = {0}, sAuthCode[AUTH_CODE_LEN + 1] = {0}, sIcData[IC_DATA_LEN + 1] = {0};
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sIstRespCode[RESP_CODE_LEN + 1] = {0}, sRespLog[RESP_LOG_LEN + 1] = {0};
    char sRespId[INST_ID_LEN + 1] = {0}, sMerchOrderNo[ORDER_NO_LEN] = {0};
    char sOrrn[RRN_LEN + 1] = {0}, sOTransDate[TRANS_DATE_LEN + 1] = {0}; // *pcFeeDesc = NULL, *pcFeeFlag = NULL;
    char sCardNoEnc[255 + 1] = {0}, sKeyName[255 + 1] = {0};
    double dAmount = 0.0, dRefundAmt = 0.0; //, dFee = 0.0;
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstJson, "resp_log", sRespLog);
    GET_STR_KEY(pstJson, "resp_id", sRespId);
    GET_STR_KEY(pstJson, "merch_order_no", sMerchOrderNo);
    GET_STR_KEY(pstJson, "o_rrn", sOrrn);
    GET_STR_KEY(pstJson, "o_trans_date", sOTransDate);

    GET_STR_KEY(pstJson, "mcc", sMcc);
    GET_STR_KEY(pstJson, "input_mode", sInputMode);
    GET_STR_KEY(pstJson, "cond_code", sCondCode);
    GET_STR_KEY(pstJson, "pin_code", sPinCode);
    GET_STR_KEY(pstJson, "acq_id", sAcqId);
    GET_STR_KEY(pstJson, "iss_id", sIssId);
    GET_STR_KEY(pstJson, "agent_id", sAgentId);

    GET_STR_KEY(pstJson, "channel_id", sChannelId);
    //    GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
    //    GET_STR_KEY(pstJson, "channel_term_id", sChannelTermId);
    //    GET_STR_KEY(pstJson, "channel_term_sn", sChannelTermSn);
    GET_STR_KEY(pstJson, "channel_mcc", sChannelMcc);
    GET_STR_KEY(pstJson, "autho_flag", sAuthoFlag);
    GET_STR_KEY(pstJson, "batch_no", sBatchNo);
    GET_STR_KEY(pstJson, "auth_code", sAuthCode);
    GET_STR_KEY(pstJson, "ic_data", sIcData);
    //  GET_STR_KEY(pstJson, "resp_code", sRespCode);
    GET_STR_KEY(pstJson, "istresp_code", sIstRespCode);

    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    /* kms 加密存储 ,原字段屏蔽显示 */
    if (FindValueByKey(sKeyName, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
    } else {
        if (sCardNo[0] != '\0') {
            if (tHsm_Enc_Asc_Data(sCardNoEnc, sKeyName, sCardNo) < 0) {
                tLog(ERROR, "卡号加密失败.");
            }    
            else {
                tLog(DEBUG, "卡号加密成功.");
                SET_STR_KEY(pstJson,"card_encno",sCardNoEnc);
            }
        }
    }
    if ( sCardNo[0] != '\0'&& strlen(sCardNo) > 10 )
        tScreenStr(sCardNo, strlen(sCardNo), '*', 6, 4);

    GET_STR_KEY(pstJson, "card_id", sCardId);
    GET_STR_KEY(pstJson, "card_type", sCardType);
    /* 有效期不在存储 */
    //GET_STR_KEY(pstJson, "card_exp_date", sCardExpDate);
    GET_DOU_KEY(pstJson, "amount", dAmount);
    GET_DOU_KEY(pstJson, "refund_amt", dRefundAmt);
    dAmount = dAmount / 100;
    dRefundAmt = dRefundAmt / 100;
    GET_STR_KEY(pstJson, "transmit_time", sTransmitTime);
    GET_STR_KEY(pstJson, "trans_date", sTransDate);
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    GET_STR_KEY(pstJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstJson, "logic_date", sLogicDate);
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "term_id", sTermId);
    GET_STR_KEY(pstJson, "term_sn", sTermSn);
    /* 如果元交易失败，没有返回应答，是无法更新交易流水的，这时冲正查找的渠道商户号等信息是空的，会造成冲正交易被拒绝 */
    strcpy(sChannelMerchId, sMerchId);
    strcpy(sChannelTermId, sTermId);
    strcpy(sChannelTermSn, sTermSn);
    GET_STR_KEY(pstJson, "trace_no", sTraceNo);
    strcpy(sRespCode, "XX");
    snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_POS_TRANS_DETAIL(%s) "
            " VALUES('%s','%s','%s','%s','%s',%f"
            ",'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s'"
            ",'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s'"
            ",'%s','%s','%s','%s','%s','%s','%s'"
            ",'%s','%s','%s','%s','','0','0','N',%f,0,'N','','0','%s','%s')", POS_TRANS_FIELDS
            , sTransCode, sCardNo, sCardId, sCardType, sCardExpDate, dAmount
            , sTransmitTime, sTransDate, sTransTime, sSysTrace, sLogicDate, sMerchId, sTermId, sTermSn, sTraceNo, sRrn
            , sMcc, sInputMode, sCondCode, sPinCode, sAcqId, sIssId, sAgentId, sChannelId, sChannelMerchId, sChannelTermId, sChannelTermSn
            , sChannelMcc, sAuthoFlag, sBatchNo, sAuthCode, sRespCode, sIstRespCode, sRespLog
            , sRespId, sMerchOrderNo, sOrrn, sOTransDate, dRefundAmt, sCardNoEnc, sKeyName);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "添加交易rrn[%s]记录失败.", sRrn);
        tRollback();
        return -1;
    }

    tReleaseRes(pstRes);

    tLog(INFO, "交易rrn[%s]记录成功.", sRrn);
    return 0;
}

int UpdPosTransLs(cJSON *pstJson) {
    char sSqlStr[1024] = {0};
    char sRrn[RRN_LEN + 1] = {0}, sChannelRrn[12 + 1] = {0}, sHisRrn[RRN_LEN + 1] = {0}, sFeeDesc[FEE_DESC_LEN + 1] = {0}, sFeeFlag[FEE_FLAG_LEN + 1], sSettleDate[SETTLE_DATE_LEN + 1] = {0};
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sIstRespCode[RESP_CODE_LEN + 1] = {0}, sRespLog[RESP_LOG_LEN + 1] = {0};
    char sRespId[INST_ID_LEN + 1] = {0};
    char sTransDate[TRANS_DATE_LEN] = {0}, sHisTransDate[TRANS_DATE_LEN] = {0}, sChannelId[INST_ID_LEN + 1] = {0}, sChannelMerchId[MERCH_ID_LEN + 1] = {0};\
    char sChannelTermId[TERM_ID_LEN + 1] = {0}, sBatchNo[BATCH_NO_LEN + 1] = {0}, sAuthCode[AUTH_CODE_LEN + 1] = {0};
    char sChannelTermSn[SN_LEN + 1] = {0}, sChannelMcc[MCC_LEN + 1] = {0}, sAuthoFlag[AUTHO_FLAG_LEN + 1] = {0}, sAgentId[INST_ID_LEN + 1] = {0};
    char sCardBin[10 + 1] = {0}, sCardType[1 + 1] = {0}, sIssId[8 + 1] = {0}, sTransType[1 + 1] = {0};
    char sMcc[MCC_LEN + 1] = {0}, sUserCode[15 + 1] = {0}, sFeeType[2 + 1] = {0},sAgentOrganization[255 + 1],sChannelSettleDate[4 + 1]={0};//mmdd
    double dFee = 0.0;
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstJson, "card_bin", sCardBin);
    GET_STR_KEY(pstJson, "card_type", sCardType);
    GET_STR_KEY(pstJson, "iss_id", sIssId);

    GET_DOU_KEY(pstJson, "fee", dFee);
    GET_STR_KEY(pstJson, "fee_flag", sFeeFlag);
    GET_STR_KEY(pstJson, "fee_desc", sFeeDesc);
    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "o_rrn", sHisRrn);
    GET_STR_KEY(pstJson, "resp_code", sRespCode);
    GET_STR_KEY(pstJson, "istresp_code", sIstRespCode);
    GET_STR_KEY(pstJson, "resp_log", sRespLog);
    GET_STR_KEY(pstJson, "resp_id", sRespId);
    GET_STR_KEY(pstJson, "settle_date", sSettleDate);
    GET_STR_KEY(pstJson, "trans_date", sTransDate);
    GET_STR_KEY(pstJson, "o_trans_date", sHisTransDate);
    GET_STR_KEY(pstJson, "channel_id", sChannelId);
    GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
    GET_STR_KEY(pstJson, "channel_term_id", sChannelTermId);
    GET_STR_KEY(pstJson, "channel_term_sn", sChannelTermSn);
    GET_STR_KEY(pstJson, "channel_rrn", sChannelRrn);
    GET_STR_KEY(pstJson, "channel_mcc", sChannelMcc);
    GET_STR_KEY(pstJson, "channel_settle_date",sChannelSettleDate);
    GET_STR_KEY(pstJson, "autho_flag", sAuthoFlag);
    GET_STR_KEY(pstJson, "batch_no", sBatchNo);
    GET_STR_KEY(pstJson, "auth_code", sAuthCode);
    GET_STR_KEY(pstJson, "agent_id", sAgentId);
    GET_STR_KEY(pstJson, "mcc", sMcc);
    GET_STR_KEY(pstJson, "trans_type", sTransType);
    GET_STR_KEY(pstJson, "user_code", sUserCode);
    GET_STR_KEY(pstJson, "fee_type", sFeeType);
    GET_STR_KEY(pstJson, "agent_organization",sAgentOrganization);
    if (sFeeType[0] == '\0') {
        sFeeType[0] = 'B';
    }
    tLog(DEBUG, "trans_type[%s]", sTransType);
    if (sTransType[0] == '\0') {
        sTransType[0] = '0';
    }
    tLog(DEBUG, "trans_type[%s]", sTransType);
    //    "RESP_CODE,ISTRESP_CODE,RESP_DESC,RESP_ID,SETTLE_DATE,FEE,FEE_FLAG,FEE_DESC"
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_POS_TRANS_DETAIL SET "
            " RESP_CODE='%s', ISTRESP_CODE='%s',RESP_DESC='%s', RESP_ID='%s' "
            " ,SETTLE_DATE='%s',FEE=%f,FEE_DESC='%s',FEE_FLAG='%s'"
            " ,CHANNEL_ID='%s',CHANNEL_MERCH_ID='%s',CHANNEL_TERM_ID='%s',CHANNEL_TERM_SN='%s',CHANNEL_MCC='%s',CHANNEL_SETTLE_DATE='%s'"
            " ,AUTHO_FLAG='%s', O_RRN='%s', O_TRANS_DATE='%s', BATCH_NO='%s',AUTH_CODE='%s',MCC='%s'"
            ", CARD_ID='%s',CARD_TYPE='%s',ISS_ID='%s'"
            " ,LAST_MOD_TIME=sysdate"
            " ,AGENT_ID='%s',TRANS_TYPE='%s',USER_CODE='%s',FEE_TYPE='%s',CHANNEL_RRN='%s', AGENT_ORGANIZATION = '%s'"
            " WHERE RRN='%s' and TRANS_DATE='%s'"
            , sRespCode, sIstRespCode, sRespLog, sRespId
            , sSettleDate, dFee, sFeeDesc, sFeeFlag
            , sChannelId, sChannelMerchId, sChannelTermId, sChannelTermSn, sChannelMcc,sChannelSettleDate
            , sAuthoFlag, sHisRrn, sHisTransDate, sBatchNo, sAuthCode, sMcc
            , sCardBin, sCardType, sIssId, sAgentId, sTransType, sUserCode, sFeeType, sChannelRrn, sAgentOrganization
            , sRrn, sTransDate);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "更新交易date[%s]rrn[%s]交易结果失败.", sTransDate, sRrn);
        return -1;
    }

    tReleaseRes(pstRes);
    tLog(INFO, "更新交易date[%s]rrn[%s]结果成功.", sTransDate, sRrn);
    return 0;
}

int AddInlineTransLs(cJSON *pstJson) {
    char sSqlStr[1024] = {0};
    char sRrn[RRN_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0}, sCardNo[CARD_NO_LEN + 1] = {0};
    char sCardId[CARD_ID_LEN + 1] = {0}, sCardType[CARD_TYPE_LEN + 1] = {0}, sCardExpDate[EXP_DATE_LEN + 1] = {0};
    char sTransmitTime[TRANSMIT_TIME_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN] = {0}, sTransTime[TRANS_TIME_LEN + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0}, sLogicDate[LOGIC_DATE_LEN + 1] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0};
    char sTermId[TERM_ID_LEN + 1] = {0}, sTermSn[SN_LEN + 1] = {0}, sTraceNo[TRACE_NO_LEN + 1] = {0}, sMcc[MCC_LEN + 1] = {0};
    char sAcqId[INST_ID_LEN + 1] = {0}, sAgentId[INST_ID_LEN + 1] = {0}, sChannelId[INST_ID_LEN + 1] = {0};
    char sChannelMerchId[MERCH_ID_LEN + 1] = {0}, sChannelTermId[TERM_ID_LEN + 1] = {0};
    char sChannelTermSn[SN_LEN + 1] = {0}, sChannelMcc[MCC_LEN + 1] = {0}, sAuthoFlag[AUTHO_FLAG_LEN + 1] = {0};
    char sBatchNo[BATCH_NO_LEN + 1] = {0}, sAuthCode[AUTH_CODE_LEN + 1] = {0}, sIcData[IC_DATA_LEN + 1] = {0};
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sIstRespCode[RESP_CODE_LEN + 1] = {0}, sRespLog[RESP_LOG_LEN + 1] = {0};
    char sRespId[INST_ID_LEN + 1] = {0}, sMerchOrderNo[ORDER_NO_LEN] = {0};
    char sOrrn[RRN_LEN + 1] = {0}, sOTransDate[TRANS_DATE_LEN + 1] = {0};
    char sOrderNo[ORDER_NO_LEN + 1] = {0}, sOrderUrl[100 + 1] = {0}, sMobile[11 + 1] = {0}, sAccountName[50 + 1] = {0}, sCertNo[18 + 1] = {0};
    double dAmount = 0.0, dRefundAmt = 0.0; //, dFee = 0.0;
    char sCardNoEnc[255 + 1] = {0}, sKeyName[255 + 1] = {0}, sMobileEnc[255 + 1] = {0}, sCertNoEnc[255 + 1] = {0};
    char sCallBackUrl[100 + 1] = {0};/*add by Gjq at 20180328 APP二维码支付结果通知推送地址*/
    OCI_Resultset *pstRes = NULL;
    GET_STR_KEY(pstJson, "resp_log", sRespLog);
    GET_STR_KEY(pstJson, "resp_id", sRespId);
    GET_STR_KEY(pstJson, "merch_order_no", sMerchOrderNo);
    GET_STR_KEY(pstJson, "o_rrn", sOrrn);
    GET_STR_KEY(pstJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstJson, "mcc", sMcc);
    GET_STR_KEY(pstJson, "acq_id", sAcqId);

    GET_STR_KEY(pstJson, "callback_url",sCallBackUrl);
    GET_STR_KEY(pstJson, "channel_id", sChannelId);
    GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
    GET_STR_KEY(pstJson, "channel_term_id", sChannelTermId);
    GET_STR_KEY(pstJson, "channel_term_sn", sChannelTermSn);
    GET_STR_KEY(pstJson, "channel_mcc", sChannelMcc);
    GET_STR_KEY(pstJson, "autho_flag", sAuthoFlag);
    GET_STR_KEY(pstJson, "batch_no", sBatchNo);
    GET_STR_KEY(pstJson, "auth_code", sAuthCode);
    GET_STR_KEY(pstJson, "ic_data", sIcData);
    GET_STR_KEY(pstJson, "resp_code", sRespCode);
    GET_STR_KEY(pstJson, "istresp_code", sIstRespCode);

    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "card_id", sCardId);
    GET_STR_KEY(pstJson, "card_type", sCardType);
    //GET_STR_KEY(pstJson, "card_exp_date", sCardExpDate);
    GET_DOU_KEY(pstJson, "amount", dAmount);
    GET_DOU_KEY(pstJson, "refund_amt", dRefundAmt);
    GET_STR_KEY(pstJson, "agent_id", sAgentId);
    GET_STR_KEY(pstJson, "transmit_time", sTransmitTime);
    GET_STR_KEY(pstJson, "trans_date", sTransDate);
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    GET_STR_KEY(pstJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstJson, "logic_date", sLogicDate);
    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    GET_STR_KEY(pstJson, "term_id", sTermId);
    GET_STR_KEY(pstJson, "term_sn", sTermSn);
    GET_STR_KEY(pstJson, "trace_no", sTraceNo);

    /*GET_STR_KEY(pstJson, "order_no", sOrderNo);*/
    GET_STR_KEY(pstJson, "seed", sOrderNo); /*二维码主扫 支付订单号*/
    GET_STR_KEY(pstJson, "order_url", sOrderUrl);
    GET_STR_KEY(pstJson, "mobile", sMobile);
    GET_STR_KEY(pstJson, "account_name", sAccountName);
    GET_STR_KEY(pstJson, "cer_no", sCertNo);
    dAmount = dAmount / 100;

    /* kms 加密存储 ,原字段屏蔽显示 */
    if (FindValueByKey(sKeyName, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
    } else {
        if (sCardNo[0] != '\0') {
            if (tHsm_Enc_Asc_Data(sCardNoEnc, sKeyName, sCardNo) < 0)
                tLog(ERROR, "卡号加密失败.");
            else
                tLog(DEBUG, "卡号加密成功.");
        }
        if (sMobile[0] != '\0') {
            if (tHsm_Enc_Asc_Data(sMobileEnc, sKeyName, sMobile) < 0)
                tLog(ERROR, "手机号加密失败.");
            else
                tLog(DEBUG, "手机号加密成功.");
        }
        if (sCertNo[0] != '\0') {
            if (tHsm_Enc_Asc_Data(sCertNoEnc, sKeyName, sCertNo) < 0)
                tLog(ERROR, "证件号加密失败.");
            else
                tLog(DEBUG, "证件号加密成功.");
        }

    }
    /*
    if (sCardNo[0] != '\0')
        tScreenStr(sCardNo, strlen(sCardNo), '*', 6, 4);
    if (sMobile[0] != '\0')
        tScreenStr(sMobile, strlen(sMobile), '*', 3, 4);
    if (sCertNo[0] != '\0')
        tScreenStr(sCertNo, strlen(sCertNo), '*', 6, 4);
     */

    tLog(DEBUG, "[%s][%s][%s]", sCardNo, sMobile, sCertNo);
    snprintf(sSqlStr, sizeof (sSqlStr), "INSERT INTO B_INLINE_TARNS_DETAIL(%s) "
            " VALUES('%s','%s','%s','%s','%s',%f"
            ",'%s','%s','%s','%s','%s'"
            ",'%s','%s','%s','%s','%s','%s'"
            ",'%s','%s','%s','%s','%s','%s','%s'"
            ",'0','4','N','N','XX','%s','%f'"
            ",'%s','%s','%s','%s','%s','%s','%s'"
            ", '%s','%s','%s','%s','%s')", INLINE_TRANS_FIELDS
            , sTransCode, sCardNo, sCardId, sCardType, sCardExpDate, dAmount
            , sTransmitTime, sTransDate, sTransTime, sSysTrace, sLogicDate
            , sMerchId, sTermId, sTermSn, sTraceNo, sRrn, sMcc
            , sAcqId, sAgentId, sChannelId, sChannelMerchId, sChannelTermId, sChannelTermSn, sChannelMcc
            , sBatchNo, dRefundAmt
            , sOrderNo, sOrderUrl, sMobile, sAccountName, sCertNo, sMerchOrderNo, sOrrn
            , sCardNoEnc, sMobileEnc, sCertNoEnc, sKeyName, sCallBackUrl);
    tLog(INFO, "sql[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "添加交易rrn[%s]记录失败.", sRrn);
        tRollback();
        return -1;
    }

    tLog(INFO, "交易rrn[%s]记录成功.", sRrn);
    return 0;
}

int UpdInlineTransLs(cJSON *pstJson) {

    char sSqlStr[1024] = {0};
    char sRrn[RRN_LEN + 1] = {0};
    char sTransDate[TRANS_DATE_LEN] = {0};
    char sBatchNo[BATCH_NO_LEN + 1] = {0};
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRespLog[RESP_LOG_LEN + 1] = {0};
    char sRespId[INST_ID_LEN + 1] = {0};
    char sMerchOrderNo[ORDER_NO_LEN + 1] = {0}, sOrderUrl[100 + 1] = {0};
    char sFeeDesc[FEE_DESC_LEN + 1] = {0}, sSettleDate[8 + 1] = {0}, sAgentId[INST_ID_LEN + 1] = {0}, sFeeFlag[1 + 1] = {0};
    char sUserCode[15 + 1] = {0}, sFeeType[2 + 1] = {0};
    char sChannelMerchId[15 + 1] = {0}, sChannelTermId[8 + 1] = {0};//motify by  GJQ at 20180307
    double dFee = 0.0;
    OCI_Resultset *pstRes = NULL;
    char sTransCode[6 + 1] = {0}; //add by GuoJiaqing at 20171226, 用于区分二维码主扫、被扫交易与其他交易更新的不同的内容
    char sCardBin[10 + 1] = {0}, sCardType[CARD_TYPE_LEN + 1] = {0}, sCardExpDate[EXP_DATE_LEN + 1] = {0};
    char sCardNoEnc[255 + 1] = {0},sCardNo[21+1] = {0}, sKey[32+1] = {0};
    char sIstRespCode[8+1] = {0},sAgentOrganization[255 + 1];
     
    GET_DOU_KEY(pstJson, "fee", dFee);
    GET_STR_KEY(pstJson, "fee_flag", sFeeFlag);
    GET_STR_KEY(pstJson, "fee_desc", sFeeDesc);
    GET_STR_KEY(pstJson, "rrn", sRrn);
    GET_STR_KEY(pstJson, "resp_code", sRespCode);
    GET_STR_KEY(pstJson, "resp_log", sRespLog);
    GET_STR_KEY(pstJson, "resp_id", sRespId);
    GET_STR_KEY(pstJson, "trans_date", sTransDate);
    GET_STR_KEY(pstJson, "batch_no", sBatchNo);
    GET_STR_KEY(pstJson, "qr_order_no", sMerchOrderNo); /*收单系统 自动生成的商户订单号*/
    GET_STR_KEY(pstJson, "code_url", sOrderUrl);
    GET_STR_KEY(pstJson, "settle_date", sSettleDate);
    GET_STR_KEY(pstJson, "channel_merch_id", sChannelMerchId);
    GET_STR_KEY(pstJson, "channel_term_id", sChannelTermId);  //add by GJQ at 20180307  
    GET_STR_KEY(pstJson, "agent_id", sAgentId);
    GET_STR_KEY(pstJson, "user_code", sUserCode);
    GET_STR_KEY(pstJson, "fee_type", sFeeType);
    GET_STR_KEY(pstJson, "agent_organization",sAgentOrganization);
    if (sFeeType[0] == '\0') {
        sFeeType[0] = 'B';
    }
    
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    tLog(DEBUG,"trans_code = [%s]",sTransCode);
    
    GET_STR_KEY(pstJson, "istresp_code", sIstRespCode);
    GET_STR_KEY(pstJson, "card_bin", sCardBin);
    GET_STR_KEY(pstJson, "card_type", sCardType);
    GET_STR_KEY(pstJson, "card_no", sCardNo);
    GET_STR_KEY(pstJson, "card_no_enc", sCardNoEnc);
    GET_STR_KEY(pstJson, "key_name", sKey);
    tTrim(sCardNoEnc);
    if ( sCardNo[0] != '\0'&& strlen(sCardNo) > 10 )
        tScreenStr(sCardNo, strlen(sCardNo), '*', 6, 4);
    
    //    "RESP_CODE,ISTRESP_CODE,RESP_DESC,RESP_ID,SETTLE_DATE,FEE,FEE_FLAG,FEE_DESC"
    /*BEGIN modify by GuoJiaqing at 20171226                                                                *
     * 1、银联二维码主扫、被扫交易的手续费在异步通知中计算并更新；                                              *        
     * 2、微信、支付宝二维码主被扫交易的手续费在请求交易流中进行了计算并更新；                                   *
     * 3、银联、微信、支付宝二维码主被扫交易中，系统生成的商户订单号、获取到的结算日期 皆在请求交易流中 进行了更新；*
     * 所以本次二维码主扫、被扫交易的手续费、商户订单号、清算日期 在此不再进行更新  操作                          *
    */    
    //motify by GJQ at 20180307  银联二维码交易跳转生活圈 更新跳转后的商户与终端，未跳转的商户更新 原商户号终端号 到 CHANNEL_MERCH_ID、CHANNEL_TERM_ID
    //motify by GJQ at 20180709  快捷消费  更新加密卡号与 屏蔽的明文卡号
    if( sTransCode[3] == '1' || sTransCode[3] == '2'  || !memcmp(sTransCode,"0AQ000",6) ) {
        snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET"
                " CHANNEL_MERCH_ID='%s', CHANNEL_TERM_ID='%s'"
                " ,RESP_CODE='%s', ISTRESP_CODE= '%s', RESP_DESC='%s', RESP_ID='%s'"
                " ,ORDER_URL='%s',LAST_MOD_TIME=sysdate"
                " ,AGENT_ID='%s',USER_CODE='%s'"
                ",CARD_ID='%s',CARD_TYPE='%s'"
                ",CARD_NO= '%s',CARD_NO_ENCRYPT= '%s', KEY_NAME = '%s', AGENT_ORGANIZATION = '%s'"
                " WHERE RRN='%s' and TRANS_DATE='%s'"
                , sChannelMerchId, sChannelTermId
                , sRespCode, sIstRespCode, sRespLog, sRespId
                , sOrderUrl, sAgentId, sUserCode
                , sCardBin, sCardType
                , sCardNo, sCardNoEnc, sKey, sAgentOrganization
                , sRrn, sTransDate);
    }
    else {
        snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET "
                " RESP_CODE='%s', RESP_DESC='%s', RESP_ID='%s' "
                " ,ORDER_URL='%s',MERCH_ORDER_NO='%s'"
                " ,fee=%.02f,fee_desc='%s',settle_date='%s',fee_flag='%s',channel_merch_id = '%s'"
                " ,LAST_MOD_TIME=sysdate"
                " ,AGENT_ID='%s',USER_CODE='%s',FEE_TYPE='%s',AGENT_ORGANIZATION = '%s'"
                " WHERE RRN='%s' and TRANS_DATE='%s'"
                , sRespCode, sRespLog, sRespId
                , sOrderUrl, sMerchOrderNo
                , dFee, sFeeDesc, sSettleDate, sFeeFlag, sChannelMerchId, sAgentId, sUserCode, sFeeType, sAgentOrganization
                , sRrn, sTransDate);
    }
    tLog(DEBUG, "sSqlStr = [%s]",sSqlStr);
    /*END modify by GuoJiaqing at 20171226  */
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "sSqlStr = [%s]",sSqlStr);
        tLog(ERROR, "更新交易date[%s]rrn[%s]交易结果失败.", sTransDate, sRrn);
        return -1;
    }

    tReleaseRes(pstRes);
    tLog(INFO, "更新交易date[%s]rrn[%s]结果成功.", sTransDate, sRrn);
    return 0;
}