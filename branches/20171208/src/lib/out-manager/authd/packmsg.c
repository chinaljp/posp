/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "t_extiso.h"
#include "t_tools.h"
#include "t_redis.h"
#include "t_signal.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "err.h"
#include "trans_type_tbl.h"

/* ����λͼ */

static unsigned char g_UnAuthenpackMap[] = {\
25, 34, 35, 36, 255};
static unsigned char g_AuthenpackMap[] = {\
25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 255};

TranBitMap g_staBitMap[] = {
    /* ���״���   ԭ���״���  ԭԭ������ �����λͼ                       �����λͼ                      */
    { "02A000", "", "", g_UnAuthenpackMap, g_AuthenpackMap}, /* posʵ����Ȩ */
    { "0AA000", "", "", g_UnAuthenpackMap, g_AuthenpackMap}, /* appʵ����Ȩ */
    { "0AA001", "", "", g_UnAuthenpackMap, g_AuthenpackMap}, /* app����ʵ����Ȩ */
    { "0AA002", "", "", g_UnAuthenpackMap, g_AuthenpackMap}, /* app��ʵ����Ȩ */
    /*APP */
    { "0AA000", "", "", g_UnAuthenpackMap, g_AuthenpackMap}, /* ʵ����Ȩ */
    { "", "", "", NULL, NULL}
};

/* unpack */
int GetFld(int iBitNo, cJSON *pstTranJson, cJSON * pstJson) {

    cJSON *pjson;

    char strandtime[14 + 1] = {0};
    switch (iBitNo) {

        case 25: /* ������ */

            pjson = cJSON_GetObjectItem(pstJson, "tranCode");
            if (NULL == pjson) return -1;
            SET_STR_KEY(pstTranJson, "trans_code", pjson->valuestring);
            return iBitNo;

        case 26: /* �û�id */

            pjson = cJSON_GetObjectItem(pstJson, "userId");
            if (NULL == pjson) return -1;
            SET_STR_KEY(pstTranJson, "user_id", pjson->valuestring);
            return iBitNo;

        case 34: /* rrn */

            pjson = cJSON_GetObjectItem(pstJson, "rrn");
            if (NULL == pjson) return -1;
            SET_STR_KEY(pstTranJson, "rrn", pjson->valuestring);
            return iBitNo;

        case 35: /* ��Ȩ������ */

            pjson = cJSON_GetObjectItem(pstJson, "returnCode");
            if (NULL == pjson) return -1;
            SET_STR_KEY(pstTranJson, "resp_code", pjson->valuestring);
            return iBitNo;

        case 36: /* ������Ϣ */

            pjson = cJSON_GetObjectItem(pstJson, "returnMsg");
            if (NULL == pjson) return -1;
            SET_STR_KEY(pstTranJson, "resp_desc", pjson->valuestring);
            return iBitNo;

        default:
            tLog(ERROR, "���ı�����[%d]������", iBitNo);
            break;
    }

    return ( 0);
}

/* pack */
int SetFld(int iBitNo, cJSON *pstTranJson, cJSON *pstOutJson) {

    char sTranTime[14 + 1] = {0};
    char card_no[19 + 1] = {0};
    char txnTime[14 + 1] = {0};
    char sCardtype[2 + 1] = {0};
    char sBuf[50] = {0};
    switch (iBitNo) {

        case 25: /* trans_code */
            GET_STR_KEY(pstTranJson, "trans_code", sBuf);
            SET_STR_KEY(pstOutJson, "tranCode", sBuf);

        case 26: /* user_id */
            GET_STR_KEY(pstTranJson, "user_id", sBuf);
            SET_STR_KEY(pstOutJson, "userId", sBuf);
            return iBitNo;
        case 27: /* ����ͨ������ */
            GET_STR_KEY(pstTranJson, "bankGeneralName", sBuf);
            SET_STR_KEY(pstOutJson, "bankGeneralName", sBuf);
            return iBitNo;
        case 28: /* �������� */
            GET_STR_KEY(pstTranJson, "bankName", sBuf);
            SET_STR_KEY(pstOutJson, "bankName", sBuf);

            return iBitNo;
        case 29: /* ���п����� */
            GET_STR_KEY(pstTranJson, "card_type", sCardtype);
            if (!memcmp(sCardtype, "1", 1)) {
                SET_STR_KEY(pstOutJson, "accountType", "02");
            } else {
                SET_STR_KEY(pstOutJson, "accountType", "01");
            }
            return iBitNo;
        case 30: /*  ���п��� */
            GET_STR_KEY(pstTranJson, "card_no", sBuf);
            SET_STR_KEY(pstOutJson, "accountNo", sBuf);
            return iBitNo;
        case 31: /* �˻����� */
            GET_STR_KEY(pstTranJson, "account_name", sBuf);
            SET_STR_KEY(pstOutJson, "accountName", sBuf);
            return iBitNo;
        case 32: /* ֤������ */
            //	GET_STR_KEY(pstTranJson, "idType", pcBuf); 
            SET_STR_KEY(pstOutJson, "idType", "0");
            return iBitNo;
        case 33: /* ֤���� */
            GET_STR_KEY(pstTranJson, "cer_no", sBuf);
            SET_STR_KEY(pstOutJson, "idNum", sBuf);
            return iBitNo;
        case 34: /* �ֻ��� */
            GET_STR_KEY(pstTranJson, "mobile", sBuf);
            SET_STR_KEY(pstOutJson, "idTel", sBuf);
            return iBitNo;
        case 35: /* rrn */
            GET_STR_KEY(pstTranJson, "rrn", sBuf);
            SET_STR_KEY(pstOutJson, "rrn", sBuf);
            return iBitNo;
        default:
            tLog(ERROR, "��װ�ı�����[%d]������", iBitNo);
    }

    return ( 0);
}



//int ModuleUnpack(void *pvNetTran, char *pcMsg, int iMsgLen ){

int ResponseMsg(cJSON *pstRepDataJson, cJSON *pstDataJson) {

    int i, iRet, iLenField, iLen;
    char sMsgType[5], sProcCode[7], sProcCodeEx[3], sNmiCode[4];
    char caBitMapOrg[BITMAP_BYTE];
    char sErr[128], sPktDesc[32];
    char *sBuf;
    UCHAR *pcBitMap, sBitMapOrg[BITMAP_SIZE + 1];
    cJSON *pstJson, *pjson, *unPack1;
    char *sJsondate;
    char sJson[1024] = {0}, sTransCode[7] = {0};
    char sJsMsg[4096] = {0}, sOutMsg[4096] = {0}, sInMsg[4096] = {0};
    /* ��ӡԭʼ���� */
    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "����ת��Jsonʧ��.");
        cJSON_Delete(pstJson);
        return -1;
    }
    tLog(DEBUG, "���յ�����:%s", pstDataJson);
    /* ��ӡԭʼ���� */
    GET_STR_KEY(pstDataJson, "msg", sJsMsg);
    iRet = strlen(sJsMsg);
    tAsc2Bcd((UCHAR*) sOutMsg, (UCHAR*) sJsMsg, iRet << 1, LEFT_ALIGN);
    tLog(DEBUG, "���յ�����:%s", sOutMsg);
    pstJson = cJSON_Parse(sOutMsg);
    pjson = cJSON_GetObjectItem(pstJson, "tranCode");
    if (NULL == pjson) {
        tLog(ERROR, "��ȡ���������");
        return -1;
    }
    tStrCpy(sTransCode, pjson->valuestring, 6);
    tLog(DEBUG, "������:%s", sTransCode);


    /* ȡ�ý��׵�Bit Map��׼���� */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++) {
        if (0 == memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)) {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0') {
        tLog(ERROR, "����[%s]δ������λͼ.", sTransCode);
        return -1;
    }


    /* ����Bit Map��׼�������������� */
    for (i = 0; pcBitMap[i] != 255; i++) {
        iRet = GetFld(pcBitMap[i], pstRepDataJson, pstJson);
        if (iRet < 0) {
            tLog(ERROR, "��⽻��[%s]������[%d]����[%d]"
                    , sTransCode, pcBitMap[i], iRet);
            return ( iRet);
        }

        tLog(DEBUG, "��⽻��[%s]������[%d][%d]", sTransCode, pcBitMap[i], iRet);
    }
    return 0;
}

/* ���ڷ���: ���������; ��ڷ���: ���Ӧ����. */
//int ModulePack(void *pvNetTran, char *pcMsg, int *piMsgLen ) {

int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {

    int i, iRet;
    UCHAR *pcBitMap;
    char sJsMsg[4096] = {0}, sOutMsg[4096] = {0}, *pcMsg = NULL;
    cJSON * pstOutMsg = NULL;
    char sTransCode[7] = {0};
    pstOutMsg = cJSON_CreateObject();
    if (NULL == pstOutMsg) {
        tLog(ERROR, "����ת��Jsonʧ��.");
        cJSON_Delete(pstOutMsg);
        return -1;
    }
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
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
        tLog(ERROR, "��װ����[%s]������[%d][%d]"
                , sTransCode, pcBitMap[i], iRet);
    }
    /* ��json��ʽת���ַ��� */
    pcMsg = cJSON_PrintUnformatted(pstOutMsg);
    strcpy(sJsMsg, pcMsg);
    if (pcMsg)
        free(pcMsg);
    iRet = strlen(sJsMsg);
    tBcd2Asc((UCHAR*) sOutMsg, (UCHAR*) sJsMsg, iRet << 1, LEFT_ALIGN);
    SET_STR_KEY(pstReqJson, "msg", sOutMsg);
    return 0;
}


