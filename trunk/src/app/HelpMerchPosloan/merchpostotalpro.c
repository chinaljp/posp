/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
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

int UpdateRefund(char *pcMerchId, char *pcStatus);
int GetRrn(char *pcRrn,char *pcSysTrace);
int UnFrozenWallet(char *pcMerchId, char *SerialNum, char *pcAmount, char *pcFrozenStatus);
int BackMoney();

int merchpospro() {
    char sDate[8 + 1] = {0},sCurrentDate[8 + 1] = {0},sBeforeDate[8 + 1] = {0},sSettleDate[8 + 1] = {0};
    int i = 0;
    /*�ۼ��̻����ÿ����׽��*/
    if ( PosCdTotal() < 0 ) {
        tLog(ERROR, "����ͨ�̻����ÿ����׽���ۼ�ʧ�ܣ�");
        return ( -1 );
    }
    /*�ۼ��̻���ά�뽻�׽��*/
    tGetDate(sCurrentDate, "", -1);
    tStrCpy(sDate,sCurrentDate,8);
    
    tAddDay(sDate, -1);
    tStrCpy(sSettleDate,sDate,8);
    
    //������Ϊ�ǽ����գ�������ά�뽻�׽���ۼ�
    if ( IsSettleDate(sCurrentDate) == 0 ) {       
        /* ��ȡ��һ�������գ��ڼ���û��̫���ģ�����ֻ���15��� */
        for (i = 0; i < 15; i++) {
            if (IsSettleDate(sDate) == 0) {
                tStrCpy(sBeforeDate, sDate, 8);
                break;
            }
            tAddDay(sDate, -1);
        }
        if(InlineTotal(sBeforeDate,sSettleDate) < 0) {
           tLog(ERROR,"�ۼ��̻���ά�뽻�׶�ʧ��!");
           return ( -1 );
        }
    }
    else {
        tLog(ERROR,"��ǰ����[%s]Ϊ�ǽ����գ����ۼƶ�ά�뽻�׽�",sCurrentDate);
    }
    
    /*�������ͨ�̻��ڹ涨�����ڵĽ��׶��Ƿ�����涨���������̻�����ѷ��ظ��̻������������̻�����ѿ۳�*/
    if( BackMoney() < 0 ) {
        tLog(ERROR,"����ͨ�̻�Ѻ�𷵻�����ʧ�ܣ�ע��鿴ԭ�򣡣���");
    }
    
    return ( 0 );
}

//�ۼ��̻����ÿ����׽�� 
int PosCdTotal() {
    int iCnt = 0;
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    
    tLog(INFO,"=================�ۼƴ��ǿ����׶�=========================");
    snprintf(sSqlStr,sizeof(sSqlStr),"merge into b_term_activity_merch o \
                                using (select h.merch_id as merch_id,sum(h.amount) as amount from b_pos_trans_detail_his h \
                                join b_cups_trans_detail c on c.channel_rrn=h.rrn and c.trans_date=h.trans_date and c.chk_flag='Y' and h.card_type = '1' \
                                join b_term_activity_merch b on h.merch_id = b.merch_id and b.create_time < h.create_time \
                                where c.settle_date=to_char(sysdate-1,'YYYYMMDD') group by h.merch_id ) p \
                                on (p.merch_id=o.merch_id and o.merch_remission_status = '0' and o.status = '1') \
                                when matched then \
                                update set o.trans_total_amt =o.trans_total_amt+p.amount");
    tLog(DEBUG, "sql[%s]", sSqlStr);
    if(tExecute(&pstRes, sSqlStr) < 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "============�̻��ۼ����ÿ����׶����ʧ��=============");
        tRollback();
        return ( -1 );
    }
    else {
        iCnt = tGetAffectedRows();
        tLog(INFO, "============�̻��ۼ����ÿ����׶���³ɹ�,�����̻���[%d]=============",iCnt);
        tCommit();
    }
    tReleaseRes(pstRes);
    return ( 0 );
}

//�ۼ��̻���ά�뽻�׽�� 
int InlineTotal(char *pcBeforeDate, char *pcSettleDate) {
    int iCnt = 0;
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstQrcodeRes = NULL;
    
    tLog(INFO,"=================�ۼƶ�ά�뽻�׶�, [%s] - [%s]=========================",pcBeforeDate,pcSettleDate);
    snprintf(sSqlStr,sizeof(sSqlStr),"merge into b_term_activity_merch o \
                        using (select h.merch_id as merch_id,sum(h.amount) as amount from b_inline_tarns_detail_his h \
                                join b_term_activity_merch b on h.merch_id = b.merch_id and b.create_time < h.create_time \
                                where h.trans_date between '%s' and '%s' and h.check_flag = 'Y' and h.trans_code != '0AQ000' group by h.merch_id) p \
                        on (o.merch_id = p.merch_id and o.merch_remission_status = '0' and o.status = '1') \
                        when matched then \
                                update set o.trans_total_amt =o.trans_total_amt+p.amount",pcBeforeDate,pcSettleDate);
    tLog(DEBUG, "sql[%s]", sSqlStr);
    if(tExecute(&pstQrcodeRes, sSqlStr) < 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "============�̻��ۼƶ�ά�뽻�׶����ʧ��=============");
        tRollback();
        return ( -1 );
    }
    else {
        iCnt = tGetAffectedRows();
        tLog(INFO, "============�̻��ۼƶ�ά�뽻�׶���³ɹ�,�����̻���[%d]=============",iCnt);
        tCommit();
    }
    tReleaseRes(pstQrcodeRes);
    return ( 0 );
}

//��������ͨ�̻������
int BackMoney() {
    char sSqlStr[1024] = {0};
    char sMerchId[15+1] = {0}, sSerialNum[32+1] = {0}, sMerchReStat[2+1] = {0};
    char sEndDate[8+1] = {0}, sCurrentDate[8+1] = {0};
    double dServAmt = 0.00, dTotalAmt = 0.00, dTransTotalAmt = 0.00;
    char sAmount[20+1] = {0};/*�̻������*/
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select merch_id,serial_num,service_charge,total_amt,trans_total_amt,"
                           "end_date,substr(to_char(sysdate,'yyyymmddhh24miss'),1,8),merch_remission_status"
                        " from b_term_activity_merch where merch_remission_status in ('0','a','b') and status = '1'");
    
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
        MEMSET(sSerialNum);
        MEMSET(sEndDate);
        MEMSET(sCurrentDate);
        MEMSET(sAmount);
        MEMSET(sMerchReStat);
        dServAmt = 0.00; 
        dTotalAmt = 0.00; 
        dTransTotalAmt = 0.00;
        
        STRV(pstRes, 1, sMerchId);
        STRV(pstRes, 2, sSerialNum);
        DOUV(pstRes, 3, dServAmt);
        DOUV(pstRes, 4, dTotalAmt);
        DOUV(pstRes, 5, dTransTotalAmt);
        STRV(pstRes, 6, sEndDate);
        STRV(pstRes, 7, sCurrentDate);
        STRV(pstRes, 8, sMerchReStat);
        
        tTrim(sMerchId);
        tTrim(sSerialNum);
        tTrim(sEndDate);
        tTrim(sCurrentDate);
        tTrim(sMerchReStat);
        
        tLog(DEBUG,"�����[%.02f]",dServAmt);
        /*Ѻ����dServAmt תΪ�ַ�������*/ 
        sprintf(sAmount,"%.02f",dServAmt);
        tLog(DEBUG,"��ǰ����sCurrentDate[%s]",sCurrentDate);
        
        if( (memcmp(sCurrentDate,sEndDate,8) >= 0 && !DBL_CMP(dTransTotalAmt,dTotalAmt) && !DBL_EQ(dTransTotalAmt,dTotalAmt)) || sMerchReStat[0] == 'b' ) {
            /*�۳�*/
            if ( sMerchReStat[0] == 'b' ) {
                tLog(INFO,"����״̬: [%s],Ѻ��۳�ʧ�ܣ������۳���", sMerchReStat); 
            } 
            else {
                tLog(INFO,"�ﵽ�涨����[%s],����ͨ�̻�[%s]�ۼƽ��׶�Ϊ[%.02f],С���������[%.02f],Ѻ��۳�.",
                     sEndDate,sMerchId,dTransTotalAmt,dTotalAmt); 
                /*������Ҫ��������ݳ�ʼ״̬����������״̬Ϊ��ʼ״̬��0����ÿ��ʼ����һ��ʱ������һ�����ݸ��³� h-�����У��ٽ��д������*/
                UpdateRefund(sMerchId, "h");
            }
            
            /*�۳�����*/
            if( UnFrozenWallet(sMerchId,sSerialNum,sAmount,"6")<0 ) {
                tLog(ERROR,"�۳��̻�[%s]�����ʧ��",sMerchId);
                /*����״̬����״̬*/
                if( sMerchReStat[0] != 'b' ) {
                    if( UpdateRefund(sMerchId, "b") < 0 ) {
                        tLog(ERROR,"�̻�[%s]����ѿ۳�ʧ��,�۳�ʧ�� ״̬����ʧ��!!!",sMerchId);
                    }
                }
                continue;
            }
            if( UpdateRefund(sMerchId, "2") < 0 ) {
                tLog(ERROR,"�̻�[%s]����ѿ۳��ɹ�,�۳��ɹ� ״̬����ʧ��!!!",sMerchId);
                continue;
            }
            tLog(INFO,"�̻�[%s]����ѿ۳��ɹ�,�۳����[%.2f]",sMerchId,dServAmt);
        }
        else if( (memcmp(sCurrentDate,sEndDate,8) <= 0 && DBL_CMP(dTransTotalAmt,dTotalAmt)) || 
                 (memcmp(sCurrentDate,sEndDate,8) <= 0 && DBL_EQ(dTransTotalAmt,dTotalAmt)) || sMerchReStat[0] == 'a' ) {
            /*����*/
            if( sMerchReStat[0] == 'a' ) {
                tLog(INFO,"����״̬��[%s]��Ѻ�𷵻�ʧ�ܣ�����������", sMerchReStat);
            }
            else {
                tLog(INFO,"�涨����[%s]��,����ͨ�̻�[%s]�ۼƽ��׶�Ϊ[%.02f],���ڵ��ڹ涨���׶�[%.02f],Ѻ�𷵻�",
                    sEndDate,sMerchId,dTransTotalAmt,dTotalAmt);
                /*������Ҫ��������ݳ�ʼ״̬����������״̬Ϊ��ʼ״̬��0����ÿ��ʼ����һ��ʱ������һ�����ݸ��³� h-�����У��ٽ��д������*/
                UpdateRefund(sMerchId, "h");
            }
            
            /*�ⶳ����*/
            if( UnFrozenWallet(sMerchId,sSerialNum,sAmount,"5")<0 ) {
                tLog(ERROR,"�����̻�[%s]�����ʧ��",sMerchId);
                /*����״̬����״̬*/
                if( sMerchReStat[0] != 'a' ) {
                    if( UpdateRefund(sMerchId, "a") < 0 ) {
                        tLog(ERROR,"�̻�[%s]����ѷ���ʧ��,����ʧ�� ״̬����ʧ��!!!",sMerchId);
                    }
                }
                continue;
            }
            if( UpdateRefund(sMerchId, "1") < 0 ) {
                tLog(ERROR,"�̻�[%s]����ѷ����ɹ�,�����ɹ� ״̬����ʧ��!!!",sMerchId);
                continue;
            }
            tLog(INFO,"�̻�[%s]����ѷ����ɹ�,�������[%.2f]",sMerchId,dServAmt);
        }
        else {
            /*��������*/
            tLog(INFO,"�涨����[%s]��,����ͨ�̻�[%s]�ۼƽ��׶�Ϊ[%.02f],С�ڹ涨���׶�[%.02f],��������",
                    sEndDate,sMerchId,dTransTotalAmt,dTotalAmt);
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
    
    tLog(ERROR, "������������������ͨ�̻����̻�����Ѵ������");
    
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
    int iResCnt = 0;
    
    if (pcStatus[0] == '1' || pcStatus[0] == '2') {
        snprintf(sSqlStr,sizeof(sSqlStr),"update b_term_activity_merch set merch_remission_status = '%s',status = 'X',merch_remission_time = sysdate,last_mod_time = sysdate,"
                            " remark = '%s' where merch_id = '%s'",pcStatus,"�����ر�",pcMerchId);
    }
    else {
        snprintf(sSqlStr,sizeof(sSqlStr),"update b_term_activity_merch set merch_remission_status = '%s',last_mod_time = sysdate"
                            " where merch_id = '%s'",pcStatus,pcMerchId);
    }
    
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        tLog(ERROR, "��������ͨ�̻�[%s]��Ѻ�𷵻�״̬[%s]ʧ��,1-�ѷ���; a-����ʧ��; 2-�ѿ۳�; b-�۳�ʧ��; ",pcMerchId,pcStatus);
        return ( -1 );
    }
    tCommit();
    tLog(INFO, "Ӱ���¼��[%d]", iResCnt);

    return ( 0 );
}