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
#include "t_redis.h"

int OrderNotice(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char *psTran = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sTransCode[6 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    DUMP_JSON(pstJson, psTran);
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "交易[%s]失败[%s],不发订单通知交易.", sRrn, sRespCode);
        return 0;
    }
    
        /* 获取订单路由信息 */
    if (FindNoticeTransfer(pstTransJson) < 0) {
        cJSON_Delete(pstTransJson);
        tLog(ERROR, "获取商户订单路由信息失败");
        return -1;
    }

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "%s%s", sTransCode, sRrn);
    stQMsgData.pstDataJson = pstTransJson;

    iRet = tSvcACall("NOTICE_Q", &stQMsgData);
    if (iRet < 0) {
        tLog(DEBUG, "调用订单通知模块失败.");
        AddNoticeLs(pstTransJson);
    } else {
        tLog(INFO, "调用notice服务成功.");
    }
    return 0;
}
