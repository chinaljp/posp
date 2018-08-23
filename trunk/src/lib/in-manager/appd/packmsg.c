#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include "t_extiso.h"
#include "t_tools.h"
#include "trans_type_tbl.h"


#include "t_redis.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "trans_code.h"



/*************
 **  ������  **
 *************/

/* ʵ����֤ */
static unsigned char g_AuthenUnpackMap[] = {\
1, 2, 11, 22, 25, 26, 32, 38, 42, 255};

static unsigned char g_AuthenPackMap[] = {\
11, 12, 13, 37, 39, 63, 255};

/* �ս��Ȳ�ѯ */
static unsigned char g_QuotaUnpackMap[] = {\
1, 11, 42, 255};
static unsigned char g_QuotaPackMap[] = {\
11, 12, 13, 37, 39, 42, 49, 53, 63, 255};

/* �ս�����֪ͨ */
static unsigned char g_caUndoPurchaseUnpackMap[] = {\
1, 11, 37, 41, 42, 60, 255};
static unsigned char g_caUndoPurchasePackMap[] = {\
11, 12, 13, 37, 39, 42, 63, 255};


/* ��ɨ���� */
static unsigned char g_UnActivescanpackMap[] = {\
1, 4, 6, 11, 42, 44, 58, 62, 255};
static unsigned char g_ActivescanpackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* ��ɨ���� */
static unsigned char g_UnPassivescanpackMap[] = {\
1, 4, 8,11, 42, 44, 58, 62, 255};
static unsigned char g_PassivescanpackMap[] = {\
4, 5, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* ��ѯ */
static unsigned char g_UnQueryresultpackMap[] = {\
1, 6, 11, 42, 44, 255};
static unsigned char g_QueryresultpackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* ��ά��֧������ */
static unsigned char g_UnTdRevokepackMap[] = {\
1, 4, 6, 11, 42, 44, 58, 255};
static unsigned char g_TdRevokepackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* �˿� */
static unsigned char g_UnRefundpackMap[] = {\
1, 4, 6, 11, 42, 44, 58, 255};
static unsigned char g_RefundpackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* �˿��ѯ */
static unsigned char g_UnQueryRefundpackMap[] = {\
1, 6, 11, 37, 42, 44, 255};
static unsigned char g_QueryRefundpackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/* �������� */
static unsigned char g_UnRevokepackMap[] = {\
1, 4, 6, 11, 42, 44, 58, 255};
static unsigned char g_RevokepackMap[] = {\
4, 6, 11, 12, 13, 37, 39, 42, 44, 63, 255};

/*EPOS �޿��������*/
static unsigned char g_UnQuickPaypackMap[] = {\
1, 3, 4, 42, 255};
static unsigned char g_QuickPaypackMap[] = {\
37, 39, 42, 63, 255};
/*EPOS ���Ѳ�ѯ����ѯ֧���Ƿ�ɹ�)*/
static unsigned char g_UnQuickPayRefundpackMap[] = {\
1, 4, 6, 37, 42, 255};
static unsigned char g_QuickPayRefundpackMap[] = {\
1, 4, 6, 12, 13, 37, 39, 42, 60, 61, 62,63, 255};




TranBitMap g_staBitMap[] = {
    /* ������   ��Ϣ������ �����λͼ                      �����λͼ                   */
    { "0AA000", "", g_AuthenUnpackMap, g_AuthenPackMap}, /* �̻�ʵ����֤ */
    { "0AA001", "", g_AuthenUnpackMap, g_AuthenPackMap}, /* ����ʵ����֤ */
    { "0AA002", "", g_AuthenUnpackMap, g_AuthenPackMap}, /* ���п�ʵ����֤ */
    { "0AA100", "", g_QuotaUnpackMap, g_QuotaPackMap}, /* �ս��޶��ѯ  */

    { "0AB100", "", g_UnActivescanpackMap, g_ActivescanpackMap}, /* ֧������ɨģʽ */
    { "0AB200", "", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* ֧������ɨģʽ */
    { "0AB300", "", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* ֧����֧����ѯ */
    { "0AB600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap}, /* ֧����֧������ */
    
    //   { "AS0004", "60", g_UnRefundpackMap, g_RefundpackMap}, /* �˿� */
    //   { "AS0005", "60", g_UnQueryRefundpackMap, g_QueryRefundpackMap}, /* �˿��ѯ */
    //  { "AS0006", "60", g_UnRevokepackMap, g_RevokepackMap}, /* �������� */

    { "0AW100", "", g_UnActivescanpackMap, g_ActivescanpackMap}, /* ΢����ɨģʽ */
    { "0AW200", "", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* ΢�ű�ɨģʽ */
    { "0AW300", "", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* ΢��֧����ѯ */
    { "0AW600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap}, /* ΢��֧������ */
    
    //    { "AS0004", "60", g_UnRefundpackMap, g_RefundpackMap}, /* �˿� */
    //    { "AS0005", "60", g_UnQueryRefundpackMap, g_QueryRefundpackMap}, /* �˿��ѯ */
    //    { "AS0006", "60", g_UnRevokepackMap, g_RevokepackMap}, /* �������� */

    { "TA0010", "", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* �ս�����ȷ�� */
    
    { "0AY100", "", g_UnActivescanpackMap, g_ActivescanpackMap}, /* ������ά�뱻ɨɨģʽ app��֧�ֲ����*/
    { "0AY200", "", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* ������ά����ɨģʽ */
    { "0AY300", "", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* ������ά��֧����ѯ */
    { "0AY600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap}, /* ������ά��֧������ app��֧�ֲ���� */
    { "0AQ000", "", g_UnQuickPaypackMap, g_QuickPaypackMap}, /*Epos �޿��������*/
    { "0AQ300", "", g_UnQuickPayRefundpackMap, g_QuickPayRefundpackMap}, /*Epos �޿�������Ѳ�ѯ*/
    { "", "", NULL, NULL}
};

int GetFld(int iBitNo, cJSON *pstNetTran, cJSON *jsonStr) {
    char sBuf[1024] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0};
    
    switch (iBitNo) {

        case 1: /* ������ */
            GET_STR_KEY(pstNetTran, "transCode", sBuf);
            SET_STR_KEY(jsonStr, "trans_code", sBuf);
            break;

        case 2: /* ��Ȩ���� */
            GET_STR_KEY(pstNetTran, "accountNo", sBuf);
            SET_STR_KEY(jsonStr, "card_no", sBuf);
            break;
        case 3: /* card_id ����޿����� ���ڼ������׿���Ϣ*/
            GET_STR_KEY(pstNetTran, "cardId", sBuf);
            SET_STR_KEY(jsonStr, "id", sBuf);
            break;
        case 4: /* ���׽�� */
            GET_STR_KEY(pstNetTran, "amount", sBuf);
            SET_DOU_KEY(jsonStr, "amount", atol(sBuf));
            break;

            /* ������ */
        case 6:/* ������ */
            GET_STR_KEY(pstNetTran, "transCode", sTransCode);
            GET_STR_KEY(pstNetTran, "orderNum", sBuf);
            if (!memcmp(sTransCode, "0AW100", 6) || !memcmp(sTransCode, "0AB100", 6)) {
                tLog(DEBUG,"===============seed[%s]",sBuf);
                SET_STR_KEY(jsonStr, "seed", sBuf);
            } else {
                tLog(DEBUG,"===============qr_order_no[%s]",sBuf);
                SET_STR_KEY(jsonStr, "qr_order_no", sBuf);
            }
            break;
        /*Begin add by GuoJiaQing at 20180328 app������ά����ɨ ����֧�����֪ͨ��ַ*/
        case 8:/* ����֧�����֪ͨ��ַ notifyUrl */
            GET_STR_KEY(pstNetTran, "notifyUrl", sBuf);
            SET_STR_KEY(jsonStr, "callback_url", sBuf);
            break;
        /*End add by GuoJiaQing at 20180328 app������ά����ɨ ����֧�����֪ͨ��ַ*/
        case 11: /* �ն���ˮ�� */
            GET_STR_KEY(pstNetTran, "sellCardRRN", sBuf);
            SET_STR_KEY(jsonStr, "trace_no", sBuf);
            break;

        case 22: /* ������ */
            GET_STR_KEY(pstNetTran, "accountType", sBuf);
            SET_STR_KEY(jsonStr, "card_type", "2");
            break;

        case 25: /* ��ȨԤ���ֻ��� */
            GET_STR_KEY(pstNetTran, "idTel", sBuf);
            SET_STR_KEY(jsonStr, "mobile", sBuf);
            break;

        case 26: /* ��Ȩ���֤�� */
            GET_STR_KEY(pstNetTran, "idNum", sBuf);
            SET_STR_KEY(jsonStr, "cer_no", sBuf);
            break;

        case 32: /* ��Ȩ���� */
            GET_STR_KEY(pstNetTran, "accountName", sBuf);
            SET_STR_KEY(jsonStr, "account_name", sBuf);
            tLog(INFO, "ʵ������[%s]", sBuf);
            break;

        case 37: /* �÷�1: �յ�ϵͳ��ˮ��(ԭ������ˮ��) */
            GET_STR_KEY(pstNetTran, "transCode", sTransCode);
            if( !memcmp("0AQ300",sTransCode,6) ) {
                GET_STR_KEY(pstNetTran, "indexRRN", sBuf);
                SET_STR_KEY(jsonStr, "o_rrn", sBuf);
            }
            else {
                GET_STR_KEY(pstNetTran, "sOldRrn", sBuf);
                SET_STR_KEY(jsonStr, "o_rrn", sBuf);
            }
            break;

        case 38: /* ֤������ */
            GET_STR_KEY(pstNetTran, "idType", sBuf);
            SET_STR_KEY(jsonStr, "idType", sBuf);
            break;

        case 41: /* �ն˱�� */
            GET_STR_KEY(pstNetTran, "termId", sBuf);
            SET_STR_KEY(jsonStr, "term_id", sBuf);
            break;

        case 42: /* �̻���� */
            GET_STR_KEY(pstNetTran, "cardTermCode", sBuf);
            SET_STR_KEY(jsonStr, "merch_id", sBuf);
            break;
            
        case 44: /* ֧������ 001���� 002΢�� 003֧���� */
            GET_STR_KEY(pstNetTran, "attachContent", sBuf);
            SET_STR_KEY(jsonStr, "input_module", sBuf);
            break;

        case 58: /* �������� */
            GET_STR_KEY(pstNetTran, "orderTitle", sBuf);
            SET_STR_KEY(jsonStr, "order_header", sBuf);
            tLog(DEBUG, "��������[%s].", sBuf);
            break;

        case 60: /* ԭ������ˮ�� */
            GET_STR_KEY(pstNetTran, "oldCardRRN", sBuf);
            SET_STR_KEY(jsonStr, "o_trace_no", sBuf);
            break;

        case 62: /* �������� */
            GET_STR_KEY(pstNetTran, "orderDesc", sBuf);
            SET_STR_KEY(jsonStr, "private_data", sBuf);
            tLog(DEBUG, "62[%s].", sBuf);
            break;

        default:
            tLog(ERROR, "���ı�����[%d]������", iBitNo);
            break;
    }

    return ( 0);
}

/* pack */
int SetFld(int iBitNo, cJSON *pstNetTran, cJSON *pstOutJson) {
    char sBuf[1024] = {0}, sTmp[512] = {0};
    int iBuf = 0;
    double dAmount = 0.0;
    switch (iBitNo) {
        case 1: /* ������ */
            GET_STR_KEY(pstNetTran, "trans_code", sBuf);
            SET_STR_KEY(pstOutJson, "transCode", sBuf);
            return iBitNo;
            
        case 4: /* ���׽�� */

            GET_DOU_KEY(pstNetTran, "amount", dAmount);
            snprintf(sTmp, sizeof (sTmp), "%012.f", dAmount);
            SET_STR_KEY(pstOutJson, "amount", sTmp);

            return iBitNo;

        case 5: /* ������url */
            GET_STR_KEY(pstNetTran, "code_url", sBuf);
            SET_STR_KEY(pstOutJson, "codeurl", sBuf);
            return iBitNo;

        case 6: /* ������ */
            GET_STR_KEY(pstNetTran, "qr_order_no", sBuf);
            SET_STR_KEY(pstOutJson, "orderNum", sBuf);
            return iBitNo;

        case 11: /* �ն���ˮ�� */
            GET_STR_KEY(pstNetTran, "trace_no", sBuf);
            SET_STR_KEY(pstOutJson, "sellCardRRN", sBuf);
            return iBitNo;

        case 12: /* ����ʱ�� */
            GET_STR_KEY(pstNetTran, "trans_time", sBuf);
            SET_STR_KEY(pstOutJson, "sellCardTime", sBuf);
            return iBitNo;

        case 13: /* �������� */
            GET_STR_KEY(pstNetTran, "trans_date", sBuf);
            SET_STR_KEY(pstOutJson, "sellCardDate", sBuf);
            return iBitNo;

        case 37: /* �����ο��� */
            GET_STR_KEY(pstNetTran, "rrn", sBuf);
            SET_STR_KEY(pstOutJson, "indexRRN", sBuf);
            return iBitNo;

        case 39: /* Ӧ���� */
            GET_STR_KEY(pstNetTran, "resp_code", sBuf);
            SET_STR_KEY(pstOutJson, "respCode", sBuf);
            return iBitNo;


        case 42: /* �̻���� */
            GET_STR_KEY(pstNetTran, "merch_id", sBuf);
            SET_STR_KEY(pstOutJson, "cardTermCode", sBuf);
            return iBitNo;

        case 44: /* ֧������ 001���� 002΢�� 003֧���� */
            GET_STR_KEY(pstNetTran, "input_module", sBuf);
            SET_STR_KEY(pstOutJson, "attachContent", sBuf);
            return iBitNo;

        case 49: /* �̻��ս��� */
            GET_STR_KEY(pstNetTran, "total_limit", sBuf);
            SET_STR_KEY(pstOutJson, "maxPosition", sBuf);
            return iBitNo;

        case 53: /* �̻����ö�� */
            GET_STR_KEY(pstNetTran, "usable_limit", sBuf);
            SET_STR_KEY(pstOutJson, "currentPosition", sBuf);
            return iBitNo;
            
        case 60: /* ֧�����Ӧ���� */
            GET_STR_KEY(pstNetTran, "o_resp_code", sBuf);
            SET_STR_KEY(pstOutJson, "oriRpCd", sBuf);
            return iBitNo;
        case 61: /* ֧�����Ӧ������ */
            GET_STR_KEY(pstNetTran, "o_resp_desc", sBuf);
            SET_STR_KEY(pstOutJson, "oriRpDesc", sBuf);
            return iBitNo;
        case 62: /* ����״̬ */
            MEMSET(sBuf);
            GET_INT_KEY(pstNetTran, "tx_state", iBuf);
            if ( iBuf == 2 ) {
                tStrCpy(sBuf,"00",2);
            }
            else if ( iBuf == 0 || iBuf == 1 || iBuf == 3 ) {
                tStrCpy(sBuf,"FD",2);
            }
            else {
                tStrCpy(sBuf,"96",2);
            }
            SET_STR_KEY(pstOutJson, "oriTrxStatus", sBuf);
            return iBitNo;
            
        case 63: /* Ӧ������ */
            GET_STR_KEY(pstNetTran, "resp_desc", sBuf);
            SET_STR_KEY(pstOutJson, "respDesc", sBuf);
            return iBitNo;

        default:
            tLog(ERROR, "��װ�ı�����[%d]������", iBitNo);
    }

    return 0;
}

//int ModuleUnpack(char *pcMsg, int iMsgLen, void *pvNetTran) {

int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {
    int i, iRet;
    char sTransCode[7] = {0}, *pcMsg = NULL;
    UCHAR *pcBitMap;
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0}, sTrace[6 + 1] = {0}, sTmp[128] = {0};
    cJSON *pstJson = NULL;
    UCHAR sMsg[MSG_MAX_LEN] = {0};
    TransCode stTransCode;
    GET_STR_KEY(pstDataJson, "msg", sMsg);

    int iMsgLen = strlen((const char *) sMsg);
    pstJson = cJSON_Parse((const char *) sMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "ת����Ϣʧ��,��������.");
        return -1;
    }

    /* ��ӡԭʼ���� */
    DUMP_JSON(pstJson, pcMsg);
    /* ��ȡjsonStr���� */

    GET_STR_KEY(pstJson, "transCode", sTransCode);

    tLog(DEBUG, "������transcode=[%s]", sTransCode);

    /* ȡ�ý��׵�Bit Map��׼���� */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if (0 == memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)) {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }
    tLog(DEBUG, "TransCode=[%s]", g_staBitMap[i].sTransCode);

    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "����[%s]δ������λͼ.", sTransCode);
        return -1;
    }

    /* ����Bit Map��׼�������������� */
    for (i = 0; pcBitMap[i] != 255; i++) {
        iRet = GetFld(pcBitMap[i], pstJson, pstReqJson);
        if (iRet < 0) {
            tLog(ERROR, "��⽻��[%s]������[%d]����[%d]!"
                    , sTransCode, pcBitMap[i], iRet);
            return ( iRet);
        }
        tLog(DEBUG, "��⽻��[%s]������[%d]", sTransCode, pcBitMap[i]);
    }

    {
        /* ��ȡ�������ڣ�ʱ�䣬rrn��ÿһ�����׶��Ǵ�manager��ͳ��� */
        tGetDate(sDate, "", -1);
        tGetTime(sTime, "", -1);
        snprintf(sTmp, sizeof (sTmp), "%s%s", sDate + 5, sTime);
        SET_STR_KEY(pstReqJson, "trans_date", sDate);
        SET_STR_KEY(pstReqJson, "trans_time", sTime);
        sprintf(sTmp, "%s%s", sDate + 4, sTime);
        SET_STR_KEY(pstReqJson, "transmit_time", sTmp);
        if (GetSysTrace(sTrace) < 0) {
            tLog(ERROR, "��ȡϵͳ��ˮ��ʧ��,���׷���.");
            return -1;
        }
        sprintf(sTmp, "%s%s", sTime, sTrace);
        SET_STR_KEY(pstReqJson, "rrn", sTmp);
        SET_STR_KEY(pstReqJson, "sys_trace", sTrace);
        SET_STR_KEY(pstReqJson, "logic_date", sDate);
    }

    if (FindTransCode(&stTransCode, sTransCode) < 0) {
        tLog(ERROR, "������δ����,���׷�������.");
        return -1;
    }
    if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
        if (!strcmp(stTransCode.sGroupCode, POS_GROUP)) {
            if (AddPosTransLs(pstReqJson) < 0) {
                tLog(ERROR, "��ˮ��¼ʧ��,���׷�������.");
                return -1;
            }
        } else if (!strcmp(stTransCode.sGroupCode, INLINE_GROUP)) {
            if (AddInlineTransLs(pstReqJson) < 0) {
                tLog(ERROR, "��ˮ��¼ʧ��,���׷�������.");
                return -1;
            }
        }
    }

    return 0;
}

//int ModulePack(char *pcMsg, int *piMsgLen, void *pvNetTran) {

int ResponseMsg(cJSON *pstRepDataJson, cJSON *pstDataJson) {
    int i, iRet;
    unsigned char *pcBitMap;
    char sTransCode[7] = {0}, *pcMsg = NULL;
    cJSON * pstOutMsg = NULL;
    TransCode stTransCode;
    pstOutMsg = cJSON_CreateObject();
    if (NULL == pstOutMsg) {
        tLog(ERROR, "��������Jsonʧ��.");
        return -1;
    }
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);

    /* �����ˮ ���������λ����ĸ��ʹ��INLINE_TRANS_DETAIL�����ֵ�ʹ��POS_TRANS_DETAIL*/
    if (FindTransCode(&stTransCode, sTransCode) < 0) {
        tLog(ERROR, "������δ����,���׷�������.");
    }
    /* ������ˮ ���������λ����ĸ��ʹ��INLINE_TRANS_DETAIL�����ֵ�ʹ��POS_TRANS_DETAIL*/
    if (TRUE_FLAG == stTransCode.sLogFlag[0]) {
        if (!strcmp(stTransCode.sGroupCode, POS_GROUP)) {
            UpdPosTransLs(pstDataJson);
        } else if (!strcmp(stTransCode.sGroupCode, INLINE_GROUP)) {
            UpdInlineTransLs(pstDataJson);
        }
    }
    /* ȡ�ý��׵�Bit Map��׼���� */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if (0 == memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)) {
            pcBitMap = g_staBitMap[i].pcPackBitmap;
            break;
        }
    }
    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "����[%s]δ�������λͼ.", sTransCode);
        return -1;
    }
    /* ����Bit Map��׼�������������� */
    for (i = 0; pcBitMap[i] != 255; i++) {
        iRet = SetFld(pcBitMap[i], pstDataJson, pstOutMsg);
        if (iRet < 0) {
            tLog(ERROR, "��װ����[%s]������[%d]����[%d]!", sTransCode, pcBitMap[i], iRet);
            return ( iRet);
        }
        tLog(DEBUG, "��װ����[%s]������[%d]", sTransCode, pcBitMap[i]);
    }
    DUMP_JSON(pstOutMsg, pcMsg);
    pcMsg = cJSON_PrintUnformatted(pstOutMsg);
    SET_STR_KEY(pstRepDataJson, "msg", (const char*) pcMsg);
    if (pcMsg)
        free(pcMsg);
    cJSON_Delete(pstOutMsg);
    return ( 0);
}

