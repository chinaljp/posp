/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2016��11��30��, ����9:00
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

    /* ������Ľ��� */
    if (NoSettleProc() < 0)
        return -1;

    /* ����ƽ */
    if (EqualProc() < 0)
        return -1;

    /* ���� */
    if (LongProc() < 0)
        return -1;
#if 0
    /* �̿� */
    if (ShortProc() < 0)
        return -1;
#endif

    return;
}