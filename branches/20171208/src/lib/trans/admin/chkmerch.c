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
#include "status.h"

int ChkMerch(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0};
    Merch stMerch;
    cJSON *pstAgentJson = NULL;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    if (FindMerchInfoById(&stMerch, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "未找到商户[%s]的信息.", sMerchId);
        return -1;
    }

    /* 检查状态 */
    if (STAT_OPEN != stMerch.sStatus[0]) {
        ErrHanding(pstTransJson, "C2", "商户[%s]状态[%s]非正常.", sMerchId, stMerch.sStatus);
        return -1;
    }

    SET_STR_KEY(pstTransJson, "merch_name", stMerch.sCName);
    SET_STR_KEY(pstTransJson, "merch_p_name", stMerch.sPName); /* 凭条打印名称 */
    tLog(INFO, "商户[%s][%s]检查成功.", sMerchId, stMerch.sCName);
    return 0;
}