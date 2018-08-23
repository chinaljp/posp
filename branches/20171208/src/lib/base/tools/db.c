/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ocilib.h>
#include "t_log.h"
#include "t_db.h"

static OCI_Connection* g_pstConn = NULL;
static OCI_Statement* g_pstState = NULL;

/*������*/
void tErrHandler(OCI_Error *pstErr) {

    tLog(ERROR, "%s:%s[%s]"
            , OCI_ErrorGetType(pstErr) == OCI_ERR_WARNING ? "warning" : "error"
            //, OCI_ErrorGetOCICode(pstErr)
            , OCI_ErrorGetString(pstErr)
            , OCI_GetSql(OCI_ErrorGetStatement(pstErr)));
}

/*���ݿ����ӳ�ʼ��*/
int tInitDb(char *pcTnsName, char *pcUserId, char *pcPwd) {
    //��ʼ��OCI��
    if (!OCI_Initialize(tErrHandler, NULL, OCI_ENV_DEFAULT | OCI_ENV_CONTEXT)) {
        tLog(FATAL, "OCI_Initialize failure!");
        tErrHandler(OCI_GetLastError());
        return -1;
    }
    //��������
    g_pstConn = OCI_ConnectionCreate(pcTnsName, pcUserId, pcPwd, OCI_SESSION_DEFAULT);
    if (NULL == g_pstConn) {
        tLog(FATAL, "OCI_ConnectionCreate failure!");
        tErrHandler(OCI_GetLastError());
        return -1;
    }
    //�������� 
    g_pstState = OCI_StatementCreate(g_pstConn);
    if (NULL == g_pstState) {
        tLog(FATAL, "OCI_StatementCreate failure!");
        OCI_StatementFree(g_pstState);
        return -1;
    }
    return 0;
}

//ִ�в�ѯ,���ؽ����,һ��Ҫ�ͷŽ����

int tExecute(OCI_Resultset **ppstRes, char *pcSql) {
    //tLog(DEBUG, "OCI_ExecuteStmt start![%p][%s]",g_pstState,pcSql );
    if (!OCI_ExecuteStmt(g_pstState, pcSql)) {
        //tErrHandler(OCI_GetLastError());
        return -1;
    }
    //tLog(DEBUG, "Sql Exec Succ!");
    //�õ�����ֵ��  
    *ppstRes = OCI_GetResultset(g_pstState);
    return 0;
}

OCI_Statement* tExecuteEx(OCI_Resultset **ppstRes, char *pcSql) {
    //�������� 
    OCI_Statement* pstState;
    pstState = OCI_StatementCreate(g_pstConn);
    if (NULL == pstState) {
        tLog(FATAL, "OCI_StatementCreate failure!");
        return NULL;
    }
    if (!OCI_ExecuteStmt(pstState, pcSql)) {
        OCI_ReleaseResultsets(pstState);
        return NULL;
    }
    //�õ�����ֵ��  
    *ppstRes = OCI_GetResultset(pstState);
    //OCI_ReleaseResultsets(pstState);
    return pstState;
}

void tReleaseResEx(OCI_Statement *pstState) {
    if (pstState) {
        OCI_ReleaseResultsets(pstState);
    }
}

/* ִ�и��µĲ��� */
int tExecuteUpd(char *pcSql) {
    int iNum = 0;
    OCI_Resultset *pstUpdRes = NULL;
    OCI_Statement *pstUpdStat = NULL;

    pstUpdStat = tExecuteEx(&pstUpdRes, pcSql);
    if (NULL == pstUpdStat) {
        tLog(DEBUG, "sql[%s]err.", pcSql);
        return -1;
    }
    iNum = OCI_GetAffectedRows(pstUpdStat);
    //tLog(DEBUG, "����[%d]����¼.", iNum);
    OCI_ReleaseResultsets(pstUpdStat);
    OCI_StatementFree(pstUpdStat);
    return iNum;
}

/* updateִ�е����� */
int tGetAffectedRows() {
    return OCI_GetAffectedRows(g_pstState);
}

int tQueryCount(char *pcSqlStr) {
    int iCounts;
    char *pcSql;
    pcSql = pcSqlStr;
    OCI_Resultset *pstRes = NULL;

    if (tExecute(&pstRes, pcSql) < 0) {
        tLog(ERROR, "sql[%s] err!", pcSql);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", pcSql);
        tReleaseRes(pstRes);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iCounts);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        iCounts = 0;
    }
    tReleaseRes(pstRes);
    return ( iCounts);
}

//�ͷŽ����

void tReleaseRes(OCI_Resultset *pstRes) {
    if (pstRes) {
        OCI_ReleaseResultsets(g_pstState);
    }
}

//�ͷ����ݿ���Դ

void tDoneDb() {
    //�������  
    if (g_pstState) {
        OCI_StatementFree(g_pstState);
    }
    //�������  
    if (g_pstConn) {
        OCI_ConnectionFree(g_pstConn);
    }
    //���OCI��  
    OCI_Cleanup();
}

void tCommit() {
    if (g_pstConn) {
        tLog(DEBUG, "commit.");
        OCI_Commit(g_pstConn);
    }
}

void tRollback() {
    if (g_pstConn) {
        tLog(DEBUG, "rollback.");
        OCI_Rollback(g_pstConn);
    }
}