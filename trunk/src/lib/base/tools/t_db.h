/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t_oci.h
 * Author: feng.gaoo
 *
 * Created on 2016��12��14��, ����8:29
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

    /*������*/
    void tErrHandler(OCI_Error *pstErr);
    /*���ݿ����ӳ�ʼ��*/
    int tInitDb(char *pcTnsName, char *pcUserId, char *pcPwd);
    /*ִ�в�ѯ,���ؽ����,һ��Ҫ�ͷŽ����tReleaseRes*/
    int tExecute(OCI_Resultset **ppstRes, char *pcSql);
    
    int tExecutePre(OCI_Resultset **ppstRes, char *pcSql, int i, int d, int s, ...);
    OCI_Statement* tExecutePreEx(OCI_Resultset **ppstRes, char *pcSql, int i, int d, int s, ...);
    
    /* ִ�в�ѯ�������ͷ�,�ڲ��Ѿ��Զ��ͷ� */
    OCI_Statement* tExecuteEx(OCI_Resultset **ppstRes, char *pcSql);
    void tReleaseResEx(OCI_Statement *pstState);
    int tExecuteUpd(char *pcSql);
    /* ��ѯ����,���ؼ��� */
    int tQueueCnt(char *pcSql);
    /*��ѯdml������õ�����*/
    int tGetAffectedRows();
    //�ͷŽ����
    void tReleaseRes(OCI_Resultset *pstRes);
    //�ͷ����ݿ���Դ
    void tDoneDb();
    void tCommit();
    void tRollback();



#ifdef __cplusplus
}
#endif

#endif /* T_OCI_H */

