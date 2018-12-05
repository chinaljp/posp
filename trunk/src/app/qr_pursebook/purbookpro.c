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
#include "t_redis.h"
#include "t_cjson.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "param.h"
#include "qrpursebook.h"
#include <iconv.h>

static char g_sTransCode[6+1];
int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);

int DealRespMsg(char *pcRespMsg) {
    
    char sMerchId[15 + 1]         = {0}; //�̻���
    char sChannelMerchId[15 + 1]  = {0};//�����̻��ţ�������ά����ת����Ȧ�󣬻�ȡ����������ʹ��
    char sOrderNo[32 + 1]         = {0}; //�̻������� 
    //char sPayMethod[10+1] = {0};
    char sRrn[12 + 1]             = {0}; //rrn �޿�������� ����
    char sRespCode[10 + 1]        = {0}; //֧�����
    char sRespDescGBK[200]           = {0};
    char sRespDesc[400]       = {0};
    //char sTransCode[6 + 1]        = {0};
    char sSettleKey[38 + 1]       = {0}; //��������
    char sBuf[16 + 1]             = {0};
    char sSettleSysTrace[6 + 1]   = {0}; //������ά��ϵͳ���ٺ�
    char sSettleTransTime[10 + 1] = {0}; //������ά�뽻�״���ʱ��
    char sAmount[12 + 1] = {0},sOrderAmount[12 + 1] = {0}, sCardAttr[2 + 1] = {0}, sCardType[1 + 1] = {0}, sChannelSettleDate[4] = {0},sSettleDate[8+1]={0};
    double dAmount = 0.00, dOrderAmount = 0.00;
    
    char sSqlStr[1024]      = {0};
    char sValidFlag[1 + 1]  = {0};
    char sSettleFlag[1 + 1] = {0};
    OCI_Resultset *pstRes   = NULL;
    OCI_Statement *pstState = NULL;
    
    cJSON *pstJson = NULL;

    tLog(DEBUG,"pcRespMsg = [%s]",pcRespMsg);
    pstJson = cJSON_Parse(pcRespMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "��ȡ��Ϣʧ��,��������.");
        return -1;
    } 
    
    memset(g_sTransCode, 0, sizeof (g_sTransCode));
    GET_STR_KEY(pstJson,"orderNo",sOrderNo);
    //GET_STR_KEY(pstJson,"payMethod",sPayMethod);
    GET_STR_KEY(pstJson,"respCode",sRespCode);
    GET_STR_KEY(pstJson,"trans_code",g_sTransCode);
    
    /*���� ��ά�뽻��֪ͨ �� �޿�������ѽ���֪ͨ */
    if( !memcmp(g_sTransCode,"0AQ000",6) ) {
        /*��ȡ���ʽ��׶�����Ϣ*/
        GET_STR_KEY(pstJson,"rrn",sRrn);
        GET_STR_KEY(pstJson,"mrchntNo",sMerchId);
        /*ʹ��rrn��ȡ�޿�������Ѷ�����*/
        if( FindMerchOrderNo( sOrderNo, sRrn ) < 0 ) {
            tLog(ERROR,"��ȡ���ν���[%s]�Ķ�����[%s]��ʧ�ܣ�",sRrn,sOrderNo);
            return ( -1 );
        }
         /*EPOS �޿�������� ֧�����ʧ����� �Ĵ��� */
        if( !memcmp(sRespCode,"96",2) ) {
            GET_STR_KEY(pstJson,"respDesc",sRespDesc);
            code_convert("utf-8","gb2312", sRespDesc, strlen(sRespDesc), sRespDescGBK, sizeof (sRespDescGBK));
            tLog(INFO, "utf-8[%s], gb2312[%s]", sRespDesc, sRespDescGBK);
            tLog(WARN,"rrn[%s],�����ţ�[%s],EPOS �޿��������֧�����ʧ��,respDesc:[%s]!",sRrn,sOrderNo,sRespDescGBK);
            /*���·�������*/
            if( UpRespDesc(sRespCode,sRespDescGBK,sRrn) < 0 ){
                tLog(ERROR,"���±��ν���[%s]��Ӧ������ʧ�ܣ�",sRrn);
                return ( -1 );
            }
            return ( 0 );
        }/*����00 �����ԭ����Ӧ����*/
        else {
            tLog(WARN,"rrn[%s],�����ţ�[%s],EPOS �޿��������֧�������֧���ɹ�!��sRespCode[%s]",sRrn,sOrderNo,sRespCode);
            /*����ԭ����Ӧ��*/
            if( UpRespDesc(sRespCode,"SUCCESS",sRrn) < 0 ){
                tLog(ERROR,"���±��ν���[%s]��Ӧ������ʧ�ܣ�",sRrn);
                return ( -1 );
            }
        }
    }
    else {
        /*��ȡ���ʽ��׶������̻���*/
        if( FindMerchId( sMerchId, sChannelMerchId, sOrderNo) < 0 ) {
            tLog(ERROR,"��ȡ���ζ���[%s]���̻���ʧ�ܣ�",sOrderNo);
            return ( -1 );
        }

        if(sChannelMerchId[0] == '\0' || !memcmp(sMerchId,sChannelMerchId,15) ) {
            tLog(INFO,"�̻�[%s]�Ķ�����[%s],֧�����[%s]", sMerchId,sOrderNo,sRespCode);
        }
        else {
            tLog(INFO,"�̻�[%s]��ת�����̻�[%s]�Ķ�����[%s],֧�����[%s]", sMerchId,sChannelMerchId,sOrderNo,sRespCode);
        }
    }
    
    /*add by GJQ at 20171218,  ������ά���첽֪ͨ��֧���ɹ�������ˮ */
    if( !memcmp(sRespCode,"00",2) && !memcmp(g_sTransCode,"CUPS00",6) ) {
        GET_STR_KEY(pstJson,"settleKey",sSettleKey);
        GET_STR_KEY(pstJson,"txnAmt",sAmount);//���׽��Żݺ�Ľ�
        GET_STR_KEY(pstJson,"origTxnAmt",sOrderAmount);//�������׽��Ż�ǰ�Ľ�
        GET_STR_KEY(pstJson,"cardAttr",sCardAttr);//sCardAttr�� 01 - ��ǿ��� 02 - ���ǿ�
        GET_STR_KEY(pstJson,"settleDate",sChannelSettleDate); //��������������������
        if( sCardAttr[1] == '1' ) {
            strcpy(sCardType,"0");
        } 
        else if( sCardAttr[1] == '2'  ) {
            strcpy(sCardType,"1");
        }
        /*�������ת��*/
        dAmount = atof(sAmount);
        dAmount = dAmount / 100;
        if( sOrderAmount[0] == '\0' ) {
            dOrderAmount = dAmount;    
        }
        else 
        {
            dOrderAmount = atof(sOrderAmount);
            dOrderAmount = dOrderAmount / 100;
        }
        
        
        memcpy(sBuf, sSettleKey+22, sizeof(sBuf)); //��ȡϵͳ���ٺ� ���״���ʱ��
        memcpy(sSettleSysTrace,sBuf,6);            //��ȡϵͳ���ٺ�
        sSettleSysTrace[7] = '\0';
        memcpy(sSettleTransTime,sBuf+strlen(sSettleSysTrace),10);//��ȡ���״���ʱ��
        tLog(INFO,"sSettleKey[%s],sSettleSysTrace[%s],sSettleTransTime[%s]",sSettleKey,sSettleSysTrace,sSettleTransTime);   
        
        /*��ȡsettle_date(����)*/
        GetSysSettleDate(sSettleDate,sChannelSettleDate);
        
        if( UpCupsSettleKey(sCardType,sSettleSysTrace,sSettleTransTime,sChannelSettleDate,sSettleDate,sOrderNo) < 0 ) {
            tLog(ERROR, "�����̻�[%s]�Ķ���[%s]��card_type,settle_sys_trace��settle_trans_timeʧ��.",sMerchId,sOrderNo);
            return ( -1 );
        }
        
        if( UpCupsAmt(dAmount,dOrderAmount,sOrderNo) < 0 ) {
            tLog(ERROR, "�����̻�[%s]�Ķ���[%s]�Ľ��׽��[%.02f],�������׽��[%.02f]ʧ��.",sMerchId,sOrderNo,dAmount,dOrderAmount);
            return ( -1 );
        }
        
        /* ����������ά�뽻�׵������Ѳ����µ���ˮ���� */ 
        tLog(INFO,"#######################################������ά���첽֪ͨ�м��������ѿ�ʼ��#######################################");
        if( MerchUpFee( sMerchId,sChannelMerchId,sOrderNo,sCardType,sAmount ) < 0 ) {
            tLog(ERROR,"�����̻�[%s]�Ķ���[%s]��������ʧ��!",sMerchId,sOrderNo);
            return ( -1 );
        }
        tLog(INFO,"#######################################������ά���첽֪ͨ�м��������ѽ�����#######################################");
        
    }
    
    /*��ѯ�Ƿ�valid_flag = '0' ֧���ɹ�״̬ ���ǣ���ֱ�ӷ���success*/
    snprintf(sSqlStr,sizeof(sSqlStr),"select valid_flag,settle_flag from b_inline_tarns_detail "
                        "where merch_order_no = '%s' and trans_code not in ('02B300','02W300','02Y300','0AY300','0AW300','0AB300','0AQ300')",sOrderNo);
    tLog(DEBUG,"sSqlStr = [%s]",sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
       tLog(ERROR, "sql[%s] err!", sSqlStr);
       return ( -1 );
    }

    if (NULL == pstRes) {
       tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
       return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
       STRV(pstRes, 1, sValidFlag);
       STRV(pstRes, 2, sSettleFlag);
       tTrim(sValidFlag);
       tTrim(sSettleFlag);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
       tLog(ERROR, "δ�ҵ���¼.");
       tReleaseRes(pstRes);
       return ( -1 );
    }
    tReleaseRes(pstRes);
    tLog(DEBUG,"valid_flag[%s]:[%s],settle_flag[%s]:[%s]",sValidFlag,sValidFlag[0] == '0' ? "��֧��":"δ֧��",
            sSettleFlag,sSettleFlag[0] == 'M'?"������":"δ����");
    
    /*��֧������δ���������Ǯ����ֵ�� valid_flag ��0 - ��֧��״̬*/
    if(sValidFlag[0] == '0') {
        /*��֧���ɹ���δ�������ֵǮ��*/
        if( sSettleFlag[0] == 'N' ) {
            tLog(INFO,"����[%s]��֧��δ���ˣ����г�ֵ",sOrderNo);
            if( AddQrWallet(sOrderNo) < 0 ) {
                tLog(ERROR,"����[%s]��ֵǮ��ʧ��",sOrderNo);
                /*Ǯ����ֵʧ�� �������ף� ���ڶ��˽��в�¼*/
                //return ( -1 );
            }
        }
        else {
            tLog(INFO,"����[%s]��֧����������",sOrderNo);
            return ( 0 );
        }
    }
    /*sRespCode��00-֧���ɹ�, valid_flag ��4 - δ֧��״̬*/
    else if( sValidFlag[0] == '4' && !memcmp(sRespCode,"00",2) ) {
        /*֧���ɹ�����δ�������ֵǮ��*/
        if( sSettleFlag[0] == 'N' ) {
            tLog(INFO,"����[%s]֧���ɹ���δ���ˣ����г�ֵ",sOrderNo);
            if( AddQrWallet(sOrderNo) < 0 ) {
                tLog(ERROR,"����[%s]��ֵǮ��ʧ��",sOrderNo);
                /*Ǯ����ֵʧ�� �������ף� ���ڶ��˽��в�¼*/
                //return ( -1 );
            }
        }
        else {
            tLog(INFO,"����[%s]֧���ɹ���������",sOrderNo);
        }
        /*����valid_flag*/
        if( UpValidFlag( sOrderNo ) < 0 ) {
            tLog(ERROR, "�����̻�[%s]�Ķ���[%s]��valid_flag��־ʧ��.",sMerchId,sOrderNo);
            return ( -1 );
        }
    } 
    else {
        tLog(INFO,"����[%s]֧�����[%s],δ֧����֧��ʧ��",sOrderNo,sRespCode);
        return ( -1 );
    }
    /*���׽����֧���ɹ� ����ԭ����Ӧ��ʱ ���Ѹ��½��׵����ͱ�־ �˴��������������ͱ�־
     * if ( !memcmp(g_sTransCode,"0AQ000",6) ) {
        if(  UpNoticeFlag( sRrn ) < 0 ) {
            tLog(ERROR, "���½���[%s]�����ͱ�־notice_flagʧ��.",sRrn);
            return ( -1 );
        }
    }*/
    return ( 0 );
}

int GoBackMsg(char *pcBackFlag) {
    char *sMsg = NULL;
    char sRepSvrId[32] = {0};
    cJSON *pstJson = NULL;
    
    snprintf(sRepSvrId, sizeof (sRepSvrId), "%s_REP", GetSvrId());
    tLog(INFO,"SvrId : [%s]",sRepSvrId);
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return ( -1 );
    }
    SET_STR_KEY(pstJson,sRepSvrId,pcBackFlag);
    sMsg = cJSON_PrintUnformatted(pstJson);
    
    /*��������*/
    if (tSendMsg(sRepSvrId, sMsg, strlen(sMsg)) < 0) {
        tLog(ERROR, "������Ϣ��SvrId[%s]ʧ��,data[%s].", sRepSvrId,sMsg);
        free(sMsg);
        cJSON_Delete(pstJson);
        return ( -1 );
    }
    free(sMsg);
    cJSON_Delete(pstJson);
    
    return ( 0 );
}

/* ��ֵǮ�� */
int AddQrWallet(char *pcMerchOrderNo) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[32 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    
    
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    if ( Net2Wallet( pcMerchOrderNo,pstNetJson ) ) {
        tLog(ERROR,"����[%s]��Ǯ����ֵ���ĳ���",pcMerchOrderNo);
        return ( -1 );
    }
    /*��ȡpstNetJson �е�rrn ������sKey*/
    GET_STR_KEY(pstNetJson, "rrn", sRrn);
    if ( !memcmp(g_sTransCode,"0AQ000",6) ) {
        SET_STR_KEY(pstNetJson, "walletType", "400"); //Ǯ������  
        SET_STR_KEY(pstNetJson, "postType", "1500"); //���֧������
    }
    else {
        SET_STR_KEY(pstNetJson, "walletType", "200"); //Ǯ������   
        SET_STR_KEY(pstNetJson, "postType", "1200");//��ά�뱻ɨ
    }
    
    SET_STR_KEY(pstNetJson, "trans_code", "00T600");
    
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T600%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("00T600_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        tLog(DEBUG, "��ά�뽻��[%s]Ǯ������ʧ��.", sRrn);
        cJSON_Delete(pstNetJson);
        return ( -1 );
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
        if ( !memcmp(sRespCode, "00", strlen(sRespCode)) ) {
            tLog(DEBUG, "����[%s]Ǯ�����˳ɹ�.", sRrn);
        } else {
            tLog(ERROR, "Ǯ������ʧ��[%s:%s].", sRespCode, sResvDesc);
            cJSON_Delete(pstNetJson);
            if (NULL != pstRecvDataJson) {
                cJSON_Delete(pstRecvDataJson);
            }
            return ( -1 );
            /*Ǯ������ʧ�� �������ף� ���ڶ��˽��в�¼*/
        }
    }
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);  
    
    return ( 0 );
}

int Net2Wallet(char *pcMerchOrderNo,cJSON *pstNetJson) {
    double dAmount = 0.0, dFee = 0.0;
    char sMerchId[15 + 1] = {0}, sSettleDate[8 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sSysTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0};
    char sSqlStr[1024] = {0};
    OCI_Resultset *pstRes = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select amount,fee,merch_id,settle_date,rrn,sys_trace,trans_date,trans_time from "
                "b_inline_tarns_detail where merch_order_no = '%s' and trans_code not in ('02Y300','0AY300','02W300','02B300','0AW300','0AB300','0AQ300')",pcMerchOrderNo);
    
    if (tExecute(&pstRes, sSqlStr) < 0) {
       tLog(ERROR, "sql[%s] err!", sSqlStr);
       return ( -1 );
    }

    if (NULL == pstRes) {
       tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
       return ( -1 );
    }
    while (OCI_FetchNext(pstRes)) {
        DOUV(pstRes, 1, dAmount);
        DOUV(pstRes, 2, dFee);
        STRV(pstRes, 3, sMerchId);
        STRV(pstRes, 4, sSettleDate);
        STRV(pstRes, 5, sRrn);
        STRV(pstRes, 6, sSysTrace);
        STRV(pstRes, 7, sTransDate);
        STRV(pstRes, 8, sTransTime);
       
       
       tTrim(sMerchId);
       tTrim(sSettleDate);
       tTrim(sRrn);
       tTrim(sSysTrace);
       tTrim(sTransDate);
       tTrim(sTransTime);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
       tLog(ERROR, "δ�ҵ���¼.");
       tReleaseRes(pstRes);
       return ( -1 );
    }
    tReleaseRes(pstRes);
    
    SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
    
    if ( !memcmp(g_sTransCode,"0AQ000",6) ) {
         SET_STR_KEY(pstNetJson, "trans_type", "QUICKPAY"); //�����������п�����ά�롢����޿��Ľ��� 
    }
    else {
         SET_STR_KEY(pstNetJson, "trans_type", "INLINE"); //�����������п�����ά�롢����޿��Ľ��� 
    }
   
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_DOU_KEY(pstNetJson, "transAmount", dAmount);
    SET_DOU_KEY(pstNetJson, "settleAmount", dAmount - dFee);
    SET_STR_KEY(pstNetJson, "sys_trace", sSysTrace);
    SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
    SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
    SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);
    
    return ( 0 );
}


int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen,
        char *outbuf, size_t outlen) {
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}
