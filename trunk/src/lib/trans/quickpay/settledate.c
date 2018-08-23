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

/* 获取快捷无卡消费结算日 */
int GetQuSettleDate(cJSON *pstJson, int *piFlag) {
    char sDate[8 + 1] = {0}, sTransCode[6 + 1] = {0};
    cJSON * pstTransJson = NULL;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    /*获取 快捷无卡消费交易结算日期*/
    FindCardSettleDate(sDate);
    
    SET_STR_KEY(pstTransJson, "settle_date", sDate);
    tLog(ERROR, "获取结算日成功,使用T+1日[%s]结算.", sDate);
    return 0;
}