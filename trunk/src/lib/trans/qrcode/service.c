/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_extiso.h"
#include "t_macro.h"
#include "resp_code.h"
#include "trans_code.h"

/* 框架的函数 */
char *GetSvrId();
TransCode g_stTransCode;

/* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0) {
        return -1;
    }
    if (LoadTransCodeByCode(&g_stTransCode, GetSvrId()) < 0) {
        return -1;
    }
    return 0;
}

/* 模块退出清理接口:针对初始化中的操作进行清理 */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    return 0;
}

/* 转发类需要设置pcKey，管理类等不需要转发的设置pcKey=NULL 
 * 只在请求中会调
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    cJSON *pstDataJson = NULL;
    char sRrn[RRN_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN + 1] = {0};

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "rrn", sRrn);
    snprintf(pcKey, tKey, "%s_%s", GetSvrId(), sRrn);
    tLog(DEBUG, "Key[%s]", pcKey);
    return 1;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstJson) {
    char sTransCode[6 + 1] = {0};
    cJSON *pstDataJson = NULL;

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    tLog(DEBUG,"trans_code = [%s]",sTransCode);
    if(sTransCode[2] == 'Y') {
        snprintf(pcSvrId, tSvrId, "CUPSCANPAY_Q");
        tLog(DEBUG,"银联二维码交易，pcSvrId = [%s]",pcSvrId);
    }
    else {
        //微信、支付宝 二维码交易
        snprintf(pcSvrId, tSvrId, "SCANPAY_Q");
    }
    
    return 0;
}