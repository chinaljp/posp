/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ocilib.h>
#include <stdarg.h>
#include "t_log.h"
#include "t_db.h"

static OCI_Connection* g_pstConn = NULL;
static OCI_Statement* g_pstState = NULL;

/*错误处理*/
void tErrHandler(OCI_Error *pstErr) {

    tLog(ERROR, "%s:%s[%s]"
            , OCI_ErrorGetType(pstErr) == OCI_ERR_WARNING ? "warning" : "error"
            //, OCI_ErrorGetOCICode(pstErr)
            , OCI_ErrorGetString(pstErr)
            , OCI_GetSql(OCI_ErrorGetStatement(pstErr)));
}

/*数据库连接初始化*/
int tInitDb(char *pcTnsName, char *pcUserId, char *pcPwd) {
    //初始化OCI库
    if (!OCI_Initialize(tErrHandler, NULL, OCI_ENV_DEFAULT | OCI_ENV_CONTEXT)) {
        tLog(FATAL, "OCI_Initialize failure!");
        tErrHandler(OCI_GetLastError());
        return -1;
    }
    //建立联接
    g_pstConn = OCI_ConnectionCreate(pcTnsName, pcUserId, pcPwd, OCI_SESSION_DEFAULT);
    if (NULL == g_pstConn) {
        tLog(FATAL, "OCI_ConnectionCreate failure!");
        tErrHandler(OCI_GetLastError());
        return -1;
    }
    //建立声明 
    g_pstState = OCI_StatementCreate(g_pstConn);
    if (NULL == g_pstState) {
        tLog(FATAL, "OCI_StatementCreate failure!");
        OCI_StatementFree(g_pstState);
        return -1;
    }
    return 0;
}

//执行查询,返回结果集,一定要释放结果集

int tExecute(OCI_Resultset **ppstRes, char *pcSql) {
    //tLog(DEBUG, "OCI_ExecuteStmt start![%p][%s]",g_pstState,pcSql );
    if (!OCI_ExecuteStmt(g_pstState, pcSql)) {
        //tErrHandler(OCI_GetLastError());
        return -1;
    }
    //tLog(DEBUG, "Sql Exec Succ!");
    //得到返回值集  
    *ppstRes = OCI_GetResultset(g_pstState);  
    return 0;
}

/*SQL 绑定变量 add by Guo Jia Qing*/
int tExecutePre(OCI_Resultset **ppstRes, char *pcSql, int i, int d, int s, ...) {
    
    int iCount = 0;
    va_list sArgv;//申请参数列表变量  
    va_start(sArgv, s);//设置开始变量为传过来的参数: s 
    //tLog(DEBUG, "OCI_ExecuteStmt start![%p][%s]",g_pstState,pcSql );
    OCI_Prepare(g_pstState,pcSql);
    /*绑定int类型*/
    for(iCount = 0; iCount < i; iCount++) {
        int iNum = 0;
        iNum = va_arg(sArgv, int);
        tLog(DEBUG,"iNum=[%d]",iNum);
        OCI_BindInt(g_pstState, va_arg(sArgv, char*), &iNum);
    }
    /*绑定double类型*/
    for(iCount = 0; iCount < d; iCount++) {
        double dNum = 0.00;
        dNum = va_arg(sArgv, double);
        tLog(DEBUG,"dNum=[%.02f]",dNum);
        OCI_BindDouble(g_pstState, va_arg(sArgv, char*), &dNum);
    }
    /*绑定字符串类型*/
    for(iCount = 0; iCount < s; iCount++) {
        char *sStr = NULL;
        sStr = va_arg(sArgv, char*);
        tLog(DEBUG,"sStr=[%s]",sStr);
        OCI_BindString(g_pstState, va_arg(sArgv, char*), sStr, strlen(sStr));
    }
  
    tLog(DEBUG,"pcSql[%s]",pcSql);
    if (!OCI_Execute(g_pstState)) {
        //tErrHandler(OCI_GetLastError());
        return -1;
    }

    //tLog(DEBUG, "Sql Exec Succ!");
    //得到返回值集  
    *ppstRes = OCI_GetResultset(g_pstState);
    
    va_end(sArgv);//“释放”sArgv  
    return 0;
}

OCI_Statement* tExecutePreEx(OCI_Resultset **ppstRes, char *pcSql, int i, int d, int s, ...) {
    int iCount = 0;
    va_list sArgv;//申请参数列表变量  
    va_start(sArgv, s);//设置开始变量为传过来的参数: s 
    
    //建立声明 
    OCI_Statement* pstState;
    //tLog(DEBUG, "OCI_Execute start![%p][%s]",g_pstConn,pcSql);
    pstState = OCI_StatementCreate(g_pstConn);
    if (NULL == pstState) {
        tLog(FATAL, "OCI_StatementCreate failure!");
        return NULL;
    }
    
    OCI_Prepare(pstState,pcSql);
    /*绑定int类型*/
    for(iCount = 0; iCount < i; iCount++) {
        int iNum = 0;
        iNum = va_arg(sArgv, int);
        tLog(DEBUG,"iNum=[%d]",iNum);
        OCI_BindInt(pstState, va_arg(sArgv, char*), &iNum);
    }
    /*绑定double类型*/
    for(iCount = 0; iCount < d; iCount++) {
        double dNum = 0.00;
        dNum = va_arg(sArgv, double);
        tLog(DEBUG,"dNum=[%.02f]",dNum);
        OCI_BindDouble(pstState, va_arg(sArgv, char*), &dNum);
    }
    /*绑定字符串类型*/
    for(iCount = 0; iCount < s; iCount++) {
        char *sStr = NULL;
        sStr = va_arg(sArgv, char*);
        tLog(DEBUG,"sStr=[%s]",sStr);
        OCI_BindString(pstState, va_arg(sArgv, char*), sStr, strlen(sStr));
    }
  
    tLog(DEBUG,"pcSql[%s]",pcSql);
    if (!OCI_Execute(pstState)) {
        OCI_ReleaseResultsets(pstState);
        return NULL;
    }
    
    //得到返回值集  
    *ppstRes = OCI_GetResultset(pstState);
    va_end(sArgv);//“释放”sArgv 
    
    return pstState;
}



OCI_Statement* tExecuteEx(OCI_Resultset **ppstRes, char *pcSql) {
    //建立声明 
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
    //得到返回值集  
    *ppstRes = OCI_GetResultset(pstState);
    //OCI_ReleaseResultsets(pstState);
    return pstState;
}

void tReleaseResEx(OCI_Statement *pstState) {
    if (pstState) {
        OCI_ReleaseResultsets(pstState);
    }
}

/* 执行更新的操作 */
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
    //tLog(DEBUG, "更新[%d]条记录.", iNum);
    OCI_ReleaseResultsets(pstUpdStat);
    OCI_StatementFree(pstUpdStat);
    return iNum;
}

/* update执行的条数 */
int tGetAffectedRows() {
    return OCI_GetAffectedRows(g_pstState);
}

int tQueueCnt(char *pcSql) {
    OCI_Resultset *pstRes = NULL;
    int iCnt = 0;
    if (!OCI_ExecuteStmt(g_pstState, pcSql)) {
        return -1;
    }
    pstRes = OCI_GetResultset(g_pstState);
    if (OCI_FetchNext(pstRes)) {
        INTV(pstRes, 1, iCnt);
    }
    tReleaseRes(pstRes);
    return iCnt;
}

//释放结果集

void tReleaseRes(OCI_Resultset *pstRes) {
    if (pstRes) {
        OCI_ReleaseResultsets(g_pstState);
    }
}

//释放数据库资源

void tDoneDb() {
    //清除声明  
    if (g_pstState) {
        OCI_StatementFree(g_pstState);
    }
    //清除连接  
    if (g_pstConn) {
        OCI_ConnectionFree(g_pstConn);
    }
    //清除OCI库  
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