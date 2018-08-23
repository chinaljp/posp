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

/* 判断返回结果 */
int JudgeResult(char *pcMsg) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char sRespCode[2 + 1] = {0};
    pstJson = cJSON_Parse(pcMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "获取消息失败,放弃处理.");
        return -1;
    }
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "获取data数据失败,消息放弃处理.");
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

/*银联重置密钥申请*/
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
        tLog(ERROR, "创建Json失败.");
        return;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "创建Key-Json失败.");
        return;
    }
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);

    strcat(sTransmitTime, sDate + 4);
    strcat(sTransmitTime, sTime);
    if (GetSysTrace(sSysTrace) != 0) {
        tLog(ERROR, "生成系统流水号失败.");
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
            tLog(ERROR, "获取消息失败.");
            puts("获取消息失败.");
            break;
        case MSG_TIMEOUT:
            tLog(DEBUG, "[%s]等待消息超时[%d].", "009800", TIMEOUT);
            fprintf(stderr, "[%s]等待消息超时[%d].", "009800", TIMEOUT);
            break;
        case MSG_SUCC:
            /* 失败消息已经在函数内部打印了，无需再判断 */
            if (JudgeResult(sMsg) == 0) {
                tLog(DEBUG, "zpk申请成功!");
                puts("zpk申请成功!");
            } else {
                tLog(DEBUG, "zpk申请失败!");
                puts("zpk申请失败!");
            }
            break;
    }
}

int main(int argc, char* pcArgv[]) {

    if (tInitLog(pcArgv[0]) < 0) {
        fprintf(stderr, "初始化[%s]日志失败.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }
    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis连接失败.");
        return -1;
    }

    if (tOpenDb() < 0) {
        tLog(ERROR, "数据库连接失败.");
        return -1;
    }

    Cups009800();

    tCloseRedis();
    tDoneLog();
    tCloseDb();
    return (EXIT_SUCCESS);
}

