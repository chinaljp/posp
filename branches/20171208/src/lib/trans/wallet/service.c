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
#include "trans_code.h"

/* 框架的函数 */
char *GetSvrId();

/* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
int AppServerInit(int iArgc, char *pcArgv[]) {
    if (LoadTransCodeByGroupId(GetSvrId()) < 0)
    {
        return -1;
    }
    return 0;
}

/* 模块退出清理接口:针对初始化中的操作进行清理 */
int AppServerDone(void) {
    return 0;
}

/* 转发类需要设置pcKey，管理类等不需要转发的设置pcKey=NULL 
 * 只在请求中会调
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstTransJson) {
    char sRrn[12 + 1] = {0};

    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    //  GET_STR_KEY(pstDataJson, "trans_date", sTransDate);

    snprintf(pcKey, tKey, "%s_%s", GetSvrId(), sRrn);
    tLog(DEBUG, "Key[%s]", pcKey);
    return 1;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstTransJson) {
    char sSvrId[SVRID_LEN + 1] = {0}, sTransCode[6 + 1] = {0};

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    if (NULL != strstr("00T400,00T500,00T600,00T700,00T800,00T900,00AQ00", sTransCode))
    {
        strcpy(pcSvrId, "TRANSD_Q");
    } else
    {
        strcpy(pcSvrId, "ACCTD_Q");
    }
    return 0;
}

/* 空处理 */
int NullProc(cJSON *pstJson, int *piFlag) {
    return 0;
}

/* 返回处理，将应答的信息copy到data中 */
int RespPro(cJSON *pstTransJson, int *piFlag) {
    char sRespCode[32 + 1] = {0};
    char sRespLog[4096 + 1] = {0};
    cJSON *pstRespJson = NULL;

    pstRespJson = GET_JSON_KEY(pstTransJson, "response");
    COPY_JSON(pstTransJson, pstRespJson);

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "resp_desc", sRespLog);
    tLog(INFO, "响应码:%s:%s.", sRespCode, sRespLog);
    return 0;
}

int BeginProc(cJSON *pstTransJson, int iMsgType) {
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sRrn[12 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sTmp[256 + 1] = {0};
    TransCode stTransCode;

    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    FindTransCode(&stTransCode, sTransCode);

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    snprintf(sTmp, sizeof (sTmp), "%s_%s_%s", sTransCode, sMerchId, sRrn);
    zlog_put_mdc("key", sTmp);
    return 0;
}

int EndProc(cJSON *pstTransJson, int iMsgType) {
    return 0;
}

/* 查询钱包账户余额 返回处理 */
int RespQueryPro(cJSON *pstTransJson, int *piFlag) {
    cJSON *pstRepJson = NULL, *pstSubDataJson = NULL;
    double dPosD0Wallet = 0.00, dPosD0BusinessFrozenWallet = 0.00;
    double dPosD0PreWallet = 0.00, dPosD0BusinessFrozenPreWallet = 0.00;
    char sRespCode[2 + 1] = {0}, sRespDesc[4096] = {0}, sData[1024] = {0};

    pstRepJson = GET_JSON_KEY(pstTransJson, "response");

    GET_STR_KEY(pstRepJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRepJson, "resp_desc", sRespDesc);
    GET_STR_KEY(pstRepJson, "data", sData);
    tLog(DEBUG, "data[%s]", sData);
    tLog(INFO, "响应码:%s:%s.", sRespCode, sRespDesc);

    pstSubDataJson = cJSON_Parse(sData);
    if (NULL == pstSubDataJson)
    {
        tLog(ERROR, "无data数据.");
        return 0;
    }

    GET_DOU_KEY(pstSubDataJson, "posD0Wallet", dPosD0Wallet); //POS D0可提现钱包
    //GET_DOU_KEY(pstSubDataJson, "posD0BusinessFrozenWallet", dPosD0BusinessFrozenWallet); /* 110 - POS D0可提现业务冻结钱包 */

    GET_DOU_KEY(pstSubDataJson, "posD0PreWallet", dPosD0PreWallet); //D0 待入账钱包
    //GET_DOU_KEY(pstSubDataJson, "posD0BusinessFrozenPreWallet", dPosD0BusinessFrozenPreWallet); /* 111 - POS D0待入账业务冻结钱包 */

    SET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    SET_STR_KEY(pstTransJson, "resp_desc", sRespDesc);
    SET_DOU_KEY(pstTransJson, "WalletAmt", dPosD0Wallet);
    SET_DOU_KEY(pstTransJson, "PreWalletAmt", dPosD0PreWallet);

    cJSON_Delete(pstSubDataJson);

    return ( 0);
}