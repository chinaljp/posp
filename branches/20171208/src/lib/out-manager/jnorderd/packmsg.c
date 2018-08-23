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
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "trans_type_tbl.h"


/* ����λͼ */
static unsigned char g_UnOrderJsonpackMap[] = {\
1, 3, 5, 6, 7, 8, 10, 255};
static unsigned char g_OrderJsonpackMap[] = {\
1, 2, 3, 4, 5, 8, 9, 255};
static unsigned char g_UnconJsonpackMap[] = {\
1, 3, 5, 6, 8, 10, 255};
static unsigned char g_conJsonpackMap[] = {\
1, 2, 3, 4, 5, 8, 9, 10, 14, 15, 16, 17, 18, 19, 20, 21, 22, 255};

TranBitMap g_staBitMap[] = {
    /* ���״���   ԭ���״���  ԭԭ������ �����λͼ     �����λͼ        */
    { "M21000", "", "", g_UnOrderJsonpackMap, g_OrderJsonpackMap}, /* ������ѯ */
    { "M20000", "", "", g_UnconJsonpackMap, g_conJsonpackMap}, /* ��������֪ͨ */
    { "M20002", "", "", g_UnconJsonpackMap, g_conJsonpackMap}, /* ��������֪ͨ */
    { "", "", "", NULL, NULL}
};

/* unpack */
int GetFld(int iBitNo, cJSON *pstNetTran, cJSON *pstJson) {
    char sBuf[31] = {0};
    switch (iBitNo) {

        case 1: /* ������ */
            GET_STR_KEY(pstJson, "tranCode", sBuf);
            SET_STR_KEY(pstNetTran, "trans_code", sBuf);
            return iBitNo;

        case 3: /*������*/
            GET_STR_KEY(pstJson, "traceNo", sBuf);
            SET_STR_KEY(pstNetTran, "trace_no", sBuf);
            return iBitNo;

        case 5: /*	������ */
            GET_STR_KEY(pstJson, "orderId", sBuf);
            SET_STR_KEY(pstNetTran, "merch_order_no", sBuf);
            return iBitNo;

        case 6:/* Ӧ����*/
            GET_STR_KEY(pstJson, "responseCode", sBuf);
            SET_STR_KEY(pstNetTran, "resp_code", sBuf);
            return iBitNo;

        case 7: /* Ӧ������ */
            GET_STR_KEY(pstJson, "responseDesc", sBuf);
            SET_STR_KEY(pstNetTran, "resp_desc", sBuf);
            return iBitNo;

        case 8: /* �̻��� */
            GET_STR_KEY(pstJson, "merchantId", sBuf);
            SET_STR_KEY(pstNetTran, "merch_id", sBuf);
            return iBitNo;

        case 10: /* ���׽�� */
            GET_STR_KEY(pstJson, "amount", sBuf);
            SET_STR_KEY(pstNetTran, "amount", sBuf);
            return iBitNo;

        default:
            tLog(ERROR, "���ı�����[%d]������", iBitNo);
            break;
    }

    return ( 0);
}

/* pack */
int SetFld(int iBitNo, cJSON *pstNetTran, cJSON *pstJson) {
    char sBuf[512] = {0}, sBuf1[512] = {0}, sBuf2[512] = {0};
    switch (iBitNo) {

        case 1: /* ������ */
            GET_STR_KEY(pstNetTran, "trans_code", sBuf);
            SET_STR_KEY(pstJson, "tranCode", sBuf);
            return iBitNo;
        case 2: /* ����ʱ�� */
            GET_STR_KEY(pstNetTran, "trans_time", sBuf);
            GET_STR_KEY(pstNetTran, "trans_date", sBuf1);
            sprintf(sBuf2, "%s%s", sBuf1, sBuf);
            SET_STR_KEY(pstJson, "tranTime", sBuf2);
            return iBitNo;
        case 3: /* ������ˮ�� */
            GET_STR_KEY(pstNetTran, "trace_no", sBuf);
            SET_STR_KEY(pstJson, "traceNo", sBuf);
            return iBitNo;
        case 4: /* ����ϵͳ���� */
            GET_STR_KEY(pstNetTran, "acq_id", sBuf);
            SET_STR_KEY(pstJson, "platCode", sBuf);
            return iBitNo;
        case 5: /*  ������ */
            GET_STR_KEY(pstNetTran, "merch_order_no", sBuf);
            SET_STR_KEY(pstJson, "orderId", sBuf);
            return iBitNo;

        case 8: /* �̻��� */
            GET_STR_KEY(pstNetTran, "merch_id", sBuf);
            SET_STR_KEY(pstJson, "merchantId", sBuf);
            return iBitNo;
        case 9: /* �̻����� */
            GET_STR_KEY(pstNetTran, "merch_name", sBuf);
            SET_STR_KEY(pstJson, "merchantName", sBuf);
            return iBitNo;
        case 10: /* ���׽�� */
            GET_STR_KEY(pstNetTran, "amount", sBuf);
            SET_STR_KEY(pstJson, "amount", sBuf);
            return iBitNo;

        case 14: /* ����ʱ��*/
            GET_STR_KEY(pstNetTran, "trans_time", sBuf);
            GET_STR_KEY(pstNetTran, "trans_date", sBuf1);
            sprintf(sBuf2, "%s%s", sBuf1, sBuf);
            SET_STR_KEY(pstJson, "txnTime", sBuf2);
            return iBitNo;
        case 15: /* ��Ȩ�� */
            GET_STR_KEY(pstNetTran, "trans_time", sBuf);
            SET_STR_KEY(pstJson, "authCode", sBuf);
            return iBitNo;
        case 16: /* �����ο��� */
            GET_STR_KEY(pstNetTran, "rrn", sBuf);
            SET_STR_KEY(pstJson, "rrn", sBuf);
            return iBitNo;


        case 21: /* ����ʱ�� */
            GET_STR_KEY(pstNetTran, "trans_time", sBuf);
            SET_STR_KEY(pstJson, "orgTranTime", sBuf);
            return iBitNo;
        case 22: /* ԭϵͳ�ο��� */
            GET_STR_KEY(pstNetTran, "o_rrn", sBuf);
            SET_STR_KEY(pstJson, "orgRrn", sBuf);
            return iBitNo;

        default:
            tLog(ERROR, "��װ�ı�����[%d]������", iBitNo);
    }

    return ( 0);
}

//int ModuleUnpack(void *pvNetTran, char *pcMsg, int iMsgLen) {

int ResponseMsg(cJSON *pstRepDataJson, cJSON *pstDataJson) {

    int i = 0, iRet = 0;
    UCHAR *pcBitMap = NULL;
    cJSON *pstJson = NULL;
    char sTransCode[7] = {0}, sJsMsg[4096] = {0}, sOutMsg[4096] = {0};

    /* ��ӡԭʼ���� */
    GET_STR_KEY(pstDataJson, "msg", sJsMsg);
    iRet = strlen(sJsMsg);
    tAsc2Bcd((UCHAR*) sOutMsg, (UCHAR*) sJsMsg, iRet, LEFT_ALIGN);
    pstJson = cJSON_Parse(sOutMsg);
    GET_STR_KEY(pstJson, "tranCode", sTransCode);

    tLog(DEBUG, "������[%s]", sTransCode);

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
            tLog(ERROR, "��⽻��[%s]������[%d]����[%d]", sTransCode, pcBitMap[i], iRet);
            return ( iRet);
        }

        tLog(DEBUG, "��⽻��[%s]������[%d][%d]", sTransCode, pcBitMap[i], iRet);
    }
    return 0;
}
/* ���ڷ���: ���������; ��ڷ���: ���Ӧ����. */
//int ModulePack(void *pvNetTran, char *pcMsg, int *piMsgLen) {

int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {
    int i, iRet;
    UCHAR *pcBitMap;
    char sJsMsg[4096] = {0}, sTransCode[7] = {0}, sOutMsg[4096] = {0}, *pcMsg = NULL;
    cJSON * pstOutMsg = NULL;

    pstOutMsg = cJSON_CreateObject();
    if (NULL == pstOutMsg) {
        tLog(ERROR, "��������Jsonʧ��.");
        cJSON_Delete(pstOutMsg);
        return -1;
    }
    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);

    tLog(DEBUG, "������[%s] ", sTransCode);

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
    //sleep(5);
    /* ����Bit Map��׼�������������� */
    for (i = 0; pcBitMap[i] != 255; i++) {
        iRet = SetFld(pcBitMap[i], pstDataJson, pstOutMsg);
        if (iRet < 0) {
            tLog(ERROR, "��װ����[%s]������[%d]����[%d]!", sTransCode, pcBitMap[i], iRet);
            return ( iRet);
        }
        tLog(ERROR, "��װ����[%s]������[%d][%d]", sTransCode, pcBitMap[i], iRet);
    }

    /* ��json��ʽת���ַ��� */
    pcMsg = cJSON_PrintUnformatted(pstOutMsg);
    strcpy(sJsMsg, pcMsg);
    if (pcMsg)
        free(pcMsg);
    iRet = strlen(sJsMsg);
    tBcd2Asc((UCHAR*) sOutMsg, (UCHAR*) sJsMsg, iRet << 1, LEFT_ALIGN);
    SET_STR_KEY(pstReqJson, "msg", sOutMsg);
    cJSON_Delete(pstOutMsg);
    return 0;
}





