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
    int iRet = -1, iLen = 0, iCnt = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrName[64] = {0};
    time_t tmStart;
    struct timeval tvS, tvS1;

    snprintf(sSvrName, sizeof (sSvrName), "%s_Q", GetSvrId());
    tLog(DEBUG, "[%s]����,�����ɹ�.", sSvrName);
    //��ʼʱ������5���ӡһ��δ�鵽�������־
    time(&tmStart);
    gettimeofday(&tvS, NULL);
    tvS1 = tvS;
    while (g_iQuitLoop) {
        QrRequestMsg(&tvS);
        EposRequestMsg(&tvS1);
        //sleep(0.5);
        //���߰�����
        usleep(500000);
    }
    tLog(INFO, "[%s]����,�˳��ɹ�.", sSvrName);
    return 0;
}

int QrRequestMsg(struct timeval *tvS) {
    int iRet = 0;
    cJSON *pstJson = NULL;
    char *pcReqSvrId = NULL, *pcReqKey = NULL;
    char sReqSvrId[64] = {0}, sKey[64] = {0}, sRrn[16 + 1] = {0}, sTransCode[16 + 1] = {0};
    char sQueryCount[2] = {0}, sTransTime[16 + 1] = {0};
    int iSecs = 0;
    time_t tmEnd;
    struct timeval tvE;

    /* ɨ����� */
    //����ʱ��
    time(&tmEnd);
    gettimeofday(&tvE, NULL);
    iSecs = tvE.tv_sec - (*tvS).tv_sec;
    //5���ӡһ����־��ֹ������־��̫��
    if (iSecs > 3) {
        *tvS = tvE;
        iRet = ScanQrTrans(tvS);
        if (iRet < 0) {
            /* -2����δ���ҵ���ά����ˮ */
            if (iRet != -2)
                tLog(ERROR, "���Ҵ��ص��Ķ�ά����ˮ����");
            return -1;
        }
        return 0;
    }
    return 0;
}

int EposRequestMsg(struct timeval * tvS) {
    int iRet = 0;
    cJSON *pstJson = NULL;
    char *pcReqSvrId = NULL, *pcReqKey = NULL;
    char sReqSvrId[64] = {0}, sKey[64] = {0}, sRrn[16 + 1] = {0}, sTransCode[16 + 1] = {0};
    char sQueryCount[2] = {0}, sTransTime[16 + 1] = {0};

    pstJson = cJSON_CreateObject();

    /* ɨ����� */
    iRet = ScanEposTrans(tvS);
    if (iRet < 0) {
        /* -2����δ���ҵ�Epos��ˮ */
        if (iRet != -2)
            tLog(ERROR, "���Ҵ��ص���Epos��ˮ����");
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
    char sMerchOrderNo[64] = {0};
    char sRespCode[2 + 1] = {0};

    pstJson = cJSON_Parse(pcRespMsg);

    /* ����ҵ�� */
    GET_STR_KEY(pstJson, "respCode ", sRespCode);
    GET_STR_KEY(pstJson, "orderNo", sMerchOrderNo);

    if (!memcmp(sRespCode, "00", 2)) {
        //����valid_flagΪ0
        if (UpdQRNoticeFlag(pstJson) < 0) {
            tLog(ERROR, "NOTICE����������Notice�ɹ���־ʧ��.");
        }
        tLog(ERROR, "��ά�뽻�׻ص��ɹ�.");
    }

    cJSON_Delete(pstJson);
    return 0;
}