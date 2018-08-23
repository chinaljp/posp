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

#define SVRFUNC_INIT        "AppServerInit"
#define SVRFUNC_UNPACK      "RequestMsg"
#define SVRFUNC_PACK        "ResponseMsg"
#define SVRFUNC_GETKEY      "AppGetKey"
#define SVRFUNC_SVRNAME     "GetSvrName"
#define SVRFUNC_DONE        "AppServerDone"

    /* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
    typedef int (*pvLibInit)(int iArgc, char *pcArgv[]);
    /* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
    typedef int (*pvLibDone)(void);
    /* ����ӿ� */
    typedef int ( *pvRequestMsg)(cJSON *pstReqDataJson, cJSON *pstDataJson);
    /* ����ӿ� */
    typedef int ( *pvResponseMsg)(cJSON *pstRepDataJson, cJSON *pstDataJson);
    /* ��ȡ���������ӿ� */
    typedef int ( *pvGetKey)(char *pcKey, size_t tKey, cJSON *pstInJson, cJSON *pstOutJson);
    /* ��ȡ��һ���ڵ�ķ������ӿ� */
    typedef int ( *pvGetSvrName)(char *pSvrId, size_t tSvrId, cJSON *pstInJson, cJSON *pstOutJson);

    typedef struct {
        pvLibInit pfAppServerInit;
        pvLibDone pfAppServerDone;
        pvRequestMsg pfRequestMsg;
        pvResponseMsg pfResponseMsg;
        pvGetKey pfGetKey;
        pvGetSvrName pfGetSvrName;
        void *pvLib; /* ��̬����ʾ�� */
    } LibInfo; /* ģ��Ķ�̬����Ϣ */

    extern LibInfo g_stLibInfo;

    int InitDyLib(char *pcLibName);
    void DoneDyLib();

#ifdef __cplusplus
}
#endif

#endif /* DYLIB_H */

