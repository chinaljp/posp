/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   wallet.c
 * Author: pangpang
 *
 * Created on 2017��4��28��, ����11:31
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "t_redis.h"
#include "settle_date.h"
#include "t_macro.h"
#include "status.h"

typedef struct {
    char sTransCode[6 + 1];
    char sPostType[4 + 1];
} PostType;
/* �������ͺͽ�����ת�� */
PostType g_staPostType[] = {
    /* pos */
    {"02W100", "1200"},
    {"02W200", "1200"},
    {"02W300", "1200"},
    {"02B100", "1200"},
    {"02B200", "1200"},
    {"02B300", "1200"},
    {"02W600", "2200"},
    {"02B600", "2200"},
    {"02Y600", "2200"},
    /* �ֻ�app */
    {"0AW100", "1200"},
    {"0AW200", "1200"},
    {"0AW300", "1200"}, 
    {"0AB100", "1200"},
    {"0AB200", "1200"},
    {"0AB300", "1200"},
    {"0AB600", "2200"},
    {"0AW600", "2200"}
};

void GetPostType(char *pcPostType, char *pcTransCode) {
    int i;
    for (i = 0; i < 64; i++) {
        if (!strcmp(g_staPostType[i].sTransCode, pcTransCode)) {
            strcpy(pcPostType, g_staPostType[i].sPostType);
        }                 
    }
}

void Net2Wallet(cJSON *pstTransJson, cJSON *pstNetJson) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = {0}, sSettleDate[8 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sOrrn[RRN_LEN + 1]  = {0}, sTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransType[1 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sCardType[1 + 1] = {0}, sInputMode[3 + 1] = {0}, sPostType[4 + 1] = {0};

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "o_rrn", sOrrn);//΢�š�֧���� ��ά���ѯ���� ֧���ɹ�ʱ ���˴��� ��Ҫʹ��ԭ����rrn
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "sys_trace", sTrace);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "settle_date", sSettleDate);
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    //��������
    SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
    SET_STR_KEY(pstNetJson, "trans_type", "INLINE"); //�����������п����Ƕ�ά��Ľ���
    
    if( sTransCode[3] == '3' ) {
        //΢�š�֧���� ��ά���ѯ���� ֧���ɹ�ʱ ���˴��� rrnʹ��ԭ���׵�sOrrn
        SET_STR_KEY(pstNetJson, "rrn", sOrrn);
    }
    else {
        SET_STR_KEY(pstNetJson, "rrn", sRrn);
    }
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY(pstNetJson, "transAmount", dAmount / 100);
    SET_DOU_KEY(pstNetJson, "settleAmount", dAmount / 100 - dFee);
    SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
    SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
    SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
    SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);
    /* 
     * 210����ά��D0������ҵ�񶳽�Ǯ��
     * 211����ά��D0������ҵ�񶳽�Ǯ��(����)
     *  */
    SET_STR_KEY(pstNetJson, "walletType", "200"); //Ǯ������    
    GetPostType(sPostType, sTransCode);
    SET_STR_KEY(pstNetJson, "postType", sPostType);
}

/* ��ά����ɨ���� ,ʧ�ܲ�����*/
int AddQrWallet(cJSON *pstTransJson, int *piFlag) {
    cJSON  *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "����[%s]ʧ��[%s],��Ǯ������.", sRrn, sRespCode);
        return 0;
    }
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return 0;
    }
    Net2Wallet(pstTransJson, pstNetJson);
    SET_STR_KEY(pstNetJson, "trans_code", "00T600");
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T600%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    //iRet = tSvcCall("00T600_Q", pstNetJson, &pstRecvJson, 30);
   /*iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    
    if (iRet < 0) {
        tLog(DEBUG, "��ά�뽻��[%s]Ǯ������ʧ��.", sRrn);
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
        GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
        if (memcmp(sRespCode, "00", 2)) {
            tLog(ERROR, "Ǯ������ʧ��[%s:%s].", sRespCode, sResvDesc);
        } else {
            tLog(DEBUG, "����[%s]Ǯ�����˳ɹ�.", sRrn);
        }
    }
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);
    */
    iRet = tSvcACall("WALLET_Q", &stQMsgData);
    if (iRet < 0)
    {
        if (MSG_TIMEOUT == iRet)
        {
            ErrHanding(pstTransJson, sRespCode, "Ǯ�����˳�ʱ,����.");
        } else
            ErrHanding(pstTransJson, sRespCode, "Ǯ������ʧ��,����.");
    } 
    else
    {
        tLog(INFO, "Ǯ�����������ͳɹ�.");
    }
    cJSON_Delete(pstNetJson);
    
    return 0;
}

void Net2FrozenWallet(cJSON *pstTransJson, cJSON *pstNetJson) {
    char sMerchId[15 + 1] = {0}, sUserCode[15 + 1] = {0}, sTransType[1 + 1] = {0};
    char sRrn[12 + 1] = {0}, sTrace[7] = {0};
    double dAmount = 0.0, dFee = 0.0;
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "sys_trace", sTrace);
    SET_STR_KEY(pstNetJson, "walletType", "200");
    SET_STR_KEY(pstNetJson, "frozenType", "31");
    SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
    SET_STR_KEY(pstNetJson, "user_code", sUserCode);
    SET_DOU_KEY(pstNetJson, "amount", dAmount / 100 - dFee);
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
}

int FrozenWallet(cJSON *pstTransJson, int *piFlag) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char  sRrn[RRN_LEN + 1] = {0}, sRespCode[2 + 1] = {0}, sResvDesc[4096 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    Net2FrozenWallet(pstTransJson, pstNetJson);
    SET_STR_KEY(pstNetJson, "trans_code", "00T400");

    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T400%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            ErrHanding(pstTransJson, "96", "����[%s]Ǯ������֪ͨ��ʱ.", sRrn);
        } else
            ErrHanding(pstTransJson, "96", "����[%s]Ǯ������֪ͨʧ��.", sRrn);
        cJSON_Delete(pstNetJson);
        return -1;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
    if (memcmp(sRespCode, "00", 2)) {
        ErrHanding(pstTransJson, "96", "Ǯ������ʧ��[%s:%s].", sRespCode, sResvDesc);
    } else
        tLog(INFO, "����[%s]Ǯ������ɹ�.", sRrn);
    cJSON_Delete(pstNetJson);
    if (NULL != pstRecvDataJson)
        cJSON_Delete(pstRecvDataJson);

    return 0;
}

void Net2UnFrozenWallet(cJSON *pstTransJson, cJSON *pstNetJson) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = {0}, sSettleDate[8 + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sTrace[6 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransType[1 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sCardType[1 + 1] = {0}, sInputMode[3 + 1] = {0}, sPostType[4 + 1] = {0};

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_DOU_KEY(pstTransJson, "fee", dFee);
    GET_STR_KEY(pstTransJson, "sys_trace", sTrace);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "settle_date", sSettleDate);
    GET_STR_KEY(pstTransJson, "trans_type", sTransType);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "input_mode", sInputMode);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    //��������
    SET_STR_KEY(pstNetJson, "merchantNo", sMerchId);
    SET_STR_KEY(pstNetJson, "trans_type", "INLINE"); //�����������п����Ƕ�ά��Ľ���
    SET_STR_KEY(pstNetJson, "rrn", sRrn);
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY(pstNetJson, "transAmount", dAmount / 100);
    SET_DOU_KEY(pstNetJson, "settleAmount", dAmount / 100 - dFee);
    SET_STR_KEY(pstNetJson, "sys_trace", sTrace);
    SET_STR_KEY(pstNetJson, "trans_date", sTransDate);
    SET_STR_KEY(pstNetJson, "trans_time", sTransTime);
    SET_STR_KEY(pstNetJson, "settle_date", sSettleDate);
    SET_STR_KEY(pstNetJson, "walletType", "210");

    GetPostType(sPostType, sTransCode);
    SET_STR_KEY(pstNetJson, "postType", sPostType);
}

int UnFrozenWallet(cJSON *pstTransJson, int *piFlag) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = {0}, sRrn[RRN_LEN + 1] = {0}, sResvDesc[4096 + 1] = {0};
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject();
    if (NULL == pstNetJson) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    if (!memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "����[%s]�ɹ�,�ָ�Ǯ��.", sRrn);
        Net2UnFrozenWallet(pstTransJson, pstNetJson);
        SET_STR_KEY(pstNetJson, "trans_code", "00T600");
        tGetUniqueKey(stQMsgData.sSvrId);
        snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T600%s", sRrn);
        stQMsgData.pstDataJson = pstNetJson;
        iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
        //iRet = tSvcCall("00T600_Q", pstNetJson, &pstRecvJson, 30);
        if (iRet < 0) {
            if (MSG_TIMEOUT == iRet) {
                ErrHanding(pstTransJson, "96", "����[%s]Ǯ���ⶳ��ʱ.", sRrn);
            } else
                tLog(ERROR, "����[%s]Ǯ���ⶳʧ��.", sRrn);
        } else {
            pstRecvDataJson = stPMsgData.pstDataJson;
            GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
            GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
            if (memcmp(sRespCode, "00", 2)) {
                tLog(ERROR, "Ǯ���ⶳ����ʧ��[%s:%s].", sRespCode, sResvDesc);
            } else {
                tLog(INFO, "����[%s]Ǯ���ⶳ���˳ɹ�.", sRrn);
            }
        }
        cJSON_Delete(pstNetJson);
        if (NULL != pstRecvDataJson)
            cJSON_Delete(pstRecvDataJson);
        return 0;
    }
    tLog(INFO, "����[%s]ʧ��,�ⶳǮ��......", sRrn);
    Net2FrozenWallet(pstTransJson, pstNetJson);
    SET_STR_KEY(pstNetJson, "trans_code", "00T500");
    tGetUniqueKey(stQMsgData.sSvrId);
    snprintf(stQMsgData.sKey, sizeof (stQMsgData.sKey), "00T500%s", sRrn);
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall("WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10);
    //iRet = tSvcCall("00T500_Q", pstNetJson, &pstRecvJson, 30);
    if (iRet < 0) {
        if (MSG_TIMEOUT == iRet) {
            ErrHanding(pstTransJson, "96", "����[%s]Ǯ���ⶳ��ʱ.", sRrn);
        } else
            tLog(ERROR, "����[%s]Ǯ���ⶳʧ��.", sRrn);
        cJSON_Delete(pstNetJson);
        return 0;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY(pstRecvDataJson, "resp_code", sRespCode);
    GET_STR_KEY(pstRecvDataJson, "resp_desc", sResvDesc);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "Ǯ���ⶳʧ��[%s:%s].", sRespCode, sResvDesc);
    } else {
        tLog(INFO, "����[%s]Ǯ���ⶳ�ɹ�.", sRrn);
    }
    cJSON_Delete(pstNetJson);
    cJSON_Delete(pstRecvDataJson);
    return 0;
}

/*������ѯ���� ���ؽ�����  ��ѯ���׷��� ֧���ɹ��Ľ�� ���ж�ԭ������ˮ �Ƿ����첽֪ͨ�� ������Ǯ����ֵ�� 
 * ��û�н���Ǯ����ֵ�򱾽������н���Ǯ����ֵ 
 * Ǯ����ֵʱ ��ԭ������ˮ���м������м���  ����˴�����Ǯ����ֵʱ �첽֪ͨҲͬʱ����Ǯ����ֵ �����ظ����ˣ�
 */
int AddQrWalletEx(cJSON *pstTransJson, int *piFlag) {
    char sSqlStr[512 + 1] = {0};
    char sRespCode[2 + 1] = {0},sMerchOrderNo[32 + 1] = {0},sRrn[RRN_LEN + 1] = {0};
    int iCnt = 0;
    
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    GET_STR_KEY(pstTransJson,"rrn",sRrn);
    GET_STR_KEY(pstTransJson,"qr_order_no",sMerchOrderNo);
    
    if (memcmp(sRespCode, "00", 2)) {
        tLog(DEBUG, "����[%s]ʧ��[%s],��Ǯ������.", sRrn, sRespCode);
        return 0;
    }
    
    snprintf(sSqlStr,sizeof(sSqlStr),"select count(1) from b_inline_tarns_detail where merch_order_no = '%s' and settle_flag = 'N' "
            "and trans_code in (select trans_code from s_trans_code where settle_flag = '1') ",sMerchOrderNo);
    iCnt = tQueryCount(sSqlStr);
    if (iCnt < 0) {
        tLog(ERROR, "sSqlStr[%s]", sSqlStr);
        return ( -1);
    }
    if (1 == iCnt) {
        tLog(INFO,"����[%s]���첽֪ͨ������Ǯ��δ����,����Ǯ�����˴���... ###iCnt = [%d]",sMerchOrderNo,iCnt );
        AddQrWallet( pstTransJson, piFlag );
        tLog(INFO,"����Ǯ�����˴�����ϡ�",sMerchOrderNo);
    }else if (0 == iCnt) {
        tLog(WARN, "�˶���[%s]�����첽֪ͨ�����н�����Ǯ����ֵ,�˴�����Ǯ�����˴���###iCnt = [%d]",sMerchOrderNo,iCnt );
    }
    
    return ( 0 );
}