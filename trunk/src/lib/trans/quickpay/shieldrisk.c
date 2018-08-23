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
#include "t_redis.h"
#include "trace.h"

void Net2Risk(cJSON *pstJson, cJSON *pstNetJson);
int SendToShield(cJSON *pstJson);
int RepSendToShield(cJSON *pstJson);
int GetConfInfo(char *pcConfName, char *pcTabName, char *pcValue);

/*ͬ�ܷ��ϵͳ-����*/
int ExehieldRisk(cJSON *pstJson, int *piFlag) {

    cJSON *pstTransJson = NULL;
    char sSqlStr[1024] = {0}, sKeyValue[1 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    /*�������л�ȡͬ�ܷ��ϵͳ����*/
    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key = 'WITH_SHIELD_SWITCH'");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sKeyValue);
        tTrim(sKeyValue);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    /*ͬ�ܷ��ϵͳ���أ�sKeyValue ������ 1 ʱ��ֱ�ӷ���*/
    if(sKeyValue[0] != '1') {
        tLog(INFO,"ͬ�ܷ��ϵͳ�ѹر�,KEY_VALUE=[%s]",sKeyValue);
        return ( 0 );
    }

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    if (SendToShield(pstTransJson) < 0) {
        ErrHanding(pstTransJson, "T9", "ͬ�ܷ�ع����鲻ͨ��.");
        return ( -1);
    }

    return ( 0);
}

/*ͬ�ܷ��ϵͳ-����*/
int ExehieldRiskRep(cJSON *pstJson, int *piFlag) {
    cJSON *pstTransJson = NULL;
    char sSqlStr[1024] = {0}, sKeyValue[1 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    /*�������л�ȡͬ�ܷ��ϵͳ����*/
    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key = 'WITH_SHIELD_SWITCH'");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sKeyValue);
        tTrim(sKeyValue);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);

    /*ͬ�ܷ��ϵͳ���أ�sKeyValue ������ 1 ʱ��ֱ�ӷ���*/
    if(sKeyValue[0] != '1') {
        tLog(INFO,"ͬ�ܷ��ϵͳ�ѹر�,KEY_VALUE=[%s]",sKeyValue);
        return ( 0 );
    }

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    RepSendToShield(pstTransJson);

    return ( 0);
}

/*�鷢�͵�ͬ�ܷ��ϵͳ����Ϣ ������*/
int SendToShield(cJSON *pstJson) {

    cJSON *pstRecvDataJson = NULL, *pstNetJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0}, sTraSeq[12 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sRuleExplan[255 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return 0;
    }
    GET_STR_KEY(pstJson, "rrn", sRrn);

    Net2Risk(pstJson, pstNetJson);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "TDFK%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("TDFK_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            tLog(ERROR, "����[%s]����ͬ�ܷ�ع����鳬ʱ.", sRrn);
        } else
            tLog(ERROR, "����[%s]����ͬ�ܷ�ع��򷵻�ʧ��.", sRrn);
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "respCode", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "respDesc", sResvDesc);
        GET_STR_KEY(pstRecvDataJson, "ruleExplanation", sRuleExplan);
        GET_STR_KEY(pstRecvDataJson, "traSeq", sTraSeq);

        tLog(DEBUG, "����[%s]���ؽ��ͣ���%s��", sTraSeq, sRuleExplan);
        if (memcmp(sRespCode, "00", 2)) {
            tLog(ERROR, "����[%s]ʧ��,ͬ�ܷ�ع�����ʧ��[%s:%s].", sTraSeq, sRespCode, sResvDesc);
            cJSON_Delete(pstNetJson);
            if (NULL != pstRecvDataJson) {
                cJSON_Delete(pstRecvDataJson);
            }
            return ( -1);
        } else {
            tLog(INFO, "����[%s]�ɹ�,ͬ�ܷ�ع�����ͨ��.", sTraSeq);
        }
    }
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);

    return ( 0);
}

/*���ؽ������е���Ϣ*/
int RepSendToShield(cJSON *pstJson) {
    cJSON *pstNetJson = NULL;
    char sRrn[RRN_LEN + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData;

    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return 0;
    }
    GET_STR_KEY(pstJson, "rrn", sRrn);

    Net2Risk(pstJson, pstNetJson);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "RSPTDFK%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;

    iRet = tSvcACall("RSPTDFK_Q", &stQMsgData);
    if (iRet < 0) {
        tLog(WARN, "���׷��أ�����֪ͨ��ͬ�ܷ��ϵͳʧ��.");
    } else {
        tLog(INFO, "���׷��أ�����֪ͨ��ͬ�ܷ��ϵͳ�ɹ�.");
    }
    tLog(DEBUG, "�ѷ���֪ͨ");
    cJSON_Delete(pstNetJson);

    return ( 0);
}

void Net2Risk(cJSON *pstJson, cJSON *pstTransJson) {
    char sTransCode[6 + 1] = {0}, sOrderNo[100] = {0}, sRrn[RRN_LEN + 1] = {0};
    char sTermSn[30 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sMerchName[128] = {0}, sMcc[4 + 1] = {0}, sMerchType[1 + 1] = {0}, sAmount[13 + 1] = {0};
    char sTransTime[6 + 1] = {0}, sDate[10 + 1] = {0}, sTime[8 + 1] = {0};
    char sHour[2 + 1] = {0}, sMinu[2 + 1] = {0}, sSec[2 + 1] = {0};
    char sTransTmStamp[18 + 1] = {0};
    double dAmount = 0.00;
    
    int iRet = 0;
    /*********************data******************************/
    char sPartnerCode[6+1] = {0};
    char sSecretKey[32+1] = {0};
    /*��ȡ�����е� PROD KEY */
    iRet = GetConfInfo("shield.conf", "PROD", sPartnerCode);
    if( iRet < 0 ) {
        /*���ö�ȡʧ��ֱ�Ӹ�ֵ*/
        SET_STR_KEY(pstTransJson, "partnerCode", "kratos"); //ͬ�ܲ�Ʒ��ʶ
    }
    else {
        SET_STR_KEY(pstTransJson, "partnerCode", sPartnerCode); //ͬ�ܲ�Ʒ��ʶ
    }
    iRet = GetConfInfo("shield.conf", "KEY", sSecretKey);
    if( iRet < 0 ) {
       /*���ö�ȡʧ��ֱ�Ӹ�ֵ*/
       SET_STR_KEY(pstTransJson, "secretKey", "ef90bdf0a1ae4dc1b640654de731306e")
    }
    else {
        SET_STR_KEY(pstTransJson, "secretKey", sSecretKey); //�ӿ���Կ,ÿ��Ӧ�ö�Ӧһ����Կ(POS)
    }
    tLog(DEBUG,"sPartnerCode = [%s],sSecretKey = [%s]",sPartnerCode,sSecretKey);
    //SET_STR_KEY(pstTransJson, "partnerCode", "kratos"); //ͬ�ܲ�Ʒ��ʶ
    //SET_STR_KEY(pstTransJson, "secretKey", "ef90bdf0a1ae4dc1b640654de731306e"); //�ӿ���Կ,ÿ��Ӧ�ö�Ӧһ����Կ(POS)

    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    SET_STR_KEY(pstTransJson, "tradeCode", sTransCode);

    tLog(DEBUG, "sTransCode[%s]", sTransCode);
    SET_STR_KEY(pstTransJson, "eventId", "qrcode");
    /*����������,��ά�붩���ţ���ɨʱ����*/
    GET_STR_KEY(pstJson, "seed", sOrderNo);
    SET_STR_KEY(pstTransJson, "qCodeOrderNo", sOrderNo);

    SET_STR_KEY(pstTransJson, "appName", "pos");

    GET_STR_KEY(pstJson, "term_sn", sTermSn);
    SET_STR_KEY(pstTransJson, "deviceId", sTermSn);

    GET_STR_KEY(pstJson, "merch_id", sMerchId);
    SET_STR_KEY(pstTransJson, "merNo", sMerchId);

    GET_STR_KEY(pstJson, "merch_name", sMerchName);
    SET_STR_KEY(pstTransJson, "merName", sMerchName);

    GET_STR_KEY(pstJson, "mcc", sMcc);
    SET_STR_KEY(pstTransJson, "mcc", sMcc);

    /*�̻�����: 0:��׼��1:�Żݣ�2:����*/
    GET_STR_KEY(pstJson, "merch_type", sMerchType);
    SET_STR_KEY(pstTransJson, "merType", sMerchType);

    GET_DOU_KEY(pstJson, "amount", dAmount);
    sprintf(sAmount, "%.2lf", dAmount / 100);
    tLog(DEBUG, "dAmount[%f],sAmount[%s]", dAmount, sAmount);
    SET_STR_KEY(pstTransJson, "traAmount", sAmount);

    GET_STR_KEY(pstJson, "rrn", sRrn);
    SET_STR_KEY(pstTransJson, "traSeq", sRrn);

    /*�¼�����ʱ��*/
    GET_STR_KEY(pstJson, "trans_time", sTransTime);
    memcpy(sHour, sTransTime, 2);
    memcpy(sMinu, sTransTime + 2, 2);
    memcpy(sSec, sTransTime + 4, 2);

    tGetDate(sDate, "-", -1);
    sprintf(sTime, "%s:%s:%s", sHour, sMinu, sSec);
    sprintf(sTransTmStamp, "%s %s", sDate, sTime);
    SET_STR_KEY(pstTransJson, "eventOccurTime", sTransTmStamp);

    /*���ؽ���������*/
    char sIstRespCode[2 + 1] = {0}, sRespCode[2 + 1] = {0}, sRespDesc[256] = {0};
    GET_STR_KEY(pstJson, "istresp_code", sIstRespCode);
    GET_STR_KEY(pstJson, "resp_code", sRespCode);
    //GET_STR_KEY(pstJson,"resp_desc",sRespDesc);
    GET_STR_KEY(pstJson, "resp_log", sRespDesc);

    SET_STR_KEY(pstTransJson, "istrespCode", sIstRespCode);
    SET_STR_KEY(pstTransJson, "payStatus", sRespCode);
    SET_STR_KEY(pstTransJson, "payResult", sRespDesc);

    /*��ά�������Ϣ*/
    char sCodeUrl[255] = {0}, sSeed[100] = {0}, sMerchOrderNo[100] = {0};
    GET_STR_KEY(pstJson, "code_url", sCodeUrl); /*��ɨ���� ��ά��URL*/
    GET_STR_KEY(pstJson, "seed", sSeed); /* ��ɨ ɨ�赽�� ��ά�뵥��*/
    GET_STR_KEY(pstJson, "qr_order_no", sMerchOrderNo); /*��ϵͳ�Զ����ɵĶ�ά�붩����*/

    SET_STR_KEY(pstTransJson, "qCodeOrderNo", sSeed);
    SET_STR_KEY(pstTransJson, "qCodeOrderUrl", sCodeUrl);
}
/*��ȡͬ�ܶٷ�������ļ��� ͬ�ܲ�Ʒ��ʶ �� �ӿ���Կ
 *��Σ�
 *      pcConfName  �����ļ�����
 *      pcTabName   ��ǩ����
 *���Σ�
 *      pcValue ��ǩ��Ӧ��ֵ
 */
int GetConfInfo(char *pcConfName, char *pcTabName, char *pcValue) {
    char *DirPath = NULL;
    char FullPath[32] = {0};

    FILE *fp = NULL;
    char sBuf[1024] = {0};

    DirPath = getenv("HOME");
    sprintf(FullPath, "%s/etc/%s", DirPath, pcConfName);
    tLog(DEBUG, "FullPath = [%s]", FullPath);

    fp = fopen(FullPath, "r");
    if (NULL == fp) {
        tLog(DEBUG,"�ļ�[%s]��ʧ��\n", FullPath);
        return ( -1);
    }
    tLog(DEBUG,"�ļ�[%s]�򿪳ɹ�\n", FullPath);

    //��ȡһ������ fgets
    while (fgets(sBuf, 1024, fp)) {
        if (strstr(sBuf, pcTabName)) {
            fgets(sBuf, 1024, fp);
            tTrim(sBuf);
            tStrCpy(pcValue,sBuf,strlen(sBuf));
            break;
        }
    }
    fclose(fp);
    fp = NULL;    
    return ( 0);
}
