/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dylib.h
 * Author: feng.gaoo
 *
 * Created on 2016��12��12��, ����7:55
 */

#ifndef DYLIB_H
#define DYLIB_H

#ifdef __cplusplus
extern "C" {
#endif
#include "t_cjson.h"

#define BEGIN_WORK      '1'   /* ��ʼ���� */
#define COMMIT_WORK     '2'   /* �ύ���� */

#define SVRFUNC_INIT        "AppServerInit"
#define SVRFUNC_DO          "AppServerDo"
#define SVRFUNC_GETKEY      "AppGetKey"
#define SVRFUNC_SVRNAME     "GetSvrName"
#define SVRFUNC_DONE        "AppServerDone"
#define SVRFUNC_BEGIN       "BeginProc"
#define SVRFUNC_END         "EndProc"

#define NORMALFLOW_NUM          32      /* ÿֻ�������������������   */

    typedef struct {
        char sFuncId[50 + 1]; /* ���������� */
        char sTransactAttr[8 + 1]; /* ������������� */
    } TransFlow; /* ������������Ϣ */
    /*
        typedef struct {
            int iFuncNum;
            NormalFlow *pstNormalFlow;
        } NormalFlowCfg;
     */
    /* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
    typedef int (*pvLibInit)(int iArgc, char *sArgv[]);
    /* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
    typedef int (*pvLibDone)(void);
    /* �������ӿ� */
    typedef int ( *pvFuncProc)(cJSON *pstJson, int *piFlag);
    typedef int ( *pvBeginProc)(cJSON *pstJson, int iMsgType);
    typedef int ( *pvEndProc)(cJSON *pstJson, int iMsgType);
    /* ��ȡ���������ӿ� */
    typedef int ( *pvGetKey)(char *pcKey, size_t tKey, cJSON *pstJson);
    /* ��ȡ��һ���ڵ�ķ������ӿ� */
    typedef int ( *pvGetSvrName)(char *pSvrId, size_t tSvrId, cJSON *pstJson);

    typedef struct {
        char sTransCode[256 + 1]; /* ������(��������) */
        int iFuncNum;
        TransFlow staTransFlow[NORMALFLOW_NUM];
    } TransCode; /* �������ýṹ */

    typedef struct {
        char sGroupId[256 + 1]; /*  ������ */
        char sLibName[256 + 1]; /* ��̬������ */
        int iGroupNum; /* ���ڽ�������� */
        TransCode staTransCode[64];
        pvLibInit pfAppServerInit;
        pvLibDone pfAppServerDone;
        pvGetKey pfGetKey;
        pvGetSvrName pfGetSvrName;
        pvBeginProc pfBeginProc;
        pvEndProc pfEndProc;
        void *pvLib; /* ��̬����ʾ�� */
    } LibInfo; /* ģ��Ķ�̬����Ϣ */

    extern LibInfo g_stLibInfo_Q, g_stLibInfo_P;


#ifdef __cplusplus
}
#endif

#endif /* DYLIB_H */

