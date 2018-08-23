/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"

int PosRequestMsg();
int InlineRequestMsg();
int FailInlineResquestMsg(int iDayFlag) ;
//int ResponseMsg(char *pcRespMsg);
#if 0
int SendMsg(char *pcReqSvrId, char *pcKey, cJSON *pstDataJson) {
    cJSON *pstJson = NULL, *pcstSubJson = NULL;
    char sRepSvrId[64] = {0};
    char *pcMsg = NULL;

    /* �жϽ��յ�svrid��Ӧ���svrid�Ƿ�Ϊ�� */
    if (NULL == pcReqSvrId || pcReqSvrId[0] == '\0') {
        tLog(ERROR, "�������[%s]�п�ֵ,�޷�����.", pcReqSvrId);
        return -1;
    }
    /* ���͵Ľ��׶��� */
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_P", GetSvrId());
    SET_STR_KEY(pstJson, "svrid", sRepSvrId);
    SET_STR_KEY(pstJson, "key", pcKey);

    pcstSubJson = cJSON_Duplicate(pstDataJson, 1);

    SET_JSON_KEY(pstJson, "data", pcstSubJson);


    pcMsg = cJSON_PrintUnformatted(pstJson);
    if (tSendMsg(pcReqSvrId, pcMsg, strlen(pcMsg)) < 0) {
        tLog(ERROR, "������Ϣ��SvrId[%s]ʧ��,data[%s].", pcReqSvrId, pcMsg);
    }
    cJSON_Delete(pstJson);
    return 0;
}
#endif
/* ������ */
int RequestProc() {
    int iRet = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrName[64] = {0};
    char sTransGroup[6 + 1] = {0};
    char sCurTime[6+1] = {0}; 
    
    snprintf(sSvrName, sizeof (sSvrName), "%s_Q", GetSvrId());
    tLog(INFO, "[%s]����,�����ɹ�.", sSvrName);
    memcpy( sTransGroup, GetTransGroup(), strlen(GetTransGroup()) );
    tTrim(sTransGroup);
    
    tLog(INFO,"������ָ��ɨ��Ľ�����ˮ��[%s]",sTransGroup);
    
    while (g_iQuitLoop) {
        /*ָ����Ҫɨ�Ľ�����ˮ��*/
        if( !memcmp(sTransGroup,"POS",3) ) {
            PosRequestMsg();
        } else if( !memcmp(sTransGroup,"INLINE",6) ) {
            InlineRequestMsg();
        } else if( !memcmp(sTransGroup,"FAIL",4) ) {
            /*��ȡ��ǰʱ��*/
            tGetTime(sCurTime, "", -1);
            /*0������0��10������ ǰһ�յĴ�֧��״̬����ʧ�ܵĵĶ�ά�뽻���е���ʷ��ˮ���У���ʱ�� ȥɨ����ʷ�� �������� ��ά��ʧ�ܵĽ���;
            ��ά��ʧ�ܽ�������֪ͨ��ʱ���ٽ���ɨ����ˮ������*/
            if(memcmp(sCurTime,"000000",6) >= 0 && memcmp(sCurTime,"001000",6) <= 0 ) {
                tLog(INFO,"��ǰʱ��sCurTime:[%s]��ɨ����ʷ������ǰһ�յĶ�ά�뽻��ʧ�ܵ�֪ͨ",sCurTime);
                FailInlineResquestMsg(1);
                /*175 + 5 ��ɨ��һ����ʷ��ˮ��*/
                sleep(175);
            } else {
                FailInlineResquestMsg(0);
            }   
        }
        sleep(5);
    }
    tLog(INFO, "[%s]����,�˳��ɹ�.", sSvrName);
    return 0;
}

int PosRequestMsg() {
    int iRet = 0;
    /* ɨ����� */
    iRet = ScanPosTrans();
    if (iRet < 0) {
        /* -2����δ���ҵ�������ˮ */
        if (iRet != -2)
            tLog(ERROR, "��ѯ�����͵�pos������ˮ����");
        return -1;
    }
    return 0;
}

int InlineRequestMsg() {
    int iRet = 0;
    /* ɨ����� */
    iRet = ScanInlineTrans();
    if (iRet < 0) {
        /* -2����δ���ҽ�����ˮ */
        if (iRet != -2)
            tLog(ERROR, "��ѯ�����͵Ķ�ά�뽻����ˮ����");
        return -1;
    }
    return 0;
}

int FailInlineResquestMsg(int iDayFlag) {
    int iRet = 0;
    /* ɨ����� */
    if ( 0 == iDayFlag ) {
        iRet = ScanFailInlineTrans();
        if (iRet < 0) {
            /* -2����δ���ҽ�����ˮ */
            if (iRet != -2)
                tLog(ERROR, "��ѯ�����͵Ķ�ά��ʧ�ܵĽ�����ˮ����");
            return -1;
        }
    } else if ( 1 == iDayFlag ){
        iRet = ScanDayFailInlineTrans();
        if (iRet < 0) {
            /* -2����δ���ҽ�����ˮ */
            if (iRet != -2)
                tLog(ERROR, "��ѯ�����͵Ķ�ά��ʧ�ܵĽ�����ˮ����");
            return -1;
        }
    }
    
    return 0;
}

/* Ӧ���� ��ʱ�ò���*/
#if 0
int ResponseProc() {
    int iRet = -1, iLen = 0;
    char sMsg[MSG_MAX_LEN] = {0};
    char sSvrName[64] = {0};

    snprintf(sSvrName, sizeof (sSvrName), "%s_P", GetSvrId());
    tLog(DEBUG, "[%s]����,�����ɹ�.", sSvrName);
    while (g_iQuitLoop) {
        memset(sMsg, 0, sizeof (sMsg));
        iRet = tRecvMsg(sSvrName, sMsg, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                break;
            case MSG_TIMEOUT:
                tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                //tLog(DEBUG, "response msg[%s]", sMsg);
                if (ResponseMsg(sMsg) < 0) {
                    tLog(ERROR, "Ӧ����ʧ��,��������.");
                }
                break;
        }
    }
    tLog(INFO, "[%s]����,�˳��ɹ�.", sSvrName);
    return 0;
}

int ResponseMsg(char *pcRespMsg) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char sMerchOrderNo[64] = {0};
    char sRespCode[2 + 1] = {0};

    pstJson = cJSON_Parse(pcRespMsg);

    /* ����ҵ�� */
    GET_STR_KEY(pstJson, "respCode ", sRespCode);
    GET_STR_KEY(pstJson, "orderNo", sMerchOrderNo);

    if (!memcmp(sRespCode, "00", 2)) {
        //����valid_flagΪ0
        if (UpdQRNoticeFlag(pstJson) < 0) {
            tLog(ERROR, "NOTICE����������Notice�ɹ���־ʧ��.");
        }
        tLog(ERROR, "��ά�뽻�׻ص��ɹ�.");
    }

    cJSON_Delete(pstJson);
    return 0;
}
#endif