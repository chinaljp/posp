#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "t_tools.h"


static char g_sDbConf[256]; /* db configuration file */
static char g_sAppConf[256]; /* app configuration file */
static char g_cDebugMode;
static char g_sStartTime[8 + 1]; /* 开始时间 */
static char g_sEndTime[8 + 1]; /* 结束时间 */
static char g_sSvrId[8]; /* mSvrInit*/
static char g_sSvcName[64]; /* SvcCall */
static char g_sLogName[64];
static char g_sChannelId[9];
static char g_sAppNum[10 + 1]; /* 应用数量 */

static void ShowVersion() {
    printf("(compile time %s %s)\n", __DATE__, __TIME__);
#ifdef _USE_SOFT
    printf("use soft encrypt instead of HSM\n");
#endif
}

static void ShowHelp() {

    puts("Usage: nacd [-dhvV] [-f CONF_FILE]");
    puts("");
    puts("Options are:");
    puts("-d	Toggle debug flag. -d set debug on.\n");

    puts("-f\tSet configuration file name. File must in `$HOME/etc'");
    puts("	directory. If ommited here, use `$HOME/etc/NACD_CONF' as default.\n");

    puts("-h	Print this help page.\n");

    puts("-v	Show verion information and exit.");

}

int InitParam(int iArgc, char *pcArgv[]) {
    int iOpt;
    int i;

    /* Set initial value */
    g_cDebugMode = 0;
    memset(g_sLogName, 0, sizeof (g_sLogName));
    memset(g_sStartTime, 0, sizeof (g_sStartTime));
    memset(g_sEndTime, 0, sizeof (g_sEndTime));
    memset(g_sSvrId, 0, sizeof (g_sSvrId));
    memset(g_sSvcName, 0, sizeof (g_sSvcName));
    memset(g_sAppConf, 0, sizeof (g_sAppConf));
    strcpy(g_sLogName, pcArgv[0]);
    tGetDate(g_sStartTime, "", -1);
    tAddDay(g_sStartTime, -1);
    g_sAppNum[0] = '1';
    /* 加上-N */
    while ((iOpt = getopt(iArgc, pcArgv, "dhr:R:v:V:s:S:f:i:I:c:C:n:N:")) != -1)
        switch (iOpt) {
            case 'd': /* debug flag */
                g_cDebugMode = 1;
                break;
            case 'h': /* show help infomation */
                ShowHelp();
                return (-1);
            case 'v':
            case 'V':
                ShowVersion();
                return (-1);
            case 'r':
            case 'R':
                /* 日志注册的名称*/
                strncpy(g_sLogName, optarg, 64);
                break;
            case 'c':
            case 'C':
                /* 日志注册的名称*/
                strncpy(g_sChannelId, optarg, 9);
                break;
            case 's':
            case 'S':
                strncpy(g_sStartTime, optarg, 64);
                break;
            case 'i':
            case 'I':
                strncpy(g_sSvrId, optarg, 8);
                break;
            case 'n':
            case 'N':
                strncpy(g_sAppNum, optarg, 10);
                break;
            case 'f':
                strncpy(g_sAppConf, optarg, 254);
                break;
            case '?':
                fprintf(stderr, "无法识别的参数[-%c]", iOpt);
                fprintf(stderr, "输入%s -h查看帮助", g_sLogName);
                return (-1);
            case ':':
                fprintf(stderr, "参数丢失[-%c]", optopt);
                fprintf(stderr, "输入%s -h查看帮助", g_sLogName);
                return (-1);
        }
#if 0
    if (g_sStartTime[0] == '\0') {
        fprintf(stderr, "Basic Usage: %s  -s [StartTime] \n", g_sLogName);
        return ( -1);
    }
#endif
    return (0);
}

int GetAppNum() {
    return atoi(g_sAppNum);
}

int is_debug() {
    return g_cDebugMode;
}

char *GetConfFile() {
    return g_sDbConf;
}

char *GetLogName() {
    return g_sLogName;
}

char *GetAppConfName() {
    return g_sAppConf;
}

char *GetSvcName() {
    return g_sSvcName;
}

char *GetSvrId() {
    return g_sSvrId;
}

char *GetDate() {
    return g_sStartTime;
}

char *GetChannelId() {
    return g_sChannelId;
}


