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
extern char *GetSvrId();

/* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadTransCode() < 0) {
        return -1;
    }
    if (LoadRespCode() < 0) {
        return -1;
    }
    return 0;
}

/* 模块退出清理接口:针对初始化中的操作进行清理 */
int AppServerDone(void) {
    tLog(DEBUG, "AppServerDone");
    return 0;
}

/* 返回处理，将应答的信息copy到data中 */
int RespPro(cJSON *pstJson, int *piFlag) {
    cJSON *pstRepJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRespDesc[50] = {0};
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_STR_KEY(pstRepJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRepJson, "resp_desc", sRespDesc);
    SET_STR_KEY(pstJson, "resp_code", sRespCode);
    SET_STR_KEY(pstJson, "resp_desc", sRespDesc);
    ErrHanding(pstJson, sRespCode);
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
    switch (iMsgType) {
            /* 请求交易 */
        case REQ_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
                if (AddInlineTransLs(pstTransJson) < 0) {
                    tLog(ERROR, "流水记录失败,交易放弃处理.");
                    return -1;
                }
                tCommit();
                tLog(INFO, "交易[%s]预流水添加成功.", sRrn);
            } else {
                tLog(INFO, "交易[%s]不记录交易流水.", sRrn);
            }
            iRet = 0;
            break;
            /* 应答交易*/
        case REP_MSG:
#if 0
            pstRespJson = GET_JSON_KEY(pstDataJson, "response");
            if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
                if (UpdPosTransResultLs(pstRespJson) < 0) {
                    tLog(ERROR, "更新POS交易流水[%s]结果失败,交易放弃处理.", sRrn);
                    return -1;
                } else {
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
    switch (iMsgType) {

            /* 请求交易流处理的数据等 */
        case REQ_MSG:
#if 0
            if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
                if (UpdInlineTransLs(pstTransJson) < 0) {
                    tLog(ERROR, "更新POS交易流水结果失败,交易放弃处理.");
                    return -1;
                } else {
                    tLog(INFO, "交易[%s]更新流水成功.", sRrn);
                }
                tCommit();
            }
#endif
            iRet = 0;
            break;

            /* 应答交易*/
        case REP_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
                if (UpdInlineTransLs(pstTransJson) < 0) {
                    tLog(ERROR, "更新POS交易流水[%s]结果失败,交易放弃处理.", sRrn);
                    return -1;
                } else {
                    tLog(INFO, "交易[%s]更新流水成功.", sRrn);
                }
                tCommit();
            }
            iRet = 0;
            break;
    }
    return iRet;
}

/* 转发类需要设置pcKey，管理类等不需要转发的设置pcKey=NULL 
 * 只在请求中会调
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    char sRrn[RRN_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN + 1] = {0}, sTransCode[6 + 1] = {0};
    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    GET_STR_KEY(pstJson, "rrn", sRrn);
    snprintf(pcKey, tKey, "%s_%s%s", GetSvrId(), sTransDate, sRrn);
    tLog(DEBUG, "Key[%s]", pcKey);

    if (!memcmp(sTransCode, "02A100", 6) ||
            !memcmp(sTransCode, "0AA100", 6)) {
        return 0;
    }
    return 1;
}

int RespSucc(cJSON *pstJson, int *piFlag) {
    ErrHanding(pstJson, "00", "交易成功");
    return 0;
}

int GetSvrName(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson) {
    char sTransCode[TRANS_CODE_LEN + 1];
    char sSvrId[SVRID_LEN + 1] = {0};
    GET_STR_KEY(pstInJson, "trans_code", sTransCode);
    if (!memcmp(sTransCode, "02A100", 6) ||
            !memcmp(sTransCode, "0AA100", 6)) {
        return -1;
    } else {
        snprintf(pSvrId, tSvrId, "AUTH_Q");
    }
    return 0;
}