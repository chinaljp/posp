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
int ExehieldRisk(cJSON *pstTransJson, int *piFlag) {
    char sSqlStr[1024] = {0}, sKeyValue[1 + 1] = {0};
    char sTransCode[6 + 1] = {0};
    double dAmount = 0.00;
    OCI_Resultset *pstRes = NULL;

    /*���׽��Ϊ0�Ľ�ֹ����*/
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    dAmount /= 100;
    tLog(INFO, "����[%s],���׽��Ϊ[%.02f]", sTransCode, dAmount);
    if (strstr("020000,T20000,024000,024100,M20000", sTransCode) && DBL_ZERO(dAmount))
    {
        tLog(ERROR, "����[%s],���׽��Ϊ[%.02f],��ֹ����", sTransCode, dAmount);
        ErrHanding(pstTransJson, "13", "���׽���Ϊ��.");
        return ( -1);
    }
    /*�������л�ȡͬ�ܷ��ϵͳ����*/
    if (FindValueByKey(sKeyValue, "WITH_SHIELD_SWITCH") < 0)
    {
        ErrHanding(pstTransJson, "96", "����key[WITH_SHIELD_SWITCH]ֵ,ʧ��.");
        return -1;
    }

    /*ͬ�ܷ��ϵͳ���أ�sKeyValue ������ 1 ʱ��ֱ�ӷ���*/
    if (sKeyValue[0] != '1')
    {
        tLog(INFO, "ͬ�ܷ��ϵͳ�ѹر�,KEY_VALUE=[%s]", sKeyValue);
        return ( 0);
    }

    if (SendToShield(pstTransJson) < 0)
    {
        ErrHanding(pstTransJson, "D9", "ͬ�ܷ�ع����鲻ͨ��.");
        return ( -1);
    }

    return ( 0);
}

/*ͬ�ܷ��ϵͳ-����*/
/* ��Ӧ���ײ�Ҫ����-1 */
int ExehieldRiskRep(cJSON *pstTransJson, int *piFlag) {
    char sKeyValue[1 + 1] = {0}, sRespCode[RESP_CODE_LEN + 1] = {0};

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    /*�������л�ȡͬ�ܷ��ϵͳ����*/
    if (FindValueByKey(sKeyValue, "WITH_SHIELD_SWITCH") < 0)
    {
        tLog(INFO, "����key[WITH_SHIELD_SWITCH]ֵʧ��,����.");
        return 0;
    }
    /*ͬ�ܷ��ϵͳ���أ�sKeyValue ������ 1 ʱ��ֱ�ӷ���*/
    if (sKeyValue[0] != '1')
    {
        tLog(INFO, "ͬ�ܷ��ϵͳ�ѹر�,KEY_VALUE=[%s],����.", sKeyValue);
        return ( 0);
    }
    RepSendToShield(pstTransJson);
    return ( 0);
}

/*�鷢�͵�ͬ�ܷ��ϵͳ����Ϣ ������*/
int SendToShield(cJSON *pstTransJson) {

    cJSON *pstRecvDataJson = NULL, *pstNetJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0}, sTraSeq[12 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sRuleExplan[255 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson)
    {
        tLog(ERROR, "��������Jsonʧ��.");
        return 0;
    }
    GET_STR_KEY(pstTransJson, "rrn", sRrn);

    Net2Risk(pstTransJson, pstNetJson);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "TDFK%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("TDFK_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0)
    {
        if (MSG_TIMEOUT == iRet)
        {
            tLog(ERROR, "����[%s]����ͬ�ܷ�ع����鳬ʱ.", sRrn);
        } else
            tLog(ERROR, "����[%s]����ͬ�ܷ�ع��򷵻�ʧ��.", sRrn);
    } else
    {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "respCode", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "respDesc", sResvDesc);
        GET_STR_KEY(pstRecvDataJson, "ruleExplanation", sRuleExplan);
        GET_STR_KEY(pstRecvDataJson, "traSeq", sTraSeq);

        tLog(DEBUG, "����[%s]���ؽ���:[%s]", sTraSeq, sRuleExplan);
        if (memcmp(sRespCode, "00", 2))
        {
            tLog(ERROR, "����[%s]ʧ��,ͬ�ܷ�ع�����ʧ��[%s:%s].", sTraSeq, sRespCode, sResvDesc);
            cJSON_Delete(pstNetJson);
            if (NULL != pstRecvDataJson)
            {
                cJSON_Delete(pstRecvDataJson);
            }
            return ( -1);
        } else
        {
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
    if (NULL == pstNetJson)
    {
        tLog(ERROR, "��������Jsonʧ��.");
        return 0;
    }
    GET_STR_KEY(pstJson, "rrn", sRrn);

    Net2Risk(pstJson, pstNetJson);

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "RSPTDFK%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;

    iRet = tSvcACall("RSPTDFK_Q", &stQMsgData);
    if (iRet < 0)
    {
        tLog(WARN, "���׷��أ�����֪ͨ��ͬ�ܷ��ϵͳʧ��.");
    } else
    {
        tLog(INFO, "���׷��أ�����֪ͨ��ͬ�ܷ��ϵͳ�ɹ�.");
    }
    tLog(DEBUG, "�ѷ���֪ͨ");
    cJSON_Delete(pstNetJson);

    return ( 0);
}

void Net2Risk(cJSON *pstJson, cJSON *pstTransJson) {
    char sTransCode[6 + 1] = {0}, sOrderNo[100] = {0}, sRrn[RRN_LEN + 1] = {0};
    char sTermSn[30 + 1] = {0}, sCardNo[19 + 1] = {0}, sCardMedia[2 + 1] = {0}, sInputMode[3 + 1] = {0}, sCardType[1 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sMerchName[128] = {0}, sMcc[4 + 1] = {0}, sMerchType[1 + 1] = {0}, sAmount[13 + 1] = {0};
    char sTransTime[6 + 1] = {0}, sDate[10 + 1] = {0}, sTime[8 + 1] = {0};
    char sHour[2 + 1] = {0}, sMinu[2 + 1] = {0}, sSec[2 + 1] = {0};
    char sTransTmStamp[18 + 1] = {0};
    double dAmount = 0.00;

    int iRet = 0;

    /*********************data******************************/
    char sPartnerCode[6 + 1] = {0};
    char sSecretKey[32 + 1] = {0};
    /*��ȡ�����е� PROD KEY */
    tLog(INFO, "ͬ�ܲ�Ʒ��ʶ");
    iRet = GetConfInfo("shield.conf", "PROD", sPartnerCode);
    if (iRet < 0)
    {
        /*���ö�ȡʧ��ֱ�Ӹ�ֵ*/
        SET_STR_KEY(pstTransJson, "partnerCode", "kratos"); //ͬ�ܲ�Ʒ��ʶ
    } else
    {
        SET_STR_KEY(pstTransJson, "partnerCode", sPartnerCode); //ͬ�ܲ�Ʒ��ʶ
    }
    tLog(INFO, "�ӿ���Կ");
    iRet = GetConfInfo("shield.conf", "KEY", sSecretKey);
    if (iRet < 0)
    {
        /*���ö�ȡʧ��ֱ�Ӹ�ֵ*/
        SET_STR_KEY(pstTransJson, "secretKey", "ef90bdf0a1ae4dc1b640654de731306e")
    } else
    {
        SET_STR_KEY(pstTransJson, "secretKey", sSecretKey); //�ӿ���Կ,ÿ��Ӧ�ö�Ӧһ����Կ(POS)
    }
    tLog(DEBUG, "sPartnerCode = [%s],sSecretKey = [%s]", sPartnerCode, sSecretKey);

    //SET_STR_KEY(pstTransJson, "partnerCode", "kratos"); //ͬ�ܲ�Ʒ��ʶ
    //SET_STR_KEY(pstTransJson, "secretKey", "ef90bdf0a1ae4dc1b640654de731306e"); //�ӿ���Կ,ÿ��Ӧ�ö�Ӧһ����Կ(POS)

    GET_STR_KEY(pstJson, "trans_code", sTransCode);
    SET_STR_KEY(pstTransJson, "tradeCode", sTransCode);

    tLog(DEBUG, "sTransCode[%s]", sTransCode);

    SET_STR_KEY(pstTransJson, "eventId", "posp");
    /*�������� ���׽��׶����� */
    if (!memcmp(sTransCode, "M2", 1))
    {
        GET_STR_KEY(pstJson, "merch_order_no", sOrderNo);
        SET_STR_KEY(pstTransJson, "transactionId", sOrderNo);
    }

    SET_STR_KEY(pstTransJson, "appName", "pos");

    GET_STR_KEY(pstJson, "term_sn", sTermSn);
    SET_STR_KEY(pstTransJson, "deviceId", sTermSn);

    GET_STR_KEY(pstJson, "card_no", sCardNo);
    SET_STR_KEY(pstTransJson, "cardNumber", sCardNo);

    GET_STR_KEY(pstJson, "input_mode", sInputMode);
    memcpy(sCardMedia, sInputMode, 2);
    tLog(DEBUG, "sCardMedia[%s],sInputMode[%s]", sCardMedia, sInputMode);
    /*������ 01 -�ֹ�  02 - ����  05 - �Ӵ�ʽоƬ�� 07 - �ǽӴ�ʽоƬ��   */
    SET_STR_KEY(pstTransJson, "tradeMedium", sCardMedia);

    GET_STR_KEY(pstJson, "card_type", sCardType);
    SET_STR_KEY(pstTransJson, "cardType", sCardType);

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
    if (NULL == fp)
    {
        tLog(DEBUG, "�ļ�[%s]��ʧ��\n", FullPath);
        return ( -1);
    }
    tLog(DEBUG, "�ļ�[%s]�򿪳ɹ�\n", FullPath);

    //��ȡһ������ fgets
    while (fgets(sBuf, 1024, fp))
    {
        if (strstr(sBuf, pcTabName))
        {
            fgets(sBuf, 1024, fp);
            tTrim(sBuf);
            tStrCpy(pcValue, sBuf, strlen(sBuf));
            break;
        }
    }
    fclose(fp);
    fp = NULL;
    return ( 0);
}