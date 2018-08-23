/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dylib.h
 * Author: feng.gaoo
 *
 * Created on 2016年12月12日, 下午7:55
 */

#ifndef DYLIB_H
#define DYLIB_H

#ifdef __cplusplus
extern "C" {
#endif
#include "t_cjson.h"

#define SVRFUNC_INIT        "AppServerInit"
#define SVRFUNC_UNPACK      "RequestMsg"
#define SVRFUNC_PACK        "ResponseMsg"
#define SVRFUNC_GETKEY      "AppGetKey"
#define SVRFUNC_SVRNAME     "GetSvrName"
#define SVRFUNC_DONE        "AppServerDone"

    /* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
    typedef int (*pvLibInit)(int iArgc, char *pcArgv[]);
    /* 模块退出清理接口:针对初始化中的操作进行清理 */
    typedef int (*pvLibDone)(void);
    /* 拆包接口 */
    typedef int ( *pvRequestMsg)(cJSON *pstReqDataJson, cJSON *pstDataJson);
    /* 拆包接口 */
    typedef int ( *pvResponseMsg)(cJSON *pstRepDataJson, cJSON *pstDataJson);
    /* 获取交易主键接口 */
    typedef int ( *pvGetKey)(char *pcKey, size_t tKey, cJSON *pstInJson, cJSON *pstOutJson);
    /* 获取下一个节点的服务名接口 */
    typedef int ( *pvGetSvrName)(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson);

    typedef struct {
        pvLibInit pfAppServerInit;
        pvLibDone pfAppServerDone;
        pvRequestMsg pfRequestMsg;
        pvResponseMsg pfResponseMsg;
        pvGetKey pfGetKey;
        pvGetSvrName pfGetSvrName;
        void *pvLib; /* 动态库访问句柄 */
    } LibInfo; /* 模块的动态库信息 */

    extern LibInfo g_stLibInfo;

    int InitDyLib(char *pcLibName);
    void DoneDyLib();

#ifdef __cplusplus
}
#endif

#endif /* DYLIB_H */

