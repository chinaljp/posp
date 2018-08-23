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
#include "t_cjson.h"
#include "t_macro.h"
#include "trans_code.h"
#include "t_redis.h"

/* 框架的函数 */
char *GetSvrId();

/* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadRespCode() < 0)
    {
        return -1;
    }
    if (LoadTransCodeByGroupId(GetSvrId()) < 0)
    {
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
int RespPro(cJSON *pstTransJson, int *piFlag) {
    cJSON *pstRepJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0};
    char sBanlance[AMOUNT_LEN + 1] = {0}, sRrn[12 + 1] = {0};
    char sAcctType[2 + 1] = {0};
    char sAmtType[2 + 1] = {0};
    char sTransCode[6 + 1] = {0};
    char sBalanceSign[1 + 1] = {0};
    char sAuthCode[6 + 1] = {0};
    char sIcData[1024 + 1] = {0};
    char sRespId[8 + 1] = {0}, sRespDesc[256 + 1] = {0};
    double dAmount = 0.0;

    //DUMP_JSON(pstJson);
    pstRepJson = GET_JSON_KEY(pstTransJson, "response");
    GET_STR_KEY(pstRepJson, "istresp_code", sRespCode);
    SET_STR_KEY(pstTransJson, "istresp_code", sRespCode);
    /*38域 授权码*/
    GET_STR_KEY(pstRepJson, "auth_code", sAuthCode);
    SET_STR_KEY(pstTransJson, "auth_code", sAuthCode);
    /*54域*/
    GET_STR_KEY(pstRepJson, "balance", sBanlance);
    GET_STR_KEY(pstRepJson, "acct_type", sAcctType);
    GET_STR_KEY(pstRepJson, "amt_type", sAmtType);
    GET_STR_KEY(pstRepJson, "balance_sign", sBalanceSign);
    SET_STR_KEY(pstTransJson, "amt_type", sAmtType);
    SET_STR_KEY(pstTransJson, "acct_type", sAcctType);
    SET_STR_KEY(pstTransJson, "balance_sign", sBalanceSign);
    SET_STR_KEY(pstTransJson, "balance1", sBanlance);
    /*55域*/
    GET_STR_KEY(pstRepJson, "ic_data", sIcData);
    DEL_KEY(pstTransJson, "ic_data");
    SET_STR_KEY(pstTransJson, "ic_data", sIcData);

    /*100域*/
    GET_STR_KEY(pstRepJson, "resp_id", sRespId);
    SET_STR_KEY(pstTransJson, "resp_id", sRespId);

    GET_STR_KEY(pstRepJson, "channel_rrn", sRrn);
    SET_STR_KEY(pstTransJson, "channel_rrn", sRrn);

    GetRespCode(sRespDesc, sRespCode);
    ErrHanding(pstTransJson, sRespCode, "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
#if 0
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);

    if (!memcmp(sTransCode, "020000", 6) && DBL_EQ(dAmount, 1000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    else if (!memcmp(sTransCode, "T20000", 6) && DBL_EQ(dAmount, 100000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    else if (!memcmp(sTransCode, "M20000", 6) && DBL_EQ(dAmount, 1000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    else if (!memcmp(sTransCode, "M20002", 6) && DBL_EQ(dAmount, 1000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    else if (!memcmp(sTransCode, "020002", 6) && DBL_EQ(dAmount, 2000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    else if (!memcmp(sTransCode, "024000", 6) && DBL_EQ(dAmount, 1000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    else if (!memcmp(sTransCode, "024002", 6) && DBL_EQ(dAmount, 2000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    else if (!memcmp(sTransCode, "024100", 6) && DBL_EQ(dAmount, 2000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    else if (!memcmp(sTransCode, "024102", 6) && DBL_EQ(dAmount, 4000))
        ErrHanding(pstTransJson, "99", "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
#endif 
    return 0;
}

int RespSucc(cJSON *pstDataJson, int *piFlag) {

    SET_STR_KEY(pstDataJson, "resp_id", "49000000");
    ErrHanding(pstDataJson, "00", "交易成功");
    return 0;
}

/* 转发类需要设置pcKey，管理类等不需要转发的设置pcKey=NULL 
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstTransJson) {
    //  char sRrn[RRN_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0}, sTransCode[6 + 1] = {0};

    GET_STR_KEY(pstTransJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    //  GET_STR_KEY(pstDataJson, "trans_date", sTransDate);

    //snprintf(pcKey, tKey, "ACCT_%s", sSysTrace);
    snprintf(pcKey, tKey, "%s_%s%s", GetSvrId(), sTransCode + 4, sSysTrace);
    tLog(DEBUG, "Key[%s]", pcKey);

    return 1;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstTransJson) {
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0};
    char sSvrId[SVRID_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0}, sChannelId[8 + 1] = {0};

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "channel_id", sChannelId);
    /* 日结确认和手机日结通知 */
    if (!memcmp(sTransCode, "TA0010", 6)
            || !memcmp(sTransCode, "T20010", 6)
            || !memcmp(sTransCode, "022100", 6)
            )
    {
        return -1;
    }
    
    /* 浦发渠道*/
    if (!memcmp(sChannelId, "48560000", 8)) {
        strcpy(pcSvrId, "PUFA_Q");
        tLog(INFO, "使用浦发渠道");
        return 0;
    }
    /* 普通渠道或银联优惠类 */
    strcpy(pcSvrId, "CUPS_Q");
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
                if (AddPosTransLs(pstTransJson) < 0)
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
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (UpdPosTransLs(pstTransJson) < 0)
                {
                    tLog(ERROR, "更新POS交易流水结果失败,交易放弃处理.");
                    return -1;
                } else
                {
                    tLog(INFO, "交易[%s]更新流水成功.", sRrn);
                }
                tCommit();
            }
            break;
            /* 应答交易*/
        case REP_MSG:
            if (TRUE_FLAG == stTransCode.sLogFlag[0])
            {
                if (UpdPosTransResultLs(pstTransJson) < 0)
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