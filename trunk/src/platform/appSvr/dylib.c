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
#include "dylib.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "param.h"

LibInfo g_stLibInfo_Q, g_stLibInfo_P; /* Q������������P��Ӧ������ */

int LoadTrans(LibInfo *pstLIbInfo, char *pcConfig, char *pcTransName) {
    int i;
    char *pcTranDesc = "i,(c33,c9)";
    char *pcTranName = "SEQ_NUM,SEQ";
    char sFile[256] = {0};

    snprintf(sFile, sizeof (sFile), "%s/etc/%s", getenv("HOME"), pcConfig);
    if (tGetConfig(sFile, pcTranDesc, pcTranName, &pstLIbInfo->stNormalFlowCfg, pcTransName) < 0) {
        tLog(ERROR, "��ȡ���ݿ������ļ���[%s]���������.", pcTransName);
        return ( -1);
    }
    for (i = 0; i < pstLIbInfo->stNormalFlowCfg.iFuncNum; i++) {
        tLog(DEBUG, "No[%d] Func[%s] work[%s]"
                , i, pstLIbInfo->stNormalFlowCfg.pstNormalFlow[i].sFuncId
                , pstLIbInfo->stNormalFlowCfg.pstNormalFlow[i].sTransactAttr);
    }
    if (0 == i) {
        tLog(ERROR, "[%s]������[%s]��������������.", sFile, pcTransName);
        return -1;
    }
    return 0;
}

int LoadQTrans(char *pcConfig, char *pcTransName) {
    return LoadTrans(&g_stLibInfo_Q, pcConfig, "REQ");
}

int LoadPTrans(char *pcConfig, char *pcTransName) {
    return LoadTrans(&g_stLibInfo_P, pcConfig, "REP");
}

int InitDyLib(LibInfo *pstLIbInfo, char *pcTransName, char *pcLibName) {
    char sPath[256] = {0};

    tStrCpy(pstLIbInfo->sSvrName, pcTransName, 6);
    snprintf(sPath, sizeof (sPath), "%s/lib/%s", getenv("HOME"), pcLibName);
    pstLIbInfo->pvLib = dlopen(sPath, RTLD_NOW);
    if (NULL == pstLIbInfo->pvLib) {
        tLog(ERROR, "���ض�̬��[%s]����[%s].", sPath, dlerror());
        return -1;
    }
    pstLIbInfo->pfAppServerInit = (pvLibInit) dlsym(pstLIbInfo->pvLib, SVRFUNC_INIT);
    if (NULL == pstLIbInfo->pfAppServerInit) {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_INIT, errno, strerror(errno));
        return -1;
    }

    pstLIbInfo->pfAppServerDone = (pvLibDone) dlsym(pstLIbInfo->pvLib, SVRFUNC_DONE);
    if (NULL == pstLIbInfo->pfAppServerDone) {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_DONE, errno, strerror(errno));
        return -1;
    }
    pstLIbInfo->pfGetKey = (pvGetKey) dlsym(pstLIbInfo->pvLib, SVRFUNC_GETKEY);
    if (NULL == pstLIbInfo->pfGetKey) {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_GETKEY, errno, strerror(errno));
        return -1;
    }
    pstLIbInfo->pfGetSvrName = (pvGetSvrName) dlsym(pstLIbInfo->pvLib, SVRFUNC_SVRNAME);
    if (NULL == pstLIbInfo->pfGetSvrName) {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_SVRNAME, errno, strerror(errno));
        return -1;
    }
    return 0;
}

int InitQDyLib(char *pcTransName, char *pcLibName) {
    return InitDyLib(&g_stLibInfo_Q, pcTransName, pcLibName);
}

int InitPDyLib(char *pcTransName, char *pcLibName) {
    return InitDyLib(&g_stLibInfo_P, pcTransName, pcLibName);
}

void DoneQDyLib() {
    if (g_stLibInfo_Q.pvLib != NULL)
        dlclose(g_stLibInfo_Q.pvLib);
}

void DonePDyLib() {
    if (g_stLibInfo_P.pvLib != NULL)
        dlclose(g_stLibInfo_P.pvLib);
}

/* ִ�н��״������еĵ��������� */
int DoStep(LibInfo *pstLIbInfo, cJSON *pstJson) {
    int iFlag = 0, iRet = 0, iNo = 0;
    NormalFlow *pstNormalFlow = NULL;
    pvFuncProc pvFunc;

    pstNormalFlow = pstLIbInfo->stNormalFlowCfg.pstNormalFlow;
    do {
        /* �ҵ��߱���ִ�к��� */
        tLog(INFO, "===>����[%s]Step[%d]����[%s] ����[%s]."
                , pstLIbInfo->sSvrName, iNo, pstNormalFlow[iNo].sFuncId, pstNormalFlow[iNo].sTransactAttr);
        pvFunc = (pvFuncProc) dlsym(pstLIbInfo->pvLib, pstNormalFlow[iNo].sFuncId);
        if (pvFunc == NULL) {
            tLog(ERROR, "��̬��ȡ��̬��[%p]�еĺ���[%s]ָ��ʧ��.", pstLIbInfo->pvLib, pstNormalFlow[iNo].sFuncId);
            iRet = -1;
            break;
        }
        if (BEGIN_WORK == pstNormalFlow[iNo].sTransactAttr[0]) {
            NULL;
        }
        iRet = pvFunc(pstJson, &iFlag);
        tLog(INFO, "<===����[%s]ִ�н���,ִ�н��[%d]", pstNormalFlow[iNo].sFuncId, iRet);
        /* ʧ���˳� */
        if (iRet < 0) {
            tRollback();
            iRet = -1;
            break;
        }
        /* �ж����õ�����,�����һ������������1���ύ��������� */
        if ('1' == pstNormalFlow[iNo].sTransactAttr[0]
                && (iNo + 1) < pstLIbInfo->stNormalFlowCfg.iFuncNum
                && '0' == pstNormalFlow[iNo + 1].sTransactAttr[0]) {
            tCommit();
        }
        /* ����ִ����һ���� */
        iNo++;
    } while (iNo < pstLIbInfo->stNormalFlowCfg.iFuncNum);
    tLog(INFO, "������ִ�н���.");
    /* ǿ���ύ���� */
    if (iRet == 0) {
        tCommit();
    }
    return iRet;
}