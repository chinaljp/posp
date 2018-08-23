/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   merchproduct.c
 * Author: Administrator
 *
 * Created on 2017��7��14��, ����2:42
 * Update on 2017��9��4��, ����5:16
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

//int RepFromHX();
int ApplyOpenMerchProd();
int Process();
int RepFromHX();
char g_sMerchId[15+1];

int g_iQutLoop;
/*add by  gjq at 20170904 begin */
/*������� ֧����PID ΢��PID*/
static char g_sBbPid[16+1];
static char g_sWxPid[8+1];
/*add by  gjq at 20170904 end */

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

/***********************
 * �����̻���ͨ��Ʒ����  *
 ***********************/
int main(int argc, char* pcArgv[]) {
    /*add by  gjq at 20170904 begin */
    //printf("argc : %d\n",argc);
    if(argc != 5) {
        printf("����ο��� merchproduct ��־���� ֧����PID ΢��PID ��ͨ����\n");
        return (0);
    }
    int i= 0; /*���������̻���ͨ��Ʒ����*/
    /*add by  gjq at 20170904 end  */
    
    int iRet = 0; 
    g_iQutLoop = 1; 
    tDaemon();
    
     /* �����źţ�ֻ�����Ƴ��ź� */
    SigProc();
    sigset(SIGUSR1, SigQuit);
    
    if (tInitLog(pcArgv[1]) < 0) {
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
    MEMSET(g_sBbPid);
    MEMSET(g_sWxPid);
    /*add by  gjq at 20170904 begin */
    /*֧����PID*/
    tStrCpy(g_sBbPid,pcArgv[2],strlen(pcArgv[2]));
    /*΢��PID*/
    tStrCpy(g_sWxPid,pcArgv[3],strlen(pcArgv[3]));
    tLog(DEBUG,"֧����PID��[%s],΢��PID: [%s]",g_sBbPid,g_sWxPid);
    /*add by  gjq at 20170904 end */
    
    while(g_iQutLoop) {
        iRet = Process();
        if(iRet < 0) {
            tLog(ERROR, "�̻�[%s]��ͨ��Ʒ����ʧ�ܣ�",g_sMerchId);
            sleep(1);
        }
        else if(iRet == 1){
            tLog(INFO, "���̻���Ҫ��ͨ��Ʒ��",g_sMerchId);
            sleep(3);
        }
        else{
            /*tLog(INFO, "�ѷ����̻�[%s]��ͨ��Ʒ���룡",g_sMerchId);*/
            tLog(INFO, "�̻�[%s]��ͨ��Ʒ����ɹ���",g_sMerchId);
            i++;
            tLog(DEBUG,"��ͨ��Ʒ����ɹ�[%d]��,��Ҫ��ͨ[%s]��",i,pcArgv[4]);
            if( atoi(pcArgv[4]) == i ) {
                g_iQutLoop = 0;
            }
            
        }    
    }
    
    tCloseRedis();
    tDoneLog();
    tCloseDb();
    return (EXIT_SUCCESS);
}
 /*�̻���ͨ��Ʒ����*/
int Process(){
    char sRowid[18+1];
    char sSqlStr[1024];
    int iCount = 0;
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    MEMSET(sRowid);
    MEMSET(sSqlStr);
    
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "select rowid,chnmerch_id from b_merch_send_detail \
                where merchenter_flag = '1' and prod_open_flag = 'n' and rownum = 1 for update nowait");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return ( -1 );
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sRowid);
        STRV(pstRes, 2, g_sMerchId);
    }
    iCount = OCI_GetRowCount(pstRes);
    if (0 == iCount) {
        tLog(ERROR, "δ�ҵ���¼.");
        /*�Ƿ�rollback �Ӵ�����*/
        tReleaseRes(pstRes);
        return ( 1 );
    }
    tReleaseRes(pstRes);
    
    /*�����̻���ͨ״̬Ϊz-���뿪ͨδ���ؽ��*/
    MEMSET(sSqlStr);
    snprintf(sSqlStr, sizeof (sSqlStr), 
                "update b_merch_send_detail set prod_open_flag = 'z', \
                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                where rowid = '%s'",sRowid);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "�����̻�[%s]��ͨ��Ʒ����״̬ʧ��.", g_sMerchId);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tCommit();
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    
    /*���뿪ͨ��Ʒ*/
    if( ApplyOpenMerchProd() < 0 ) {
        tLog(ERROR, "�̻�[%s]��ͨ��Ʒʧ��.", g_sMerchId);
        return ( -1 );
    }
    sleep(2);
    /*����Ӧ�𲢴���*/
    if(RepFromHX() < 0){
        tLog(INFO, "����Ӧ��ʧ��.");
        /*����Ӧ��ʧ�ܣ���ʱ ���ٴ�����*/       
        return ( -1 );
    }
    
    return ( 0 );
}

/*���뿪ͨ��Ʒ*/
int ApplyOpenMerchProd(){
    
    char *pcMsg = NULL;
    cJSON * pstJson = NULL;
    cJSON * pstDataJson = NULL;
    cJSON * pstMerchProdJson = NULL;
    cJSON * pstWxProdJson = NULL;
    cJSON * pstAlipayProdJson = NULL;
    
    char sSysTrace[6 + 1]; /*ϵͳ��ˮ�ţ�������key*/
    char sKey[64];
    
    MEMSET(sSysTrace);
    MEMSET(sKey);
    
    /*��װJSON*/  
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
    pstMerchProdJson = cJSON_CreateObject();
    if (NULL == pstMerchProdJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return -1;
    }
    pstWxProdJson = cJSON_CreateObject();
    if (NULL == pstWxProdJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return -1;
    }
    pstAlipayProdJson = cJSON_CreateObject();
    if (NULL == pstAlipayProdJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return -1;
    }
    
    if (GetSysTrace(sSysTrace) != 0) {
        tLog(ERROR, "����ϵͳ��ˮ��ʧ��.");
    }
    strcat(sKey, "OPENPROD_");
    strcat(sKey, sSysTrace);
    
    SET_STR_KEY(pstJson, "svrid", "HUAXIA_CHN");
    SET_STR_KEY(pstJson, "key", sKey);
    /*********************data******************************/
    SET_STR_KEY(pstDataJson,"transcode","ydzf202");
    SET_STR_KEY(pstDataJson,"agentNo","201707280000266");
    SET_STR_KEY(pstDataJson,"merchantNo",g_sMerchId);
    /*΢��֧��*/
    SET_STR_KEY(pstWxProdJson,"onlineRate","0.0038");
    SET_STR_KEY(pstWxProdJson,"tradeCode","50554");
    SET_STR_KEY(pstWxProdJson,"bizCateGory","204");
    
    /*������΢�ŵ�PID  motify by gjq  at 20170904 begin */
    //SET_STR_KEY(pstWxProdJson,"source","33957971"); 
    SET_STR_KEY(pstWxProdJson,"source",g_sWxPid);
    /*motify by gjq  at 20170904 end */
    
    /*֧����֧��*/
    SET_STR_KEY(pstAlipayProdJson,"onlineRate","0.0038");
    SET_STR_KEY(pstAlipayProdJson,"bizCategory","2015050700000000");/*δ֪��������*/
    
    /*������֧������PID  motify by gjq  at 20170904 begin */
    //SET_STR_KEY(pstAlipayProdJson,"source","2088721946133655");
    SET_STR_KEY(pstAlipayProdJson,"source",g_sBbPid);
    /*motify by gjq  at 20170904 end */
    
    
    /*�ӷ�֧*/
    SET_JSON_KEY(pstMerchProdJson,"wxProduct",pstWxProdJson);
    SET_JSON_KEY(pstMerchProdJson,"alipayProduct",pstAlipayProdJson);
    SET_JSON_KEY(pstDataJson,"merchantProduct",pstMerchProdJson);
    /*����֧*/
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    
    pcMsg = cJSON_PrintUnformatted(pstJson);
    /*��������*/
    if (tSendMsg("HUAXIA_CHN", pcMsg, strlen(pcMsg)) < 0) {
        tLog(ERROR, "������Ϣ��SvrId[HUAXIA_CHN]ʧ��,data[%s].", pcMsg);
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
    OCI_Statement *pstState = NULL;
    
    char sSqlStr[1024];
    char sMsg[MSG_MAX_LEN];
    char sRespCode[10 + 1];
    char sRespMsg[1024 + 1];
    char sWxRespCode[10 + 1];
    char sWxRespMsg[1024 + 1];
    char sAliRespCode[10 + 1];
    char sAliRespMsg[1024 + 1];
    char sWRespCode[10 + 1] = {0},sBRespCode[10 + 1] = {0};
    
    cJSON *pstJson = NULL;
    cJSON *pstDataJson = NULL;
    
    
    while(1) {
        MEMSET(sMsg);
        iRet = tRecvMsgEX("YDZF202", sMsg, &iLen, TIMEOUT);
        if(iRet == MSG_ERR) {
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                return ( -1 );//�ٴη���ͨ��Ʒ����
        }
        if(iRet == MSG_TIMEOUT) {
            /*��ʱ��һֱ�ȴ�����*/
            tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", "YDZF202", TIMEOUT);
            sleep(3);
            i++;
            if(i == 3) {
                tLog(ERROR, "��ʱ����[%d]�Σ��˳��ȴ���",i);
                return ( -1 );
            }            
        }
        if(iRet == MSG_SUCC) {
                pstJson = cJSON_Parse(sMsg);
                if (NULL == pstJson) {
                    tLog(ERROR, "��ȡ��Ϣʧ��,��������.");
                    return ( -1 ); //���������Ƿ�һֱ�ȴ����ٴη���ͨ��Ʒ����
                }
                else {
                    MEMSET(sRespCode);
                    MEMSET(sRespMsg);
                    MEMSET(sWxRespCode);
                    MEMSET(sWxRespMsg);
                    MEMSET(sAliRespCode);
                    MEMSET(sAliRespMsg);
                    MEMSET(sSqlStr);
                    
                    pstDataJson = GET_JSON_KEY(pstJson,"data");
                    
                    GET_STR_KEY(pstDataJson,"respCode",sRespCode);
                    GET_STR_KEY(pstDataJson,"respMsg",sRespMsg);
                    GET_STR_KEY(pstDataJson,"wxRespCode",sWxRespCode);
                    GET_STR_KEY(pstDataJson,"wxRespMsg",sWxRespMsg); 
                    GET_STR_KEY(pstDataJson,"aliRespCode",sAliRespCode);
                    GET_STR_KEY(pstDataJson,"aliRespMsg",sAliRespMsg); 
                    
                    cJSON_Delete(pstJson);

                 /* �����������룬�Ȳ��Է�����Ϣ������  
                  * snprintf(sSqlStr, sizeof (sSqlStr), 
                        "update b_merch_send_detail set wxrespcode = '%s', \
                            wxrespmsg = '%s', alirespcode = '%s', alirespmsg = '%s', \
                            last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                        where chnmerch_id = '%s'",sWxRespCode,sWxRespMsg,sAliRespCode,sAliRespMsg,g_sMerchId);*/
                   /* snprintf(sSqlStr, sizeof (sSqlStr), 
                        "update b_merch_send_detail set wxrespcode = '%s', alirespcode = '%s', \
                            last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                        where chnmerch_id = '%s'",sWxRespCode,sAliRespCode,g_sMerchId);*/
                    
                    /*�ѿ�ͨ�ɹ��Ĳ�Ʒ���ڶ������뿪ͨʱ���ѿ�ͨ�ɹ��Ĳ�Ʒ���᷵�������£� ������*/
                    /*��ѯ���������̻���Ӧ�Ĳ�Ʒ��ͨ������ */
                    snprintf(sSqlStr, sizeof (sSqlStr), "select wxrespcode,alirespcode  from b_merch_send_detail \
                        where chnmerch_id = '%s'",g_sMerchId);
                    if (tExecute(&pstRes, sSqlStr) < 0) {
                        tLog(ERROR, "sql[%s] err!", sSqlStr);
                        return ( -1 );
                    }
                    if (NULL == pstRes) {
                        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
                        return ( -1 );
                    }
                    while (OCI_FetchNext(pstRes)) {
                        STRV(pstRes, 1, sWRespCode);
                        STRV(pstRes, 2, sBRespCode);
                        
                        tTrim(sWRespCode);
                        tTrim(sBRespCode);
                    }
                    if (0 == OCI_GetRowCount(pstRes)) {
                        tLog(ERROR, "δ�ҵ���¼.");
                        tReleaseRes(pstRes);
                        return ( -1 );
                    }
                    tReleaseRes(pstRes);
                    
                    MEMSET(sSqlStr);
                    tLog(DEBUG,"sWRespCode=[%s],sBRespCode=[%s]",sWRespCode,sBRespCode);
                    if(!memcmp(sWRespCode,"000000",6)) {
                        tLog(DEBUG,"�����̻�[%s]�ѿ�ͨ΢��֧��");
                        snprintf(sSqlStr, sizeof (sSqlStr), 
                            "update b_merch_send_detail set  alirespcode = '%s', \
                                last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                            where chnmerch_id = '%s'",sAliRespCode,g_sMerchId);
                    } 
                    else if(!memcmp(sBRespCode,"000000",6)) {
                        tLog(DEBUG,"�����̻�[%s]�ѿ�֧ͨ����֧��");
                        snprintf(sSqlStr, sizeof (sSqlStr), 
                            "update b_merch_send_detail set  wxrespcode = '%s', \
                                last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                            where chnmerch_id = '%s'",sWxRespCode,g_sMerchId);
                    }
                    else {
                        tLog(DEBUG,"�����̻�[%s]΢��֧����֧����֧�� ��δ��ͨ");
                        snprintf(sSqlStr, sizeof (sSqlStr), 
                            "update b_merch_send_detail set wxrespcode = '%s', alirespcode = '%s', \
                                last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                            where chnmerch_id = '%s'",sWxRespCode,sAliRespCode,g_sMerchId);
                    }
                    pstState = tExecuteEx(&pstRes, sSqlStr);
                    if (pstState == NULL || tGetAffectedRows() <= 0) {
                        tLog(DEBUG, "sql[%s]", sSqlStr);
                        tLog(ERROR, "���·����̻�[%s]��Ϣʧ��.", g_sMerchId);
                    }
                    tCommit();
                    tReleaseResEx(pstState);
                    OCI_StatementFree(pstState);
                    if( !memcmp( sRespCode,"000000",6 ) ) {
                        /*���¿�ͨ�����3�У�y-΢�š�֧�������ѿ�ͨ,b-֧������ͨ��w-΢�ſ�ͨ */
                        if( !memcmp(sWxRespCode,"000000",6) && !memcmp(sAliRespCode,"000000",6) ) {
                            /*y-΢�š�֧�������ѿ�ͨ*/
                            MEMSET(sSqlStr);
                            snprintf(sSqlStr, sizeof (sSqlStr), 
                                "update b_merch_send_detail set prod_open_flag = 'y', \
                                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                                where chnmerch_id = '%s'",g_sMerchId);
                        }
                        else if ( !memcmp(sWxRespCode,"000000",6) ) {
                            /*w-΢�ſ�ͨ*/
                            MEMSET(sSqlStr);
                            snprintf(sSqlStr, sizeof (sSqlStr), 
                                "update b_merch_send_detail set prod_open_flag = 'w', \
                                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                                where chnmerch_id = '%s'",g_sMerchId);
                        }
                        else if ( !memcmp(sAliRespCode,"000000",6) ) {
                            /*b-֧������ͨ*/
                            MEMSET(sSqlStr);
                            snprintf(sSqlStr, sizeof (sSqlStr), 
                                "update b_merch_send_detail set prod_open_flag = 'b', \
                                    last_datetime = to_char(systimestamp,'yyyymmddhh24miss')  \
                                where chnmerch_id = '%s'",g_sMerchId);
                        }
                        else {
                            /*΢�� ֧���� �Կ�ͨʧ��*/
                            tLog(INFO, "�̻�[%s]��ͨ��Ʒʧ��.", g_sMerchId);
                            return ( -1 );
                        }
                        pstState = tExecuteEx(&pstRes, sSqlStr);
                        if (pstState == NULL || tGetAffectedRows() <= 0) {
                           // tLog(DEBUG, "sql[%s]", sSqlStr);
                            tLog(ERROR, "�����̻�[%s]��ͨ��Ʒ��־ʧ��,�����ѷ���[%s].", g_sMerchId,sRespCode);
                            tReleaseResEx(pstState);
                            OCI_StatementFree(pstState);
                            return ( -1 );
                        }
                        tCommit();  
                        tReleaseResEx(pstState);
                        OCI_StatementFree(pstState);

                        tLog(INFO, "�̻�[%s]��ͨ��Ʒ�ɹ�.", g_sMerchId);
                    } 
                    else {
                        tLog(INFO, "�̻�[%s]��ͨ��Ʒʧ��.", g_sMerchId);
                        return ( -1 );
                    }
                }
                break;
        }
    }
    return ( 0 );
}