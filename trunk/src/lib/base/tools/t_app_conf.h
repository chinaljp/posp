/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t_app_conf.h
 * Author: feng.gaoo
 *
 * Created on 2016年12月11日, 下午11:08
 */

#ifndef T_APP_CONF_H
#define T_APP_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char sRedisIp[64];
        char sRedisPort[16];
    } RedisConf;

    typedef struct {
        char sUserId[64];
        char sPwd[64];
        char sTnsName[64];
    } DbConf;

    typedef struct {
        char sHsmIP1[64];
        char sHsmPort1[16];
        char sHsmIP2[64];
        char sHsmPort2[64];
    } HsmConf;

#define APP_NAME    "app.conf"

    //  extern RedisConf g_stRedisConf;
    //  extern DbConf    g_stDbConf;

    //连接redis
    int tOpenRedis();
    void tCloseRedis();
    //连接数据库
    int tOpenDb();
    void tCloseDb();


#ifdef __cplusplus
}
#endif

#endif /* T_APP_CONF_H */

