/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Gjq
 *
 * Created on 2018��6��8��, ����3:51
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_app_conf.h"
#include "t_redis.h"



int main(int argc, char* pcArgv[]) {

    if (tInitLog(pcArgv[0]) < 0) {
        fprintf(stderr, "��ʼ��[%s]��־ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }
    
    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis����ʧ��.");
        tDoneLog();
        return -1;
    }

    if (tOpenDb() < 0) {
        tLog(ERROR, "���ݿ�����ʧ��.");
        tDoneLog();
        tCloseRedis();
        return -1;
    }
    merchpospro();
    tDoneLog();
    tCloseRedis();
    tCloseDb();
    return (EXIT_SUCCESS);
}