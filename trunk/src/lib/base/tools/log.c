/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <zlog.h>
#include "t_log.h"
#include "t_redis.h"

zlog_category_t *g_stLogCxt;

/* 初始化日志 */
int tInitLog(char *pcLogName) {
    int iRet = -1;
    char sFile[256] = {0};
    /* 注册日志 */
    snprintf(sFile, sizeof (sFile), "%s/etc/%s", getenv("HOME"), LOG_NAME);
    iRet = zlog_init(sFile);
    if (iRet) {
        fprintf(stderr, "zlog init err![%s].\n", sFile);
        return -1;
    }
    g_stLogCxt = zlog_get_category(pcLogName);
    if (!g_stLogCxt) {
        printf("get log cat name[%s] fail\n", pcLogName);
        zlog_fini();
        return -1;
    }
    zlog_put_mdc("logname", pcLogName);

    zlog_info(g_stLogCxt, "zlog init succ![%s]", sFile);
    return 0;
}

void tSetLogName(char *pcLogName) {
    zlog_put_mdc("logname", pcLogName);
}

/* 重新加载 */
int tReloadLog() {
    char sFile[256] = {0};
    snprintf(sFile, sizeof (sFile), "%s/etc/%s", getenv("HOME"), LOG_NAME);
    return zlog_reload(sFile);
}

/* 关闭日志，最后一步处理 */
void tDoneLog() {
    zlog_fini();
}

void tDumpHex(char *pcTitle, char *pcData, int iLen) {
    char sTmp[MSG_MAX_LEN] = {0};
    int i;
    for (i = 0; i < iLen; i++) {
        sprintf(sTmp + i * 3, "%02X ", (unsigned char) pcData[i]);
    }
    tLog(DEBUG, "%s:%s", pcTitle, sTmp);
}
