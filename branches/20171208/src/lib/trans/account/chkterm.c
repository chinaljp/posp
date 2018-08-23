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
#include "t_macro.h"
#include "status.h"
#include "term.h"

int ChkTerm(cJSON *pstJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    Terminal stTerminal;
    cJSON * pstTransJson = NULL, *pstTermJson = NULL;


    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);

    if (FindTermInfoById(&stTerminal, sMerchId, sTermId) < 0) {
        ErrHanding(pstTransJson, "C1", "未找到商户[%s]终端[%s]的信息.", sMerchId, sTermId);
        return -1;
    }
    pstTermJson = cJSON_CreateObject();
    if (NULL == pstTermJson) {
        ErrHanding(pstTransJson, "C1", "创建商户[%s]终端[%s]的json结构失败.", sMerchId, sTermId);
        return -1;
    }
    /* 检查状态 */
    if (STAT_OPEN != stTerminal.sStatus[0]) {
        ErrHanding(pstTransJson, "C1", "商户[%s]终端[%s]状态[%s]非正常.", sMerchId, sTermId, stTerminal.sStatus);
        cJSON_Delete(pstTermJson);
        return -1;
    }

    SET_STR_KEY(pstTermJson, "tmk_key", stTerminal.sTmk);
    SET_JSON_KEY(pstJson, "terminal", pstTermJson);
    tLog(INFO, "商户[%s]终端[%s]状态[%s]正常.", sMerchId, sTermId, stTerminal.sStatus);
    return 0;
}