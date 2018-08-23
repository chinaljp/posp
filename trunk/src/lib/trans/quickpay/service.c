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

/* 返回处理，将应答的信息copy到data中 */
int RespPro(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0};
    char sRrn[12 + 1] = {0};

    char sRespId[8 + 1] = {0}, sRespDesc[256 + 1] = {0}, sRespLog[256 + 1] = {0};
    
    //char *sMsg = NULL;
    //DUMP_JSON(pstJson,sMsg);
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    
    GET_STR_KEY(pstRepJson, "resp_code", sRespId);
    SET_STR_KEY(pstDataJson, "istresp_code", sRespId);
    GET_STR_KEY(pstRepJson, "resp_desc", sRespDesc);
    SET_STR_KEY(pstDataJson, "resp_desc", sRespDesc);
    
    GET_STR_KEY(pstRepJson, "rrn", sRrn);
    if( !memcmp(sRespId,"000000",6) ) {
        SET_STR_KEY(pstDataJson, "resp_code", "00");
    }
    else {
        SET_STR_KEY(pstDataJson, "resp_code", "Q6");
    }
    
    //GetRespCode(sRespDesc, sRespCode);
    //ErrHanding(pstDataJson, sRespCode, "交易[%s]返回[%s]:%s", sRrn, sRespCode, sRespDesc);
    tLog(ERROR,"交易[%s]返回[%s]:[%s]",sRrn,sRespId,sRespDesc);
    tTrim(sRespDesc);
    sprintf(sRespLog,"交易[%s]返回[%s]:[%s]",sRrn,sRespId,sRespDesc);
    SET_STR_KEY(pstDataJson, "resp_log", sRespLog);
    return 0;
}

int QueryReturnInfo(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sRrn[12+1] = {0}, sRespId[8 + 1] = {0}, sRespDesc[200 + 1] = {0}, sRespLog[200 + 1] = {0};
    
    char sORespCode[6+1] = {0}, sORespDesc[200] = {0};
    int iTxStatus = 0;
    
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
   
    /*快捷无卡消费支付结果*/
    GET_STR_KEY(pstRepJson, "o_resp_code", sORespCode);
    SET_STR_KEY(pstDataJson, "o_resp_code", sORespCode);
    GET_STR_KEY(pstRepJson, "o_resp_desc", sORespDesc);
    SET_STR_KEY(pstDataJson, "o_resp_desc", sORespDesc);
    GET_INT_KEY(pstRepJson, "tx_state", iTxStatus);
    SET_INT_KEY(pstDataJson, "tx_state", iTxStatus);
    tLog(INFO,"原交易支付结果，sORespCode:[%s],sORespDesc:[%s],iTxStatus:[%d].",sORespCode,sORespDesc,iTxStatus);
    
    /*应答码*/
    GET_STR_KEY(pstRepJson, "resp_code", sRespId);
    SET_STR_KEY(pstDataJson, "istresp_code", sRespId);
    SET_STR_KEY(pstDataJson, "resp_code", sRespId);
    
    /*应答描述*/
    GET_STR_KEY(pstRepJson, "resp_desc", sRespDesc);
    SET_STR_KEY(pstDataJson, "resp_desc", sRespDesc);
    
    GET_STR_KEY(pstRepJson, "rrn", sRrn);
    tLog(ERROR,"交易[%s]返回[%s]:[%s]",sRrn,sRespId,sRespDesc);
    tTrim(sRespDesc);
    sprintf(sRespLog,"交易[%s]返回[%s]:[%s]",sRrn,sRespId,sRespDesc);
    SET_STR_KEY(pstDataJson, "resp_log", sRespLog);
    
    return( 0 );
}

/* 转发类需要设置pcKey，管理类等不需要转发的设置pcKey=NULL 
 */
int AppGetKey(char *pcKey, size_t tKey, cJSON *pstJson) {
    cJSON *pstDataJson = NULL;
    //  char sRrn[RRN_LEN + 1] = {0}, sTransDate[TRANS_DATE_LEN + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0}, sTransCode[6 + 1] = {0};

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "获取data数据失败,消息放弃处理.");
        return -1;
    }
    GET_STR_KEY(pstDataJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    
    snprintf(pcKey, tKey, "QUICK_%s%s", sTransCode+4,sSysTrace);
    tLog(DEBUG, "Key[%s]", pcKey);

    return 1;
}

int GetSvrName(char *pcSvrId, size_t tSvrId, cJSON *pstJson) {
    cJSON *pstDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0};
    char sSvrId[SVRID_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0}, sChannelId[8 + 1] = {0};

    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "channel_id", sChannelId);
#if 0   
    /* 浦发渠道*/
    if (!memcmp(sChannelId, "48560000", 8)) {
        if (sRespCode[0] == '\0') {
            strcpy(pcSvrId, "PUFA_Q");
            tLog(INFO, "使用浦发渠道");
        } else {
            GET_STR_KEY(pstJson, "svrid", sSvrId);
            strcpy(pcSvrId, sSvrId);
        }
        return 0;
    }
#endif   
    /*钱客通渠道*/
    if ( sRespCode[0] == '\0' ) {
        strcpy(pcSvrId, "QUICKPAY_Q");
    } else {
        GET_STR_KEY(pstJson, "svrid", sSvrId);
        strcpy(pcSvrId, sSvrId);
    }
    return 0;
}