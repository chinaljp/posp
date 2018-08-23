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
#include <ocilib.h>
#include "t_tools.h"
#include "t_db.h"
#include "dylib.h"
#include "t_config.h"
#include "t_log.h"
#include "param.h"


LibInfo g_stLibInfo_Q, g_stLibInfo_P; /* Q������������P��Ӧ������ */
#if 0

int LoadTrans(LibInfo *pstLIbInfo, char *pcConfig, char *pcTransName) {
    int i, iGroupId = 0;
    char *pcTranDesc = "i,(c33,c9)";
    char *pcTranName = "SEQ_NUM,SEQ";
    char sFile[256] = {0};
    char sTransName[64] = {0};

    snprintf(sFile, sizeof (sFile), "%s/etc/%s", getenv("HOME"), pcConfig);
    /*
     * ������һ�����̴���һ֧���ף����ڸ�Ϊһ����������һ�齻��
     */
    if (tGetConfig(sFile, pcTranDesc, pcTranName, &pstLIbInfo->stNormalFlowCfg, sTransName) < 0)
    {
        tLog(ERROR, "��ȡ���ݿ������ļ���[%s]���������.", sTransName);
        return ( -1);
    }
    for (i = 0; i < pstLIbInfo[iGroupId]->stNormalFlowCfg.iFuncNum; i++)
    {
        tLog(DEBUG, "No[%d] Func[%s] work[%s]"
                , i, pstLIbInfo[iGroupId]->stNormalFlowCfg.pstNormalFlow[i].sFuncId
                , pstLIbInfo[iGroupId]->stNormalFlowCfg.pstNormalFlow[i].sTransactAttr);
    }
    iGroupId++;
    if (0 == i)
    {
        tLog(ERROR, "[%s]������[%s]��������������.", sFile, sTransName);
        return -1;
    }
    return 0;
}
#endif

int LoadTrans(LibInfo *pstLIbInfo, char *pcQueueType, char *pcGroupId) {
    char sSqlStr[256] = {0}, sTransCode[6 + 1] = {0};
    char sAttr[2] = {0}, sFuncId[50 + 1] = {0};
    int iGroupNum = 0, i, iFlowId = 0;
    OCI_Resultset *pstRes = NULL;

    /* �������ڵĽ����� */
    snprintf(sSqlStr, sizeof (sSqlStr), "select t.TRANS_CODE from S_TRANS_CODE t where t.GROUP_CODE='%s'", pcGroupId);
    if (tExecute(&pstRes, sSqlStr) < 0)
    {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes))
    {
        STRV(pstRes, 1, sTransCode);
        tStrCpy(pstLIbInfo->staTransCode[iGroupNum++].sTransCode, sTransCode, 6);
    }
    pstLIbInfo->iGroupNum = iGroupNum;
    tReleaseRes(pstRes);
    if (0 == OCI_GetRowCount(pstRes))
    {
        tLog(ERROR, "GroupId[%s]�޽���������,�����[S_TRANS_CODE].", pcGroupId);
        return -1;
    }
    /* ���ҽ����� */
    for (i = 0; i < pstLIbInfo->iGroupNum; i++)
    {
        snprintf(sSqlStr, sizeof (sSqlStr), "select trans_code,FLOW_ID,FUNC_ID,ATTR from S_TRANS_FLOW t"
                " where t.TRANS_CODE='%s' and t.QUEUE_TYPE='%s' order by t.FLOW_ID ASC"
                , pstLIbInfo->staTransCode[i].sTransCode, pcQueueType);
        if (tExecute(&pstRes, sSqlStr) < 0)
        {
            tLog(ERROR, "sql[%s] err!", sSqlStr);
            return -1;
        }
        while (OCI_FetchNext(pstRes))
        {
            STRV(pstRes, 1, sTransCode);
            INTV(pstRes, 2, iFlowId);
            STRV(pstRes, 3, sFuncId);
            STRV(pstRes, 4, sAttr);
            tStrCpy(pstLIbInfo->staTransCode[i].staTransFlow[iFlowId].sFuncId, sFuncId, strlen(sFuncId));
            tStrCpy(pstLIbInfo->staTransCode[i].staTransFlow[iFlowId].sTransactAttr, sAttr, strlen(sAttr));
        }
        pstLIbInfo->staTransCode[i].iFuncNum = iFlowId + 1;
        tReleaseRes(pstRes);
        if (0 == OCI_GetRowCount(pstRes))
        {
            tLog(ERROR, "GroupId[%s]TransCode[%s:%s]�޽���������.", pcGroupId, pstLIbInfo->staTransCode[i].sTransCode, pcQueueType);
            continue;
        }
        tLog(ERROR, "GroupId[%s]TransCode[%s:%s]������[%d]���سɹ�."
                , pcGroupId, pstLIbInfo->staTransCode[i].sTransCode, pcQueueType
                , pstLIbInfo->staTransCode[i].iFuncNum);
    }
    return 0;
}

int LoadQTrans(char *pcGroupId) {
    return LoadTrans(&g_stLibInfo_Q, "Q", pcGroupId);
}

int LoadPTrans(char *pcGroupId) {
    return LoadTrans(&g_stLibInfo_P, "P", pcGroupId);
}

int InitDyLib(LibInfo *pstLIbInfo, char *pcGroupId, char *pcLibName) {
    char sPath[256] = {0};

    tStrCpy(pstLIbInfo->sGroupId, pcGroupId, strlen(pcGroupId));
    snprintf(sPath, sizeof (sPath), "%s/lib/%s", getenv("HOME"), pcLibName);
    pstLIbInfo->pvLib = dlopen(sPath, RTLD_NOW);
    if (NULL == pstLIbInfo->pvLib)
    {
        tLog(ERROR, "���ض�̬��[%s]����[%s].", sPath, dlerror());
        return -1;
    }
    pstLIbInfo->pfAppServerInit = (pvLibInit) dlsym(pstLIbInfo->pvLib, SVRFUNC_INIT);
    if (NULL == pstLIbInfo->pfAppServerInit)
    {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_INIT, errno, strerror(errno));
        return -1;
    }

    pstLIbInfo->pfAppServerDone = (pvLibDone) dlsym(pstLIbInfo->pvLib, SVRFUNC_DONE);
    if (NULL == pstLIbInfo->pfAppServerDone)
    {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_DONE, errno, strerror(errno));
        return -1;
    }
    pstLIbInfo->pfGetKey = (pvGetKey) dlsym(pstLIbInfo->pvLib, SVRFUNC_GETKEY);
    if (NULL == pstLIbInfo->pfGetKey)
    {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_GETKEY, errno, strerror(errno));
        return -1;
    }
    pstLIbInfo->pfGetSvrName = (pvGetSvrName) dlsym(pstLIbInfo->pvLib, SVRFUNC_SVRNAME);
    if (NULL == pstLIbInfo->pfGetSvrName)
    {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_SVRNAME, errno, strerror(errno));
        return -1;
    }

    pstLIbInfo->pfBeginProc = (pvBeginProc) dlsym(pstLIbInfo->pvLib, SVRFUNC_BEGIN);
    if (NULL == pstLIbInfo->pfBeginProc)
    {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_BEGIN, errno, strerror(errno));
        return -1;
    }

    pstLIbInfo->pfEndProc = (pvEndProc) dlsym(pstLIbInfo->pvLib, SVRFUNC_END);
    if (NULL == pstLIbInfo->pfEndProc)
    {
        tLog(ERROR, "��ȡ��̬��[%s]�к���[%s]ָ�����[%d:%s].", sPath, SVRFUNC_END, errno, strerror(errno));
        return -1;
    }
    return 0;
}

int InitQDyLib(char *pcGroupId, char *pcLibName) {
    MEMSET_ST(g_stLibInfo_Q);
    return InitDyLib(&g_stLibInfo_Q, pcGroupId, pcLibName);
}

int InitPDyLib(char *pcTransName, char *pcLibName) {
    MEMSET_ST(g_stLibInfo_P);
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
int DoStep(LibInfo *pstLIbInfo, char *pcTransCode, cJSON * pstJson, char *pcKey) {
    int iFlag = 0, iRet = 0, iNo = 0, i;
    TransCode *pstTransCode = NULL;
    TransFlow *pstTransFLow = NULL;
    pvFuncProc pvFunc;

    for (i = 0; i < pstLIbInfo->iGroupNum; i++)
    {
        if (!memcmp(pstLIbInfo->staTransCode[i].sTransCode, pcTransCode, 6))
        {
            pstTransCode = pstLIbInfo->staTransCode + i;
            break;
        }
    }
    if (NULL == pstTransCode)
    {
        tLog(ERROR, "Key[%s]δ���ؽ�����[%s]�Ľ���������.", pcKey, pcTransCode);
        return -1;
    }
    pstTransFLow = pstTransCode->staTransFlow;
    do
    {
        /* �ҵ��߱���ִ�к��� */
        tLog(INFO, "===>����[%s]Step[%d]����[%s],����[%s]."
                , pstTransCode->sTransCode, iNo, pstTransFLow[iNo].sFuncId, pstTransFLow[iNo].sTransactAttr);
        pvFunc = (pvFuncProc) dlsym(pstLIbInfo->pvLib, pstTransFLow[iNo].sFuncId);
        if (pvFunc == NULL)
        {
            tLog(ERROR, "��̬��ȡ��̬��[%p]�еĺ���[%s]ָ��ʧ��.", pstLIbInfo->pvLib, pstTransFLow[iNo].sFuncId);
            iRet = -1;
            break;
        }
        if (BEGIN_WORK == pstTransFLow[iNo].sTransactAttr[0])
        {
            NULL;
        }
        iRet = pvFunc(pstJson, &iFlag);
        tLog(INFO, "<===����[%s]ִ�н���,ִ�н��[%d].", pstTransFLow[iNo].sFuncId, iRet);
        /* ʧ���˳� */
        if (iRet < 0)
        {
            tRollback();
            iRet = -1;
            break;
        }
        /* �ж����õ�����,�����һ������������1���ύ��������� */
        if ('1' == pstTransFLow[iNo].sTransactAttr[0]
                && (iNo + 1) < pstTransCode->iFuncNum
                && '0' == pstTransFLow[iNo + 1].sTransactAttr[0])
        {
            tCommit();
        }
        /* ����ִ����һ���� */
        iNo++;
    } while (iNo < pstTransCode->iFuncNum);
    tLog(INFO, "������ִ�н���.");
    /* ǿ���ύ���� */
    if (iRet == 0)
    {
        tCommit();
    }
    return iRet;
}