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
#include "settle_date.h"
#include "t_macro.h"
#include "status.h"

/* 获取银行卡结算日 */
int GetCardSettleDate(cJSON *pstJson, int *piFlag) {
    char sDate[8 + 1] = {0}, sRespCode[2 + 1] = {0},sChannelSettleDate[8 + 1]={0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
#if 0
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "交易失败[%s],不计算结算日.", sRespCode);
        return 0;
    }
#endif
    GET_STR_KEY(pstTransJson, "channel_settle_date", sChannelSettleDate);
    GetSysSettleDate(sDate,sChannelSettleDate);
    SET_STR_KEY(pstTransJson, "settle_date", sDate);
    tLog(ERROR, "获取结算日[%s]成功.", sDate);
    return 0;
}

/* 获取二维码结算日 */
int GetQrSettleDate(cJSON *pstJson, int *piFlag) {
    char sDate[8 + 1] = {0}, sRespCode[2 + 1] = {0};
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "交易失败[%s],不计算结算日.", sRespCode);
        return 0;
    }
    FindQrSettleDate(sDate);
    SET_STR_KEY(pstTransJson, "settle_date", sDate);
    tLog(ERROR, "获取结算日成功,使用T+1日[%s]结算.", sDate);
    return 0;
}