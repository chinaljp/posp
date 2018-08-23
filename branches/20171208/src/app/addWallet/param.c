/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <zlog.h>
#include "param.h"
#include "t_tools.h"

static char g_sLogName[64 + 1]; /* 日志名称 */
//static char g_sQSvrId[64 + 1]; /* 请求服务标识 */
static char g_sSvrId[64 + 1]; /* 请求服务标识 */
static char g_sAppNum[10]; /* 应用数量 */
static char g_sLibName[64 + 1]; /* 动态库名称 */
static char g_sInterval[10 + 1]; /* 超时处理，3秒的倍数 */

int GetAppNum() {
    return atoi(g_sAppNum);
}

int GetInterval() {

    return atoi(g_sInterval);
}

char *GetSvrId() {

    return g_sSvrId;
}

char *GetLogName() {

    return g_sLogName;
}

char *GetLibName() {

    return g_sLibName;
}

static void ShowHelp(int iArgc, char* pcArgv[]) {

    printf("Usage: %s [-hH] [-f APP.CONF] \n", pcArgv[0]);
    puts("Options are:");
    puts("-d	Toggle debug flag. -d set debug on.\n");

    puts("-f\tSet configuration file name. File must in `$HOME/etc'");
    puts("	directory. If ommited here, use `$HOME/etc/NACD_CONF' as default.\n");

    puts("-h	Print this help page.\n");

    puts("-v	Show verion information and exit.");

}

/*
 * -l safsvr -s SAF -n 2
 */
int InitParam(int iArgc, char* pcArgv[]) {
    int iOpt = -1;

    /* 默认值 */
    g_sAppNum[0] = '1';
    tStrCpy(g_sLogName, pcArgv[0], strlen(pcArgv[0]));
    memset(g_sSvrId, 0, sizeof (g_sSvrId));
    memset(g_sLibName, 0, sizeof (g_sLibName));
    strcpy(g_sInterval, "30");
    while ((iOpt = getopt(iArgc, pcArgv, "hH:l:L:q:Q:s:S:n:N:t:T:")) != -1)
        switch (iOpt) {
            case 'h': /* show help infomation */
                ShowHelp(iArgc, pcArgv);
                return (-1);
            case 'l':
            case 'L':
                tStrCpy(g_sLogName, optarg, 64);
                break;
            case 't':
            case 'T':
                tStrCpy(g_sInterval, optarg, 10);
                break;
            case 's':
            case 'S':
                tStrCpy(g_sSvrId, optarg, 64);
                break;
            case 'b':
            case 'B':
                tStrCpy(g_sLibName, optarg, 64);
                break;
            case 'n':
            case 'N':
                tStrCpy(g_sAppNum, optarg, 10);
                break;
            case '?':
                fprintf(stderr, "无法识别的参数[-%c]\n", iOpt);
                fprintf(stderr, "输入%s -h查看帮助\n", pcArgv[0]);
                return (-1);
            case ':':
                fprintf(stderr, "参数丢失[-%c]\n", optopt);
                fprintf(stderr, "输入%s -h查看帮助\n", pcArgv[0]);
                return (-1);
        }
    if (g_sAppNum[0] == '\0'
            || g_sLogName[0] == '\0' || g_sInterval[0] == '\0') {
        fprintf(stderr, "Basic Usage: %s  -l [Log Name] -s [Svr Name]  -n [App Num]\n", pcArgv[0]);
        return -1;
    }
    fprintf(stderr, "%s  -s %s -n %s -t %s -l %s 成功\n", pcArgv[0], g_sSvrId, g_sAppNum, g_sInterval, g_sLogName);
    return 0;
}