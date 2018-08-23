/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SendMerchMsg.c
 * Author: Administrator
 *
 * Created on 2017��7��12��, ����5:57
 * Update on 2017��9��4��, ����5:11
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_db.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "t_app_conf.h"
#include "trace.h"
#include "t_daemon.h"

int RepFromHX();
int GetMerchMsg();
int Process();

char g_sMerchId[15+1];
char g_sKey[64]; /*������ƥ�䷵�ص���Ϣ*/
/*add by GJQ */
char g_MerchShortName[128]; /*�̻����*/

int g_iQutLoop;

void SigQuit(int iSigNo) {
    int i;
    tLog(DEBUG, "�յ�[%d]�˳��ź�!", iSigNo);
    g_iQutLoop = 0;
}

/* �����źţ�ֻ�����Ƴ��ź� */
void SigProc() {
    int i;
    for (i = 0; i < NSIG; i++) {
        if (i != SIGSEGV)
            sigset(i, SIG_IGN);
    }
}

/*
 * 
 */
int main(int argc, char* pcArgv[]) {
    
    /*add by  gjq at 20170906 begin */
    //printf("argc : %d\n",argc);
    if(argc != 4) {
        printf("����ο��� merchmsgenter ��־���� �̻���� ��פ����\n");
        return (0);
    }
    int i= 0; /*���������̻���פ����*/
    /*add by  gjq at 20170906 end  */
    
    
    int iRet = 0;
    int inum = 0;
    char sSqlStr[1024];
    OCI_Resultset *pstInsRes = NULL;
    MEMSET(sSqlStr);
    
    g_iQutLoop = 1; 
    tDaemon();
    
     /* �����źţ�ֻ�����Ƴ��ź� */
    SigProc();
    sigset(SIGUSR1, SigQuit);
    
    if (tInitLog(pcArgv[0]) < 0) {
        fprintf(stderr, "��ʼ��[%s]��־ʧ��.\n", pcArgv[1]);
        return (EXIT_FAILURE);
    }
    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis����ʧ��.");
        tDoneLog();
        return -1;
    }

    if (tOpenDb() < 0) {
        tLog(ERROR, "���ݿ�����ʧ��.");
        tDoneLog();
        tCloseRedis();
        return -1;
    }
    MEMSET(g_MerchShortName);
    tStrCpy(g_MerchShortName,pcArgv[2],strlen(pcArgv[2]));
    tLog(DEBUG,"�̻����g_MerchShortName[%s]",g_MerchShortName);
    
    snprintf(sSqlStr,sizeof(sSqlStr),"INSERT INTO B_MERCH_SEND_DETAIL (MERCH_ID,LAST_DATETIME) \
                    SELECT a.MERCH_ID,to_char(systimestamp,'yyyymmddhh24miss') \
                    FROM B_LIFE_MERCH a  WHERE STATUS = '1' \
                    AND NOT EXISTS (SELECT 1 FROM B_MERCH_SEND_DETAIL b WHERE a.MERCH_ID = b.MERCH_ID)");

    while(g_iQutLoop) {
        iRet = Process();
        if(iRet < 0) {
            tLog(ERROR, "�̻�[%s]��פʧ�ܣ�",g_sMerchId);
            sleep(1);
        }
        else if(iRet == 1) {
            if(i == 0) {
                tLog(INFO, "���̻�����פ��");
                /*���̻�����פʱ����Ƿ������̻���Ҫ��פ */
                tLog(INFO, "����Ƿ������̻���Ҫ��פ��");
                if(tExecute(&pstInsRes, sSqlStr) < 0) {
                    tLog(DEBUG, "sql[%s]", sSqlStr);
                    return(-1);/*test add */
                }
                tCommit();
                tReleaseRes(pstInsRes);
                /*���3�������̻���פ�������˳�*/
                inum++;
                if(inum == 3) {
                    tLog(INFO, "�������̻���Ҫ��פ���˴η��񵽴˽���(^_^)��");
                    return ( 0 );
                }
            }
            else {
                tLog(INFO, "[%d]���̻���פ�ɹ���",i);
                i=0;
            }
        }
        else {
            i++;
            /*�̻���פ3000�� �����˳� add by gjq  at 20170904*/
            tLog(DEBUG,"�ѳɹ���פ�̻�[%d]��,��Ҫ��פ[%s]",i,pcArgv[3]);
            if(atoi(pcArgv[3]) == i){
                g_iQutLoop = 0;
            }  
        }  
    }
    
    tCloseRedis();
    tDoneLog();
    tCloseDb();
    return (EXIT_SUCCESS);
}
 /*�̻���פ����*/
int Process(){
    char sSqlStr[1024];
    int iCount = 0;
    OCI_Resultset *pstRes = NULL;

    MEMSET(sSqlStr);
    
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "select merch_id from b_merch_send_detail where send_status = '0' and rownum = 1 for update nowait");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return ( -1 );
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, g_sMerchId);
    }
    iCount = OCI_GetRowCount(pstRes);
    if (0 == iCount) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        /*�̻���פ��ϻ������̻���פ*/
        return ( 1 );
    }
    tReleaseRes(pstRes);
    
    /*�����̻���Ϣ����״̬Ϊ2-������*/
    MEMSET(sSqlStr);
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "update b_merch_send_detail set send_status = '2', \
                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                where merch_id = '%s'",g_sMerchId);
    
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "�����̻�[%s]��Ϣ����״̬ʧ��.", g_sMerchId);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    
    /*��ȡ�̻���Ϣ������*/
    if( GetMerchMsg() < 0 ) {
        tLog(ERROR, "�̻�[%s]��Ϣ����ʧ��.", g_sMerchId);
        return ( -1 );
    }
    
    /*�����̻���Ϣ����״̬Ϊ1-�ѷ���*/
    MEMSET(sSqlStr);
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "update b_merch_send_detail set send_status = '1', \
                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                where merch_id = '%s'",g_sMerchId);
    
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "�����̻�[%s]��Ϣ����״̬ʧ��.", g_sMerchId);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    sleep(2);
    /*����Ӧ�𲢴���*/
    if(RepFromHX() < 0) {
        tLog(ERROR, "�˴ν���Ӧ��ʧ�ܣ�");
        /*����Ӧ��ʧ�ܣ�����Ӧ����Ϣʧ�ܡ����ճ�ʱ��������3�Σ����̻�����������������פ*/
        /*�����̻���Ϣ����״̬Ϊ0-δ����*/
        MEMSET(sSqlStr);
        snprintf(sSqlStr, sizeof (sSqlStr), 
                    "update b_merch_send_detail set send_status = '0', \
                        last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                    where merch_id = '%s'",g_sMerchId);

        if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
            tLog(DEBUG, "sql[%s]", sSqlStr);
            tLog(ERROR, "�����̻�[%s]��Ϣ����״̬ʧ��.", g_sMerchId);
            return -1;
        }
        tCommit();
        tReleaseRes(pstRes);
        return ( -1 );
    }
    
    return ( 0 );
}

/*��ȡ�̻���Ϣ������*/
int GetMerchMsg(){
    char sSqlStr[1024];
    int iCount = 0;
    OCI_Resultset *pstRes = NULL;
    
    char *pcMsg = NULL;
    cJSON * pstJson = NULL;
    cJSON * pstDataJson = NULL;
    cJSON * pstMerchInfoJson = NULL;
    cJSON * pstContactInfoJson = NULL;
    cJSON * pstBankCardJson = NULL;
    /*JSON * pstAddrInfoJson = NULL;*/
        
    char sTmp[128];
    char sFullName[60+1];         /*�̻�����      */
    char sContactName[64+1];      /*��ϵ������    */
    char sContactPhone[20+1];     /*��ϵ���ֻ���  */
    char sContactPersonType[30+1];/*��ϵ������    */
    
    char sSysTrace[6 + 1]; /*ϵͳ��ˮ�ţ�������key*/
    
    MEMSET(sTmp);
    MEMSET(sFullName);
    MEMSET(sContactName);
    MEMSET(sContactPhone);
    MEMSET(sContactPersonType);
    MEMSET(sSysTrace);
    
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "select merch_name, \
                case when handler_name is not null then handler_name else legal_name end, \
                handler_mobile, \
                case when handler_name is not null then 'AGENT' else 'LEGAL_PERSON' end \
                from b_life_merch where merch_id = '%s'",g_sMerchId);
    
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    
    while (OCI_FetchNext(pstRes)) {
        /*�̻�ȫ�ƹ���ֻȡǰ60�ֽ�*/
        STRV(pstRes, 1, sTmp);
        snprintf(sFullName,sizeof(sFullName),sTmp);
        
        STRV(pstRes, 2, sContactName);
        STRV(pstRes, 3, sContactPhone);
        STRV(pstRes, 4, sContactPersonType);
        
        tTrim(sFullName);
        tTrim(sContactName);
        tTrim(sContactPhone);
        tTrim(sContactPersonType);
        /*tLog(ERROR, "sTmp = [%s],sFullName = [%s],sContactName = [%s],sContactPhone = [%s],sContactPersonType = [%s]",
                sTmp,sFullName,sContactName,sContactPhone,sContactPersonType);*/
    }
    iCount = OCI_GetRowCount(pstRes);
    if (0 == iCount) {
        tLog(ERROR, "δ�ҵ��̻�[%s]��Ϣ,iCount = [%d].",g_sMerchId,iCount);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
   
    /*ȡֵ ��װJSON*/  
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return -1;
    }
    pstMerchInfoJson = cJSON_CreateObject();
    if (NULL == pstMerchInfoJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return -1;
    }
    pstContactInfoJson = cJSON_CreateObject();
    if (NULL == pstContactInfoJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return -1;
    }
    pstBankCardJson = cJSON_CreateObject();
    if (NULL == pstBankCardJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return -1;
    }
    /*
    pstAddrInfoJson = cJSON_CreateObject();
    if (NULL == pstAddrInfoJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return -1;
    }
    */
    if (GetSysTrace(sSysTrace) != 0) {
        tLog(ERROR, "����ϵͳ��ˮ��ʧ��.");
    }
    
    MEMSET(g_sKey);
    strcat(g_sKey, "ENTER_");
    strcat(g_sKey, sSysTrace);
    
    SET_STR_KEY(pstJson, "svrid", "HUAXIA_CHN");
    SET_STR_KEY(pstJson, "key", g_sKey);
    /*********************data******************************/
    SET_STR_KEY(pstDataJson,"transcode","ydzf201");
    SET_STR_KEY(pstDataJson,"agentNo","201707280000266");
    /**********�̻���Ϣ************/
    SET_STR_KEY(pstMerchInfoJson,"fullName",sFullName);
    //SET_STR_KEY(pstMerchInfoJson,"shortName","Ǯ��ͨ�����ݣ�");/* �̻���ƣ����ݿ��д��ֶι���������д��ֵ��*/
    //SET_STR_KEY(pstMerchInfoJson,"shortName","����Ǯ��ͨ");
    SET_STR_KEY(pstMerchInfoJson,"shortName",g_MerchShortName);
    
    SET_STR_KEY(pstMerchInfoJson,"servicePhone","00000");/*�ͷ��绰 ��ʱû�У�ȡ��ֵ00000*/
    /*��ϵ����Ϣ*/
    SET_STR_KEY(pstContactInfoJson,"contactName",sContactName);
    SET_STR_KEY(pstContactInfoJson,"contactPhone",sContactPhone);
    SET_STR_KEY(pstContactInfoJson,"contactPersonType",sContactPersonType);
    SET_STR_KEY(pstContactInfoJson,"contactIdCard","110106198906253625"); /*���֤��ʹ�ÿ������֤*/
    /*���㿨��Ϣ*/
    SET_STR_KEY(pstBankCardJson,"bankAccountNo","10238000000156276");
    SET_STR_KEY(pstBankCardJson,"bankAccountName","�������б��������ĵ���֧��");
    SET_STR_KEY(pstBankCardJson,"idCard","110106198906253625");
    SET_STR_KEY(pstBankCardJson,"bankAccountType","2");
    SET_STR_KEY(pstBankCardJson,"bankAccountLineNo","304100011803");
    SET_STR_KEY(pstBankCardJson,"bankAccountAddress","�����к������ĵ���2��A����������һ��");
     /*��ϵ�˵�ַ -- �Ǳ��������* /
    SET_STR_KEY(pstAddrInfoJson,"province",);
    SET_STR_KEY(pstAddrInfoJson,"city",);
    SET_STR_KEY(pstAddrInfoJson,"district",);
    SET_STR_KEY(pstAddrInfoJson,"address",);
    */
    SET_JSON_KEY(pstMerchInfoJson,"contactInfo",pstContactInfoJson);
    SET_JSON_KEY(pstMerchInfoJson,"bankCardInfo",pstBankCardJson);
    /*SET_JSON_KEY(pstMerchInfoJson,"addressInfo",pstAddrInfoJson);*/
    SET_JSON_KEY(pstDataJson,"merchantInfo",pstMerchInfoJson);
    
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    
    pcMsg = cJSON_PrintUnformatted(pstJson);
    /*��������*/
    if (tSendMsg("HUAXIA_CHN", pcMsg, strlen(pcMsg)) < 0) {
        tLog(ERROR, "������Ϣ��SvrId[HUAXIA_CHN]ʧ��,data[%s].", pcMsg);
        free(pcMsg);
        cJSON_Delete(pstJson);
        return ( -1 );
    }
    free(pcMsg);
    cJSON_Delete(pstJson);
    
    return ( 0 );
}

/*����Ӧ����Ϣ������*/
int RepFromHX() {
    int i = 0;
    int iLen,iRet;
    OCI_Resultset *pstRes = NULL;
    char sSqlStr[1024];
    char sMsg[MSG_MAX_LEN];
    char sKey_p[64];
    char sChnMerchId[15+1];
    char sAppKey[32 + 1];
    char sRespCode[10 + 1];
    char sRespMsg[100 + 1];
    cJSON *pstJson = NULL;
    cJSON *pstDataJson = NULL;
    
    while(1) {
        iLen = 0;
        MEMSET(sMsg);
        iRet = tRecvMsgEX("YDZF201", sMsg, &iLen, TIMEOUT);
        if(iRet == MSG_ERR) {
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                sleep(10);
                return ( -1 );
        }
        if(iRet == MSG_TIMEOUT) {
            tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", "YDZF201", TIMEOUT);
            i++;
            sleep(3);/*��ʱ���ȴ�3��������գ���ʱ3���˳��˴ν��գ�������һ������*/
            if(i == 3){
                tLog(ERROR, "��ʱ����[%d]�Σ��˳��ȴ���",i);
                /*return ( -1 ); ���ڷ��س�ʱ���̻���Ϣ ���ٷ��� ��פ����*/
            }
        }
        if(iRet == MSG_SUCC) {
                pstJson = cJSON_Parse(sMsg);
                if (NULL == pstJson) {
                    tLog(ERROR, "��ȡ��Ϣʧ��,��������.");
                    return ( -1 );
                }
                else {
                    MEMSET(sChnMerchId);
                    MEMSET(sAppKey);
                    MEMSET(sRespCode);
                    MEMSET(sRespMsg);
                    MEMSET(sSqlStr);
                    
                    GET_STR_KEY(pstJson,"key",sKey_p);
                    if( !memcmp( g_sKey, sKey_p, strlen(g_sKey) ) ) {
                        pstDataJson = GET_JSON_KEY(pstJson,"data");
                    }
                    else {
                        tLog(INFO,"�˴�������Ϣkey[%s]����յ���key[%s]��һ�£�",g_sKey,sKey_p);
                        /*key��һ�£������˴�Ӧ����Ϣ������������һ��Ӧ����Ϣ������ƥ��*/
                        continue;
                    }
                    
                    GET_STR_KEY(pstDataJson,"merchantNo",sChnMerchId);
                    GET_STR_KEY(pstDataJson,"appkey",sAppKey);
                    GET_STR_KEY(pstDataJson,"respCode",sRespCode);
                    /*GET_STR_KEY(pstDataJson,"respMsg",sRespMsg); ���ڷ�����Ϣ��������զ�����´��ֶ�����*/
                    
                    cJSON_Delete(pstJson);
                    
                    snprintf(sSqlStr, sizeof (sSqlStr), 
                        "update b_merch_send_detail set chnmerch_id = '%s', \
                            app_key = '%s', resp_code = '%s', resp_msg = '%s', \
                            last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                        where merch_id = '%s'", sChnMerchId,sAppKey,sRespCode,sRespMsg,g_sMerchId);

                    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
                        /*tLog(DEBUG, "sql[%s]", sSqlStr);*/
                        tLog(ERROR, "�������������̻�[%s]��Ϣʧ��.", sChnMerchId);
                    }
                    tReleaseRes(pstRes);
                    if(!memcmp( sRespCode,"000000",6 )) {
                        MEMSET(sSqlStr);
                        snprintf(sSqlStr, sizeof (sSqlStr), 
                            "update b_merch_send_detail set merchenter_flag = '1', \
                                last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                            where merch_id = '%s'",g_sMerchId);

                        if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
                            tLog(DEBUG, "sql[%s]", sSqlStr);
                            tLog(ERROR, "�����̻�[%s]��פ�ɹ���־ʧ��.", g_sMerchId);
                        }
                        tReleaseRes(pstRes); 
                        tLog(INFO, "�̻�[%s]��פ�ɹ�", g_sMerchId);
                    }
                    tCommit();
                }
                break;
        }
    }
    
    return ( 0 );
}