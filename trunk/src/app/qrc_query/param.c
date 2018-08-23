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

static char g_sLogName[64]; /* 日志名称 */
static char g_sSvrName[64]; /* 服务名称 */
static char g_sAppNum[10]; /* 应用数量 */
static char g_sLibName[64 + 1]; /* 动态库名称 */
static char g_sTimeInterval[10]; /* 自动扫库间隔 */

int GetAppNum() {
    return atoi(g_sAppNum);
}

char *GetSvrId() {
    return g_sSvrName;
}

char *GetLogName() {
    return g_sLogName;
}

char *GetLibName() {
    return g_sLibName;
}

int GetTimeInterval() {
    return atoi(g_sTimeInterval);
}

static void ShowHelp(int iArgc, char* pcArgv[]) {

    printf("Usage: %s [-hH] [-f APP.CONF] \n", pcArgv[0]);
    puts("Options are:");
    puts("-d	Toggle debug flag. -d set debug on.\n");

    puts("-f\tSet configuration file name. File must in `$HOME/etc'");
    puts("	directory. If ommited here, use `$HOME/etc/NACD_CONF' as default.\n");

    puts("-h	Print this help page.\n");

    puts("-v	Show verion information and exit.");

    puts("-c  Maximum number of notifications.");

}

/*
 * -l safsvr -s SAF -n 2
 */
int InitParam(int iArgc, char* pcArgv[]) {
    int iOpt = -1;

    /* 默认值 */
    g_sAppNum[0] = '1';
    tStrCpy(g_sLogName, pcArgv[0], strlen(pcArgv[0]));
    memset(g_sSvrName, 0, sizeof (g_sSvrName));
    memset(g_sLibName, 0, sizeof (g_sLibName));
    while ((iOpt = getopt(iArgc, pcArgv, "hH:l:L:b:B:s:S:n:N:T:t:")) != -1)
        switch (iOpt) {
            case 'h': /* show help infomation */
                ShowHelp(iArgc, pcArgv);
                return (-1);
            case 'l':
            case 'L':
                tStrCpy(g_sLogName, optarg, 64);
                break;
            case 's':
            case 'S':
                tStrCpy(g_sSvrName, optarg, 64);
                break;
            case 'b':
            case 'B':
                tStrCpy(g_sLibName, optarg, 64);
                break;
            case 'n':
            case 'N':
                tStrCpy(g_sAppNum, optarg, 10);
                break;
            case 't':
            case 'T':
                tStrCpy(g_sTimeInterval, optarg, 10);
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
            || g_sLogName[0] == '\0' || g_sSvrName[0] == '\0' || g_sTimeInterval[0] == '\0') {
        fprintf(stderr, "Basic Usage: %s  -l [Log Name] -s [Svr Name]  -n [App Num] -t [Time Interval]\n", pcArgv[0]);
        return -1;
    }
    fprintf(stderr, "%s  -s %s -n %s -l %s -b %s -t %s 成功\n", pcArgv[0], g_sSvrName, g_sAppNum, g_sLogName, g_sLibName, g_sTimeInterval);
    return 0;
}