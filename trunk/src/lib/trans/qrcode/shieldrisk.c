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
#include "t_redis.h"
#include "trace.h"

void Net2Risk(cJSON *pstJson, cJSON *pstNetJson);
int SendToShield(cJSON *pstJson);
int RepSendToShield(cJSON *pstJson);
int GetConfInfo(char *pcConfName, char *pcTabName, char *pcValue);

/*同盾风控系统-请求*/
int ExehieldRisk(cJSON *pstJson, int *piFlag) {

    cJSON *pstTransJson = NULL;
    char sSqlStr[1024] = {0}, sKeyValue[1 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    /*参数表中获取同盾风控系统开关*/
    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key = 'WITH_SHIELD_SWITCH'");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sKeyValue);
        tTrim(sKeyValue);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    /*同盾风控系统开关，sKeyValue 不等于 1 时，直接返回*/
    if(sKeyValue[0] != '1') {
        tLog(INFO,"同盾风控系统已关闭,KEY_VALUE=[%s]",sKeyValue);
        return ( 0 );
    }

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    if (SendToShield(pstTransJson) < 0) {
        ErrHanding(pstTransJson, "T9", "同盾风控规则检查不通过.");
        return ( -1);
    }

    return ( 0);
}

/*同盾风控系统-返回*/
int ExehieldRiskRep(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sSqlStr[1024] = {0}, sKeyValue[1 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    /*参数表中获取同盾风控系统开关*/
    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key = 'WITH_SHIELD_SWITCH'");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sKeyValue);
        tTrim(sKeyValue);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "未找到记录.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    /*同盾风控系统开关，sKeyValue 不等于 1 时，直接返回*/
    if(sKeyValue[0] != '1') {
        tLog(INFO,"同盾风控系统已关闭,KEY_VALUE=[%s]",sKeyValue);
        return ( 0 );
    }

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    RepSendToShield(pstTransJson);

    return ( 0);
}

/*组发送到同盾风控系统的信息 并发送*/
int SendToShield(cJSON *pstJson) {

    cJSON *pstRecvDataJson = NULL, *pstNetJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0}, sTraSeq[12 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sRuleExplan[255 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return 0;
    }
    GET_STR_KEY(pstJson, "rrn", sRrn);

    Net2Risk(pstJson, pstNetJson);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "TDFK%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("TDFK_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            tLog(ERROR, "交易[%s]接收同盾风控规则检查超时.", sRrn);
        } else
            tLog(ERROR, "交易[%s]接收同盾风控规则返回失败.", sRrn);
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "respCode", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "respDesc", sResvDesc);
        GET_STR_KEY(pstRecvDataJson, "ruleExplanation", sRuleExplan);
        GET_STR_KEY(pstRecvDataJson, "traSeq", sTraSeq);

        tLog(DEBUG, "交易[%s]返回解释：【%s】", sTraSeq, sRuleExplan);
        if (memcmp(sRespCode, "00", 2)) {
            tLog(ERROR, "交易[%s]失败,同盾风控规则检查失败[%s:%s].", sTraSeq, sRespCode, sResvDesc);
            cJSON_Delete(pstNetJson);
            if (NULL != pstRecvDataJson) {
                cJSON_Delete(pstRecvDataJson);
            }
            return ( -1);
        } else {
            tLog(INFO, "交易[%s]成功,同盾风控规则检查通过.", sTraSeq);
        }
    }
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);

    return ( 0);
}

/*返回交易流中的信息*/
int RepSendToShield(cJSON *pstJson) {
    cJSON *pstNetJson = NULL;
    char sRrn[RRN_LEN + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData;

    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "创建发送Json失败.");
        return 0;
    }
    GET_STR_KEY(pstJson, "rrn", sRrn);

    Net2Risk(pstJson, pstNetJson);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "RSPTDFK%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;

    iRet = tSvcACall("RSPTDFK_Q", &stQMsgData);
    if (iRet < 0) {
        tLog(WARN, "交易返回，发送通知到同盾风控系统失败.");
    } else {
        tLog(INFO, "交易返回，发送通知到同盾风控系统成功.");
    }
    tLog(DEBUG, "已发送通知");
    cJSON_Delete(pstNetJson);

    return ( 0);
}

void Net2Risk(cJSON *pstJson, cJSON *pstTransJson) {
    char sTransCode[6 + 1] = {0}, sOrderNo[100] = {0}, sRrn[RRN_LEN + 1] = {0};
    char sTermSn[30 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sMerchName[128] = {0}, sMcc[4 + 1] = {0}, sMerchType[1 + 1] = {0}, sAmount[13 + 1] = {0};
    char sTransTime[6 + 1] = {0}, sDate[10 + 1] = {0}, sTime[8 + 1] = {0};
    char sHour[2 + 1] = {0}, sMinu[2 + 1] = {0}, sSec[2 + 1] = {0};
    char sTransTmStamp[18 + 1] = {0};
    double dAmount = 0.00;
    
    int iRet = 0;
    /*********************data******************************/
    char sPartnerCode[6+1] = {0};
    char sSecretKey[32+1] = {0};
    /*读取配置中的 PROD KEY */
    iRet = GetConfInfo("shield.conf", "PROD", sPartnerCode);
    if( iRet < 0 ) {
        /*配置读取失败直接赋值*/
        SET_STR_KEY(pstTransJson, "partnerCode", "kratos"); //同盾产品标识
    }
    else {
        SET_STR_KEY(pstTransJson, "partnerCode", sPartnerCode); //同盾产品标识
    }
    iRet = GetConfInfo("shield.conf", "KEY", sSecretKey);
    if( iRet < 0 ) {
       /*配置读取失败直接赋值*/
       SET_STR_KEY(pstTransJson, "secretKey", "ef90bdf0a1ae4dc1b640654de731306e")
    }
    else {
        SET_STR_KEY(pstTransJson, "secretKey", sSecretKey); //接口密钥,每个应用对应一个密钥(POS)
    }
    tLog(DEBUG,"sPartnerCode = [%s],sSecretKey = [%s]",sPartnerCode,sSecretKey);
    //SET_STR_KEY(pstTransJson, "partnerCode", "kratos"); //同盾产品标识
    //SET_STR_KEY(pstTransJson, "secretKey", "ef90bdf0a1ae4dc1b640654de731306e"); //接口密钥,每个应用对应一个密钥(POS)

    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    SET_STR_KEY(pstTransJson, "tradeCode", sTransCode);

    tLog(DEBUG, "sTransCode[%s]", sTransCode);
    SET_STR_KEY(pstTransJson, "eventId", "qrcode");
    /*请求交易流中,二维码订单号，主扫时会有*/
    GET_STR_KEY(pstJson, "seed", sOrderNo);
    SET_STR_KEY(pstTransJson, "qCodeOrderNo", sOrderNo);

    SET_STR_KEY(pstTransJson, "appName", "pos");

    GET_STR_KEY(pstJson, "term_sn", sTermSn);
    SET_STR_KEY(pstTransJson, "deviceId", sTermSn);

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    SET_STR_KEY(pstTransJson, "merNo", sMerchId);

    GET_STR_KEY(pstJson, "merch_name", sMerchName);
    SET_STR_KEY(pstTransJson, "merName", sMerchName);

    GET_STR_KEY(pstJson, "mcc", sMcc);
    SET_STR_KEY(pstTransJson, "mcc", sMcc);

    /*商户类型: 0:标准，1:优惠；2:减免*/
    GET_STR_KEY(pstJson, "merch_type", sMerchType);
    SET_STR_KEY(pstTransJson, "merType", sMerchType);

    GET_DOU_KEY(pstJson, "amount", dAmount);
    sprintf(sAmount, "%.2lf", dAmount / 100);
    tLog(DEBUG, "dAmount[%f],sAmount[%s]", dAmount, sAmount);
    SET_STR_KEY(pstTransJson, "traAmount", sAmount);

    GET_STR_KEY(pstJson, "rrn", sRrn);
    SET_STR_KEY(pstTransJson, "traSeq", sRrn);

    /*事件发生时间*/
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    memcpy(sHour, sTransTime, 2);
    memcpy(sMinu, sTransTime + 2, 2);
    memcpy(sSec, sTransTime + 4, 2);

    tGetDate(sDate, "-", -1);
    sprintf(sTime, "%s:%s:%s", sHour, sMinu, sSec);
    sprintf(sTransTmStamp, "%s %s", sDate, sTime);
    SET_STR_KEY(pstTransJson, "eventOccurTime", sTransTmStamp);

    /*返回交易流中组*/
    char sIstRespCode[2 + 1] = {0}, sRespCode[2 + 1] = {0}, sRespDesc[256] = {0};
    GET_STR_KEY(pstJson, "istresp_code", sIstRespCode);
    GET_STR_KEY(pstJson, "resp_code", sRespCode);
    //GET_STR_KEY(pstJson,"resp_desc",sRespDesc);
    GET_STR_KEY(pstJson, "resp_log", sRespDesc);

    SET_STR_KEY(pstTransJson, "istrespCode", sIstRespCode);
    SET_STR_KEY(pstTransJson, "payStatus", sRespCode);
    SET_STR_KEY(pstTransJson, "payResult", sRespDesc);

    /*二维码相关信息*/
    char sCodeUrl[255] = {0}, sSeed[100] = {0}, sMerchOrderNo[100] = {0};
    GET_STR_KEY(pstJson, "code_url", sCodeUrl); /*被扫交易 二维码URL*/
    GET_STR_KEY(pstJson, "seed", sSeed); /* 主扫 扫描到的 二维码单号*/
    GET_STR_KEY(pstJson, "qr_order_no", sMerchOrderNo); /*本系统自动生成的二维码订单号*/

    SET_STR_KEY(pstTransJson, "qCodeOrderNo", sSeed);
    SET_STR_KEY(pstTransJson, "qCodeOrderUrl", sCodeUrl);
}
/*读取同盾顿风控配置文件中 同盾产品标识 和 接口密钥
 *入参：
 *      pcConfName  配置文件名称
 *      pcTabName   标签名称
 *出参：
 *      pcValue 标签对应的值
 */
int GetConfInfo(char *pcConfName, char *pcTabName, char *pcValue) {
    char *DirPath = NULL;
    char FullPath[32] = {0};

    FILE *fp = NULL;
    char sBuf[1024] = {0};

    DirPath = getenv("HOME");
    sprintf(FullPath, "%s/etc/%s", DirPath, pcConfName);
    tLog(DEBUG, "FullPath = [%s]", FullPath);

    fp = fopen(FullPath, "r");
    if (NULL == fp) {
        tLog(DEBUG,"文件[%s]打开失败\n", FullPath);
        return ( -1);
    }
    tLog(DEBUG,"文件[%s]打开成功\n", FullPath);

    //获取一行数据 fgets
    while (fgets(sBuf, 1024, fp)) {
        if (strstr(sBuf, pcTabName)) {
            fgets(sBuf, 1024, fp);
            tTrim(sBuf);
            tStrCpy(pcValue,sBuf,strlen(sBuf));
            break;
        }
    }
    fclose(fp);
    fp = NULL;    
    return ( 0);
}
