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
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"




int RequestMsg();
int ResponseMsg(char *pcRespMsg);

int SendMsg(char *pcReqSvrId, char *pcKey, cJSON *pstDataJson) {
    cJSON *pstJson = NULL, *pcstSubJson = NULL;
    char sRepSvrId[64] = {0};
    char *pcMsg = NULL;

    /* �жϽ��յ�svrid��Ӧ���svrid�Ƿ�Ϊ�� */
    if (NULL == pcReqSvrId || pcReqSvrId[0] == '\0') {
        tLog(ERROR, "�������[%s]�п�ֵ,�޷�����.", pcReqSvrId);
        return -1;
    }
    /* ���͵Ľ��׶��� */
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());
    SET_STR_KEY(pstJson, "svrid", sRepSvrId);
    SET_STR_KEY(pstJson, "key", pcKey);

    pcstSubJson = cJSON_Duplicate(pstDataJson, 1);

    SET_JSON_KEY(pstJson, "data", pcstSubJson);


    pcMsg = cJSON_PrintUnformatted(pstJson);
    if (tSendMsg(pcReqSvrId, pcMsg, strlen(pcMsg)) < 0) {
        tLog(ERROR, "������Ϣ��SvrId[%s]ʧ��,data[%s].", pcReqSvrId, pcMsg);
    }
    cJSON_Delete(pstJson);
    return 0;
}

/* ������ */
int RequestProc() {
    char sSvrName[64] = {0};

    snprintf(sSvrName, sizeof (sSvrName), "%s_Q", GetSvrId());
    tLog(DEBUG, "[%s]����,�����ɹ�.", sSvrName);
    while (g_iQuitLoop) {
        RequestMsg();
        /* ���5�뷢��һ�β�ѯ */
        sleep(GetTimeInterval());
    }
    tLog(INFO, "[%s]����,�˳��ɹ�.", sSvrName);
    return 0;
}

int RequestMsg() {
    cJSON *pstJson = NULL;  
    int iRet = 0;
    pstJson = cJSON_CreateObject();
    /* ɨ����� */
    iRet = ScanQrTrans(pstJson);
    if (iRet < 0) {
        /* -2����δ���ҵ���ά����ˮ */
        if (iRet != -2)
            tLog(ERROR, "����δ֧����ά����ˮ�����");
        return -1;
    }
    cJSON_Delete(pstJson);
    return 0;
}

/* Ӧ���� */
int ResponseProc() {
    int iRet = -1, iLen = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrName[64] = {0};

    snprintf(sSvrName, sizeof (sSvrName), "%s_P", GetSvrId());
    tLog(DEBUG, "[%s]����,�����ɹ�.", sSvrName);
    while (g_iQuitLoop) {
        memset(sMsg, 0, sizeof (sMsg));
        iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                break;
            case MSG_TIMEOUT:
                tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                //tLog(DEBUG, "response msg[%s]", sMsg);
                if (ResponseMsg(sMsg) < 0) {
                    tLog(ERROR, "Ӧ����ʧ��,��������.");
                }
                break;
        }
    }
    tLog(INFO, "[%s]����,�˳��ɹ�.", sSvrName);
    return 0;
}

int ResponseMsg(char *pcRespMsg) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char sRepSvrId[64] = {0}, sRepKey[64] = {0};
    char sRespCode[2 + 1] = {0}, sMerchOrderNo[27 + 1] = {0}, sTransCode[6 + 1] = {0};

    pstJson = cJSON_Parse(pcRespMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "��ȡ��Ϣʧ��,��������.");
        return -1;
    }
    GET_STR_KEY(pstJson, "svrid", sRepSvrId);
    GET_STR_KEY(pstJson, "key", sRepKey);
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "��ȡdata����ʧ��,��Ϣ��������.");
        cJSON_Delete(pstJson);
        return -1;
    }
    /* ����ҵ�� */
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "qr_order_no", sMerchOrderNo);
    
    
    tLog(ERROR, "������[%s]������[%s]�Ĳ�ѯ���׷�����Ϊ[%s]", sTransCode, sMerchOrderNo, sRespCode);
    if (!memcmp(sRespCode, "00", 2)) {
        //����valid_flagΪ0
        //if (UpdQRTrans(pstDataJson) < 0) {
        //    tLog(ERROR, "NOTICE����������Notice�ɹ���־ʧ��.");
        //}
        tLog(ERROR, "������[%s]������[%s]����֧���ɹ�.", sTransCode, sMerchOrderNo);
    }

    cJSON_Delete(pstJson);
    return 0;
}

