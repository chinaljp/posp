/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <zlog.h>
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "dylib.h"

LibInfo g_stLibInfo;

int InitDyLib(char *pcLibName) {
    char sPath[256] = {0};

    snprintf(sPath, sizeof (sPath), "%s/lib/%s", getenv("HOME"), pcLibName);
    g_stLibInfo.pvLib = dlopen(sPath, RTLD_NOW);
    if (NULL == g_stLibInfo.pvLib) {
        tLog(ERROR, "加载动态库[%s]出错[%s].", sPath, dlerror());
        return -1;
    }
    g_stLibInfo.pfAppServerInit = (pvLibInit) dlsym(g_stLibInfo.pvLib, SVRFUNC_INIT);
    if (NULL == g_stLibInfo.pfAppServerInit) {
        tLog(ERROR, "获取动态库[%s]中函数[%s]指针出错[%d:%s].", sPath, SVRFUNC_INIT, errno, strerror(errno));
        return -1;
    }

    g_stLibInfo.pfAppServerDone = (pvLibDone) dlsym(g_stLibInfo.pvLib, SVRFUNC_DONE);
    if (NULL == g_stLibInfo.pfAppServerDone) {
        tLog(ERROR, "获取动态库[%s]中函数[%s]指针出错[%d:%s].", sPath, SVRFUNC_DONE, errno, strerror(errno));
        return -1;
    }

    g_stLibInfo.pfRequestMsg = (pvRequestMsg) dlsym(g_stLibInfo.pvLib, SVRFUNC_UNPACK);
    if (NULL == g_stLibInfo.pfRequestMsg) {
        tLog(ERROR, "获取动态库[%s]中函数[%s]指针出错[%d:%s].", sPath, SVRFUNC_UNPACK, errno, strerror(errno));
        return -1;
    }

    g_stLibInfo.pfResponseMsg = (pvResponseMsg) dlsym(g_stLibInfo.pvLib, SVRFUNC_PACK);
    if (NULL == g_stLibInfo.pfResponseMsg) {
        tLog(ERROR, "获取动态库[%s]中函数[%s]指针出错[%d:%s].", sPath, SVRFUNC_PACK, errno, strerror(errno));
        return -1;
    }
    g_stLibInfo.pfGetKey = (pvGetKey) dlsym(g_stLibInfo.pvLib, SVRFUNC_GETKEY);
    if (NULL == g_stLibInfo.pfGetKey) {
        tLog(ERROR, "获取动态库[%s]中函数[%s]指针出错[%d:%s].", sPath, SVRFUNC_GETKEY, errno, strerror(errno));
        return -1;
    }
    g_stLibInfo.pfGetSvrName = (pvGetSvrName) dlsym(g_stLibInfo.pvLib, SVRFUNC_SVRNAME);
    if (NULL == g_stLibInfo.pfGetSvrName) {
        tLog(ERROR, "获取动态库[%s]中函数[%s]指针出错[%d:%s].", sPath, SVRFUNC_SVRNAME, errno, strerror(errno));
        return -1;
    }
    return 0;
}

void DoneDyLib() {
    if (g_stLibInfo.pvLib != NULL)
        dlclose(g_stLibInfo.pvLib);
}