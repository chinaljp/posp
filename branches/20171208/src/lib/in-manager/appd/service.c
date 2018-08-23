/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_extiso.h"
#include "trans_code.h"
#include "t_macro.h"

extern char *GetSvrId();

/* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0) {
        return -1;
    }
    tLog(DEBUG, "AppServerInit");
    
    if (LoadTransCode() < 0) {
        return -1;
    }
    return 0;
}

/* 模块退出清理接口:针对初始化中的操作进行清理 */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    tFreeConfig();
    return 0;
}

/* posd  pstInJson是8583报文，pstOutJson是结构体*/

/* 请求,应答都回调 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstInJson, cJSON *pstOutJson) {
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sMerchId[MERCH_ID_LEN + 1] = {0};
    char sTermId[TERM_ID_LEN + 1] = {0}, sTermSn[SN_LEN + 1] = {0};
    char sTraceNo[7] = {0};
    GET_STR_KEY(pstOutJson, "trans_code", sTransCode);

    /* pos初始化交易无rrn */

    GET_STR_KEY(pstOutJson, "merch_id", sMerchId);
    GET_STR_KEY(pstOutJson, "trace_no", sTraceNo);
    snprintf(pcKey, tKey, "%s_%s%s", GetSvrId(), sTraceNo,sMerchId);
    return 0;
}
/* posd  pstInJson是8583报文，pstOutJson是结构体*/

/* 只在请求时回调 */
int GetSvrName(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson) {
    char sTransCode[TRANS_CODE_LEN + 1];
    TransCode stTransCode;
    GET_STR_KEY(pstOutJson, "trans_code", sTransCode);
    FindTransCode(&stTransCode, sTransCode);
    snprintf(pSvrId, tSvrId, "%s_Q", stTransCode.sGroupCode);    
    return 0;
}