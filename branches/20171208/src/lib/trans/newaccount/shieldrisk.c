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
int ExehieldRisk(cJSON *pstTransJson, int *piFlag) {
    char sSqlStr[1024] = {0}, sKeyValue[1 + 1] = {0};
    char sTransCode[6 + 1] = {0};
    double dAmount = 0.00;
    OCI_Resultset *pstRes = NULL;

    /*交易金额为0的禁止交易*/
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    dAmount /= 100;
    tLog(INFO, "交易[%s],交易金额为[%.02f]", sTransCode, dAmount);
    if (strstr("020000,T20000,024000,024100,M20000", sTransCode) && DBL_ZERO(dAmount))
    {
        tLog(ERROR, "交易[%s],交易金额为[%.02f],禁止交易", sTransCode, dAmount);
        ErrHanding(pstTransJson, "13", "交易金额不能为零.");
        return ( -1);
    }
    /*参数表中获取同盾风控系统开关*/
    if (FindValueByKey(sKeyValue, "WITH_SHIELD_SWITCH") < 0)
    {
        ErrHanding(pstTransJson, "96", "查找key[WITH_SHIELD_SWITCH]值,失败.");
        return -1;
    }

    /*同盾风控系统开关，sKeyValue 不等于 1 时，直接返回*/
    if (sKeyValue[0] != '1')
    {
        tLog(INFO, "同盾风控系统已关闭,KEY_VALUE=[%s]", sKeyValue);
        return ( 0);
    }

    if (SendToShield(pstTransJson) < 0)
    {
        ErrHanding(pstTransJson, "D9", "同盾风控规则检查不通过.");
        return ( -1);
    }

    return ( 0);
}

/*同盾风控系统-返回*/
/* 响应交易不要返回-1 */
int ExehieldRiskRep(cJSON *pstTransJson, int *piFlag) {
    char sKeyValue[1 + 1] = {0}, sRespCode[RESP_CODE_LEN + 1] = {0};

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    /*参数表中获取同盾风控系统开关*/
    if (FindValueByKey(sKeyValue, "WITH_SHIELD_SWITCH") < 0)
    {
        tLog(INFO, "查找key[WITH_SHIELD_SWITCH]值失败,忽略.");
        return 0;
    }
    /*同盾风控系统开关，sKeyValue 不等于 1 时，直接返回*/
    if (sKeyValue[0] != '1')
    {
        tLog(INFO, "同盾风控系统已关闭,KEY_VALUE=[%s],忽略.", sKeyValue);
        return ( 0);
    }
    RepSendToShield(pstTransJson);
    return ( 0);
}

/*组发送到同盾风控系统的信息 并发送*/
int SendToShield(cJSON *pstTransJson) {

    cJSON *pstRecvDataJson = NULL, *pstNetJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0}, sTraSeq[12 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sRuleExplan[255 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson)
    {
        tLog(ERROR, "创建发送Json失败.");
        return 0;
    }
    GET_STR_KEY(pstTransJson, "rrn", sRrn);

    Net2Risk(pstTransJson, pstNetJson);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "TDFK%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("TDFK_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0)
    {
        if (MSG_TIMEOUT == iRet)
        {
            tLog(ERROR, "交易[%s]接收同盾风控规则检查超时.", sRrn);
        } else
            tLog(ERROR, "交易[%s]接收同盾风控规则返回失败.", sRrn);
    } else
    {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "respCode", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "respDesc", sResvDesc);
        GET_STR_KEY(pstRecvDataJson, "ruleExplanation", sRuleExplan);
        GET_STR_KEY(pstRecvDataJson, "traSeq", sTraSeq);

        tLog(DEBUG, "交易[%s]返回解释:[%s]", sTraSeq, sRuleExplan);
        if (memcmp(sRespCode, "00", 2))
        {
            tLog(ERROR, "交易[%s]失败,同盾风控规则检查失败[%s:%s].", sTraSeq, sRespCode, sResvDesc);
            cJSON_Delete(pstNetJson);
            if (NULL != pstRecvDataJson)
            {
                cJSON_Delete(pstRecvDataJson);
            }
            return ( -1);
        } else
        {
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
    if (NULL == pstNetJson)
    {
        tLog(ERROR, "创建发送Json失败.");
        return 0;
    }
    GET_STR_KEY(pstJson, "rrn", sRrn);

    Net2Risk(pstJson, pstNetJson);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "RSPTDFK%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;

    iRet = tSvcACall("RSPTDFK_Q", &stQMsgData);
    if (iRet < 0)
    {
        tLog(WARN, "交易返回，发送通知到同盾风控系统失败.");
    } else
    {
        tLog(INFO, "交易返回，发送通知到同盾风控系统成功.");
    }
    tLog(DEBUG, "已发送通知");
    cJSON_Delete(pstNetJson);

    return ( 0);
}

void Net2Risk(cJSON *pstJson, cJSON *pstTransJson) {
    char sTransCode[6 + 1] = {0}, sOrderNo[100] = {0}, sRrn[RRN_LEN + 1] = {0};
    char sTermSn[30 + 1] = {0}, sCardNo[19 + 1] = {0}, sCardMedia[2 + 1] = {0}, sInputMode[3 + 1] = {0}, sCardType[1 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sMerchName[128] = {0}, sMcc[4 + 1] = {0}, sMerchType[1 + 1] = {0}, sAmount[13 + 1] = {0};
    char sTransTime[6 + 1] = {0}, sDate[10 + 1] = {0}, sTime[8 + 1] = {0};
    char sHour[2 + 1] = {0}, sMinu[2 + 1] = {0}, sSec[2 + 1] = {0};
    char sTransTmStamp[18 + 1] = {0};
    double dAmount = 0.00;

    int iRet = 0;

    /*********************data******************************/
    char sPartnerCode[6 + 1] = {0};
    char sSecretKey[32 + 1] = {0};
    /*读取配置中的 PROD KEY */
    tLog(INFO, "同盾产品标识");
    iRet = GetConfInfo("shield.conf", "PROD", sPartnerCode);
    if (iRet < 0)
    {
        /*配置读取失败直接赋值*/
        SET_STR_KEY(pstTransJson, "partnerCode", "kratos"); //同盾产品标识
    } else
    {
        SET_STR_KEY(pstTransJson, "partnerCode", sPartnerCode); //同盾产品标识
    }
    tLog(INFO, "接口密钥");
    iRet = GetConfInfo("shield.conf", "KEY", sSecretKey);
    if (iRet < 0)
    {
        /*配置读取失败直接赋值*/
        SET_STR_KEY(pstTransJson, "secretKey", "ef90bdf0a1ae4dc1b640654de731306e")
    } else
    {
        SET_STR_KEY(pstTransJson, "secretKey", sSecretKey); //接口密钥,每个应用对应一个密钥(POS)
    }
    tLog(DEBUG, "sPartnerCode = [%s],sSecretKey = [%s]", sPartnerCode, sSecretKey);

    //SET_STR_KEY(pstTransJson, "partnerCode", "kratos"); //同盾产品标识
    //SET_STR_KEY(pstTransJson, "secretKey", "ef90bdf0a1ae4dc1b640654de731306e"); //接口密钥,每个应用对应一个密钥(POS)

    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    SET_STR_KEY(pstTransJson, "tradeCode", sTransCode);

    tLog(DEBUG, "sTransCode[%s]", sTransCode);

    SET_STR_KEY(pstTransJson, "eventId", "posp");
    /*订单交易 组易交易订单号 */
    if (!memcmp(sTransCode, "M2", 1))
    {
        GET_STR_KEY(pstJson, "merch_order_no", sOrderNo);
        SET_STR_KEY(pstTransJson, "transactionId", sOrderNo);
    }

    SET_STR_KEY(pstTransJson, "appName", "pos");

    GET_STR_KEY(pstJson, "term_sn", sTermSn);
    SET_STR_KEY(pstTransJson, "deviceId", sTermSn);

    GET_STR_KEY(pstJson, "card_no", sCardNo);
    SET_STR_KEY(pstTransJson, "cardNumber", sCardNo);

    GET_STR_KEY(pstJson, "input_mode", sInputMode);
    memcpy(sCardMedia, sInputMode, 2);
    tLog(DEBUG, "sCardMedia[%s],sInputMode[%s]", sCardMedia, sInputMode);
    /*卡介质 01 -手工  02 - 磁条  05 - 接触式芯片卡 07 - 非接触式芯片卡   */
    SET_STR_KEY(pstTransJson, "tradeMedium", sCardMedia);

    GET_STR_KEY(pstJson, "card_type", sCardType);
    SET_STR_KEY(pstTransJson, "cardType", sCardType);

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
    if (NULL == fp)
    {
        tLog(DEBUG, "文件[%s]打开失败\n", FullPath);
        return ( -1);
    }
    tLog(DEBUG, "文件[%s]打开成功\n", FullPath);

    //获取一行数据 fgets
    while (fgets(sBuf, 1024, fp))
    {
        if (strstr(sBuf, pcTabName))
        {
            fgets(sBuf, 1024, fp);
            tTrim(sBuf);
            tStrCpy(pcValue, sBuf, strlen(sBuf));
            break;
        }
    }
    fclose(fp);
    fp = NULL;
    return ( 0);
}