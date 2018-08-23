#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 更新消费撤销原交易 */
int UpdPosTransDetail(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "交易失败[%s],不恢复原消费交易的有效标志.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "1") < 0) {
        ErrHanding(pstTransJson, "96", "更新原消费交易[%s:%s]有效标志[1]失败.", sOTransDate, sORrn);
        return -1;
    }
    tLog(DEBUG, "更新原消费交易[%s:%s]有效标志[1]成功.", sOTransDate, sORrn);
    return 0;
}

/* 更新消费撤销原交易 */
int UpdAuthTransDetail(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "交易失败[%s],不恢复原消费交易的有效标志.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "1") != 0) {
        if (UpdTransDetail("B_POS_TRANS_DETAIL_HIS", sOTransDate, sORrn, "1") != 0) {
            ErrHanding(pstTransJson, "96", "更新原消费交易[%s:%s]有效标志[1]失败.", sOTransDate, sORrn);
            return -1;
        }
    }
    tLog(DEBUG, "更新原消费交易[%s:%s]有效标志[1]成功.", sOTransDate, sORrn);
    return 0;
}

/* 更新消费冲正,预授权冲正原交易 */
int Upd0003PosTransDetail(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "交易失败[%s],不恢复原消费交易的有效标志.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "2") < 0) {
        ErrHanding(pstTransJson, "96", "更新原消费交易[%s:%s]有效标志[2]失败.", sOTransDate, sORrn);
        return -1;
    }
    tLog(DEBUG, "更新原消费交易[%s:%s]有效标志[2]成功.", sOTransDate, sORrn);
    return 0;
}

/* 更新消费撤销冲正原交易 */
int Upd0023PosTransDetail(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    char sOORrn[12 + 1] = {0}, sOOTransDate[8 + 1] = {0};
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "交易失败[%s],不恢复原消费交易的有效标志.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "2") < 0) {
        ErrHanding(pstTransJson, "96", "更新原撤销交易[%s:%s]有效标志[2]失败.", sOTransDate, sORrn);
        return -1;
    }
    tLog(INFO, "更新原撤销交易[%s:%s]有效标志[2]成功.", sOTransDate, sORrn);
    GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
    GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOOTransDate, sOORrn, "0") < 0) {
        ErrHanding(pstTransJson, "96", "更新原消费交易[%s:%s]有效标志[0]失败.", sOOTransDate, sOORrn);
        return -1;
    }
    tLog(INFO, "更新原消费交易[%s:%s]有效标志[0]成功.", sOOTransDate, sOORrn);
    return 0;
}

/******************************************************************************/
/*      函数名:     UpdJournalResv()                                          */
/*      功能说明:   冲正类交易更新原交易流水                                  */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UpdJournalResv(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sOOTransDate[8 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sOORrn[12 + 1] = {0};
    char sOOOTransDate[8 + 1] = {0}, sOOORrn[12 + 1] = {0};

    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);
    GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
    /* 获取原交易信息 */

    if (NULL != strstr("020003,020023,T20003,024003,024103,024023,024123,M20003,M20023", sTransCode)) {
        tStrCpy(sTrace, sTraceNo, 6);
    } else {
        tStrCpy(sTrace, sOTraceNo, 6);
    }
    tLog(ERROR, "trace no[%s]", sTrace);
#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* 更新原交易流水的有效标志valid_flag */
    if (UpdOrgJon(sMerchId, sTermId, sTrace, '2') != 0) {
        ErrHanding(pstTransJson, "96", "原交易更新标志失败，无原交易流水，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sTrace, sMerchId, sTermId);
        return -1;
    }
    //消费撤销冲正,预授权完成撤销冲正恢复原交易有效标志
    if (NULL != strstr("020023,024123,M20023", sTransCode)) {
        if (UpdOrgJonRecover(sOORrn) != 0) {
            ErrHanding(pstTransJson, "96", "原消费交易恢复有效标志失败，rrn[%s].", \
                sOORrn);
            return -1;
        }
    }

    //预授权完成撤销冲正恢复原预授权交易autho_flag为1
    if (NULL != strstr("024123", sTransCode)) {

        if (GetOrgRrnAndTransDate(sOORrn, sOOTransDate, sOOORrn, sOOOTransDate) != 0) {
            ErrHanding(pstTransJson, "96", "查找原原RRN[%s],TRANS_DATE[%s]失败.", \
                sOORrn, sOOTransDate);
            return -1;
        }
        if (UpdOrgJonAuthRecover(sOOORrn, sOOOTransDate, '1') != 0) {
            ErrHanding(pstTransJson, "96", "原消费交易恢复有效标志失败，rrn[%s].", \
                sOORrn);
            return -1;
        }
    }

    //预授权撤销冲正恢复原预授权有效标志
    if (0 == memcmp(sTransCode, "024023", 6)) {
        if (UpdOrgJonAuthUndo(sOORrn, sOOTransDate, '0') != 0) {
            ErrHanding(pstTransJson, "96", "原消费交易恢复有效标志失败，rrn[%s].", \
                sOORrn);
            return -1;
        }
    }

    /*预授权完成冲正恢复原预授权状态标识*/
    if (0 == memcmp(sTransCode, "024103", 6)) {
        GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
        GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);
        if (UpdOrgJonAuthRecover(sOORrn, sOOTransDate, '0') != 0) {
            ErrHanding(pstTransJson, "96", "原消费交易恢复有效标志失败，rrn[%s] date[%s].", \
                sOORrn, sOOTransDate);
            return -1;
        }
    }

    tLog(INFO, "原交易更新标志成功，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sTrace, sMerchId, sTermId);

    return 0;
}


/******************************************************************************/
/*      函数名:     UpdJournalUndo()                                          */
/*      功能说明:   撤销类交易更新原交易流水                                  */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UpdJournalUndo(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sOORrn[12 + 1] = {0}, sOOTransDate[8 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};

    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    /* 获取原交易信息 */

    tStrCpy(sTrace, sOTraceNo, 6);
    tLog(ERROR, "trace no[%s]", sTrace);


#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* 更新原交易流水的有效标志valid_flag */
    if (UpdOrgJon(sMerchId, sTermId, sTrace, '1') != 0) {
        ErrHanding(pstTransJson, "96", "原交易更新标志失败，无原交易流水，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sTrace, sMerchId, sTermId);
        return -1;
    }
    /*预授权完成撤销恢复原预授权状态标识*/
    if (0 == memcmp(sTransCode, "024102", 6)) {
        GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
        GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);
        if (UpdOrgJonAuthRecover(sOORrn, sOOTransDate, '0') != 0) {
            ErrHanding(pstTransJson, "96", "原消费交易恢复有效标志失败，rrn[%s] date[%s].", \
                sOORrn, sOOTransDate);
            return -1;
        }
    }

    tLog(INFO, "原交易更新标志成功，原流水号[%s] 原商户号[%s] 原终端号[%s].", \
                sTrace, sMerchId, sTermId);
    return 0;
}


/******************************************************************************/
/*      函数名:     UpdJournalAuth()                                          */
/*      功能说明:   预授权类交易更新原交易流水                                  */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UpdJournalAuth(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sHisTransDate[8 + 1] = {0}, sHisRrn[12 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};

    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "his_trans_date", sHisTransDate);
    GET_STR_KEY(pstTransJson, "his_rrn", sHisRrn);
    /* 获取原交易信息 */

    tLog(ERROR, "sHisRrn[%s] sHisTransDate[%s]", sHisRrn, sHisTransDate);


#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* 更新原交易流水的有效标志valid_flag */
    if (UpdOrgJonAuth(sHisRrn, sHisTransDate, '1') != 0) {
        ErrHanding(pstTransJson, "96", "原交易更新标志失败，无原交易流水，原rrn[%s] 原交易日期[%s].", \
                sHisRrn, sHisTransDate);
        return -1;
    }

    tLog(INFO, "原交易更新标志成功，原rrn[%s] 原交易日期[%s].", \
                sHisRrn, sHisTransDate);
    return 0;
}


/******************************************************************************/
/*      函数名:     UpdJournalAuthUndo()                                          */
/*      功能说明:   预授权类交易更新原交易流水                                  */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UpdJournalAuthUndo(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sHisTransDate[8 + 1] = {0}, sHisRrn[12 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};

    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "his_trans_date", sHisTransDate);
    GET_STR_KEY(pstTransJson, "his_rrn", sHisRrn);
    /* 获取原交易信息 */

    tLog(ERROR, "sHisRrn[%s] sHisTransDate[%s]", sHisRrn, sHisTransDate);


#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* 更新原交易流水的有效标志valid_flag */
    if (UpdOrgJonAuthUndo(sHisRrn, sHisTransDate, '1') != 0) {
        ErrHanding(pstTransJson, "96", "原交易更新标志失败，无原交易流水，原rrn[%s] 原交易日期[%s].", \
                sHisRrn, sHisTransDate);
        return -1;
    }

    tLog(INFO, "原交易更新标志成功，原rrn[%s] 原交易日期[%s].", \
                sHisRrn, sHisTransDate);
    return 0;
}


/******************************************************************************/
/*      函数名:     UpdJournalHis()                                          */
/*      功能说明:   退货类交易更新原交易流水                                  */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int UpdJournalHis(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sOTransDate[8 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};
    char sORrn[12 + 1] = {0};
    double dUnrefundAmt = 0L;
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_DOU_KEY(pstTransJson, "refund_amt", dUnrefundAmt);

    /* 获取原交易信息 */


#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* 更新原交易流水的有效标志valid_flag */
    if (UpdOrgJonHis(sOTransDate, sORrn, '3', dUnrefundAmt) != 0) {
        ErrHanding(pstTransJson, "96", "原交易更新标志失败，无原交易参考号[%s]，原交易日期[%s] 总退款金额[%lf].", \
                sORrn, sOTransDate, dUnrefundAmt);
        return -1;
    }

    tLog(INFO, "原交易更新标志成功，原交易参考号[%s]，原交易日期[%s] 总退款金额[%lf].", \
                sORrn, sOTransDate, dUnrefundAmt);

    return 0;
}
