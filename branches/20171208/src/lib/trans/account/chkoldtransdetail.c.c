#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "postransdetail.h"

void PrtAcctJournal(PosTransDetail *pstPosTransDetail);

void GetOMsgType(char *pcOMsgType, char *pcTransCode) {
    if (strstr("020000,020002,024102,T20000,024100,M20000,M20002,021000", pcTransCode) != NULL) {
        strcat(pcOMsgType, "0200");
    } else if (strstr("024000,024002", pcTransCode) != NULL) {
        strcat(pcOMsgType, "0100");
    }
}

void SetOldValue(cJSON * pstTransJson, PosTransDetail *pstPosTransDetail) {
    char sMerchId[16] = {0}, sCardNo[30] = {0};

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    PrtAcctJournal(pstPosTransDetail);
    tTrim(pstPosTransDetail->sTransCode);
    tTrim(pstPosTransDetail->sTransmitTime);
    if (strlen(sCardNo) == 0) {

        SET_STR_KEY(pstTransJson, "card_no", pstPosTransDetail->sCardNo);
    }
    SET_STR_KEY(pstTransJson, "auth_code", pstPosTransDetail->sAuthCode);

    SET_STR_KEY(pstTransJson, "o_rrn", pstPosTransDetail->sRrn);

    /* 浦发专用key =======*/
    SET_STR_KEY(pstTransJson, "o_pufa_rrn", pstPosTransDetail->sPufaRrn);
    SET_STR_KEY(pstTransJson, "o_sys_trace", pstPosTransDetail->sSysTrace);
    SET_STR_KEY(pstTransJson, "o_batch_no", pstPosTransDetail->sBatchNo);
    /* ==================*/
    
    SET_STR_KEY(pstTransJson, "o_trans_date", pstPosTransDetail->sTransDate);
    SET_STR_KEY(pstTransJson, "oo_rrn", pstPosTransDetail->sORrn);
    SET_STR_KEY(pstTransJson, "oo_trans_date", pstPosTransDetail->sOTransDate);

    SET_STR_KEY(pstTransJson, "channel_id", pstPosTransDetail->sChannelId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", pstPosTransDetail->sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", pstPosTransDetail->sChannelTermId);
    SET_STR_KEY(pstTransJson, "trans_type", pstPosTransDetail->sTransType);
    SET_STR_KEY(pstTransJson, "card_type", pstPosTransDetail->sCardType);
    SET_STR_KEY(pstTransJson, "card_bin", pstPosTransDetail->sCardId);
    SET_STR_KEY(pstTransJson, "term_sn", pstPosTransDetail->sTermSn);
    SET_STR_KEY(pstTransJson, "agent_id", pstPosTransDetail->sAgentId);
    SET_DOU_KEY(pstTransJson, "fee", pstPosTransDetail->dFee);
    SET_STR_KEY(pstTransJson, "settle_date", pstPosTransDetail->sSettleDate);
}
/******************************************************************************/
/*      函数名:     ChkOriginInfo()                                           */
/*      功能说明:   撤销检查原流水                                                */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkTransDetail(cJSON *pstJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};
    char sTransCode[9] = {0}, sCardNo[30] = {0}, sAddData[100] = {0};
    char sOMsgType[4 + 1] = {0}, sAcqInstId[11 + 1] = {0};
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    if (FindPosTrans(&stPosTransDetail, sMerchId, sTermId, sOTraceNo, sCardNo) != 0) {
        ErrHanding(pstTransJson, "25", "原交易检查失败，无原交易流水，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sOTraceNo, sMerchId, sTermId);
        return -1;
    }

    SetOldValue(pstTransJson, &stPosTransDetail);
    if (!memcmp(stPosTransDetail.sValidFlag, "1", 1)) {
        ErrHanding(pstTransJson, "C9", "原交易已撤销，原流水号[%s] 原商户号[%s] 原终端号[%s] 原卡号[%s].", \
                sTraceNo, sMerchId, sTermId, sCardNo);
        return -1;
    }

    GetOMsgType(sOMsgType, stPosTransDetail.sTransCode);
    strcat(sAcqInstId, "0004900");
    tStrCpy(sAcqInstId + 7, stPosTransDetail.sChannelMerchId + 3, 4);
    sprintf(sAddData, "%s%06s%10s%11s%11s"
            , sOMsgType
            , stPosTransDetail.sSysTrace
            , stPosTransDetail.sTransmitTime
            , sAcqInstId
            , "00049000000");
    SET_STR_KEY(pstTransJson, "90_data", sAddData);

    tLog(INFO, "原交易商户号[%s],终端号[%s],交易流水号[%s],清算日期[%s],交易时间[%s]", \
            sMerchId, sTermId, sOTraceNo, stPosTransDetail.sSettleDate, stPosTransDetail.sTransmitTime);
    return 0;
}

/* 消费冲正,撤销冲正,预授权冲正交易，检查原交易 */
int Chk0003TransDetail(cJSON *pstJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sAddData[100] = {0};
    char sOMsgType[4 + 1] = {0}, sAcqInstId[11 + 1] = {0};
    char sTransCode[9] = {0}, sCardNo[30] = {0};
    double dAmount = 0;
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    dAmount = dAmount / 100;

    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));

    if (FindPosTrans(&stPosTransDetail, sMerchId, sTermId, sTraceNo, sCardNo) != 0) {
        ErrHanding(pstTransJson, "00", "无原交易流水，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sTraceNo, sMerchId, sTermId);
        return -1;
    }

    SetOldValue(pstTransJson, &stPosTransDetail);
    if (!memcmp(stPosTransDetail.sValidFlag, "2", 1)) {
        ErrHanding(pstTransJson, "00", "原交易已冲正，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sTraceNo, sMerchId, sTermId);
        return -1;
    }

    if (memcmp(stPosTransDetail.sIstRespCode, "00", 2) && memcmp(stPosTransDetail.sRespCode, "XX", 2) ) {
        ErrHanding(pstTransJson, "00", "原交易失败[%s]，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                stPosTransDetail.sIstRespCode, sTraceNo, sMerchId, sTermId);
        return -1;
    }

    if (!DBL_EQ(stPosTransDetail.dAmount, dAmount)) {
        ErrHanding(pstTransJson, "25", "金额不等[%f:%f]，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                stPosTransDetail.dAmount, dAmount, sTraceNo, sMerchId, sTermId);
        return -1;
    }

    SET_STR_KEY(pstTransJson, "sys_trace", stPosTransDetail.sSysTrace);
    SET_STR_KEY(pstTransJson, "trans_time", stPosTransDetail.sTransTime);
    SET_STR_KEY(pstTransJson, "trans_date", stPosTransDetail.sTransDate);
    GetOMsgType(sOMsgType, stPosTransDetail.sTransCode);
    strcat(sAcqInstId, "0004900");
    tStrCpy(sAcqInstId + 7, stPosTransDetail.sChannelMerchId + 3, 4);
    sprintf(sAddData, "%s%06s%10s%11s%11s"
            , sOMsgType
            , stPosTransDetail.sSysTrace
            , stPosTransDetail.sTransmitTime
            , sAcqInstId
            , "00049000000");
    SET_STR_KEY(pstTransJson, "90_data", sAddData);
    tLog(INFO, "原交易商户号[%s],终端号[%s],交易流水号[%s],清算日期[%s],交易时间[%s]", \
            sMerchId, sTermId, sTraceNo, stPosTransDetail.sSettleDate, stPosTransDetail.sTransmitTime);
    return 0;
}

/* 预授权撤销和预授权完成 */
int ChkAuthTransDetail(cJSON *pstJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sOldDate[8 + 1] = {0}, sTransDate[8 + 1] = {0};
    char sTransCode[9] = {0}, sCardNo[30] = {0}, sAuthCode[6 + 1] = {0}, sOTransDate[8 + 1] = {0}, sAddData[100] = {0};
    char sCurDate[8 + 1] = {0}, sYear[4 + 1] = {0},sOTmpDate[4 + 1] = {0};
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_STR_KEY(pstTransJson, "auth_code", sAuthCode);
    //GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTmpDate);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    /* 授权码按位补零 */
    tTrim(sAuthCode);
    tStrPad(sAuthCode, 6, 0x00 | RIGHT_ALIGN);
    
    /**new version  add by gjq at 20180102*/
    tLog(DEBUG,"预授权撤销或预授权完成上送的原交易日期sOTmpDate=[%s],本次交易日期sTransDate = [%s],sTransDate+4 = [%s] ",
            sOTmpDate,sTransDate,sTransDate+4);
    if( memcmp(sOTmpDate,sTransDate+4,4) <= 0 ) {
        memcpy(sOTransDate,sTransDate,4);
        memcpy(sOTransDate+4,sOTmpDate,4);
        sOTransDate[9] = '\0';
    }
    else {
        memcpy(sYear,sTransDate,4);
        sprintf(sOTransDate,"%d%s",atoi(sYear)-1,sOTmpDate);
    }
    tLog(INFO,"原预授权交易日期sOTransDate = [%s]",sOTransDate);
    /**new version*/

    
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));
    if (FindAuthPosTrans(&stPosTransDetail, "B_POS_TRANS_DETAIL", sOTransDate, sAuthCode,sCardNo) != 0) {
        if (FindAuthPosTrans(&stPosTransDetail, "B_POS_TRANS_DETAIL_HIS", sOTransDate, sAuthCode,sCardNo) != 0) {
            ErrHanding(pstTransJson, "25", "原预授权交易检查失败，无原交易流水，原交易日期[%s] 原授权号[%s].", \
                sOTransDate, sAuthCode);
            return -1;
        }
    }

    PrtAcctJournal(&stPosTransDetail);
    SetOldValue(pstTransJson, &stPosTransDetail);
    if (!memcmp(stPosTransDetail.sValidFlag, "1", 1)) {
        ErrHanding(pstTransJson, "C9", "原交易已撤销，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sTraceNo, sMerchId, sTermId);
        return -1;
    }

    sprintf(sAddData, "%s%s%4s"
            , "000000", stPosTransDetail.sSysTrace, stPosTransDetail.sTransDate + 4);
    SET_STR_KEY(pstTransJson, "add_data", sAddData);
    tLog(INFO, "原交易商户号[%s],终端号[%s],交易流水号[%s],交易时间[%s]", \
            sMerchId, sTermId, stPosTransDetail.sTraceNo, stPosTransDetail.sTransmitTime);
    return 0;
}