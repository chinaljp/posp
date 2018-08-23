/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "postransdetail.h"

/******************************************************************************/
/*      函数名:     ChkHisOriginInfo()                                        */
/*      功能说明:   检查历史原流水                                            */
/*      输入参数:   cJSON *pstJson                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkHisOriginInfo(cJSON *pstJson, int *piFlag) {
    char sOrgTraceNo[6 + 1] = {0}, sAddData[100] = {0};
    char sRrn[12 + 1] = {0}, sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0}, sTransDate[8 + 1] = {0},sOTmpDate[4 + 1] = {0};
    char sTraceNo[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0};
    char sOMsgType[4 + 1] = {0}, sAcqInstId[8 + 1] = {0};
    int iCnt = -1;
    char sYear[4 + 1] = {0};
    double dTranAmt = 0L, dUnrefundAmt = 0L, dFee = 0;
    PosTransDetail stPosTransDetail;
    cJSON * pstTransJson;

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    //GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTmpDate); //motify by gjq at 20171220
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate); //交易日期 add by gjq at 20171220
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    dTranAmt /= 100;

    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));

    /* 获取原交易信息 */
    tLog(ERROR, "rrn[%s]", sORrn);

    //tLog(ERROR, "transcode  old [%s]", pstNetTran->sOldTransCode);
    /* 只检查隔日流水，不检查当日流水 */
    
    /*BEGIN add by gjq at  20171220 退货的原交易日期 只上送了4位日期，需要判断原交易日期的年份  并给原交易日期添加4位年份*/
    tLog(DEBUG,"退货上送的原交易日期sOTmpDate=[%s],本次交易日期sTransDate = [%s],sTransDate+4 = [%s] ",sOTmpDate,sTransDate,sTransDate+4);
    if(  memcmp(sOTmpDate,sTransDate+4,4) <= 0 ) {
        memcpy(sOTransDate,sTransDate,4);
        memcpy(sOTransDate+4,sOTmpDate,4);
        sOTransDate[9] = '\0';
    }
    else {
        memcpy(sYear,sTransDate,4); 
        sprintf(sOTransDate,"%d%s",atoi(sYear)-1,sOTmpDate);
    }
    tLog(INFO,"退货原交易日期为sOTransDate = [%s]",sOTransDate);
    /*END add by gjq at 20171220*/
    
    /*BEGIN 检索原交易失败时 导致原交易日期更新到数据库时为：月 日 4位 BUG修复*/
    DEL_KEY(pstTransJson, "o_trans_date");
    SET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    /*END 检索原交易失败时 导致原交易日期更新到数据库时为：月 日 4位 BUG修复*/
    
    if ((iCnt = FindAcctJonHis(&stPosTransDetail, sMerchId, sORrn, sOTransDate)) < 0) {
        ErrHanding(pstTransJson, "25", "原交易检查失败，无原交易流水，原商户号[%s] 原检索参考号[%s] 原交易日期[%s] iCnt [%d].", \
                sMerchId, sORrn, sOTransDate, iCnt);
        return -1;
    }

    /* 记录原始数据 */
    /*
    strcpy(pstNetTran->sOldBatchNo, stAcctJon.sBatchNo);
    sprintf(pstNetTran->sOldTermTraceNo, "%ld", stAcctJon.lTraceNo);
     */
    dUnrefundAmt = dTranAmt + stPosTransDetail.dRefundAmt;
    SET_DOU_KEY(pstTransJson, "refund_amt", dUnrefundAmt);
  /*DEL_KEY(pstTransJson, "o_trans_date");
    SET_STR_KEY(pstTransJson, "o_trans_date", stPosTransDetail.sTransDate);
  */
    GetOMsgType(sOMsgType, stPosTransDetail.sTransCode);
    strcat(sAcqInstId, "0004900");
    tStrCpy(sAcqInstId + 7, sMerchId + 3, 4);
    sprintf(sAddData, "%s%06s%10s%11s%11s"
            , sOMsgType
            , stPosTransDetail.sSysTrace
            //  , stPosTransDetail.sTransDate + 4
            , stPosTransDetail.sTransmitTime
            , sAcqInstId
            , "00049000000");
    //SET_JSON_KEY(pstJson, "his_trans_detail", stPosTransDetail);
    PrtAcctJournal(&stPosTransDetail);

    if (ChkHisAcctJon(pstTransJson, &stPosTransDetail) != 0) {
        return -1;
    }

    tLog(INFO, "退货原交易检查成功 原商户号[%s] 原检索参考号[%s] 原交易日期[%s]", \
            sMerchId, sORrn, sOTransDate);
    /* 退货交易取出原交易的 批次号、流水号、交易日期 */
    /*
        if ( CMP_E == memcmp( pstNetTran->sTransCode, "020100", TRANSCODE_LEN ) )
        {
            sprintf( pstNetTran->sAddData1, "%6s%06ld%4s"
                    , stAcctJon.sBatchNo, stAcctJon.lSysTrace, stAcctJon.sChannelTransDate+4 );
        }
     */

    SET_STR_KEY(pstTransJson, "90_data", sAddData);
    SET_STR_KEY(pstTransJson, "auth_code", stPosTransDetail.sAuthCode);
    SET_STR_KEY(pstTransJson, "channel_id", stPosTransDetail.sChannelId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", stPosTransDetail.sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", stPosTransDetail.sChannelTermId);
    SET_STR_KEY(pstTransJson, "trans_type", stPosTransDetail.sTransType);
    SET_STR_KEY(pstTransJson, "fee_type", stPosTransDetail.sFeeType);
    SET_STR_KEY(pstTransJson, "card_type", stPosTransDetail.sCardType);   
    dFee = stPosTransDetail.dFee*dTranAmt/stPosTransDetail.dAmount;
    tMake(&dFee, 2, 'u');
    SET_DOU_KEY(pstTransJson,"fee",dFee);
    SET_STR_KEY(pstTransJson, "fee_flag", "M");
    SET_STR_KEY(pstTransJson, "fee_desc", stPosTransDetail.sFeeDesc);
    /*退货改造o_amount*/
    SET_DOU_KEY(pstTransJson,"o_amount",stPosTransDetail.dAmount);
    /*add by gjq add 20180613 获取原交易的输入方式 用于判断交易使用的卡为IC卡 还是磁条卡*/
    SET_STR_KEY(pstTransJson, "o_input_mode", stPosTransDetail.sInputMode);
    
    return 0;
}

int ChkHisAcctJon(cJSON *pstTransJson, PosTransDetail *pstPosTransDetail) {
    double dTranAmt = 0L, dUnrefundAmt = 0L;
    char sOrgTraceNo[6 + 1] = {0};
    char sRrn[12 + 1] = {0};
    int iCnt = -1;
    char sTransDate[8 + 1] = {0}, sCardNo[19 + 1] = {0};

    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);
    dTranAmt /= 100;

    /* 退货只能隔日,当日不能退货 */
    if (!memcmp(pstPosTransDetail->sLogicDate, sTransDate, 8)) {
        ErrHanding(pstTransJson, "25", "原逻辑日期[%s]与当前逻辑日期[%s]一致,退货只能隔日.", pstPosTransDetail->sLogicDate, sTransDate);
        return -1;
    }

#if 0
    /* 当日只允许有一次退货交易 */
    if ((iCnt = DBGetRefundCnt(pstNetTran->sMerchId, pstNetTran->sTermId, sRrn)) < 0) {
        tErrLog(ERR, "获取商户[%s]终端[%s]Rrn[%s]退货交易次数失败.", \
                        pstNetTran->sMerchId, pstNetTran->sTermId, sRrn);
        SetRespCode(pstNetTran, "96", pcInstId);
        return -1;
    }


    if (iCnt > 0) {
        tLog(ERROR, "商户[%s]终端[%s]Rrn[%s]退货交易次数[%d]超限.", \
                        pstNetTran->sMerchId, pstNetTran->sTermId, sRrn, iCnt);
        SetRespCode(pstNetTran, "89", pcInstId);
        return -1;
    }
#endif
    if (pstPosTransDetail->sValidFlag[0] != '0' && pstPosTransDetail->sValidFlag[0] != '3') {
        ErrHanding(pstTransJson, "12", "原交易有效标志位[%c].", pstPosTransDetail->sValidFlag[0]);
        return -1;
    }

    /* 检查原交易是否成功 */
    if (memcmp(pstPosTransDetail->sRespCode, "00", 2)) {
        ErrHanding(pstTransJson, "12", "原交易失败,应答码[%s].", pstPosTransDetail->sRespCode);
        return -1;
    }
    /* 检查卡号 */
    if (strcmp(pstPosTransDetail->sCardNo, sCardNo)) {
        ErrHanding(pstTransJson, "63", "卡号不一致 原卡号[%s] 上送卡号[%s].", pstPosTransDetail->sCardNo, sCardNo);
        return -1;
    }
    /* 检查金额 */
    dUnrefundAmt = pstPosTransDetail->dAmount - pstPosTransDetail->dRefundAmt;
    tLog(DEBUG, "退款金额[%.10f], 未退金额[%.10f].", dTranAmt, dUnrefundAmt);

    /*
        if ( fabs(dTranAmt - dUnrefundAmt) > 0.00001 )
     */
    if (dTranAmt - dUnrefundAmt > 0.00001) {
        ErrHanding(pstTransJson, "13", "退款金额[%.02f]大于未退金额[%.02f].", dTranAmt, dUnrefundAmt);
        return -1;
    }

    return 0;
}



