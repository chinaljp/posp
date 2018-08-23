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

int ChkMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0};
    Merch stMerch;
    char sAccountUuid[64 + 1] = {0};
    cJSON * pstTransJson = NULL, *pstAgentJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    if (FindMerchInfoById(&stMerch, sMerchId) < 0) {
        ErrHanding(pstTransJson, "C2", "未找到商户[%s]的信息.", sMerchId);
        return -1;
    }
    //检查商户account_uuid是否存在
    if (FindMerchAcctUuid(sAccountUuid, sMerchId) < 0) {
        ErrHanding(pstTransJson, "C2", "未找到商户[%s]的account_uuid信息.", sMerchId);
        return -1;
    }

    //检查商户account_uuid是否存在
    if (strlen(sAccountUuid) == 0) {
        ErrHanding(pstTransJson, "C2", "商户[%s]account_uuid[%s]不存在.", sMerchId, sAccountUuid);
        return -1;
    }

    /* 检查状态 */
    if (STAT_OPEN != stMerch.sStatus[0]) {
        ErrHanding(pstTransJson, "C2", "商户[%s]状态[%s]非正常.", sMerchId, stMerch.sStatus);
        return -1;
    }

    SET_STR_KEY(pstTransJson, "merch_name", stMerch.sCName);
    SET_STR_KEY(pstTransJson, "merch_p_name", stMerch.sPName); /* 凭条打印名称 */
    SET_STR_KEY(pstTransJson, "user_code", stMerch.sUserCode);
    SET_STR_KEY(pstTransJson, "merch_type", stMerch.sMerchType);
    SET_STR_KEY(pstTransJson, "mcc", stMerch.sMcc);
    SET_STR_KEY(pstTransJson, "merch_level", stMerch.sMerchLevel); //商户级别 add by GJQ at 20180305
    tLog(INFO, "商户[%s:%s]状态[%s]正常.", sMerchId, stMerch.sCName, stMerch.sStatus);
    return 0;
}