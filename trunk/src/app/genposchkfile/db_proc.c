/*                                                                                                                
 * To change this license header, choose License Headers in Project Properties.                                   
 * To change this template file, choose Tools | Templates                                                         
 * and open the template in the editor.                                                                           
 */
#include <stdio.h>
#include "t_log.h"                                                                                                
#include "t_db.h"                                                                                                 
#include "t_tools.h"                                                                                                                                                                        

void GetOMsgType(char *pcOMsgType, char *pcTransCode) {
    if (strstr("020000,U20000,M20000,T20000", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "1000");
    } else if (strstr("020001,U20001", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "2002");
    } else if (strstr("020002,M20002", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "2000");
    } else if (strstr("020003,M20003,T20003", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "2001");
    } else if (strstr("024100", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "1010");
    } else if (strstr("024102", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "2010");
    } else if (strstr("024103", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "2011");
    } else if (strstr("020023,M20023", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "1001");
    } else if (strstr("024123", pcTransCode) != NULL) {
        strcpy(pcOMsgType, "1011");
    }
}

int FindPosCurSettleDate(char *pcDate) {
    char sDate[8 + 1] = {0};
    int i = 0;

    tGetDate(sDate, "", -1);
    //tAddDay(sDate, 1);
    /* �ڼ���û��̫���ģ�����ֻ���15��� */
    for (i = 0; i < 15; i++) {
        if (IsSettleDate(sDate) == 0) {
            tStrCpy(pcDate, sDate, 8);
            return 0;
        }
        tAddDay(sDate, 1);
    }
    tStrCpy(pcDate, sDate, 8);
    tLog(ERROR, "���ҽ�����ʧ��,ʹ������������Ϊ������[%s]", pcDate);
    return -1;
}

int writefilehead(FILE *fp, char *pcTransDate) {
    char sTransDate[8 + 1] = {0}, sBeforeDate[8 + 1] = {0};
    int iCount = 0;
    double dAmount = 0L, dSettAmt = 0L;
    char sSqlStr[2048];
    char sBuf[100] = {0};
    OCI_Resultset *pstRes = NULL;
    //tGetDate(sTransDate, "", -1);
    //tAddDay(sTransDate, -1);
    tStrCpy(sTransDate, pcTransDate, 8);
    tStrCpy(sBeforeDate, pcTransDate, 8);
    tAddDay(sBeforeDate, -1);
    tLog(DEBUG, "��ǰ��������Ϊ[%s]", sTransDate);

    snprintf(sSqlStr, sizeof (sSqlStr), "select  sum(num),sum(amt1)-sum(amt2),sum(fee1)-sum(fee2) from "
            "(select t.acct_flag,count(*) as num ,decode(t.acct_flag,'+',sum(a.amount),0) as amt1 "
            ",decode(t.acct_flag,'+',sum(a.amount)-sum(a.fee), 0) as fee1 "
            ",decode(t.acct_flag,'-',sum(a.amount), 0) as amt2 "
            ",decode(t.acct_flag,'-',sum(a.amount)-sum(a.fee), 0) as fee2 "
            "from b_pos_trans_detail_his a  "
            "join S_TRANS_CODE t on t.TRANS_CODE=a.TRANS_CODE and t.settle_flag='1' "
            "left join b_merch_account b on a.merch_id=b.merch_id "
            "where a.rrn in(select channel_rrn from b_cups_trans_detail where chk_flag in ('Y','B') "
            "and settle_date='%s') and a.trans_date between '%s' and '%s' and a.istresp_code is not NULL "
            "group by t.acct_flag union select t.acct_flag,count(*) as num ,decode(t.acct_flag,'+',sum(a.amount),0) as amt1 "
            ",decode(t.acct_flag,'+',sum(a.amount)-sum(a.fee), 0) as fee1 "
            ",decode(t.acct_flag,'-',sum(a.amount), 0) as amt2 "
            ",decode(t.acct_flag,'-',sum(a.amount)-sum(a.fee), 0) as fee2 "
            "from b_pos_trans_detail_his a  "
            "join S_TRANS_CODE t on t.TRANS_CODE=a.TRANS_CODE and t.settle_flag='1' "
            "left join b_merch_account b on a.merch_id=b.merch_id "
            "where a.o_rrn in(select channel_rrn from b_cups_trans_detail where chk_flag in ('Y','B') "
            "and settle_date='%s') and substr(a.trans_code,6,1) = '3' and a.resp_code='00' and a.istresp_code is not NULL and a.trans_date between '%s' and '%s' "
            "group by t.acct_flag union  select t.acct_flag,count(*) as num ,decode(t.acct_flag,'+',sum(a.amount),0) as amt1 "
            ",decode(t.acct_flag,'+',sum(a.amount)-sum(a.fee), 0) as fee1 "
            ",decode(t.acct_flag,'-',sum(a.amount), 0) as amt2 "
            ",decode(t.acct_flag,'-',sum(a.amount)-sum(a.fee), 0) as fee2 "
            "from b_pos_trans_detail_his a  "
            "join S_TRANS_CODE t on t.TRANS_CODE=a.TRANS_CODE and t.settle_flag='1' "
            "left join b_merch_account b on a.merch_id=b.merch_id "
            "where a.rrn in(select rrn from b_spdb_trans_detail where chk_flag in ('Y','B') "
            "and settle_date='%s') and a.trans_date between '%s' and '%s' "   
            "group by t.acct_flag)", sTransDate, sBeforeDate, sTransDate, sTransDate, sBeforeDate, sTransDate, sTransDate, sBeforeDate, sTransDate);
    tLog(DEBUG, "[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }


    while (OCI_FetchNext(pstRes)) {

        INTV(pstRes, 1, iCount);
        DOUV(pstRes, 2, dAmount);
        DOUV(pstRes, 3, dSettAmt);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    //fwrite("�ܽ��ױ���,�ܽ��׽��,�ܽ�����\n", sizeof ("�ܽ��ױ���,�ܽ��׽��,�ܽ�����\n"), 1, fp);
    sprintf(sBuf, "%d,%.2lf,%.2lf\n", iCount, dAmount, dSettAmt);
    fwrite(sBuf, strlen(sBuf), 1, fp);
    tLog(INFO, "�����ļ�ͷ�����ɹ�.");

    return 0;
}

int writefilebody(FILE *fp, char *pcTransDate, char *pcSettleDate) {
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1], sBeforeDate[8 + 1] = {0};
    char sTransCode[6 + 1] = {0};
    char sSqlStr[2048];
    char sBuf[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    char sRrn[12 + 1] = {0}; /* ϵͳ�ο���           */
    double dAmount; /* ���׷�����           */
    double dSettAmount; /* ������           */
    char sAccountUUID[64 + 1] = {0}; /* �˻�uuid */
    char sTransType[4 + 1] = {0}; /* ��������             */
    char sSysTrace[6 + 1] = {0}; /* �յ�ϵͳ��ˮ��       */
    char sMerchId[15 + 1] = {0}; /* �̻����             */
    char sChannelMerchId[15 + 1] = {0}; /* �����̻����         */
    char sTransmitTime[14 + 1] = {0}; /* ����ʱ��       */
    char sSettleDate[8 + 1] = {0}; /* ��������             */

    //tGetDate(sTransDate, "", -1);
    //tAddDay(sTransDate, -1);
    FindPosCurSettleDate(sSettleDate);
    tStrCpy(sTransDate, pcTransDate, 8);
    tStrCpy(sBeforeDate, pcTransDate, 8);
    tAddDay(sBeforeDate, -1);
    tLog(DEBUG, "��ǰ��������Ϊ[%s]", sTransDate);
    //fwrite("ϵͳ�ο���,���׽��,������,�˻�UUID,��������,������ˮ��,�̻����,�����̻���,����ʱ��,��������\n", sizeof ("ϵͳ�ο���,���׽��,������,�˻�UUID,��������,������ˮ��,�̻����,�����̻���,����ʱ��,��������\n"), 1, fp);
    snprintf(sSqlStr, sizeof (sSqlStr), "select a.rrn,a.amount,b.account_uuid "
            " ,a.trans_code,a.sys_trace,a.merch_id,a.channel_merch_id,a.trans_date,a.trans_time,a.amount-a.fee, a.settle_date "
            "from b_pos_trans_detail_his a join S_TRANS_CODE t on t.TRANS_CODE=a.TRANS_CODE and t.settle_flag='1' "
            "left join b_merch_account b on a.merch_id=b.merch_id "
            " where a.rrn in(select channel_rrn from b_cups_trans_detail where chk_flag in ('Y','B') "
            "and settle_date='%s') and a.trans_date between '%s' and '%s' and a.istresp_code is not NULL  union select a.rrn,a.amount,b.account_uuid  ,a.trans_code,a.sys_trace,"
            "a.merch_id,a.channel_merch_id,a.trans_date,a.trans_time,a.amount-a.fee, a.settle_date "
            "from b_pos_trans_detail_his a join S_TRANS_CODE t on t.TRANS_CODE=a.TRANS_CODE and t.settle_flag='1' "
            "left join b_merch_account b on a.merch_id=b.merch_id  where a.o_rrn in(select channel_rrn from "
            "b_cups_trans_detail where chk_flag in ('Y','B') and settle_date='%s') and substr(a.trans_code,6,1) = '3' "
            "and a.resp_code='00' and a.istresp_code is not NULL and a.trans_date between '%s' and '%s' "
            "union select a.rrn,a.amount,b.account_uuid,a.trans_code,a.sys_trace, "
            "a.merch_id,a.channel_merch_id,a.trans_date,a.trans_time,a.amount-a.fee, a.settle_date  "
            "from b_pos_trans_detail_his a join S_TRANS_CODE t on t.TRANS_CODE=a.TRANS_CODE and t.settle_flag='1' "
            "left join b_merch_account b on a.merch_id=b.merch_id  where a.rrn in(select rrn from "
            "b_spdb_trans_detail where chk_flag in ('Y','B') and settle_date='%s') and a.trans_date between '%s' and '%s' ", sTransDate, sBeforeDate, sTransDate, sTransDate, sBeforeDate, sTransDate, sTransDate, sBeforeDate, sTransDate );
    tLog(DEBUG, "[%s]", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }


    while (OCI_FetchNext(pstRes)) {
        memset(sRrn, 0, sizeof (sRrn));
        memset(sAccountUUID, 0, sizeof (sAccountUUID));
        memset(sTransCode, 0, sizeof (sTransCode));
        memset(sTransType, 0, sizeof (sTransType));
        memset(sSysTrace, 0, sizeof (sSysTrace));
        memset(sMerchId, 0, sizeof (sMerchId));
        memset(sChannelMerchId, 0, sizeof (sChannelMerchId));
        memset(sTransDate, 0, sizeof (sTransDate));
        memset(sTransTime, 0, sizeof (sTransTime));
        //memset(sSettleDate, 0, sizeof (sSettleDate));

        STRV(pstRes, 1, sRrn);
        DOUV(pstRes, 2, dAmount);
        STRV(pstRes, 3, sAccountUUID);
        STRV(pstRes, 4, sTransCode);
        STRV(pstRes, 5, sSysTrace);
        STRV(pstRes, 6, sMerchId);
        STRV(pstRes, 7, sChannelMerchId);
        STRV(pstRes, 8, sTransDate);
        STRV(pstRes, 9, sTransTime);
        DOUV(pstRes, 10, dSettAmount);
        //STRV(pstRes, 11, sSettleDate);

        tTrim(sRrn);
        tTrim(sAccountUUID);
        tTrim(sTransCode);
        tTrim(sSysTrace);
        tTrim(sMerchId);
        tTrim(sChannelMerchId);
        tTrim(sTransDate);
        tTrim(sTransTime);
        //tTrim(sSettleDate);

        strcpy(sTransmitTime, sTransDate);
        strcat(sTransmitTime, sTransTime);
        GetOMsgType(sTransType, sTransCode);
        sprintf(sBuf, "%s,%0.2lf,%0.2lf,%s,%s,%s,%s,%s,%s,%s\n", sRrn, dAmount, dSettAmount, sAccountUUID, sTransType, sSysTrace, sMerchId, sChannelMerchId, sTransmitTime, sSettleDate);
        fwrite(sBuf, strlen(sBuf), 1, fp);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    memcpy(pcSettleDate, sSettleDate, 8);
    tLog(INFO, "�����ļ������ɹ�.");

    return 0;
}

int genfile(char *pcTransDate) {
    FILE *fp;
    char sFileName[100] = {0}, sPath[100] = {0};
    char sTransDate[8 + 1] = {0};
    char sSettleDate[8 + 1] = {0};
    char sCommand[100 + 1] = {0};
    if (pcTransDate != NULL) {
        tStrCpy(sTransDate, pcTransDate, 8);
    } else {
        tGetDate(sTransDate, "", -1);
        tAddDay(sTransDate, -1);
    }
    //�����ļ�Ŀ¼������
    strcat(sPath, "/data_posp/");
    memcpy(sPath + 11, sTransDate, 4);
    strcat(sPath, "/");
    strcat(sPath, sTransDate);
    strcat(sPath, "/");

    sprintf(sCommand, "mkdir -p %s", sPath);
    system(sCommand);

    fp = fopen("/data_posp/tmp", "w+"); // ���ı���ʽ�򿪡�                                                               
    if (fp == NULL) {
        return -1; // ���ļ�ʧ�ܡ�
    }

    if (writefilehead(fp, sTransDate) < 0) {
        tLog(INFO, "�����ļ�ͷ����ʧ��.");
        fclose(fp); //�ر��ļ���
        return -1;
    }

    if (writefilebody(fp, sTransDate, sSettleDate) < 0) {
        tLog(INFO, "�����ļ��嵼��ʧ��.");
        fclose(fp); //�ر��ļ���
        return -1;
    }
    fclose(fp); //�ر��ļ���       

    /*�����ļ���*/
    strcat(sFileName, "pos-");
    strcat(sFileName, sTransDate);
    strcat(sFileName, "-");
    strcat(sFileName, sSettleDate);
    strcat(sFileName, ".csv");

    strcat(sPath, sFileName);
    puts(sPath);
    sprintf(sCommand, "mv /data_posp/tmp %s", sPath);
    system(sCommand);
    tLog(DEBUG, "�����ļ����ɳɹ�");
} 