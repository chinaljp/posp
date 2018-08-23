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
#include "tKms.h"

/* 框架的函数 */
char *GetSvrId();
TransCode g_stTransCode;

/* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0) {
        return -1;
    }
    if (LoadTransCodeByGroupId(GetSvrId()) < 0) {
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
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstDataJson) {
    char sRrn[RRN_LEN + 1] = {0};

    GET_STR_KEY(pstDataJson, "rrn", sRrn);
    snprintf(pcKey, tKey, "%s_%s", GetSvrId(), sRrn);
    tLog(DEBUG, "Key[%s]", pcKey);
    return 1;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstDataJson) {
    char sTransCode[6 + 1] = {0};
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
/* 交易流执行前处理，需要判断消息类型是请求还是应答 */
int BeginProc(cJSON *pstTransJson, int iMsgType) {
    int iRet = 0;
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sRrn[12 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sTmp[256 + 1] = {0};
    TransCode stTransCode;

    //pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    FindTransCode(&stTransCode, sTransCode);

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    snprintf(sTmp, sizeof (sTmp), "%s_%s_%s", sTransCode, sMerchId, sRrn);
    zlog_put_mdc("key", sTmp);
    switch (iMsgType)
    {
            /* 请求交易 */
        case REQ_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (AddInlineTransLs(pstTransJson) < 0)
                {
                    tLog(ERROR, "流水记录失败,交易放弃处理.");
                    return -1;
                }
                tCommit();
                tLog(INFO, "交易[%s]预流水添加成功.", sRrn);
            } else
            {
                tLog(INFO, "交易[%s]不记录交易流水.", sRrn);
            }
            iRet = 0;
            break;
            /* 应答交易*/
        case REP_MSG:
#if 0
            pstRespJson = GET_JSON_KEY(pstDataJson, "response");
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (UpdPosTransResultLs(pstRespJson) < 0)
                {
                    tLog(ERROR, "更新POS交易流水[%s]结果失败,交易放弃处理.", sRrn);
                    return -1;
                } else
                {
                    tLog(INFO, "交易[%s]更新流水成功.", sRrn);
                }
                tCommit();
            }
#endif
            iRet = 0;
            break;
    }
    return iRet;
}

/* 交易流之后处理 */
int EndProc(cJSON *pstTransJson, int iMsgType) {
    int iRet = 0;
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sRrn[12 + 1] = {0};
    TransCode stTransCode;

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    FindTransCode(&stTransCode, sTransCode);

    tLog(DEBUG, "msg:%d", iMsgType);
    switch (iMsgType)
    {
            /* 请求交易流处理的数据等 */
        case REQ_MSG:
#if 0
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (UpdInlineTransLs(pstTransJson) < 0)
                {
                    tLog(ERROR, "更新POS交易流水结果失败,交易放弃处理.");
                    return -1;
                } else
                {
                    tLog(INFO, "交易[%s]更新流水成功.", sRrn);
                }
                tCommit();
            }
#endif
            break;
            /* 应答交易*/
        case REP_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (UpdInlineTransLs(pstTransJson) < 0)
                {
                    tLog(ERROR, "更新POS交易流水[%s]结果失败,交易放弃处理.", sRrn);
                    return -1;
                } else
                {
                    tLog(INFO, "交易[%s]更新流水成功.", sRrn);
                }
                tCommit();
            }
            iRet = 0;
            break;
    }
    return iRet;
}
