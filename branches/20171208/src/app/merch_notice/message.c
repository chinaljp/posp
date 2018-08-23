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
#include <time.h>
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"
#include "t_macro.h"
#include "tKms.h"

int FindValueByKey(char *pcValue, char *pcKey) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key='%s'", pcKey);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找Key[%s]的值失败.", pcKey);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcValue);
        tTrim(pcValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无Key[%s]的值.", pcKey);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

void MerchProc() {
    /* 扫描账户通知表,自动发送交易 */
    char sSqlStr[1024] = {0}, sSql[256] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sData[2048] = {0};
    char sReqSvrId[SVRID_LEN + 1] = {0}, sKey[MSG_KEY_LEN + 1] = {0}, sRepSvrId[SVRID_LEN + 1] = {0};
    char sIdCardNo[128] = {0}, sBankAccountNo[128] = {0}, sBankAccountName[128] = {0};
    char sIdCardNoDec[128] = {0}, sBankAccountNoDec[128] = {0}, sBankAccountNameDec[128] = {0};
    char sTelDec[128] = {0}, sTel[128] = {0};
    char sKeyuKey[128] = {0};
    char sId[32 + 1] = {0}, *pcMsg = NULL, *pcTmp = NULL;
    OCI_Resultset *pstRes = NULL, *pstUpdRes = NULL;
    int iRet = -1;
    cJSON *pstDataJson = NULL, *pstNetJson = NULL;
    MsgData stMsgData;

    tLog(WARN, "开始处理商户注册......");
    if (FindValueByKey(sKeyuKey, "kms.encdata.key") < 0) {
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    tLog(DEBUG, "%s", sKeyuKey);
    // snprintf(sRepSvrId, sizeof (sReqSvrId), "%s_P", GetSvrId());
    //tLog(DEBUG, "p svrid[%s]", sRepSvrId);
    snprintf(sSqlStr, sizeof (sSqlStr), "select id,trans_code,launch_content from B_NOTICE_WAllET "
            " where status='0'and notice_cnt<11 order by create_time");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "err");
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sId);
        STRV(pstRes, 2, sTransCode);
        STRV(pstRes, 3, sData);
        tLog(DEBUG, "data[%s]", sData);
        pstNetJson = cJSON_Parse(sData);
        if (NULL == pstNetJson) {
            tLog(ERROR, "转换通知消息为json,放弃处理.");
            continue;
        }
        pstDataJson = cJSON_CreateObject();
        if (NULL == pstDataJson) {
            tLog(ERROR, "创建发送dataJson失败.");
            cJSON_Delete(pstNetJson);
            continue;
        }
        SET_STR_KEY(pstNetJson, "trans_code", sTransCode);
        SET_STR_KEY(pstNetJson, "id", sId);

        GET_STR_KEY(pstNetJson, "idCardNo", sIdCardNoDec);
        GET_STR_KEY(pstNetJson, "bankAccountNo", sBankAccountNoDec);
        GET_STR_KEY(pstNetJson, "bankAccountName", sBankAccountNameDec);
        GET_STR_KEY(pstNetJson, "mobile", sTelDec);
        if (sIdCardNoDec[0] != '\0') {
            tLog(DEBUG, "sIdCardNoDec[%s]", sIdCardNoDec);
            if (tHsm_Dec_Asc_Data(sIdCardNo, sKeyuKey, sIdCardNoDec) < 0) {
                tLog(DEBUG, "tHsm_Dec_Asc_Data idCardNo");
                continue;
            }
            tLog(DEBUG, "sIdCardNo[%s]", sIdCardNo);
            SET_STR_KEY(pstNetJson, "idCardNo", sIdCardNo);
        }
        if (sTelDec[0] != '\0') {
            tLog(DEBUG, "sTelDec[%s]", sTelDec);
            if (tHsm_Dec_Asc_Data(sTel, sKeyuKey, sTelDec) < 0) {
                tLog(DEBUG, "tHsm_Dec_Asc_Data mobile");
                continue;
            }
            tLog(DEBUG, "sTel[%s]", sTel);
            SET_STR_KEY(pstNetJson, "mobile", sTel);
        }
        if (sBankAccountNoDec[0] != '\0') {
            tLog(DEBUG, "sBankAccountNoDec[%s]", sBankAccountNoDec);
            if (tHsm_Dec_Asc_Data(sBankAccountNo, sKeyuKey, sBankAccountNoDec) < 0) {
                tLog(DEBUG, "tHsm_Dec_Asc_Data sBankAccountNo");
                continue;
            }
            tLog(DEBUG, "sBankAccountNo[%s]", sBankAccountNo);
            SET_STR_KEY(pstNetJson, "bankAccountNo", sBankAccountNo);
        }
        if (sBankAccountNameDec[0] != '\0') {
            tLog(DEBUG, "sBankAccountNameDec[%s]", sBankAccountNameDec);
            if (tHsm_Dec_Asc_Data(sBankAccountName, sKeyuKey, sBankAccountNameDec) < 0) {
                tLog(DEBUG, "tHsm_Dec_Asc_Data sBankAccountNo");
                continue;
            }
            tLog(DEBUG, "sBankAccountName[%s]", sBankAccountName);
            SET_STR_KEY(pstNetJson, "bankAccountName", sBankAccountName);
        }

        pcMsg = cJSON_PrintUnformatted(pstNetJson);
        MEMSET_ST(stMsgData);
        SET_STR_KEY(pstDataJson, "msg", pcMsg);
        /* 发送新的消息 */
        tGetUniqueKey(sKey);
        snprintf(sReqSvrId, sizeof (sReqSvrId), "%s_Q", "WEBD");
        //strcpy(stMsgData.sSvrId, sId);
        strcpy(stMsgData.sKey, sId);
        stMsgData.pstDataJson = pstDataJson;
        iRet = tSvcACall(sReqSvrId, &stMsgData);
        //iRet = tSvcACall(sReqSvrId, NULL, sId, pstDataJson);
        free(pcMsg);
        cJSON_Delete(pstDataJson);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(WARN, "无账户通知.");
        tReleaseRes(pstRes);
        return 0;
    }
    tReleaseRes(pstRes);
    tLog(DEBUG, "end");
    return 0;
}
