/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * 
 */

/* 
 * File:   main.c
 * Author: Administrator
 *
 * Created on 2017年3月24日, 下午4:02
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_app_conf.h"
#include "t_log.h"
#include "dayend_trans_detail.h"
#include "param.h"

/* 初始化应用，日志等 */
int InitApp() {

    if (tOpenDb() < 0) {
        tLog(ERROR, "数据库连接失败.");
        return -1;
    }
    return 0;
}

/* 释放加载内存和关闭连接等 */
void DoneApp() {
    tCloseDb();
    tDoneLog();
}

int DayEnd() {

    char sLogicDate[8 + 1] = {0};
    if (GetSysParam(&sLogicDate) != 0)
        return -1;
    tLog(INFO, "系统状态[日切],日切处理中...");
    if (CutOff(&sLogicDate) < 0) {
        tLog(ERROR, "系统逻辑日切失败.");
        return ( -S_CUTOFF);
    }

    tLog(INFO, "系统逻辑日切成功,新逻辑日期为[%s].", sLogicDate);
    /* 商户限额检查 */
    if (ChkLimitAmt() < 0) {
        tLog(ERROR, "清理流水失败.");
        return ( -S_LIMIT);
    }
    tLog(ERROR, "系统状态[清理流水],当日流水转历史流水处理中..."); 
    if (ClearProc(&sLogicDate) < 0) {
        tLog(ERROR, "清理流水失败.");
        return ( -S_CLEAR);
    }
    tLog(INFO, "清理流水成功,逻辑日期小于[%s]的交易流水转入历史流水.", sLogicDate);
    if (RecoveryLimit() < 0) {
        tLog(ERROR, "恢复商户D0额度失败.");
        return ( -S_RECOVLIMIT);
    }
    tLog(INFO, "商户D0额度恢复成功.");

    if (RePufaLimit() < 0) {
        tLog(ERROR, "恢复浦发渠道额度失败.");
        return ( -S_RECOVLIMIT);
    }
    tLog(ERROR, "恢复浦发渠道额度成功.");
    if (RePufaLocalMerchLimit() < 0) {
        tLog(ERROR, "恢复浦发本地商户额度失败.");
        return ( -S_RECOVLIMIT);
    }
    tLog(ERROR, "恢复浦发本地商户额度成功.");

    return (0);
}

/*
 * 
 */
int main(int iArgc, char *pcArgv[]) {

    int iRet = 0;

    /* 屏蔽信号，只保留推出信号 */
    tSigProc();

    /* 解析启动参数 */
    if ((iRet = InitParam(iArgc, pcArgv) < 0)) {
        fprintf(stderr, "应用[%s]加载参数失败.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    if (tInitLog("dayend") < 0) {
        fprintf(stderr, "初始化[%s]日志失败.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }

    sigset(SIGUSR1, tSigQuit);

    if (InitApp() < 0) {
        DoneApp();
        return (EXIT_FAILURE);
    }
    tLog(INFO, "日终处理程序始化成功.");

    iRet = DayEnd();

    if (iRet < 0)
        tLog(ERROR, "严重错误!!!日终处理程序运行失败!");
    else
        tLog(INFO, "日终处理程序运行成功!");

    DoneApp();
    return (EXIT_SUCCESS);
}

