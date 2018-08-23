/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <zlog.h>
#include "t_redis.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_macro.h"

/* 框架的函数 */
int SendRepMsg(cJSON *pstJson);
char *GetSvrId();

/* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0) {
        return -1;
    }
    return 0;
}

/* 模块退出清理接口:针对初始化中的操作进行清理 */
int AppServerDone(void) {
    HSM_UNLINK();
    return 0;
}

/* 此流是个通用的交易流，配置到流的最后一个
 * 失败的消息框架处理
 * 但是在交易流中要设置resp_code应答码
 */
#if 0
int Transfer(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    /* 管理类最后一个流 */
    ErrHanding(pstTransJson, "00", "交易成功");
    DUMP_JSON(pstJson);
    /* 发送应答消息 */
    return SendRepMsg(pstJson);
}
#endif
/* 转发类需要设置pcKey，管理类等不需要转发的设置pcKey=NULL 
 * 只在请求中会调
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    char sKey[MSG_KEY_LEN + 1] = {0};

    GET_STR_KEY(pstJson, "key", sKey);
    strcpy(pcKey, sKey);
    return 0;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstJson) {
    char sSvrId[SVRID_LEN + 1] = {0};

    GET_STR_KEY(pstJson, "svrid", sSvrId);
    strcpy(pcSvrId, sSvrId);
    return 0;
}

/* 管理类多是返回成功的 */
int RespSucc(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL;

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    ErrHanding(pstDataJson, "00", "交易成功");
    return 0;
}