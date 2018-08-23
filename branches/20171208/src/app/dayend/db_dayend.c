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
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_app_conf.h"
#include "t_log.h"
#include "dayend_trans_detail.h"
#include "t_db.h"

char *GetLogName();
int GetSysParam(char *pcLogicDate) {

    char sSqlStr[512] = {0}, sLogicDat[8 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key = 'LOGIC_DATE'");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sLogicDat);
    }
    tReleaseRes(pstRes);
    strcpy(pcLogicDate, sLogicDat);
    return 0;
}

int GetSysParamLock(char *pcLogicDate) {

    char sSqlStr[512] = {0}, sLogicDat[8 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key = 'LOGIC_DATE' for update");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sLogicDat);
    }

    strcpy(pcLogicDate, sLogicDat);
    tReleaseRes(pstRes);
    return 0;
}

int UpdLogicDate(char *pcLogicDate) {

    char sSqlStr[512] = {0};
    char sLogicDate[9];
    OCI_Resultset *pstRes = NULL;
    strcpy(sLogicDate, pcLogicDate);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE s_param  SET KEY_VALUE='%s' WHERE KEY ='LOGIC_DATE'", sLogicDate);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    return 0;
}

int CutOff(char *pcLogicDate) {

    if (GetSysParamLock(pcLogicDate) != 0) {
        tLog(ERROR, "��ȡϵͳ��Ϣ����.");
        return ( -1);
    }

    tAddDay(pcLogicDate, 1);
    if (UpdLogicDate(pcLogicDate) != 0) {
        tLog(ERROR, "����ϵͳ�߼����ڳ���.");
        return ( -1);
    }

    return ( 0);
}

int AcctPosJonlProc(char *pcLogicDate) {
    char sLogicDate[12 + 1] = {0};
    char sSqlStr[512] = {0}, sSqlStr1[512] = {0};
    OCI_Resultset *pstRes = NULL;
    tLog(INFO, "LogicDate[%s]", pcLogicDate);
    strcpy(sLogicDate, pcLogicDate);
    snprintf(sSqlStr, sizeof (sSqlStr), "insert into b_pos_trans_detail_his  select * from b_pos_trans_detail \
                where logic_date < '%s'", sLogicDate);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    snprintf(sSqlStr, sizeof (sSqlStr), "delete from b_pos_trans_detail where logic_date < '%s'", sLogicDate);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "����POS������ˮת��ʷ��ˮ���.");
    return 0;
}

int AcctLineJonlProc(char *pcLogicDate) {
    char sLogicDate[12 + 1] = {0};
    char sSqlStr[512] = {0}, sSqlStr1[512] = {0};
    OCI_Resultset *pstRes = NULL;
    strcpy(sLogicDate, pcLogicDate);
    snprintf(sSqlStr, sizeof (sSqlStr), "insert into B_Inline_Tarns_Detail_his  select * from B_Inline_Tarns_Detail \
      where logic_date < '%s'", sLogicDate);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }

    snprintf(sSqlStr, sizeof (sSqlStr), "delete from B_Inline_Tarns_Detail where logic_date < '%s'", sLogicDate);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        tRollback();
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    tLog(INFO, "�������Ͻ�����ˮת��ʷ��ˮ���.");
    return 0;
}

int ClearProc(char *pcLogicDate) {

    /* ����POS��ˮת��ʷ��ˮ */
    if (AcctPosJonlProc(pcLogicDate) < 0) {
        tLog(ERROR, "����POS������ˮת��ʷ��ˮʧ��.");
        tRollback();
        return -1;
    }

    /* ������ˮת��ʷ��ˮ */
    if (AcctLineJonlProc(pcLogicDate) < 0) {
        tLog(ERROR, "������ˮת��ʷ��ˮʧ��.");
        tRollback();
        return -1;
    }
    return 0;
}

int ChkLimitAmt() {
    char sSqlStr[512] = {0};
    char sCurDate[9] = {0};
    OCI_Resultset *pstRes = NULL;

    tGetDate(sCurDate, "", -1);
    /* ����Ƿ���ĩ */
    if (tIsMonthEnd(sCurDate)) {
        tLog(ERROR, "��ǰ���ڲ�����ĩ.", sCurDate);
        snprintf(sSqlStr, sizeof (sSqlStr), "update B_MERCH_LIMIT set "
                "D_AMT=0,C_D_AMT=0,D_WX_AMT=0,D_BB_AMT=0");
        if (tExecute(&pstRes, sSqlStr) < 0) {
            tLog(ERROR, "96", "�̻����ۼƶ���0ʧ��.");
            return -1;
        }
        if (0 == tGetAffectedRows) {
            tReleaseRes(pstRes);
            return -1;
        }
        tCommit();
        tReleaseRes(pstRes);

    } else {

        tLog(ERROR, "��ǰ��������ĩ,���ۼƶ���0", sCurDate);
        MEMSET(sSqlStr);
        pstRes = NULL;
        snprintf(sSqlStr, sizeof (sSqlStr), "update B_MERCH_LIMIT set M_BB_AMT=0"
                ",D_AMT=0,D_WX_AMT=0,D_BB_AMT=0,M_AMT=0,M_WX_AMT=0,C_D_AMT=0,C_M_AMT=0");
        if (tExecute(&pstRes, sSqlStr) < 0) {
            tLog(ERROR, "96", "�̻����ۼƶ���0ʧ��.");
            return -1;
        }
        if (0 == tGetAffectedRows) {
            tReleaseRes(pstRes);
            return -1;
        }
        tCommit();
        tReleaseRes(pstRes);
    }
    return 0;
}

int RecoveryLimit(){
   char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_MERCH_AUTH_LIMIT  SET USABLE_LIMIT = TOTAL_LIMIT,USED_LIMIT= 0");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    return 0;
}

int RePufaLimit(){
   char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_CHANNEL_MERCH  SET USABLE_LIMIT = TOTAL_LIMIT,USED_LIMIT= 0");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    return 0;
}

int RePufaLocalMerchLimit(){
   char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_CHANNEL_MERCH_CONV  SET  USED_LIMIT= 0");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    return 0;
}