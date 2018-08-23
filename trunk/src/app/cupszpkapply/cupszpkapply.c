/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2017??2??27??, ????7:52
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "t_app_conf.h"
#include "trace.h"

/*
 * 
 */

/* �жϷ��ؽ�� */
int JudgeResult(char *pcMsg) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char sRespCode[2 + 1] = {0};
    pstJson = cJSON_Parse(pcMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "��ȡ��Ϣʧ��,��������.");
        return -1;
    }
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "��ȡdata����ʧ��,��Ϣ��������.");
        cJSON_Delete(pstJson);
        return -1;
    }
    GET_STR_KEY(pstDataJson, "istresp_code", sRespCode);
    cJSON_Delete(pstJson);

    if (memcmp(sRespCode, "00", 2) == 0) {
        return 0;
    }
    return -1;
}

/*����������Կ����*/
void Cups009800() {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char *pcMsg = NULL;
    char sSysTrace[6 + 1] = {0}, sKey[64] = {0};
    int iLen, iRet;
    char sMsg[MSG_MAX_LEN] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    char sTransmitTime[10 + 1] = {0};

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "����Key-Jsonʧ��.");
        return;
    }
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);

    strcat(sTransmitTime, sDate + 4);
    strcat(sTransmitTime, sTime);
    if (GetSysTrace(sSysTrace) != 0) {
        tLog(ERROR, "����ϵͳ��ˮ��ʧ��.");
    }
    //strcat(sSysTrace, "123456");
    strcat(sKey, "ADMIN_");
    strcat(sKey, sSysTrace);
    SET_STR_KEY(pstJson, "svrid", "009800_P");
    SET_STR_KEY(pstJson, "key", sKey);

    SET_STR_KEY(pstDataJson, "trans_code", "009800");
    SET_STR_KEY(pstDataJson, "sys_trace", sSysTrace);
    SET_STR_KEY(pstDataJson, "transmit_time", sTransmitTime);
    SET_STR_KEY(pstDataJson, "fwd_inst_id", "49000000");
    SET_STR_KEY(pstDataJson, "secure_ctrl", "1600000000000000");


    SET_JSON_KEY(pstJson, "data", pstDataJson);

    pcMsg = cJSON_PrintUnformatted(pstJson);
    tSendMsg("CUPS_Q", pcMsg, strlen(pcMsg));
    free(pcMsg);
    cJSON_Delete(pstJson);

    memset(sMsg, 0, sizeof (sMsg));
    iRet = tRecvMsg("009800_P", sMsg, &iLen, TIMEOUT);
    switch (iRet) {
        case MSG_ERR:
            tLog(ERROR, "��ȡ��Ϣʧ��.");
            puts("��ȡ��Ϣʧ��.");
            break;
        case MSG_TIMEOUT:
            tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", "009800", TIMEOUT);
            fprintf(stderr, "[%s]�ȴ���Ϣ��ʱ[%d].", "009800", TIMEOUT);
            break;
        case MSG_SUCC:
            /* ʧ����Ϣ�Ѿ��ں����ڲ���ӡ�ˣ��������ж� */
            if (JudgeResult(sMsg) == 0) {
                tLog(DEBUG, "zpk����ɹ�!");
                puts("zpk����ɹ�!");
            } else {
                tLog(DEBUG, "zpk����ʧ��!");
                puts("zpk����ʧ��!");
            }
            break;
    }
}

int main(int argc, char* pcArgv[]) {

    if (tInitLog(pcArgv[0]) < 0) {
        fprintf(stderr, "��ʼ��[%s]��־ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }
    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis����ʧ��.");
        return -1;
    }

    if (tOpenDb() < 0) {
        tLog(ERROR, "���ݿ�����ʧ��.");
        return -1;
    }

    Cups009800();

    tCloseRedis();
    tDoneLog();
    tCloseDb();
    return (EXIT_SUCCESS);
}

