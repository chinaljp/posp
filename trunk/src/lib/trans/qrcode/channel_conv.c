/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"


int IsAgentLife(char *pcAgentId) {
    char sSqlStr[512] = {0}, sStatus[2] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT STATUS FROM B_LIFE_CTRL WHERE OBJECT_ID='%s'", pcAgentId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sStatus);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    if (sStatus[0] == '1') {
        return 1;
    }
    return 0;
}

int FindLifeMerch(char *pcLifeMerchId, char *pcLifeTermId, char *pcLifeMerchName, char *pcUserCode, char *pcAgentId,int * piUseCnt ) {
    char sSqlStr[512] = {0}, sTmp[1024] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;
    int iUseCnt = 0;
    /* �ж��ܿ��� */
    if (FindValueByKey(sTmp, "LIFE_AGENT_SWITCH") < 0) {
        tLog(ERROR, "����key[LIFE_AGENT_SWITCH]����Ȧ�ܿ���,ʧ��.");
        return -1;
    }
    if (sTmp[0] == '0') {
        tLog(ERROR, "����Ȧ�ܿ���״̬[%s].", sTmp[0] == '0' ? "�ر�" : "��ͨ");
        return -1;
    }

    /* �̻�ֱ�������̿��� */
    if (IsAgentLife(pcAgentId) <= 0) {
        tLog(ERROR, "������[%s]δ��ͨ����Ȧ.", pcAgentId);
        return -1;
    }

    snprintf(sSqlStr, sizeof (sSqlStr), "select mc.b_life_merch_id,mc.life_term_id,lm.merch_name,lm.use_cnt"
            " from B_LIFE_MERCH_CONV mc "
            " join B_LIFE_MERCH lm on lm.MERCH_ID=mc.b_life_merch_id "
            " WHERE mc.user_id='%s' and mc.status='1'"
            , pcUserCode);

    //tLog(DEBUG, "%s", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcLifeMerchId);
        STRV(pstRes, 2, pcLifeTermId);
        STRV(pstRes, 3, pcLifeMerchName);
        INTV(pstRes, 4, iUseCnt);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    /* �������Ȧ�̻�ȡ����ѡ�����ݿ�����̻��ź��ն˺Żᱻ��� */
    if (pcLifeMerchId[0] == '\0' || pcLifeTermId[0] == '\0')
        return -1;
    *piUseCnt = iUseCnt;
    return 0;
}

int AddLifeMerchUseCnt(char * psChanneMerchId) {
    char sSqlStr[512] = {0};
    char sIsYes[2] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "update b_life_merch set use_cnt = use_cnt+1 where merch_id = '%s'", psChanneMerchId);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "��������Ȧ�̻�[%s]�ۼƽ��״���ʧ��", psChanneMerchId);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

/*��ά�뽻����ת����Ȧ�̻���ֻ��������ά����ת��*/
int ConvLifeMerch(cJSON *pstJson, int *piFlag) {
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sMerchName[256 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sChannelTermId[8 + 1] = {0}, sChanneMerchId[15 + 1] = {0}, sAgentId[8 + 1] = {0}, sChannelId[8 + 1] = {0};
    char sChannelMerchName[256 + 1] = {0},sMerchLevel[1+1] = {0}, sUseCnt[8 + 1] = {0};
    cJSON * pstTransJson = NULL;
    int iRet = -1,iUseCnt = 0, iFindUseCnt = 0;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "merch_p_name", sMerchName);
    GET_STR_KEY(pstTransJson, "agent_id", sAgentId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    //GET_STR_KEY(pstTransJson, "channel_id", sChannelId);
/*
    if (!memcmp(sChannelId, "48560000", 8) || !memcmp(sChannelId, "49000000", 8)) {
        tLog(INFO, "�̻�������ת�Ż��̻���������ת�Ż�����");
        return 0;
    }
*/
    
    /*BEGIN time:20180925 fanghui ����Ȧ�̻������Ż�*/
    /* ��ȡ����Ȧ�̻���ת���� */
    if (FindValueByKey(sUseCnt, "LIFE_MERCH_USE_COUNT") < 0) {
        ErrHanding(pstTransJson, "96", "����key[LIFE_MERCH_USE_COUNT]����Ȧ��ת����,ʧ��.");
        return -1;
    }
    iUseCnt = atoi(sUseCnt);
    tLog(INFO,"����Ȧ�̻�������ת����[%d]",iUseCnt);
    /* END */
    
    tLog(DEBUG,"agent_id[%s],user_code[%s]",sAgentId,sUserCode);
    if (FindLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode, sAgentId,&iFindUseCnt) < 0) {
        /*BEGIN C���̻� û�п�ͨ��ת����Ȧ�̻��ģ����չ���ѡ��һ������Ȧ�̻����н��� */
        GET_STR_KEY(pstTransJson, "merch_level", sMerchLevel);
        if( sMerchLevel[0] == '3' || sMerchLevel[0] == '4' ) {
            tLog(INFO, "sMerchLevel = [%s]: C���̻�[%s]û�п�ͨ��ת����Ȧ,���а������Զ���ȡ����Ȧ�̻�.", sMerchLevel,sMerchId);
            /*ȡһ�������̻������е���Ч״̬������Ȧ�̻� (�˴����� �������� ֱ����ת���� )*/
            if ( FindLocalCityLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode,iUseCnt) < 0 ){
                /*����û�У����ȡ�����̻�����ʡ��ʡ����е�һ����Ч״̬������Ȧ�̻� ���˴� ��Ҫ�ų��������� ����ת��ʡʡ����е� ����Ȧ�̻� ��*/
                if( FindCapitalLifeMerch(sChanneMerchId, sChannelTermId, sChannelMerchName, sUserCode,iUseCnt) < 0 ) {
                    tLog(ERROR,"���̻�[%s]�޿�������Ȧ�̻���ת",sMerchId);
                    ErrHanding(pstTransJson, "Z0", "�̻�[%s]�޿�������Ȧ�̻���ת���Ѵ���ת�������ޣ�",sMerchId);
                    return ( -1 );
                }
            }
            
            /*BEGIN time:20180925 fanghui ����Ȧ�̻���ת������һ*/
            if (AddLifeMerchUseCnt(sChanneMerchId)) {
                ErrHanding(pstTransJson, "96", "��������Ȧ�̻��ۼƱ���ʧ��", sMerchId);
                return -1;
            }
            /*END*/
            
            SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sChannelTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
            tLog(INFO, "ʹ�ð�������ת������Ȧ�̻�[%s:%s].", sChanneMerchId, sChannelTermId);
        }
        /*END C���̻� û�п�ͨ��ת����Ȧ�̻��ģ����չ���ѡ��һ������Ȧ�̻����н��� */
        else {
            tLog(DEBUG, "�̻�[%s]��������Ȧ�̻�ʧ��,ʹ��ԭ�̻���.", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            tLog(INFO, "ʹ�ñ����̻�[%s:%s].", sMerchId, sTermId);
        }
    } else {
        
        
        /* BEGIN fanghui �жϹ�ע������Ȧ�Ƿ�������ת�������ƣ���������ۼ�һ�������㷵��pos*/
        if (iFindUseCnt >= iUseCnt) {
            tLog(ERROR, "��ע�Ĵ��������̻�[%s]�Ѵﵽ��ת�������ƣ�����[%d]��", sChanneMerchId, iFindUseCnt);
            ErrHanding(pstTransJson, "Z0", "�̻�[%s]��ע������Ȧ�̻���ת�����Ѵ����ޣ�", sMerchId);
            return -1;
        } else {
            /*����Ȧ�̻���ת������һ*/
            if (AddLifeMerchUseCnt(sChanneMerchId)) {
                ErrHanding(pstTransJson, "96", "��������Ȧ�̻��ۼƱ���ʧ��", sMerchId);
                return -1;
            }
            tLog(INFO,"��ע������Ȧ�̻�[%s]����ת����[%d]",sChanneMerchId,iFindUseCnt);
        }
        /*END*/
        
        if (sChanneMerchId[0] != '\0' && sChannelTermId[0] != '\0') {
            tLog(DEBUG, "�̻�[%s]ʹ������Ȧ�̻�[%s:%s].", sMerchId, sChanneMerchId, sChannelTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_id", sChanneMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sChannelTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sChannelMerchName);
            tLog(INFO, "�����̻�[%s:%s]�����̻�[%s:%s].", sMerchId, sTermId, sChanneMerchId, sChannelTermId);
        } else {
            SET_STR_KEY(pstTransJson, "channel_merch_id", sMerchId);
            SET_STR_KEY(pstTransJson, "channel_term_id", sTermId);
            SET_STR_KEY(pstTransJson, "channel_merch_name", sMerchName);
            tLog(INFO, "ʹ�ñ����̻�[%s:%s].", sMerchId, sTermId);
        }
    }
    return 0;
}


/*�����ȡ ���������̻���*/
int FindHxChanneMerch(char *pcMerchId,char *pcMerchApk,char *pcTransCode) {
   OCI_Resultset *pstRes = NULL;
   char sSqlStr[1024];
   MEMSET(sSqlStr);
   
   if(strchr(pcTransCode,'W') != NULL) {
        /*��ά�뽻�ף� ΢�Ŷ�ά�뽻�� ��ʹ�� ΢�š�֧����֧�� ����ͨ�������̻� ���� ʹ��ֻ��ͨ΢�ŵ������̻�*/
        snprintf(sSqlStr,sizeof(sSqlStr), "select chnmerch_id,app_key from "
             "(select * from b_merch_send_detail order by dbms_random.value) where prod_open_flag in ('y','w') and rownum=1");
   } 
   else {
        /*��ά�뽻�ף� ֧������ά�뽻�� ��ʹ�� ΢�š�֧����֧�� ����ͨ�������̻� ���� ʹ��ֻ��֧ͨ�����������̻�*/
        snprintf(sSqlStr,sizeof(sSqlStr), "select chnmerch_id,app_key from "
             "(select * from b_merch_send_detail order by dbms_random.value) where prod_open_flag in ('y','b') and rownum=1");
   }
   
   if ( tExecute(&pstRes, sSqlStr) < 0 ) {
       tLog(ERROR, "sql[%s] err!", sSqlStr);
       return -1;
   }
   if (NULL == pstRes) {
       tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
       return -1;
   }
   while (OCI_FetchNext(pstRes)) {
       STRV(pstRes, 1, pcMerchId);
       STRV(pstRes, 2, pcMerchApk);
   }
   if (0 == OCI_GetRowCount(pstRes)) {
       tLog(ERROR, "sql[%s] δ�ҵ���¼!", sSqlStr);
       tReleaseRes(pstRes);
       return -1;
   }
   tReleaseRes(pstRes);
    return ( 0 );
}

/* ��ά�� ֧������ ת���������̻���,��Կ add by gjq at 20170718*/
int ConvHxMerch(cJSON *pstJson, int *piFlag) {
    char sChannelMerchId[15 + 1];
    char sChnMerchApk[32+1];
    char sTransCode[6+1];
    cJSON * pstTransJson = NULL;
    
    MEMSET(sChannelMerchId);
    MEMSET(sChnMerchApk);
    MEMSET(sTransCode);
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson,"trans_code",sTransCode);
    tLog(DEBUG,"������[%s]",sTransCode);
    if( FindHxChanneMerch(sChannelMerchId,sChnMerchApk,sTransCode) < 0 ) {
        tLog(ERROR,"��ȡ���������̻���ʧ�ܣ�");
        return ( -1 );
    }
    tLog(ERROR,"��û��������̻���[%s]������Կ��",sChannelMerchId);
    SET_STR_KEY(pstTransJson, "channel_merch_id", sChannelMerchId);
    SET_STR_KEY(pstTransJson, "appkey", sChnMerchApk);
    
    return ( 0 );
}

/*��ȡ�����̻���appkey����ѯ������������Ҫ��*/
int GetMerchAppKey(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sChannelMerchId[15 + 1];
    char sAppKey[32+1];
    
    MEMSET(sChannelMerchId);
    MEMSET(sAppKey);
    
    pstTransJson = GET_JSON_KEY(pstJson,"data");
    GET_STR_KEY(pstTransJson,"channel_merch_id",sChannelMerchId);
    
    if( FindMerchApk(sChannelMerchId,sAppKey) < 0) {
       ErrHanding(pstTransJson, "96", "��ȡ�����̻�[%s]��APPKEYʧ��.", sChannelMerchId);
       return -1;
    }
    
    SET_STR_KEY(pstTransJson,"appkey",sAppKey);
    
    return ( 0 );
}