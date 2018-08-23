#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "postransdetail.h"

/******************************************************************************/
/*      函数名:     ChkOriginAuth()                                           */
/*      功能说明:   检查预授权原流水                                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int ChkOriginAuth(cJSON *pstJson, int *piFlag) {
    PosTransDetail stPosTransDetail;
    char sTrace[7] = {0}, sOTraceNo[6 + 1] = {0}, sSysTrace[6 + 1] = {0};
    char sOldDate[8 + 1] = {0}, sTransCode[6 + 1] = {0}, sAuthCode[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sOTransDate[8 + 1] = {0}, sCardNo[19 + 1] = {0}, sAddData[100] = {0};
    char sTraceNo[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0};
    int iRet = 0;
    cJSON * pstTransJson;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstTransJson, "auth_code", sAuthCode);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);

    //pstTranData->cJonFlag = 0;
    /*
     if ( pstNetTran->cSafFlag == '1' )
     {
         tErrLog( INFO, "SAF无需检查原交易." );
         return 0;
     }
     */
    memset(&stPosTransDetail, 0x00, sizeof (stPosTransDetail));

    /* 获取原交易信息 */
    tStrCpy(sTrace, sOTraceNo, 6);


    /* pstPosTransDetail.lTraceNo,pstPosTransDetail.lSysTrace, pstPosTransDetail.lChannelTrace,  pstPosTransDetail.sRrn  */
    /* 当原交易检索失败时，当日流水需要原交易记录 */
    //tStrCpy(stPosTransDetail.sTraceNo, sTrace, 6);
    //tStrCpy(stPosTransDetail.sSysTrace, pcSysTrace, 6);

    //if ( CMP_E == memcmp( pstNetTran->pcTransCode, "024100", TRANSCODE_LEN ) )
    if (0 == memcmp(sTransCode, "024100", 6)) {
        tStrCpy(stPosTransDetail.sTransCode, "024000", 6);
    } else {
        tStrCpy(stPosTransDetail.sTransCode, "024000", 6);
    }
    tLog(INFO, "trancode=[%s].", sTransCode);
    //pstTranData->stHisAcctJon = pstPosTransDetail;

    /* 授权码按位补零 */
    tTrim(sAuthCode);
    tStrPad(sAuthCode, 6, 0x00 | RIGHT_ALIGN);

    /* 预授权完成、预授权撤销只能针对一个月以内的预授权交易 */
    tStrCpy(sOldDate, sTransDate, 4);
    strcat(sOldDate, sOTransDate);
    tLog(INFO, "sOldDate[%s] sTransDate[%s]", sOldDate, sTransDate);
    if (tDateCount(sOldDate, sTransDate) > 30) {
        ErrHanding(pstTransJson, "96", "无法对一个月以外的预授权交易进行预授权完成或预授权撤销操作！原预授权交易日期[%s]"
                , sOTransDate);
        return -1;
    }

    /* 预授权完成、预授权撤销可能隔批次，需查找流水表、历史流水表 */
    iRet = FindRevAuthJon(&stPosTransDetail, sCardNo, sOTransDate, sAuthCode);
    if (iRet < 0) {
        iRet = FindRevAuthJonHis(&stPosTransDetail, sCardNo, sOTransDate, sAuthCode);
        //pstTranData->cJonFlag = 1;
    }

    if (iRet < 0) {
        ErrHanding(pstTransJson, "25", "原交易[%s]检查失败，无原交易流水，原卡号[%s] 原交易日期[%s] 原授权号[%s].", \
                stPosTransDetail.sTransCode, sCardNo, sOTransDate, sAuthCode);
        return -1;
    }

    tLog(DEBUG, "原交易[%s]原卡号[%s] 原交易日期[%s] 原授权号[%s] 原流水号[%ld] 原批次号[%s].", \
        stPosTransDetail.sTransCode, sCardNo, sOTransDate, sAuthCode, \
        stPosTransDetail.sSysTrace, stPosTransDetail.sBatchNo);

    PrtAcctJournal(&stPosTransDetail);

    SET_STR_KEY(pstTransJson, "o_trans_code", stPosTransDetail.sTransCode);
    SET_STR_KEY(pstTransJson, "o_rrn", stPosTransDetail.sORrn);

    sprintf(sAddData, "%s%s%4s"
            , "000000", stPosTransDetail.sSysTrace, stPosTransDetail.sTransDate + 4);
    SET_STR_KEY(pstTransJson, "add_data", sAddData);
    /* 检查原预授权交易是否有效 */
    if ((iRet = ChkAuthJon(pstTransJson, &stPosTransDetail)) != 0) {
        if (iRet == -1)
            ErrHanding(pstTransJson, "96", "原交易检查失败");
        else if (iRet == -2)

            ErrHanding(pstTransJson, "94", "原交易检查失败");
            //Change By LiuZe 2013-09-11 08:43
            //修改金额检查错误时返回码为64
        else if (iRet == -3)

            ErrHanding(pstTransJson, "64", "原交易检查失败");
            //End By 2013-09-11 08:43
            //Change by LiuZe 2014-01-10 14:14
            //预授权完成金额不得大于预授权金额
        else if (iRet == -4)

            ErrHanding(pstTransJson, "C8", "原交易检查失败");
        //End By 2014-01-10 14:14
        return -1;
    }

    SET_STR_KEY(pstTransJson, "channel_id", stPosTransDetail.sChannelId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", stPosTransDetail.sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_term_id", stPosTransDetail.sChannelTermId);
    SET_STR_KEY(pstTransJson, "his_trans_date", stPosTransDetail.sTransDate);
    SET_STR_KEY(pstTransJson, "his_rrn", stPosTransDetail.sRrn);
    SET_STR_KEY(pstTransJson, "trans_type", stPosTransDetail.sTransType);
    //pstTranData->stHisAcctJon = pstPosTransDetail;
    tLog(INFO, "原交易检查成功 原商户号[%s] 原终端号[%s] 原交易流水号[%ld] 原清算日期[%s]", \
            sMerchId, sTermId, sTraceNo, stPosTransDetail.sSettleDate);
    return 0;   
}

int ChkAuthJon(cJSON *pstTransJson, PosTransDetail *pstPosTransDetail) {
    double dTranAmt;
    char sTransCode[6 + 1] = {0}, sInputMode[3 + 1] = {0}, sCardNo[19 + 1] = {0};
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_DOU_KEY(pstTransJson, "amount", dTranAmt);

    dTranAmt /= 100;
    /* 冲正可多次上送，不检查有效标志。失败交易不会冲正。 */
    if (0 == memcmp(sTransCode, "020300", 6) ||
            0 == memcmp(sTransCode, "M20300", 6)) {
        return 0;
    }

    /* 检查原交易是否有效 */
    if (pstPosTransDetail->sValidFlag[0] != '0') {
        tLog(ERROR, "原交易授权码[%s],有效标志位[%s]."
                , pstPosTransDetail->sAuthCode, pstPosTransDetail->sValidFlag);
        return -1;
    }

    /* 不能对已完成的预授权交易做撤销 */
    if ((0 == memcmp(sTransCode, "024002", 6))
            && ('1' == pstPosTransDetail->sAuthoFlag[0])) {
        tLog(ERROR, "原交易授权码[%s],预授权已完成, 无法进行撤销!"
                , pstPosTransDetail->sAuthCode);
        return -1;
    }

    /* 检查原交易是否成功 */
    if (memcmp(pstPosTransDetail->sRespCode, "00", 2)) {
        tLog(ERROR, "原交易[%s]失败,流水号[%s],应答码[%s].", \
                        pstPosTransDetail->sTransCode, pstPosTransDetail->sTraceNo, pstPosTransDetail->sRespCode);
        return -1;
    }

    /* 检查原预授权交易是否已完成 */
    if ((0 == memcmp(sTransCode, "024100", 6) || 0 == memcmp(sTransCode, "027100", 6))
            &&('1' == pstPosTransDetail->sAuthoFlag[0])) {
        tLog(ERROR, "原预授权交易[%s]已完成! ", pstPosTransDetail->sAuthCode);
        return -2;
    }

    /* 手工输入时检查卡号 */
    if ((0 == memcmp(sInputMode, "011", 3))
            || (0 == memcmp(sInputMode, "012", 3))) {
        tTrim(pstPosTransDetail->sCardNo);
        tTrim(sCardNo);
        if (0 != strcmp(pstPosTransDetail->sCardNo, sCardNo)) {
            tLog(ERROR, "卡号不一致 原卡号[%s] 上送卡号[%s].", pstPosTransDetail->sCardNo, sCardNo);
            return -1;
        }
    }

    /* 检查金额 */

    //Change By LiuZe 2013-09-11 08:43
    //修改金额检查错时返回码为64
    //修改非预授权完成交易时为金额是否一致
    if (memcmp(sTransCode, "024100", 6)) {
        if (!((dTranAmt - pstPosTransDetail->dAmount) < 0.00001 && ((dTranAmt - pstPosTransDetail->dAmount) > -0.00001))) {
            tLog(ERROR, "金额不一致 原金额[%.02f] 上送金额[%.02f].", pstPosTransDetail->dAmount, dTranAmt);
            return -3;
        }
#if 0
        if ((dTranAmt - pstPosTransDetail->dAmount) > 0.00001) {
            tErrLog(ERR, "预授权完成金额超限 原金额[%.02f] 上送金额[%.02f].", pstPosTransDetail->dAmount, dTranAmt);
            return -3;
        }
#endif
    }//Change By LiuZe 2014-01-10 14:07
        //修改预授权完成金额小于等于预授权金额
    else {
        if ((dTranAmt - pstPosTransDetail->dAmount) > 0.00001) {
            tLog(ERROR, "预授权完成金额超限 原金额[%.02f] 上送金额[%.02f].", pstPosTransDetail->dAmount, dTranAmt);
            return -4;
        }

    }

    /*
    if ( ( dTranAmt - pstPosTransDetail->dAmount ) > 0.00001 )
    {
        tErrLog( ERR, "金额超限 原金额[%.02f] 上送金额[%.02f].", pstPosTransDetail->dAmount, dTranAmt );
        return -1;
    }
     */
    //End Change by 2013-09-11 08:43

    return 0;
}

