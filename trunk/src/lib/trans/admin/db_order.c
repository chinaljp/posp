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

/* 获取商户订单路由信息 */

int FindNoticeTransfer(cJSON * pstTransJson) {

    char sSqlStr[256] = {0}, sStatus[2] = {0};
    char sUrl[20 + 1] = {0}, sPort[10 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sPlatcode[20 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select url,port,platcode from b_platform_access where platcode = \
                 (select platcode from b_merch_platform_middle where merch_id='%s' and status = '1') and status = '1'", sMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找商户[%s]的订单路由资料,失败.", sMerchId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sUrl);
        STRV(pstRes, 2, sPort);
        STRV(pstRes, 3, sPlatcode);
    }

    SET_STR_KEY(pstTransJson, "order_url", sUrl);
    SET_STR_KEY(pstTransJson, "order_port", sPort);
    SET_STR_KEY(pstTransJson, "platcode", sPlatcode);

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无商户[%s]路由资料,请先配置路由", sMerchId);
        tReleaseRes(pstRes);
        return -1;
    }
    tLog(INFO, "商户[%s]订单平台码[%s],URL[%s],PORT[%s]", sMerchId, sUrl, sPort, sPlatcode);
    tReleaseRes(pstRes);
    return 0;
}

