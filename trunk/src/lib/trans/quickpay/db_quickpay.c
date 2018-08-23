/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"

/*�����������ڡ��̻������� add by gjq at 20180706*/
int UpMerchOrderNo(char *pcDate,char *pcMerchOrderNo,char *pcMerchId,char *pcRrn,char *pcTransCode) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set settle_date = '%s',merch_order_no='%s' "
                    "where merch_id = '%s' and trans_code = '%s' and rrn = '%s'",pcDate,pcMerchOrderNo,pcMerchId,pcTransCode,pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tLog(ERROR, "�����̻�[%s]�Ķ�����[%s]ʧ��.",pcMerchId,pcMerchOrderNo);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return ( 0 );
}
/*���������־*/
int UpValidflag(char * psORrn) {
    char sSqlStr[512] = {0};
    int iResCnt = 0;

    tTrim(psORrn);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET VALID_FLAG='0' WHERE rrn = '%s' ", psORrn);
    
    tLog(DEBUG,"sSqlStr[%s]",sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tLog(DEBUG, "Ӱ���¼��[%d]", iResCnt);
    return 0;
}
/*����޿�����֧��ʧ�ܺ� ����֧���������;  
 * notice_flag = 1 ֪ͨ������APP��
 * ��ʱԭ���׵�֧����� �Ѿ�ͨ�����εķ��������첽֪ͨ���µ���ԭ������ˮ�У� ��ʱ����Ҫ�˸��� 
 * ����δ���˵�ԭ���� ��Ӧ����Ϣ
 */
int UpRespDesc( char *pcRespCode, char *pcRespDesc, char *pcRrn ) {
    char sSqlStr[1024] = {0};
    int iResCnt = 0;
    
    if ( !memcmp(pcRespCode,"00",2) ) {
        snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set resp_code = '%s', istresp_code = '%s', resp_desc ='%s', last_mod_time = sysdate "
                    "where settle_flag != 'M' and  rrn = '%s' and notice_flag != '1' and resp_code != '00'",pcRespCode,pcRespCode,pcRespDesc,pcRrn);
    } 
    else {
        snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set resp_code = '%s', istresp_code = '%s', resp_desc ='%s', last_mod_time = sysdate "
                    "where settle_flag != 'M' and  rrn = '%s' and notice_flag != '1'",pcRespCode,pcRespCode,pcRespDesc,pcRrn);
    }
    
    tLog(DEBUG,"sql = [%s]",sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    
    tLog(DEBUG, "Ӱ���¼��[%d]", iResCnt);
    return ( 0 );
}

/*���� ���ͱ�־ notice_flag = '2' ��APP ����֪ͨ */
int UpNoticeFlag( char *pcRrn ) {
    char sSqlStr[1024] = {0};
    int iResCnt = 0;
    /*ע�⣺ notice_flag = '1' ֪ͨ������APP, ������� notice_flag = '2' �ٴ���APP����֪ͨ */
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set notice_flag = '2' where rrn = '%s' and notice_flag != '1'", pcRrn );
    
    tLog(DEBUG,"sql = [%s]",sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    
    tLog(DEBUG, "Ӱ���¼��[%d]", iResCnt);
    return ( 0 );
}