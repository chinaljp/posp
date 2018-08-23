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
char *GetSvrId();

/* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0)
    {
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

/* 设置交易key */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstTransJson) {
    return 0;
}

/* 只在请求处理中调用，返回1指向下一个模块，需要设置下一个模块的svrid，返回-1，不需要设置svrid，框架会自动向请求的队列返回消息 */
int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstTransJson) {
    return -1;
}

/* 管理类多是返回成功的 */
int RespSucc(cJSON *pstTransJson, int *piFlag) {
    ErrHanding(pstTransJson, "00", "交易成功");
    return 0;
}

int BeginProc(cJSON *pstTransJson, int iMsgType) {
    char sTmp[256 + 1] = {0};
    char sTransCode[6 + 1] = {0}, sMerchId[15 + 1] = {0}, sRrn[12 + 1] = {0}, sTermSn[100 + 1] = {0};

    /* 为了日志查询，打印一些业务数据 */
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    if (!memcmp(sTransCode, "029300", 6))
    {
        GET_STR_KEY(pstTransJson, "term_sn", sTermSn);
        GET_STR_KEY(pstTransJson, "rrn", sRrn);
        snprintf(sTmp, sizeof (sTmp), "%s_%s_%s", sTransCode, sTermSn, sRrn);
    } else
    {
        GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
        GET_STR_KEY(pstTransJson, "rrn", sRrn);
        snprintf(sTmp, sizeof (sTmp), "%s_%s_%s", sTransCode, sMerchId, sRrn);
    }
    zlog_put_mdc("key", sTmp);
    return 0;
}

int EndProc(cJSON *pstTransJson, int iMsgType) {
    return 0;
}
