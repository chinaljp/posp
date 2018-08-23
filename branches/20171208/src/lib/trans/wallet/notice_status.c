/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <zlog.h>
#include "t_db.h"
#include "t_redis.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_macro.h"


/* 更新二维码交易 */
int UpdQrTrans(char *pcStatus, char *pcRrn, char *pcTransDate) {
    char sSqlStr[1024] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_INLINE_TARNS_DETAIL set settle_flag='%s'"
            "where trans_date='%s' and rrn='%s'", pcStatus, pcTransDate, pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    if (0 == tGetAffectedRows()) {
        tLog(ERROR, "钱包通知成功,更新交易[%s:%s]结算状态[%s],失败.", pcTransDate, pcRrn, pcStatus);
        tReleaseRes(pstRes);
        return 0;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "钱包通知成功,更新交易[%s:%s]结算状态[%s].", pcTransDate, pcRrn, pcStatus);
    return 0;
}

//pos
int UpdPosTrans(char *pcStatus, char *pcRrn, char *pcTransDate) {
    char sSqlStr[1024] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);
    snprintf(sSqlStr, sizeof (sSqlStr), "update B_POS_TRANS_DETAIL set settle_flag='%s'"
            "where trans_date='%s' and rrn='%s'", pcStatus, pcTransDate, pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        return -1;
    }
    if (0 == tGetAffectedRows()) {
        tLog(ERROR, "钱包通知成功,更新交易[%s:%s]结算状态[%s],失败.", pcTransDate, pcRrn, pcStatus);
        tReleaseRes(pstRes);
        return 0;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "钱包通知成功,更新交易[%s:%s]结算状态[%s].", pcTransDate, pcRrn, pcStatus);
    return 0;
}

/* 入账 */
int RechargeTransStatus(cJSON *pstTransJson, int *piFlag) {
    char sRrn[12 + 1] = {0}, sRespCode[32 + 1] = {0}, sTransDate[8 + 1] = {0}, sTransType[16 + 1] = {0};
    char sRespLog[4096 + 1] = {0};

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "resp_desc", sRespLog);
    if (!memcmp(sRespCode, "00", 2)) {
        if (!memcmp(sTransType, "POS", 3)) {
            if (UpdPosTrans("M", sRrn, sTransDate) < 0) {
                ErrHanding(pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog);
                return -1;
            }
        } else {
            if (UpdQrTrans("M", sRrn, sTransDate) < 0) {
                ErrHanding(pstTransJson, "96", "无rrn[%s]的二维码交易流水[%s].", sRrn, sRespLog);
                return -1;
            }
        }
    } else if (!memcmp(sRespCode, "03010027", 8) || !memcmp(sRespCode, "03010028", 8)) {
        tLog(ERROR, "交易[%s]重复入账,默认成功.", sRrn);
        if (!memcmp(sTransType, "POS", 3)) {
            if (UpdPosTrans("M", sRrn, sTransDate) < 0) {
                ErrHanding(pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog);
                return -1;
            }
        } else {
            if (UpdQrTrans("M", sRrn, sTransDate) < 0) {
                ErrHanding(pstTransJson, "96", "无rrn[%s]的二维码交易流水[%s].", sRrn, sRespLog);
                return -1;
            }
        }
    } else {
        ErrHanding(pstTransJson, "96", "交易[%s]入账失败[%s].", sRrn, sRespLog);
    }
    return 0;
}


/* 日结 */
int CashTransStatus(cJSON *pstTransJson, int *piFlag) {
    char sRrn[12 + 1] = {0}, sRespCode[32 + 1] = {0}, sTransDate[8 + 1] = {0}, sTransType[16 + 1] = {0};
    char sRespLog[512 + 1] = {0};

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "resp_desc", sRespLog);
    if (!memcmp(sRespCode, "00", 2)) {
        if (UpdPosTrans("Y", sRrn, sTransDate) < 0) {
            ErrHanding(pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog);
            return -1;
        }
    } else if (!memcmp(sRespCode, "03010027", 8)) {
        tLog(ERROR, "交易[%s]重复充值,默认成功.", sRrn);
        if (UpdPosTrans("Y", sRrn, sTransDate) < 0) {
            ErrHanding(pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog);
            return -1;
        }
    } else {
        ErrHanding(pstTransJson, "96", "交易[%s]日结失败[%s].", sRrn, sRespLog);
    }
    return 0;
}