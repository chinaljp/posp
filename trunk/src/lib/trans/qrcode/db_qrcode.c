/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "t_cjson.h"

/*ֻ�����ڲ�ѯ����ʹ�ã����±�ɨ������ˮ�е�valid_flag*/
int UpValidflag(char * pstOrderNo) {
    char sOrderNo[30 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sOrderNo, pstOrderNo);
    tTrim(sOrderNo);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET VALID_FLAG='0' WHERE MERCH_ORDER_NO='%s' AND "
                          "TRANS_CODE NOT IN ('02B300','02W300','02Y300','0AY300','0AB300','0AW300')", sOrderNo);
    
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
        tLog(ERROR, "���¶�����[%s]�����־ʧ��",sOrderNo);
        return -1;
    }

    tReleaseRes(pstRes);
    return 0;
}

/*������ά���ѯ��� ֧���ɹ�ʱ ����settleKey�е���Ϣ��ԭ������ˮ��*/
int UpCupsSettleMessage(char * pcOrderNo, char *pcCardType, char * pcSettleSysTrace, char * pcSettleTransTime, char *pcChannelSettleDate,char * pcSettleDate) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    int iNum = 0;
    
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set card_type = '%s',settle_sys_trace = '%s',settle_trans_time = '%s',channel_settle_date = '%s',settle_date = '%s'  "
                    "where merch_order_no = '%s' and trans_code not in ('02Y300','0AY300') and "
            "not exists (select 1 from b_inline_tarns_detail where settle_sys_trace = '%s' and settle_trans_time = '%s')",
            pcCardType,pcSettleSysTrace,pcSettleTransTime,pcChannelSettleDate,pcSettleDate,pcOrderNo,pcSettleSysTrace,pcSettleTransTime );
        
    if (tExecute(&pstRes, sSqlStr) < 0 ) {
        tLog(ERROR,"sSqlStr = [%s]",sSqlStr);
        tLog(ERROR, "���¶���[%s]�� settle_sys_trace��settle_trans_time ʧ��.",pcOrderNo);
        return -1;
    }
    iNum = tGetAffectedRows();
    tLog(INFO,"������[%d]��,%s,�̻�������[%s]",iNum,
            iNum == 0? "�첽֪ͨ�п����Ѹ��£������ν����쳣���齻����ˮ��¼":"���³ɹ�",pcOrderNo);//���첽֪ͨ�������Ѿ�������˴�����0��
    tReleaseRes(pstRes);
    return ( 0 );
}

int UpCupsSettleKey(char * pcOrderNo, char * pcSettleSysTrace, char * pcSettleTransTime) {
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    int iNum = 0;
    
    snprintf( sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set settle_sys_trace = '%s',settle_trans_time = '%s' "
                    "where merch_order_no = '%s' and trans_code not in ('02Y300','0AY300') and "
            "not exists (select 1 from b_inline_tarns_detail where settle_sys_trace = '%s' and settle_trans_time = '%s')",
            pcSettleSysTrace,pcSettleTransTime,pcOrderNo,pcSettleSysTrace,pcSettleTransTime );
        
    if (tExecute(&pstRes, sSqlStr) < 0 ) {
        tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
        tLog(ERROR, "���¶���[%s]�� settle_sys_trace��settle_trans_time ʧ��.",pcOrderNo);
        return -1;
    }
    iNum = tGetAffectedRows();
    tLog(INFO,"������[%d]��,%s,�̻�������[%s]",iNum,
            iNum == 0? "�첽֪ͨ�п����Ѹ��£������ν����쳣���齻����ˮ��¼":"���³ɹ�",pcOrderNo);//���첽֪ͨ�������Ѿ�������˴�����0��
    tReleaseRes(pstRes);
    return ( 0 );
}

/*��ά����ɨ���� ���½�����ˮ�е�valid_flag, ��ά�볷�����ף����¶�ά�볷��������ˮ�е�valid_flag  ע����¶�ά����ɨ������ˮ�е�valid_flag  ����ʹ�� ����20180129 ***/
int UpValidflagInLine(char * pcSysTrace,char * pcTransDate) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    tTrim(pcSysTrace);
    tTrim(pcTransDate);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET VALID_FLAG='0' WHERE SYS_TRACE='%s' AND TRANS_DATE = '%s' AND \
                          TRANS_CODE IN ('02W100','02B100','0AW100','0AB100','02W600','02B600','0AW600','0AB600','02Y600')",pcSysTrace,pcTransDate);
    
    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "������ˮ[%s]��Ч��־ʧ��,trans_date= [%s]",pcSysTrace,pcTransDate);
        return -1;
    }

    tReleaseRes(pstRes);

    return 0;
}

/*��ѯ�����̻�appkey*/
int FindMerchApk(char *pcChannelMerchId, char *pcAppKey) {
    OCI_Resultset *pstRes = NULL;
    char sSqlStr[1024];
    
    tTrim(pcChannelMerchId);
    snprintf(sSqlStr,sizeof(sSqlStr),"select app_key \
                            from b_merch_send_detail where chnmerch_id = '%s'",pcChannelMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcAppKey);
        
        tTrim(pcAppKey);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    
    return ( 0 );
}

/* ����valid_flag��־ */
int UpdTransDetail(char *pcTable, char *pcDate, char *pcRrn, char *pcValidFlag) {
    char sSqlStr[1024];
    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE %s SET "
            " VALID_FLAG= '%s' "
            " WHERE trans_date = '%s'"
            " AND rrn = '%s' "
            , pcTable, pcValidFlag, pcDate, pcRrn);

    if (tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0) {
        tLog(ERROR, "����ԭ����[%s:%s]valid_flag[%s]ʧ��.", pcDate, pcRrn, pcValidFlag);
        return -1;
    }
    tReleaseRes(pstRes);
    tLog(INFO, "����ԭ����[%s:%s]valid_flag[%s]�ɹ�[%d].", pcDate, pcRrn, pcValidFlag, tGetAffectedRows(pstRes));
    return 0;
}

int FindOldOrderNo(char * pstOrrn, char * pstOldOrderNo) {

    char sOrderNo[30 + 1] = {0}, sORrn[12 + 1] = {0};
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sORrn, pstOrrn);
    tTrim(sORrn);
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT ORDER_NO FROM B_INLINE_TARNS_DETAIL WHERE RRN='%s'", sORrn);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "����ԭ����RRN=[%s]������ʧ��", sORrn);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sOrderNo);
    }
    strcpy(pstOldOrderNo, sOrderNo);
    tReleaseRes(pstRes);
    return 0;
}

/*�����������ڡ��̻������� add by gjq at 20171212*/
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

/*���¼����� �����ѣ�΢�š�֧���� ����ɨ����ʹ�ã�*/
int UpFeeMsg(char *pcOrderNo,char *pcFeeType,double dFee,char *pcFeeDesc) {
    char sSqlStr[1024]      = {0};
    OCI_Resultset *pstRes   = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set fee_type = '%s',fee = %.02f,fee_desc = '%s',fee_flag = 'M' "
                    "where merch_order_no = '%s' and trans_code not in ('02B300','02W300','0AB300','0AW300')",
            pcFeeType,dFee,pcFeeDesc,pcOrderNo);

    if ( tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0 ) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tReleaseRes(pstRes);
        return ( -1 );
    }
    tLog(DEBUG,"������[%d]��",tGetAffectedRows());
    tReleaseRes(pstRes);
    return ( 0 );
}
