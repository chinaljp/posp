/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   merchpostranstotal.c
 * Author: Administrator
 *
 * Created on 2017��7��17��, ����11:01
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_db.h"
#include "t_tools.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "trace.h"
#include "t_log.h"
#include "t_app_conf.h"
#include "t_daemon.h"

/*
 * 
 */

int UpdateRefund(char *pcMerchId, char *pcStatus);
int GetRrn(char *pcRrn,char *pcSysTrace);
int addWalletToAgent(char *pcAgentId, char *pcAmount);
int UnFrozenWallet(char *pcMerchId, char *SerialNum, char *pcAmount, char *pcFrozenStatus);
int BackMoney();
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

int main(int argc, char* pcArgv[]) {
    
    int iCnt = 0;
    OCI_Resultset *pstRes = NULL;
    char sCurTime[8+1];
    char sTime[2+1];
    char sSqlStr[1024];
    
    MEMSET(sCurTime);
    MEMSET(sTime);
    MEMSET(sSqlStr);
    g_iQutLoop = 1;
    
    tDaemon();
    
    /* �����źţ�ֻ�����Ƴ��ź� */
    SigProc();
    sigset(SIGUSR1, SigQuit);
    
    if (tInitLog(pcArgv[0]) < 0) {
        fprintf(stderr, "��ʼ��[%s]��־ʧ��.\n", pcArgv[0]);
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
    
    while(g_iQutLoop) {
        tGetTime(sCurTime, ":", -1);
        tLog(INFO, "��ǰʱ��[%s]",sCurTime);
        /*ȡ��ǰʱ������*/
        tStrCpy( sTime, sCurTime, 2 );
        if(atoi(sTime) == 6) {
            /*����ͨ�ۼƽ��׶� ֻ�ۼƴ��ǿ����׶�*/
            snprintf(sSqlStr,sizeof(sSqlStr),"merge into b_open_merchant o \
                                using (select user_code,sum(h.amount) as amount from b_pos_trans_detail_his h \
                                join b_cups_trans_detail c on c.channel_rrn=h.rrn and c.trans_date=h.trans_date and c.chk_flag='Y' and h.card_type = '1'\
                                where c.settle_date=to_char(sysdate-1,'YYYYMMDD') group by user_code ) p \
                                on (p.user_code=o.user_code) \
                                when matched then \
                                update set o.trans_amt =o.trans_amt+p.amount");
            if(tExecute(&pstRes, sSqlStr) < 0) {
                tLog(DEBUG, "sql[%s]", sSqlStr);
                tLog(ERROR, "============POS���ף��̻��ۼƽ��׶����ʧ��=============");
                tRollback();
            }
            else {
                iCnt = tGetAffectedRows();
                tLog(ERROR, "============POS���ף��̻��ۼƽ��׶���³ɹ�,�����̻���[%d]=============",iCnt);
                tCommit();
            }
            tReleaseRes(pstRes);
            
            /*�������ͨ�̻��ڹ涨�����ڵĽ��׶��Ƿ�����涨�������򷵻�Ѻ���̻�����������Ѻ�𷵸�������*/
            if( BackMoney() < 0) {
                tLog(ERROR,"����ͨ�̻�Ѻ�𷵻�����ʧ�ܣ�ע��鿴ԭ�򣡣���");
            }
        }
        
        sleep(3600);
    }
    tDoneLog();
    tCloseRedis();
    tCloseDb();
    return (EXIT_SUCCESS);
}

/* ����ͨ ���׽���ۼ� ��������������Ѻ��*/
int BackMoney(){
    char sSqlStr[1024] = {0};
    char sUserCode[15+1] = {0};
    char sMerchId[15+1] = {0}, sAgentId[32 + 1] = {0}, sSerialNum[32+1] = {0}, sRefundStat[2+1] = {0};
    char sCycleDate[8+1] = {0}, sCurrentDate[8+1] = {0};
    double dFreezeAmt = 0.00, dFullAmt = 0.00, dTransAmt = 0.00;
    char sAmount[20+1] = {0};/*�̻�Ѻ��*/
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select merch_id,agent_id,serial_num,freeze_amount,full_amount,trans_amt,"
                        "cycle_date,substr(to_char(sysdate,'yyyymmddhh24miss'),1,8),refund_status"
                        " from b_open_merchant where refund_status in ('0','X')");
    
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }

    while ( OCI_FetchNext(pstRes) ) { 
     tLog(DEBUG,"OCI_GetRowCount(pstRes)==[%d]",OCI_GetRowCount(pstRes));   
        MEMSET(sMerchId);
        MEMSET(sAgentId);
        MEMSET(sSerialNum);
        MEMSET(sCycleDate);
        MEMSET(sCurrentDate);
        MEMSET(sAmount);
        MEMSET(sRefundStat);
        dFreezeAmt = 0.00; 
        dFullAmt = 0.00; 
        dTransAmt = 0.00;
        
        STRV(pstRes, 1, sMerchId);
        STRV(pstRes, 2, sAgentId);
        STRV(pstRes, 3, sSerialNum);
        DOUV(pstRes, 4, dFreezeAmt);
        DOUV(pstRes, 5, dFullAmt);
        DOUV(pstRes, 6, dTransAmt);
        STRV(pstRes, 7, sCycleDate);
        STRV(pstRes, 8, sCurrentDate);
        STRV(pstRes, 9, sRefundStat);
        
        tTrim(sMerchId);
        tTrim(sAgentId);
        tTrim(sSerialNum);
        tTrim(sCycleDate);
        tTrim(sCurrentDate);
        
        tLog(DEBUG,"Ѻ����[%.02f]",dFreezeAmt);
        /*Ѻ����dFreezeAmt תΪ�ַ�������*/ 
        sprintf(sAmount,"%.02f",dFreezeAmt);
        tLog(DEBUG,"��ǰ����sCurrentDate[%s]",sCurrentDate);
        /*����ǰ���ڴ��ڵ��ڹ涨���� ���� ����ͨ�̻��ۼƽ��׶����ڹ涨���׶� �� ��Ѻ��ת�������̣�Ǯ����ֵ������*/
        /*���� �̻�Ѻ���Ѿ����۳��� �� �ض��������Ͻ�Ѻ��ת�������� ������ */
        if( (memcmp(sCurrentDate,sCycleDate,8)>= 0 && dTransAmt < dFullAmt) || sRefundStat[0] == 'X' ) {
            tLog(INFO,"������ﵽ�涨����[%s],����ͨ�̻�[%s]�ۼƽ��׶�Ϊ[%.02f],С�ڹ涨���׶�[%.02f],Ѻ��ת��������[%s]",
                     sCycleDate,sMerchId,dTransAmt,dFullAmt,sAgentId); 
            tLog(INFO,"���߳����涨����[%s],����ͨ�̻�[%s]�ۼƽ��׶�Ϊ[%.02f],���ڵ��ڹ涨���׶�[%.02f],����״̬sRefundStat��X-Ѻ���ѿ۳���Ѻ��ת��������[%s]",
                     sCycleDate,sMerchId,dTransAmt,dFullAmt,sAgentId); 
            /*Ѻ��ת�������̣�����̻�Ѻ��δ�۳� ����п۳��̻�Ѻ����� */
            if(sRefundStat[0] != 'X') {
                if( UnFrozenWallet(sMerchId,sSerialNum,sAmount,"6")<0 ) {
                    tLog(ERROR,"�۳��̻�[%s]Ѻ��ʧ��",sMerchId);
                    continue;
                } 
                /*���̻�Ѻ��ת�������̣��۳��̻�Ѻ�� ���·�����־refund_status Ϊ X-�ѿ۳��̻�Ѻ��*/
                if( UpdateRefund(sMerchId, "X") < 0 ) {
                    tLog(ERROR,"�۳��̻�[%s]Ѻ��ɹ�,Ѻ�𷵻�״̬����ʧ�ܣ�һ��Ҫ��ʱ��������������",sMerchId);
                    continue;
                }
            } 
            
            /*Ǯ����ֵ����*/
            if( addWalletToAgent(sAgentId,sAmount) <0 ){
                tLog(ERROR,"������[%s]����ʧ�ܣ�",sAgentId);
                continue;
            }
            /*���˳ɹ�����Ѻ�𷵻�״̬refund_status Ϊ: 2-����������*/
            if( UpdateRefund(sMerchId, "2") < 0 ) {
                tLog(ERROR,"������[%s]���˳ɹ�,Ѻ�𷵻�״̬����ʧ�ܣ�һ��Ҫ��ʱ��������������",sAgentId);
                continue;
            }
            tLog(INFO,"������[%s]���˳ɹ�,���˽��[%.2f]",sAgentId,dFreezeAmt);
        }
        /*����ǰ����С�ڵ��ڹ涨���� ���� ����ͨ�̻��ۼƽ��׶���ڵ��ڹ涨���׶� �� ��Ѻ�𷵻�������ͨ�̻����ʽ�ⶳ������*/
        else if( memcmp(sCurrentDate,sCycleDate,8)<= 0 && dTransAmt >= dFullAmt ) {
            tLog(INFO,"�涨����[%s]��,����ͨ�̻�[%s]�ۼƽ��׶�Ϊ[%.02f],���ڵ��ڹ涨���׶�[%.02f],Ѻ�𷵻�",
                    sCycleDate,sMerchId,dTransAmt,dFullAmt);
            /*�ⶳ����*/
            if( UnFrozenWallet(sMerchId,sSerialNum,sAmount,"5")<0 ) {
                tLog(ERROR,"�̻�[%s]Ѻ�𷵻�ʧ��",sMerchId);
                continue;
            }
            /*�ⶳ�ɹ�����Ѻ�𷵻�״̬refund_status : 1-�����̻�*/
            if( UpdateRefund(sMerchId, "1") < 0 ) {
                tLog(ERROR,"�̻�[%s]Ѻ��ⶳ�ɹ�,Ѻ�𷵻�״̬����ʧ�ܣ�һ��Ҫ��ʱ��������������",sMerchId);
                continue;
            }
            tLog(INFO,"�̻�[%s]Ѻ�𷵻��ɹ�,Ѻ����[%.2f]",sMerchId,dFreezeAmt);
        }
        else {
            tLog(INFO,"�涨����[%s]��,����ͨ�̻�[%s]�ۼƽ��׶�Ϊ[%.02f],С�ڹ涨���׶�[%.02f],��������",
                    sCycleDate,sMerchId,dTransAmt,dFullAmt);
        }
    }
    
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "��δ����������ͨ�̻���Ϣ");
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    
    tLog(ERROR, "������������������ͨ�̻���Ѻ�������");
    return ( 0 );
}

/*����������*/
int addWalletToAgent(char *pcAgentId, char *pcAmount) {
    cJSON *pstTransJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    
    pstTransJson = cJSON_CreateObject();
    if (NULL == pstTransJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return ( -1 );
    }
    SET_STR_KEY(pstTransJson, "agentNo", pcAgentId);
     /*����rrn,��ȡϵͳ��ˮ��*/
    if( GetRrn(sRrn,sSysTrace) < 0 ) {
        tLog(ERROR,"����rrnʧ��");
        return ( -1 );
    }
    tLog(DEBUG,"����rrn[%s]����ȡϵͳ��ˮ��[%s]",sRrn,sSysTrace);
    SET_STR_KEY(pstTransJson, "sys_trace", sSysTrace);
    SET_STR_KEY(pstTransJson, "rrn", sRrn);
    SET_STR_KEY(pstTransJson, "amount", pcAmount);
    SET_STR_KEY(pstTransJson, "debitType", "1");
    SET_STR_KEY(pstTransJson, "postType", "1");
    SET_STR_KEY(pstTransJson, "remark", "����ͨ����");
    SET_STR_KEY(pstTransJson, "trans_code", "00A600");
    
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00A600%s", sRrn);
    stQMsgData.pstDataJson = pstTransJson;
    iRet = tSvcCall("00A600_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            tLog(ERROR, "����[%s]ʧ��,���������˳�ʱ.", sRrn);
        } else {
            tLog(INFO, "����[%s]ʧ��,����������ʧ��.", sRrn);
        }
        return ( -1 );
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
        MEMSET(sRrn);
        GET_STR_KEY(pstRecvDataJson, "rrn", sRrn);
        if (memcmp(sRespCode, "00", 2)) {
            tLog(ERROR, "����[%s]ʧ��,����������ʧ��[%s:%s].", sRrn, sRespCode, sResvDesc);
            return ( -1 );
        } else {
            tLog(INFO, "����[%s]�ɹ�,���������˳ɹ�.", sRrn);
        }
    }
    cJSON_Delete(pstTransJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);

    return ( 0 );
}
/*�ʽ�ⶳor�۳�*/
int UnFrozenWallet(char *pcMerchId, char *SerialNum, char *pcAmount, char *pcFrozenStatus) {
    cJSON * pstTransJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    char sSysTrace[TRACE_NO_LEN + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    
    pstTransJson = cJSON_CreateObject();
    if (NULL == pstTransJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return ( -1 );
    }
    SET_STR_KEY(pstTransJson, "merchantNo", pcMerchId);
    SET_STR_KEY(pstTransJson, "optNo", SerialNum);
    /*����rrn,��ȡϵͳ��ˮ��*/
    if( GetRrn(sRrn,sSysTrace) < 0 ) {
        tLog(ERROR,"����rrnʧ��");
        return ( -1 );
    }
    tLog(DEBUG,"����rrn[%s]����ȡϵͳ��ˮ��[%s]",sRrn,sSysTrace);
    SET_STR_KEY(pstTransJson, "sys_trace", sSysTrace);
    SET_STR_KEY(pstTransJson, "rrn", sRrn);
    SET_STR_KEY(pstTransJson, "amount", pcAmount);
    SET_STR_KEY(pstTransJson, "walletType", "0");
    SET_STR_KEY(pstTransJson, "frozenBusinessType", "2");
    SET_STR_KEY(pstTransJson, "effectiveMode", "2");
    tLog(DEBUG,"����״̬ pcFrozenStatus[%s]",pcFrozenStatus);
    SET_STR_KEY(pstTransJson, "frozenStatus", pcFrozenStatus);/*frozenStatus ����״̬��1-���ᣬ4-���ֽⶳ��5-�ⶳ��6-�۳�*/
    SET_STR_KEY(pstTransJson, "frozenRemark", "����ͨѺ��");
    SET_STR_KEY(pstTransJson, "trans_code", "00TD00");
    
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00TD00%s", sRrn);
    stQMsgData.pstDataJson = pstTransJson;
    iRet = tSvcCall("00TD00_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    tLog(DEBUG,"iRet[%d]",iRet);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            if(pcFrozenStatus[0] = '5') {
                tLog(ERROR, "����[%s]ʧ��,�����̻�Ѻ��ʱ.", sRrn);
            } 
            else {
                tLog(ERROR, "����[%s]ʧ��,�۳��̻�Ѻ��ʱ.", sRrn);
            }
        } else {
            if(pcFrozenStatus[0] = '5') {
                tLog(INFO, "����[%s]ʧ��,�����̻�Ѻ��ʧ��.", sRrn);
            }
            else {
                tLog(INFO, "����[%s]ʧ��,�۳��̻�Ѻ��ʧ��.", sRrn);
            }
        }
        return ( -1 );
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
        MEMSET(sRrn);
        GET_STR_KEY(pstRecvDataJson, "rrn", sRrn);
        if (memcmp(sRespCode, "00", 2)) {
            if(pcFrozenStatus[0] == '5') {
                tLog(ERROR, "����[%s]ʧ��,�����̻�Ѻ��ʧ��[%s:%s].", sRrn, sRespCode, sResvDesc);
            }
            else {
                tLog(ERROR, "����[%s]ʧ��,�۳��̻�Ѻ��ʧ��[%s:%s].", sRrn, sRespCode, sResvDesc);
            }
            return ( -1 );
        } else {
            if(pcFrozenStatus[0] == '5') {
                tLog(INFO, "����[%s]�ɹ�,�����̻�Ѻ��ɹ�.", sRrn);
            }
            else {
                tLog(INFO, "����[%s]�ɹ�,�۳��̻�Ѻ��ɹ�.", sRrn);
            }
        }
    }
    cJSON_Delete(pstTransJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);
 
    return ( 0 );
}

int GetRrn(char *pcRrn,char *pcSysTrace) {
    char sTime[6+1] = {0}, sTrace[6+1] = {0}, sTmp[15+1] = {0};
    /*��ǰ����ʱ����ϵͳ��ˮ����ɲο���rrn*/
    tGetTime(sTime, "", -1);
    if (GetSysTrace(sTrace) < 0) {
        tLog(ERROR, "��ȡϵͳ��ˮ��ʧ��,���׷���.");
        return -1;
    }
    sprintf(sTmp, "%s%s", sTime, sTrace);
    tTrim(sTmp);
    tStrCpy(pcRrn,sTmp,12);
    tStrCpy(pcSysTrace,sTrace,6);
    
    return ( 0 );
}

int UpdateRefund(char *pcMerchId, char *pcStatus) {
    char sSqlStr[1024] = {0};
    OCI_Statement *pstState = NULL;
    OCI_Resultset *pstRes = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"update b_open_merchant set refund_status = '%s',last_mod_time = sysdate"
                        " where merch_id = '%s'",pcStatus,pcMerchId);
    
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (  pstState ==  NULL || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "��������ͨ�̻�[%s]��Ѻ�𷵻�״̬[%s]ʧ��,1-�����̻�,2-����������,X-�̻�Ѻ���ѿ۳�����״̬�ڴ���������ʧ��ʱ����֣�",pcMerchId,pcStatus);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return ( -1 );
    }
    tLog(DEBUG,"������[%d]��",tGetAffectedRows());
    tCommit();
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    
    return ( 0 );
}