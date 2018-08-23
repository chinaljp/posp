/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2016年11月30日, 下午9:00
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
#include "t_redis.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "t_log.h"
#include "param.h"
#include "dbop.h"

void SndProc(char *pcArgv[]) {

    /* 不结算的交易 */
    if (NoSettleProc() < 0)
        return -1;

    /* 对账平 */
    if (EqualProc() < 0)
        return -1;

    /* 长款 */
    if (LongProc() < 0)
        return -1;
#if 0
    /* 短款 */
    if (ShortProc() < 0)
        return -1;
#endif

    return;
}