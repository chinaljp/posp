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
#include "t_log.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_app_conf.h"
#include "t_redis.h"
#include "t_db.h"

RedisConf g_stRedisConf;
DbConf    g_stDbConf;
HsmConf   g_stHsmConf;

/* 加载redis服务的ip和端口，通过固定路径下的app.conf配置文件 */
int tLoadSvrConf()
{
    char *pcRedisDesc = "c64,c16";
    char *pcRedisName = "REDIS_IP,REDIS_PORT";
    char sFile[256] = {0};

    snprintf(sFile, sizeof (sFile), "%s/etc/%s", getenv("HOME"), APP_NAME);
    if (tGetConfig(sFile, pcRedisDesc, pcRedisName, &g_stRedisConf, "REDIS") < 0) {
        tLog(ERROR, "读取数据库配置文件的[REDIS]配置组出错.");
        return ( -1);
    }
    tLog(DEBUG, "svr ip[%s]port[%s].", g_stRedisConf.sRedisIp, g_stRedisConf.sRedisPort);
    return 0;
}

//加载数据库连接信息
int tLoadDbConf()
{
    char *pcDbDesc = "c64,c64,c64";
    char *pcDbName = "DB_USER,DB_PWD,DB_NAME";
    char sFile[256] = {0};

    snprintf(sFile, sizeof (sFile), "%s/etc/%s", getenv("HOME"), APP_NAME);
    if (tGetConfig(sFile, pcDbDesc, pcDbName, &g_stDbConf, "DB") < 0) {
        tLog(ERROR, "读取数据库配置文件的[DB]配置组出错.");
        return ( -1);
    }
  
    return 0;
}

int tLoadHsmConf()
{
    char *pcHsmDesc = "c64,c16,c64,c16";
    char *pcHsmName = "HSM_IP1,HSM_PORT1,HSM_IP2,HSM_PORT2";
    char sFile[256] = {0};

    snprintf(sFile, sizeof (sFile), "%s/etc/%s", getenv("HOME"), APP_NAME);
    if (tGetConfig(sFile, pcHsmDesc, pcHsmName, &g_stHsmConf, "HSM") < 0) {
        tLog(ERROR, "读取系统配置文件的[HSM]配置组出错.");
        return ( -1);
    }
    tLog(DEBUG, "hsm ip1[%s]port1[%s],hsm ip2[%s]port2[%s].", g_stHsmConf.sHsmIP1, g_stHsmConf.sHsmPort1,g_stHsmConf.sHsmIP2,g_stHsmConf.sHsmPort2);
    return 0;
}

int tOpenRedis()
{
    if ( tLoadSvrConf() < 0 )
    {
        return -1;
    }
        /* 连接redis */
    if (tInitSvr(g_stRedisConf.sRedisIp, atoi(g_stRedisConf.sRedisPort)) < 0) {
        return -1;
    }
    return 0;
}

void tCloseRedis()
{
    tDoneSvr();
}

int tOpenDb()
{
    if ( tLoadDbConf() < 0 )
    {
        return -1;
    }
    
    if (tInitDb(g_stDbConf.sTnsName, g_stDbConf.sUserId, g_stDbConf.sPwd) < 0) {
        return -1;
    }
    return 0;
}

void tCloseDb()
{
    tDoneDb();
}
