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
#include "term.h"
#include "t_macro.h"

int TransferJn(cJSON *pstJson, int *piFlag){

    return 0;
}

int RespSucc(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRespJson = NULL;
    char sAmount[12] = {0}, sRespCode[3] = {0}, sRespDesc[100] = {0};
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRespJson = GET_JSON_KEY(pstJson, "response");
    GET_STR_KEY(pstRespJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRespJson, "resp_desc", sRespDesc);
    if (!memcmp(sRespCode, "00", 1)) {

        GET_STR_KEY(pstRespJson, "amount", sAmount);
        SET_STR_KEY(pstDataJson, "balance1", sAmount);
        SET_STR_KEY(pstDataJson, "amt_type", "02");
        SET_STR_KEY(pstDataJson, "acct_type", "10");
        SET_STR_KEY(pstDataJson, "balance_sign", "C");
        SET_STR_KEY(pstDataJson, "ccy_code", "156");
        ErrHanding(pstDataJson, "00", "交易成功");
        return 0;
    } else {
        ErrHanding(pstDataJson, sRespCode, sRespDesc);
        return 0;
    }
}
