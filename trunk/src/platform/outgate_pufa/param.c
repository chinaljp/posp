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
#include "t_tools.h"
#include "param.h"

static char g_sLogName[64 + 1]; /* ��־���� */
static char g_sSvrName[64 + 1]; /* �������� */
static char g_sSvrId[64 + 1]; /* ����ID */
static char g_sAppNum[10 + 1]; /* Ӧ������ */
static char g_sIpL[64 + 1]; /* ip��ַ */
static char g_sPortL[10 + 1]; /*�˿�*/
static char g_sQname[20 + 1]; /*���������*/
static char g_sIpC[64 + 1]; /* ip��ַ */
static char g_sPortC[10 + 1]; /*�˿�*/
static char g_sMode[64 + 1]; /* 0ͬ��  1�첽*/
static char g_sConfig[64 + 1]; /* Ӧ������ */
static char g_cPktLenType; /*���ĳ������� Ĭ��asc��*/
static int g_iMaxTran = 200; /*�������*/
static int g_iMaxMulti = 500; /* ��󲢷��� */
static int g_iTranTimeout = 40; /* ���׳�ʱʱ�� */
static int g_iTcpTimeout = 600; /*tcp���ӳ�ʱʱ��*/

int GetAppNum() {
    return atoi(g_sAppNum);
}

char *GetSvrName() {
    return g_sSvrId;
}

char *GetSvrId() {
    return g_sSvrId;
}

char *GetLogName() {
    return g_sLogName;
}

char *GetQname() {
    return g_sQname;
}

char *GetIpL() {
    return g_sIpL;
}

char *GetIpC() {
    return g_sIpC;
}

char *GetPortL() {
    return g_sPortL;
}

char *GetPortC() {
    return g_sPortC;
}

char *GetComMode() {
    return g_sMode;
}

char *GetConfig() {
    return g_sConfig;
}

int GetMaxMulti() {
    return g_iMaxMulti;
}

int GetMaxTrans() {
    return g_iMaxTran;
}

int GetTranTimeout() {
    return g_iTranTimeout;
}

int GetTcpTimeout() {
    return g_iTcpTimeout;
}

int GetPktLenType() {
    return g_cPktLenType;
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
 * -l log_name -s pos_req -n 2
 */
int InitParam(int iArgc, char* pcArgv[]) {
    int iOpt = -1;

    /* Ĭ��ֵ */
    g_sAppNum[0] = '1';
    tStrCpy(g_sLogName, pcArgv[0], strlen(pcArgv[0]));
    memset(g_sSvrName, 0, sizeof (g_sSvrName));
    memset(g_sSvrId, 0, sizeof (g_sSvrId));
    memset(g_sConfig, 0, sizeof (g_sConfig));
    memset(g_sIpL, 0, sizeof (g_sIpL));
    memset(g_sPortL, 0, sizeof (g_sPortL));
    memset(g_sIpC, 0, sizeof (g_sIpC));
    memset(g_sPortC, 0, sizeof (g_sPortC));
    memset(g_sMode, 0, sizeof (g_sMode));
    while ((iOpt = getopt(iArgc, pcArgv, "hH:d:D:l:L:i:I:f:F:p:P:Q:s:S:t:n:N:")) != -1)
        switch (iOpt) {
            case 'h': /* show help infomation */
                ShowHelp(iArgc, pcArgv);
                return (-1);
            case 'd':
            case 'D':
                tStrCpy(g_sMode, optarg, 64);
                //g_sCommMode[0]=optarg[0];
                break;
            case 'l':
            case 'L':
                tStrCpy(g_sLogName, optarg, 64);
                break;
            case 's':
                tStrCpy(g_sSvrId, optarg, 64);
                break;
            case 'S':
                tStrCpy(g_sSvrName, optarg, 64);
                break;
            case 'Q':
                tStrCpy(g_sQname, optarg, 64);
                break;
            case 't':
                g_cPktLenType = *optarg;
                break;
            case 'f':
            case 'F':
                tStrCpy(g_sConfig, optarg, 64);
                break;
            case 'i':
                tStrCpy(g_sIpL, optarg, 64);
                break;
            case 'I':
                tStrCpy(g_sIpC, optarg, 64);
                break;
            case 'p':
                tStrCpy(g_sPortL, optarg, 64);
            case 'P':
                tStrCpy(g_sPortC, optarg, 64);
                break;
            case 'n':
            case 'N':
                tStrCpy(g_sAppNum, optarg, 10);
                break;
            case '?':
                fprintf(stderr, "�޷�ʶ��Ĳ���[-%c]\n", iOpt);
                fprintf(stderr, "����%s -h�鿴����\n", pcArgv[0]);
                return (-1);
            case ':':
                fprintf(stderr, "������ʧ[-%c]\n", optopt);
                fprintf(stderr, "����%s -h�鿴����\n", pcArgv[0]);
                return (-1);
        }
    if (g_sLogName[0] == '\0') {
        fprintf(stderr, "Basic Usage: %s -l [Log Name]\n", pcArgv[0]);
        fprintf(stderr, "%s -l %s\n", pcArgv[0], g_sLogName);
        return -1;
    }
    fprintf(stderr, "%s -f %s -s %s -n %s  -l %s -c %s -i %s -p %s -I %s -P %s -t %c -Q %s �ɹ�\n", pcArgv[0], g_sConfig, g_sSvrName, g_sAppNum, g_sLogName, g_sMode, g_sIpL, g_sPortL, g_sIpC, g_sPortC, g_cPktLenType, g_sQname);
    return 0;
}