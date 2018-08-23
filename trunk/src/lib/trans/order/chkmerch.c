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
#include "merch.h"
#include "t_macro.h"

int ChkMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0};
    Merch stMerch;
    cJSON * pstTransJson = NULL, *pstAgentJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    if (FindMerchInfoById(&stMerch, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的信息.", sMerchId);
        return -1;
    }

    SET_STR_KEY(pstTransJson, "merch_name", stMerch.sCName);
    SET_STR_KEY(pstTransJson, "user_code", stMerch.sUserCode);
    return 0;
}