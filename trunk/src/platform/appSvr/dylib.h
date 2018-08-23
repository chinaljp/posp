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

#define BEGIN_WORK      '1'   /* 开始事务 */
#define COMMIT_WORK     '2'   /* 提交事务 */

#define SVRFUNC_INIT        "AppServerInit"
#define SVRFUNC_DO          "AppServerDo"
#define SVRFUNC_GETKEY      "AppGetKey"
#define SVRFUNC_SVRNAME     "GetSvrName"
#define SVRFUNC_DONE        "AppServerDone"

#define NORMALFLOW_NUM          32      /* 每只交易最大正常处理步骤数   */

    typedef struct {
        char sFuncId[32 + 1]; /* 处理函数名称 */
        char sTransactAttr[8 + 1]; /* 事务处理控制属性 */
    } NormalFlow; /* 正常处理流信息 */

    typedef struct {
        int iFuncNum;
        NormalFlow *pstNormalFlow;
    } NormalFlowCfg;

    /* 模块初始化接口:解析自定义的命令行参数，作个性化初始化 */
    typedef int (*pvLibInit)(int iArgc, char *sArgv[]);
    /* 模块退出清理接口:针对初始化中的操作进行清理 */
    typedef int (*pvLibDone)(void);
    /* 交易流接口 */
    typedef int ( *pvFuncProc)(cJSON *pstJson, int *piFlag);
    /* 获取交易主键接口 */
    typedef int ( *pvGetKey)(char *pcKey, size_t tKey, cJSON *pstJson);
    /* 获取下一个节点的服务名接口 */
    typedef int ( *pvGetSvrName)(char *pSvrId, size_t tSvrId, cJSON *pstJson);

    typedef struct {
        char sSvrName[256 + 1]; /* 服务名称(交易名称) */
        char sLibName[256 + 1]; /* 动态库名称 */
        NormalFlowCfg stNormalFlowCfg;
        pvLibInit pfAppServerInit;
        pvLibDone pfAppServerDone;
        pvGetKey pfGetKey;
        pvGetSvrName pfGetSvrName;
        void *pvLib; /* 动态库访问句柄 */
    } LibInfo; /* 模块的动态库信息 */

    extern LibInfo g_stLibInfo_Q, g_stLibInfo_P;


#ifdef __cplusplus
}
#endif

#endif /* DYLIB_H */

