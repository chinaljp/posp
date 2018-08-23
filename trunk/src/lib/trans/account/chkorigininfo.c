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
#if 0

void GetOMsgType(char *pcOMsgType, char *pcTransCode) {
    if (strstr("020000,020002,024102,T20000,024100,M20000,M20002,021000", pcTransCode) != NULL) {
        strcat(pcOMsgType, "0200");
    } else if (strstr("024000,024002", pcTransCode) != NULL) {
        strcat(pcOMsgType, "0100");
    }
}
#endif
/******************************************************************************/
/*      函数名:     ChkOriginInfo()                                           */
/*      功能说明:   撤销检查原流水                                                */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkOriginInfo(cJSON *pstJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sOMsgType[4 + 1] = {0}, sAcqInstId[11 + 1] = {0};
    char sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sAddData[100] = {0};
    int iRet = 0;
    char sTransCode[9] = {0}, sCardNo[30] = {0};
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);

#if 0
    /* 预授权完成撤销不允许操作 */
    if ((CMP_E == memcmp(pstNetTran->sOldTransCode, "024100", strlen(pstNetTran->sOldTransCode)))
            && (CMP_E == memcmp(pstNetTran->pcTransCode, "020200", TRANSCODE_LEN))) {
        SetRespCode(pstNetTran, "40", pstTranData->stSysParam.sInstId);
        return -1;
    }
#endif
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));

    /* 获取原交易信息 */
    if (NULL != strstr("020003,020023,T20003,024003,024103,024023,024123,M20003,M20023", sTransCode)) {
        //tStrCpy(stPosTransDetail.sTraceNo, sTraceNo, 6);
    } else {
        tStrCpy(sTraceNo, sOTraceNo, 6);
    }
    tLog(ERROR, "old_trace no[%s]", sOTraceNo);
    tLog(ERROR, "trace no[%s]", sTraceNo);

    if (FindRevAcctJon(&stPosTransDetail, sMerchId, sTermId, sTraceNo) != 0) {
        ErrHanding(pstTransJson, "25", "原交易检查失败，无原交易流水，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sTraceNo, sMerchId, sTermId);
        return -1;
    }

    PrtAcctJournal(&stPosTransDetail);
    tTrim(stPosTransDetail.sTransCode);
    if (NULL != strstr("020400", sTransCode)) {
        SET_STR_KEY(pstTransJson, "o_trans_code", stPosTransDetail.sTransCode);
        SET_STR_KEY(pstTransJson, "resp_code", stPosTransDetail.sRespCode);
        DEL_KEY(pstTransJson, "trans_time");
        DEL_KEY(pstTransJson, "trans_date");
        SET_STR_KEY(pstTransJson, "trans_time", stPosTransDetail.sTransTime);
        SET_STR_KEY(pstTransJson, "trans_date", stPosTransDetail.sTransDate);
    }

    tTrim(stPosTransDetail.sTransCode);
    //tStrCpy(pstNetTran->sOOldTransCode, stPosTransDetail.sOTransCode, OLDTRANSCODE_LEN);
    tTrim(stPosTransDetail.sTransmitTime);
    //StrCpy(pstNetTran->sChannelTransDate, stPosTransDetail.sTransmitTime, 4);
    //tStrCpy(pstNetTran->sChannelTransTime, stPosTransDetail.sTransmitTime + 4, 6);

    /* 检查原交易是否有效 */
    if ((iRet = ChkAcctJon(pstTransJson, &stPosTransDetail)) != 0) {
        if (iRet == -1)
            ErrHanding(pstTransJson, "C9", "原交易检查失败.");
        else if (iRet == -2)
            ErrHanding(pstTransJson, "CA", "原交易检查失败.");
        else if (iRet == -3)
            ErrHanding(pstTransJson, "12", "原交易检查失败.");
        else if (iRet == -4)
            ErrHanding(pstTransJson, "14", "原交易检查失败.");
        else if (iRet == -5)
            ErrHanding(pstTransJson, "13", "原交易检查失败.");
            /*
             * modify by gaofeng 2016/05/11
             * T+0清算
             * */
        else if (iRet == -6)
            ErrHanding(pstTransJson, "12", "原交易检查失败.");

        //SetRespCode(pstNetTran, "96", pstTranData->stSysParam.sInstId);
        return -1;
    }

    //pstTranData->stHisAcctJon = stPosTransDetail;??????????????????????????????
    tLog(INFO, "原交易检查成功 原商户号[%s] 原终端号[%s] 原交易流水号[%s] 原清算日期[%s] 原交易时间[%s]", \
            sMerchId, sTermId, sTraceNo, stPosTransDetail.sSettleDate, stPosTransDetail.sTransmitTime);
    //tLog(INFO, "date[%s],time[%s]", pstNetTran->sChannelTransDate, pstNetTran->sChannelTransTime);
    if (strlen(sCardNo) == 0) {
        SET_STR_KEY(pstTransJson, "card_no", stPosTransDetail.sCardNo);
    }
    SET_STR_KEY(pstTransJson, "rrn", stPosTransDetail.sRrn);
    SET_STR_KEY(pstTransJson, "auth_code", stPosTransDetail.sAuthCode);

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
    if (NULL != strstr("020003,020023,T20003,024003,024103,024023,024123,M20003,M20023", sTransCode)) {
        DEL_KEY(pstTransJson, "sys_trace");
        DEL_KEY(pstTransJson, "trans_time");
        DEL_KEY(pstTransJson, "trans_date");
        SET_STR_KEY(pstTransJson, "sys_trace", stPosTransDetail.sSysTrace);
        SET_STR_KEY(pstTransJson, "trans_time", stPosTransDetail.sTransTime);
        SET_STR_KEY(pstTransJson, "trans_date", stPosTransDetail.sTransDate);
        SET_STR_KEY(pstTransJson, "o_rrn", stPosTransDetail.sRrn);
    }
    if (NULL != strstr("020023,024102,024103,024023,024123,M20023", sTransCode)) {
        SET_STR_KEY(pstTransJson, "oo_rrn", stPosTransDetail.sORrn);
        SET_STR_KEY(pstTransJson, "oo_trans_date", stPosTransDetail.sOTransDate);
    }

    if (NULL != strstr("024102,020002,M20002", sTransCode)) {
        SET_STR_KEY(pstTransJson, "his_rrn", stPosTransDetail.sRrn);
        SET_STR_KEY(pstTransJson, "his_trans_date", stPosTransDetail.sTransDate);
    }

    SET_STR_KEY(pstTransJson, "90_data", sAddData);
    SET_STR_KEY(pstTransJson, "channel_id", stPosTransDetail.sChannelId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", stPosTransDetail.sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", stPosTransDetail.sChannelTermId);
    SET_STR_KEY(pstTransJson, "trans_type", stPosTransDetail.sTransType);
    tLog(DEBUG, " 原交易[%s], rrn[%s]", sAddData, stPosTransDetail.sRrn);

    return 0;
}

int ChkAcctJon(cJSON *pstTransJson, PosTransDetail *pstPosTransDetail) {
    double dTranAmt;
    char sTransCode[6 + 1] = {0}, sInputMode[3 + 1] = {0}, sCardNo[19 + 1] = {0};

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    dTranAmt = dTranAmt / 100;
    /*
     * modify by gaofeng 2016/05/11
     * T+0已经清算的交易不能做冲正或撤销交易
     * */
    if (pstPosTransDetail->sSettleFlag[0] == '2') {
        tLog(ERROR, "原交易已T+0清算[%c].", pstPosTransDetail->sSettleFlag);
        return -6;
    }

    /* 冲正可多次上送，不检查有效标志。失败交易不会冲正。 */
    if (NULL != strstr("020003,020023,T20003,024003,024103,024023,024123,M20003,M20023", sTransCode)) {
        return 0;
    }

    /* 检查原交易是否有效 
    if (pstPosTransDetail->cValidFlag != '0') {
        tErrLog(ERR, "原交易有效标志位[%c].", pstPosTransDetail->cValidFlag);
        return -1;
    }
     */

    /* 检查原交易是否已撤销*/
    if (pstPosTransDetail->sValidFlag[0] == '1') {
        tLog(ERROR, "原交易有效标志位[%s].", pstPosTransDetail->sValidFlag);
        return -1;
    }

    /* 检查原交易是否已冲正*/
    if (pstPosTransDetail->sValidFlag[0] == '2') {
        tLog(ERROR, "原交易有效标志位[%c].", pstPosTransDetail->sValidFlag[0]);
        return -2;
    }

    /* 检查原交易是否成功 */
    if (memcmp(sTransCode, "020400", 6)) {
        if (memcmp(pstPosTransDetail->sRespCode, "00", 2)) {
            tLog(ERROR, "原交易[%s]失败,流水号[%s],应答码[%s].", \
                        pstPosTransDetail->sTransCode, pstPosTransDetail->sTraceNo, pstPosTransDetail->sRespCode);
            return -3;
        }
    }

    /* 手工输入时检查卡号 */
    if ((0 == memcmp(sInputMode, "011", 3))
            || (0 == memcmp(sInputMode, "012", 3))) {
        tTrim(pstPosTransDetail->sCardNo);
        tTrim(sCardNo);
        if (0 != strcmp(pstPosTransDetail->sCardNo, sCardNo)) {
            tLog(ERROR, "卡号不一致 原卡号[%s] 上送卡号[%s].", pstPosTransDetail->sCardNo, sCardNo);
            return -4;
        }
    }

    /* 检查金额 */

    if (fabs(dTranAmt - pstPosTransDetail->dAmount) > 0.00001) {
        tLog(ERROR, "金额不一致 原金额[%.02f] 上送金额[%.02f].", pstPosTransDetail->dAmount, dTranAmt);
        return -5;
    }

    return 0;
}

void PrtAcctJournal(PosTransDetail *pstPosTransDetail) {
    const char sCardType[][10] = {"借记卡", "信用卡"};

    tLog(DEBUG, "交易码[%s] 卡号[%s] 卡类型[%s][%c] 交易金额[%.2f]\n"
            "交易日期[%8s]     交易时间[%6s]     清算日期[%8s]     系统流水号[%s]\n"
            "交易有效标志[%s] 对账标志[%s] 预授权完成标志[%s]\n"
            "系统日期[%s] 商户标识[%s] 终端标识[%s]\n"
            "交易手续费[%.02f] 原交易rrn[%s] "
            "终端流水号[%s] 收单机构[%s] 发卡机构[%s]\n",
            pstPosTransDetail->sTransCode, pstPosTransDetail->sCardNo,
            pstPosTransDetail->sCardId, sCardType[pstPosTransDetail->sCardType[0] - '0'], pstPosTransDetail->dAmount,\
                    pstPosTransDetail->sTransDate, pstPosTransDetail->sTransTime, \
                    pstPosTransDetail->sSettleDate, pstPosTransDetail->sSysTrace, \
                    pstPosTransDetail->sValidFlag, pstPosTransDetail->sCheckFlag, pstPosTransDetail->sAuthoFlag, \
                    pstPosTransDetail->sLogicDate, pstPosTransDetail->sMerchId, pstPosTransDetail->sTermId,\
                    pstPosTransDetail->dFee, pstPosTransDetail->sORrn, \
                    pstPosTransDetail->sTraceNo, pstPosTransDetail->sAcqId, pstPosTransDetail->sIssId);
}