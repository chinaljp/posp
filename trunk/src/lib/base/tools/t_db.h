/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t_oci.h
 * Author: feng.gaoo
 *
 * Created on 2016年12月14日, 下午8:29
 */

#ifndef T_OCI_H
#define T_OCI_H

#ifdef __cplusplus
extern "C" {
#endif
#include <ocilib.h>

#define INTV(a,b,c) c=OCI_GetInt(a,b)
#define STRV(a,b,c) if (NULL==OCI_GetString(a,b)){c[0]='\0';}else{strcpy(c,OCI_GetString(a,b));}
#define DOUV(a,b,c) c=OCI_GetDouble(a,b)

#define EXEC_SQL(a,b)   \
if (tExecute(&a, b) < 0) {   \
        tErrHandler(OCI_GetLastError()); \
        return -1;  \
    }   \
      if (NULL == a) {  \
        tErrHandler(OCI_GetLastError());   \
        return -1;  \
    }

    /*错误处理*/
    void tErrHandler(OCI_Error *pstErr);
    /*数据库连接初始化*/
    int tInitDb(char *pcTnsName, char *pcUserId, char *pcPwd);
    /*执行查询,返回结果集,一定要释放结果集tReleaseRes*/
    int tExecute(OCI_Resultset **ppstRes, char *pcSql);
    
    int tExecutePre(OCI_Resultset **ppstRes, char *pcSql, int i, int d, int s, ...);
    OCI_Statement* tExecutePreEx(OCI_Resultset **ppstRes, char *pcSql, int i, int d, int s, ...);
    
    /* 执行查询，不用释放,内部已经自动释放 */
    OCI_Statement* tExecuteEx(OCI_Resultset **ppstRes, char *pcSql);
    void tReleaseResEx(OCI_Statement *pstState);
    int tExecuteUpd(char *pcSql);
    /* 查询数量,返回计数 */
    int tQueueCnt(char *pcSql);
    /*查询dml语句作用的行数*/
    int tGetAffectedRows();
    //释放结果集
    void tReleaseRes(OCI_Resultset *pstRes);
    //释放数据库资源
    void tDoneDb();
    void tCommit();
    void tRollback();



#ifdef __cplusplus
}
#endif

#endif /* T_OCI_H */

