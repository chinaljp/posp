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

int ChkTerm(cJSON *pstJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    Terminal stTerminal;
    cJSON * pstTransJson = NULL, *pstTermJson = NULL;


    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);

    if (FindTermInfoById(&stTerminal, sMerchId, sTermId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]终端[%s]的信息.", sMerchId, sTermId);
        return -1;
    }
    pstTermJson = cJSON_CreateObject();
    if (NULL == pstTermJson) {
        ErrHanding(pstTransJson, "96", "创建商户[%s]终端[%s]的json结构失败.", sMerchId, sTermId);
        return -1;
    }

    SET_STR_KEY(pstTermJson, "tmk_key", stTerminal.sTmk);

    SET_JSON_KEY(pstJson, "terminal", pstTermJson);
    return 0;
}