/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"

/* ��ɨģʽ */
int QrcodeActive(cJSON *pstTranData, int *piFlag) {
    /*   ɨ�踶�����ά���ȡ�����û�����ַ���  */

    /*  ��ɨģʽ�������������Ψһֵ��pos�޸� ��Ҫ�ܴ������������ʹ����ݴ�
    char sOrder_no[31] = {0};
    char sMerchId[16] = {0}, sRrn[13] = {0};
    cJSON * pstTransJson;
    

    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);

    sprintf(sOrder_no, "%s%s", sRrn, sMerchId);
    tLog(DEBUG, "qr_order_no[%s]", sOrder_no);
    SET_STR_KEY(pstTransJson, "qr_order_no", sOrder_no);
     */
    cJSON * pstTransJson;
    char sSeed[30] = {0};
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "seed", sSeed);
    SET_STR_KEY(pstTransJson, "qr_order_no", sSeed);
    return 0;
}


/*��ɨģʽ*/

/*��ά���µ�����*/
int QrcodeOrder(cJSON *pstTranData, int *piFlag) {

    char sOrder_no[32+1] = {0},sTransDate[8+1] = {0}, sTransCode[6 + 1] = {0};
    char sMerchId[16] = {0}, sRrn[13] = {0}, sDate[8 + 1] = {0};
    cJSON * pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "settle_date", sDate);

    sprintf(sOrder_no, "%s%s%s", sTransDate+2,sRrn+6, sMerchId);
    tLog(DEBUG, "qr_order_no[%s]", sOrder_no);
    /*������ ���������н������Ÿ��µ����ν��׵���ˮ��¼��(��ά����ɨ ��ɨ,����)*/
    if( UpMerchOrderNo(sDate,sOrder_no,sMerchId,sRrn,sTransCode) < 0 ) {
        ErrHanding(pstTransJson, "96", "�����̻�[%s]������[%s]ʧ��.", sMerchId,sOrder_no);
        return ( -1 );
    }
    
    SET_STR_KEY(pstTransJson, "qr_order_no", sOrder_no);
    return 0;
}

/*��ά���ѯ���׷��ظ�ֵ*/
void QrcodeInquiry(cJSON *pstTranData, char *pstRespCode) {
    //  char sRespCode[2+1]={0};
    char sResult_code[3] = {0}, sTrade_state[3] = {0};
    cJSON * pstTransJson = NULL, *pstRepJson = NULL;

    pstRepJson = GET_JSON_KEY(pstTranData, "response");

    GET_STR_KEY(pstRepJson, "result_code", sResult_code);
   // GET_STR_KEY(pstRepJson, "trade_state", sTrade_state);

    if (sResult_code[0] == '0') {
        strcpy(pstRespCode, "00");
    } else if (sResult_code[0] == '1') {
        strcpy(pstRespCode, "R9");
    }
    /* ����״̬   */
    if (!strcmp(sResult_code, "Q1") ||
            !strcmp(sResult_code, "Q2") ||
            !strcmp(sResult_code, "Q3") ||
            !strcmp(sResult_code, "Q4") ||
            !strcmp(sResult_code, "Q5") ||
            !strcmp(sResult_code, "Q6") ||
            !strcmp(sResult_code, "Q7") ||
            !strcmp(sResult_code, "Q8") ||
            !strcmp(sResult_code, "Q9") ||
            !strcmp(sResult_code, "R0") ||
            !strcmp(sResult_code, "R1") ||
            !strcmp(sResult_code, "R2") ||
            !strcmp(sResult_code, "R3") ||
            !strcmp(sResult_code, "R4") ||
            !strcmp(sResult_code, "R5") ||
            !strcmp(sResult_code, "R6") ||
            !strcmp(sResult_code, "R7") ||
            !strcmp(sResult_code, "R8")
            ) {
        strcpy(pstRespCode, sResult_code);

    }
}

/* �µ����׷��� */
int RespInfo(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sRespCode[2 + 1] = {0}, sCodeUrl[100] = {0};
    
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_STR_KEY(pstRepJson, "result_code", sRespCode);

    // GET_STR_KEY(pstRepJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRepJson, "code_url", sCodeUrl);

    // SET_STR_KEY(pstDataJson,"resp_code",sRespCode);
    SET_STR_KEY(pstDataJson, "code_url", sCodeUrl);
    ErrHanding(pstDataJson, sRespCode);
    return 0;
}


/****************************************************************************************/
/*      ������:     UpOrderValidProc()                  	                                */
/*      ����˵��:   ����ԭ���׽����ʶ						        */
/*      �������:   TranData *pstTranData                                                */
/*      �������:                                                                        */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                                   */
/*ֻ���ò�ѯ����ʹ�ã����±�ɨ������ˮ�е�valid_flag                                        */
/*****************************************************************************************/
int UpOrderValidProc(cJSON *pstJson, int *piFlag) {
    int iRet = 0;
    char sTransCode[6 + 1] = {0}, sRespCode[2 + 1] = {0}, sQrOrderNo[30 + 1] = {0};
    char sCardType[1 + 1]         = {0};
    char sCardAttr[2 + 1]         = {0}; //������ά�뿨���� 01 - ��ǿ� 02 - ���ǿ�
    char sSettleKey[38 + 1]       = {0}; //��������
    char sBuf[16 + 1]             = {0};
    char sSettleSysTrace[6 + 1]   = {0}; //������ά��ϵͳ���ٺ�
    char sSettleTransTime[10 + 1] = {0}; //������ά�뽻�״���ʱ��
    char sChannelSettleDate[4+1]  = {0}; //������������
    char sSettleDate[8 + 1]={0}; //ϵͳ��������
    
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "qr_order_no", sQrOrderNo);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    
    tLog(DEBUG, "TRANS_CODE = [%s],RESP_CODE=[%s]", sTransCode,sRespCode);
    tLog(DEBUG, "ORDER_NO=[%s]", sQrOrderNo);
    if ( !memcmp(sRespCode, "00", 2) ) {

        iRet = UpValidflag(sQrOrderNo);
        if (iRet < 0) {
            tLog(ERROR, "���¶�ά��valid_flag��־ʧ�ܣ�������[%s]", sQrOrderNo);
            return -1;
        }
        tLog(INFO, "���¶�����[%s]�������־�ɹ�.", sQrOrderNo);
        
        /*������ά���ѯ֧����� Ϊ ֧���ɹ� ��Ҫ����ԭ������ˮ*/
        if(sTransCode[2] == 'Y') {
            GET_STR_KEY(pstDataJson, "settleKey", sSettleKey);
            GET_STR_KEY(pstDataJson, "cardAttr", sCardAttr);
            GET_STR_KEY(pstDataJson, "ChannelSettleDate",sChannelSettleDate);
            if( sCardAttr[1] == '1' ) {
                strcpy(sCardType,"0");
            } 
            else if( sCardAttr[1] == '2'  ) {
                strcpy(sCardType,"1");
            }
            memcpy(sBuf, sSettleKey+22, sizeof(sBuf)); //��ȡϵͳ���ٺ� ���״���ʱ��
            memcpy(sSettleSysTrace,sBuf,6);            //��ȡϵͳ���ٺ�
            sSettleSysTrace[7] = '\0';
            memcpy(sSettleTransTime,sBuf+strlen(sSettleSysTrace),10);//��ȡ���״���ʱ��
            tLog(INFO,"sSettleKey[%s],sSettleSysTrace[%s],sSettleTransTime[%s]",sSettleKey,sSettleSysTrace,sSettleTransTime);
            tLog(INFO,"sTransCode = [%s],��Ҫ�������������е�ϵͳ���ٺš����״���ʱ��",sTransCode);
            
            /*��ȡsettle_date(����)*/
            GetSysSettleDate(sSettleDate,sChannelSettleDate);
                      
            iRet = UpCupsSettleMessage(sQrOrderNo,sCardType,sSettleSysTrace,sSettleTransTime,sChannelSettleDate,sSettleDate);
            if (iRet < 0) {
                tLog(ERROR, "����������ά��������������ʧ�ܣ�������[%s]", sQrOrderNo);
                return -1;
            }
            tLog(INFO, "���¶�����[%s]�������������ݳɹ�.", sQrOrderNo); 
        } 
           
    } else {
        tLog(ERROR,"RESP_CODE=[%s],����[%s]����ʧ��", sRespCode,sQrOrderNo);
        ErrHanding(pstDataJson, sRespCode);
        return ( -1 );
    }
    return 0;
}

/****************************************************************************************/
/*      ������:     UpdSettleKey()                  	                                */
/*      ����˵��:   ����ԭ���׽����ʶ						        */
/*      �������:   TranData *pstTranData                                                */
/*      �������:                                                                        */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                                   */
/*****************************************************************************************/
int UpdSettleKey(cJSON *pstJson, int *piFlag) {
    int iRet = 0;
    char sTransCode[6 + 1] = {0}, sRespCode[2 + 1] = {0}, sQrOrderNo[30 + 1] = {0};
    char sCardType[1 + 1]         = {0};
    char sCardAttr[2 + 1]         = {0}; //������ά�뿨���� 01 - ��ǿ� 02 - ���ǿ�
    char sSettleKey[38 + 1]       = {0}; //��������
    char sBuf[16 + 1]             = {0};
    char sSettleSysTrace[6 + 1]   = {0}; //������ά��ϵͳ���ٺ�
    char sSettleTransTime[10 + 1] = {0}; //������ά�뽻�״���ʱ��
    
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "qr_order_no", sQrOrderNo);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    
    tLog(DEBUG, "TRANS_CODE = [%s],RESP_CODE=[%s]", sTransCode,sRespCode);
    tLog(DEBUG, "ORDER_NO=[%s]", sQrOrderNo);
    if ( !memcmp(sRespCode, "00", 2) ) {  
        /*������ά���ѯ֧����� Ϊ ֧���ɹ� ��Ҫ����ԭ������ˮ*/
        if(sTransCode[2] == 'Y') {
            GET_STR_KEY(pstDataJson, "settleKey", sSettleKey);
            memcpy(sBuf, sSettleKey+22, sizeof(sBuf)); //��ȡϵͳ���ٺ� ���״���ʱ��
            memcpy(sSettleSysTrace,sBuf,6);            //��ȡϵͳ���ٺ�
            sSettleSysTrace[7] = '\0';
            memcpy(sSettleTransTime,sBuf+strlen(sSettleSysTrace),10);//��ȡ���״���ʱ��
            tLog(INFO,"sSettleKey[%s],sSettleSysTrace[%s],sSettleTransTime[%s]",sSettleKey,sSettleSysTrace,sSettleTransTime);
            tLog(INFO,"sTransCode = [%s],��Ҫ�������������е�ϵͳ���ٺš����״���ʱ��",sTransCode);
            
            iRet = UpCupsSettleKey(sQrOrderNo,sSettleSysTrace,sSettleTransTime);
            if (iRet < 0) {
                tLog(ERROR, "����������ά��������������ʧ�ܣ�������[%s]", sQrOrderNo);
                return -1;
            }
            tLog(INFO, "���¶�����[%s]�������������ݳɹ�.", sQrOrderNo); 
        } 
           
    } else {
        tLog(ERROR,"RESP_CODE=[%s],����[%s]����ʧ��", sRespCode,sQrOrderNo);
        ErrHanding(pstDataJson, sRespCode);
        return ( -1 );
    }
    return 0;
}


/*********************************************************************************************/
/*      ������:     UpInLineValidProc()                  	                             */
/*      ����˵��:   ���½�����Ч��ʶ(���׳ɹ����и���)					     */
/*      �������:   TranData *pstTranData                                                     */
/*      �������:                                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                                        */
/* ��ά����ɨ���� ���½�����ˮ�е�valid_flag, ��ά�볷�����ף����¶�ά�볷��������ˮ�е�valid_flag */
/**********************************************************************************************/
int UpInLineValidProc(cJSON *pstJson, int *piFlag) {
    int iRet = 0;
    char sRespCode[2 + 1] = {0}, sTransDate[8 + 1] = {0}, sSysTrace[6 + 1] = {0};
    cJSON *pstDataJson = NULL;
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstDataJson, "sys_trace", sSysTrace);
    GET_STR_KEY(pstDataJson,"trans_date",sTransDate);

    tLog(INFO, "RESP_CODE=[%s]", sRespCode);
    tLog(INFO, "SYS_TRACE=[%s]", sSysTrace);
    tLog(INFO, "TRANS_DATE=[%s]", sTransDate);
    if (!memcmp(sRespCode, "00", 2)) {

        iRet = UpValidflagInLine(sSysTrace,sTransDate);
        if (iRet < 0) {
            tLog(ERROR, "���¶�ά��valid_flag��־ʧ�ܣ���������[%s]����ˮ��[%s]", sTransDate,sSysTrace);
            return -1;
        }

        tLog(INFO, "������ˮ[%s]����Ч��־�ɹ�,trans_date = [%s].", sSysTrace,sTransDate);

        return 0;
    } else {
        tLog(ERROR,"RESP_CODE=[%s],����ʧ��", sRespCode);
        ErrHanding(pstDataJson, sRespCode);
        return ( -1 );
    }

}

/*�������ѳ���ԭ����*/
int UpdQrTransDetail(cJSON *pstJson, int *piFlag) {
    
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "����ʧ��[%s],������ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "trans_date", sOTransDate);/*�˴�sOTransDate �������� �˿��ʹ��ʱ ע��ԭ��������*/
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_INLINE_TARNS_DETAIL", sOTransDate, sORrn, "1") < 0) {
        ErrHanding(pstTransJson, "96", "����ԭ���ѽ���[%s:%s]��Ч��־[1]ʧ��.", sOTransDate, sORrn);
        return -1;
    }
    tLog(DEBUG, "����ԭ���ѽ���[%s:%s]��Ч��־[1]�ɹ�.", sOTransDate, sORrn);
    
    return ( 0 );
}


/* ���׷��� */
int QueryReturnInfo(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sSettleKey[38 + 1] = {0}, sCardAttr[2 + 1] = {0}, sChannelSettleDate[4] = {0};
    char sResultCode[2 + 1] = {0}, sRespCode[2 + 1] = {0};
    /*char sCodeUrl[100] = {0}, sTransCode[6 + 1] = {0}, sTradeState[3 + 1] = {0};*/
    double dAmount = 0.0;
    
   //char sSettleKey[38 + 1] = {0};
    /* ת����ֵ������ */
    //QrcodeInquiry(pstJson, sRespCode);
    /*�����ص����ݸ�ֵ*/
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    pstRepJson = GET_JSON_KEY(pstJson, "response");
    GET_DOU_KEY(pstRepJson, "amount", dAmount);
    SET_DOU_KEY(pstDataJson, "amount", dAmount);
    
    GET_STR_KEY(pstRepJson, "settleKey", sSettleKey);
    SET_STR_KEY(pstDataJson, "settleKey", sSettleKey);
    
    GET_STR_KEY(pstRepJson, "cardAttr", sCardAttr);
    SET_STR_KEY(pstDataJson, "cardAttr", sCardAttr);
    
    GET_STR_KEY(pstRepJson, "ChannelSettleDate",sChannelSettleDate);
    SET_STR_KEY(pstDataJson, "ChannelSettleDate", sChannelSettleDate);
            
    GET_STR_KEY(pstRepJson, "result_code", sResultCode);
    strcpy(sRespCode,sResultCode);
    //GET_DOU_KEY(pstRepJson, "settleKey", sSettleKey);
    //SET_DOU_KEY(pstDataJson, "settleKey", sSettleKey);
    
#if 0
    /*���Ķ�ά�� ���׷����� ͣ�� */
    GET_STR_KEY(pstRepJson, "trans_code", sTransCode);
    if ( !memcmp(sTransCode,"02B300",6) || !memcmp(sTransCode,"02W300",6) || 
         !memcmp(sTransCode,"0AB300",6) || !memcmp(sTransCode,"0AW300",6) ) {
        GET_STR_KEY(pstRepJson, "trade_state", sTradeState);
        if( !memcmp(sTradeState,"100",3) ) {
            strcpy(sRespCode, "Q3");
        } else if ( !memcmp(sTradeState,"101",3) ) {
            strcpy(sRespCode, "R3");
        } else if ( !memcmp(sTradeState,"102",3) ) {
            strcpy(sRespCode, "Q7");
        } else if ( !memcmp(sTradeState,"103",3) ) {
            strcpy(sRespCode, "FD");/*����������,����δ���*/
        } else if ( !memcmp(sTradeState,"104",3) ) {
            strcpy(sRespCode, "R6");
        } else {
            strcpy(sRespCode, "96");
        }
    }
#endif
    ErrHanding(pstDataJson, sRespCode);
    return 0;
}

int EmptytransFlow(cJSON *pstJson, int *piFlag) {
    tLog(INFO, "�ս�����");

    return 0;
}

int RetFalse(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    tLog(INFO, "�ݲ�֧�ִ˽���");
    ErrHanding(pstTransJson, "FA", "�ݲ�֧�ִ˽���");
    return (-1);
}

int PosVersionCtrl(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sHead[6 + 1] = {0}, sVersion[2 + 1] = {0}, sTransCode[6 + 1] = {0};
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "head", sHead);
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    tStrCpy(sVersion, sHead + 2, 2);
    tLog(INFO,"����ͷ��%s pos�汾[%s]", sHead, sVersion);
    if ( (!memcmp(sTransCode,"02B",3) || !memcmp(sTransCode,"02W100",6) )&& memcmp(sVersion,"21",2) ){
          ErrHanding(pstDataJson, "12", "Pos�汾��[%s]��֧�ִ˽���[%s].", sVersion, sTransCode );
        return -1;
    }
    return 0;
}

int TmpSetSweepType(cJSON *pstJson, int *piFlag) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL;
    char sMerchId[15 + 1] = {0}, sTransCode[6 + 1] = {0};
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
    GET_STR_KEY(pstDataJson, "merch_id", sMerchId);
    if ( !memcmp(sTransCode,"02B",3) || !memcmp(sTransCode,"0AB",3) ){
        if( FindALiPayChannel(sMerchId)==0 )
        {
            SET_STR_KEY(pstDataJson, "sweep_type", "BNAliPay"); 
        }
        else
            SET_STR_KEY(pstDataJson, "sweep_type", "ZXAliPay"); 
        
    }
    return 0;
}

int FindALiPayChannel(char *pcMerchId) {
//int FindOriTransMsg(char *pcOrderNo, char *pcTransCode, char *pcChannelMerchId,char *pcValidFlag) {
    OCI_Resultset *pstRes = NULL;
    double dAmount = 0.00;
    char sSqlStr[1024]={0},sZfbStatus[2]={0};
    MEMSET(sSqlStr);
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select decode((select key_value from s_param where key='ALIPAY_BIG_MERCH_SWITCH') ,'1','0',(select  zfb_status from WZ_MERCH where merch_id ='%s')) from dual",pcMerchId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    
     while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sZfbStatus);        
        tTrim(sZfbStatus);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);    
    tLog(INFO,"�̻�%s����״̬��%s ", pcMerchId, sZfbStatus);
    if(sZfbStatus[0]=='1')
        return ( 0 );
    else return -1;
}