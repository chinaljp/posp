/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2017??2??27??, ????7:52
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "t_app_conf.h"
#include "trace.h"

/*
 * 
 */

/* �жϷ��ؽ�� */
int JudgeResult(char *pcMsg) {

    char sTpkuZmk[33] = {0}, sTpkChk[17] = {0}, sTpkuLmk[33] = {0};
    char sTakuZmk[33] = {0}, sTakChk[17] = {0}, sTakuLmk[33] = {0};
    char sTdkuZmk[33] = {0}, sTdkChk[17] = {0}, sTdkuLmk[33] = {0};
    char sZmkuLmk[33] = {0};
    char sTmkuLmk[33] = {0}, sTmkChk[17] = {0};
    char sPinuZmk[33] = {0}, sPinChk[9] = {0}, sMacChk[9] = {0}, sTmp[64] = {0};
    char sPinuLmk[33] = {0}, sMacuLmk[17] = {0}, sOutTpkChk[16 + 1] = {0}, sOutTakChk[16 + 1] = {0};
    char a[33] = {0}, b[33] = {0}, c[33] = {0}, d[33] = {0};
    char sErr[128] = {0}, sChannelId[20 + 1] = {0}, sKeyName[5] = {0};
    char sChannelDesKey[100] = {0}, sChannelTermId[8 + 1] = {0}, sChannelMerchId[15 + 1] = {0};
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char *pcTranMsg = NULL;
    char sRespCode[2 + 1] = {0};
    pstJson = cJSON_Parse(pcMsg);
    if (NULL == pstJson) {
        tLog(ERROR, "��ȡ��Ϣʧ��,��������.");
        return -1;
    }
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    if (NULL == pstDataJson) {
        tLog(ERROR, "��ȡdata����ʧ��,��Ϣ��������.");
        cJSON_Delete(pstJson);
        return -1;
    }

    DUMP_JSON(pstDataJson, pcTranMsg);

    GET_STR_KEY(pstDataJson, "istresp_code", sRespCode);


    if (memcmp(sRespCode, "00", 2) == 0) {

        strcpy(sChannelId, "48560000");
        GET_STR_KEY(pstDataJson, "pufa_inst_key", sChannelDesKey);
        GET_STR_KEY(pstDataJson, "channel_term_id", sChannelTermId);

        if (GetPufaZmKey(sZmkuLmk, sChannelId) < 0) {
            ErrHanding(pstDataJson, "96", "ȡ�ַ�ZMK[%s]ʧ��.", sZmkuLmk);
            return -1;
        }
        tLog(DEBUG, "Zmk[%s].", sZmkuLmk);


        tLog(DEBUG, "Inst[%s],ZmkuLmk[%s].", sChannelId, sZmkuLmk);

        strncpy(sTpkuZmk, sChannelDesKey, 32);
        strncpy(sTpkChk, sChannelDesKey + 32, 8);

        tLog(DEBUG, "TpkuZmk[%s],Chk[%s].", sTpkuZmk, sTpkChk);

        strncpy(sTakuZmk, sChannelDesKey + 32 + 8, 16);
        strncpy(sTakChk, sChannelDesKey + 32 + 8 + 16, 8);

        tLog(DEBUG, "TakuZmk[%s],Chk[%s].", sTakuZmk, sTakChk);

        /* ���ȵ����ն���Կ����,����ȡ��Կֵ����ǩ��֮ǰ��֤��Կ���ƴ��ڣ� */
        {
            sprintf(sPinuLmk, "pos.pfff%s.zpk", sChannelTermId);
            if (tHsm_Gen_Zpk(sPinuLmk, sZmkuLmk, a, b) != 0) {
                tLog(INFO, "���ܻ�����TPKʧ��.");
                return -1;
            }
            sprintf(sMacuLmk, "pos.pfff%s.zak", sChannelTermId);
            if (tHsm_Gen_Cpus_Zak(sMacuLmk, sZmkuLmk, c, d) != 0) {
                tLog(INFO, "���ܻ�����TAKʧ��.");
                return -1;
            }
        }

        if (tHsm_Imp_Key(sPinuLmk, sTpkuZmk, sOutTpkChk, sZmkuLmk)) {
            ErrHanding(pstDataJson, "A7", "�ַ�ZPKתLMK����ʧ��[%s]ʧ��.", sPinuLmk);
            return -1;
        }
        tLog(INFO, "sOutTpkChk[%s]", sOutTpkChk);
        if (UpdPufaChannelKey(sPinuLmk, sChannelTermId, "ZPK") < 0) {
            ErrHanding(pstDataJson, "96", "�ַ�ZPK_LMK[%s]�ն�[%s]����ʧ��.", sPinuLmk, sChannelTermId);
            return -1;
        }

        if (tHsm_Imp_Key(sMacuLmk, sTakuZmk, sOutTakChk, sZmkuLmk)) {
            ErrHanding(pstDataJson, "A7", "�ַ�ZAKתLMK����ʧ��[%s]ʧ��.", sMacuLmk);
            return -1;
        }
        tLog(INFO, "sOutTakChk[%s]", sOutTakChk);

        if (UpdPufaChannelKey(sMacuLmk, sChannelTermId, "ZAK") < 0) {
            ErrHanding(pstDataJson, "96", "�ַ�ZAK_LMK[%s]�ն�[%s]����ʧ��.", sMacuLmk, sChannelTermId);
            return -1;
        }
        tLog(INFO, "����(����)[%s]�ն�[%s]ǩ���ɹ�.", sChannelId, sChannelTermId);
        cJSON_Delete(pstJson);
        return 0;
    }
    cJSON_Delete(pstJson);
    return -1;
}

/*ƽ̨ǩ��*/
void Pufa029000(char * pcMerchId, char * pcTermId) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char *pcMsg = NULL;
    char sSysTrace[6 + 1] = {0}, sKey[64] = {0};
    int iLen, iRet;
    char sMsg[MSG_MAX_LEN] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0};
    char sTransmitTime[10 + 1] = {0};

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "����Key-Jsonʧ��.");
        return;
    }
    tGetDate(sDate, "", -1);
    tGetTime(sTime, "", -1);

    strcat(sTransmitTime, sDate + 4);
    strcat(sTransmitTime, sTime);
    if (GetSysTrace(sSysTrace) != 0) {
        tLog(ERROR, "����ϵͳ��ˮ��ʧ��.");
    }
    strcat(sKey, "ADMIN_");
    strcat(sKey, sSysTrace);
    SET_STR_KEY(pstJson, "svrid", "PF_SIGNO_P");
    SET_STR_KEY(pstJson, "key", sKey);

    SET_STR_KEY(pstDataJson, "trans_code", "029000");
    SET_STR_KEY(pstDataJson, "sys_trace", sSysTrace);
    SET_STR_KEY(pstDataJson, "transmit_time", sTransmitTime);
    SET_STR_KEY(pstDataJson, "fwd_inst_id", "pufa");
    SET_STR_KEY(pstDataJson, "channel_merch_id", pcMerchId);
    SET_STR_KEY(pstDataJson, "channel_term_id", pcTermId);
    SET_STR_KEY(pstDataJson, "teller_no", "01");
    SET_STR_KEY(pstDataJson, "batch_no", "000001");

    SET_JSON_KEY(pstJson, "data", pstDataJson);

    pcMsg = cJSON_PrintUnformatted(pstJson);
    tSendMsg("PUFA_Q", pcMsg, strlen(pcMsg));
    free(pcMsg);
    cJSON_Delete(pstJson);

    memset(sMsg, 0, sizeof (sMsg));
    iRet = tRecvMsg("PF_SIGNO_P", sMsg, &iLen, TIMEOUT);
    switch (iRet) {
        case MSG_ERR:
            tLog(ERROR, "��ȡ��Ϣʧ��.");
            puts("��ȡ��Ϣʧ��.");
            break;
        case MSG_TIMEOUT:
            tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", "029000", TIMEOUT);
            fprintf(stderr, "[%s]�ȴ���Ϣ��ʱ[%d].", "029000", TIMEOUT);
            break;
        case MSG_SUCC:
            /* ʧ����Ϣ�Ѿ��ں����ڲ���ӡ�ˣ��������ж� */
            if (JudgeResult(sMsg) == 0) {
                tLog(DEBUG, "�ַ��ն�[%s]ǩ���ɹ�!", pcTermId);
                puts("ƽ̨ǩ���ɹ�!");
            } else {
                tLog(DEBUG, "�ַ��ն�[%s]ǩ��ʧ��!", pcTermId);
                puts("ƽ̨ǩ��ʧ��!");
            }
            break;
    }
}

int main(int argc, char* pcArgv[]) {

    if (tInitLog(pcArgv[0]) < 0) {
        fprintf(stderr, "��ʼ��[%s]��־ʧ��.\n", pcArgv[0]);
        return (EXIT_FAILURE);
    }
    if (tOpenRedis() < 0) {
        tLog(ERROR, "redis����ʧ��.");
        return -1;
    }

    if (tOpenDb() < 0) {
        tLog(ERROR, "���ݿ�����ʧ��.");
        return -1;
    }

    DbPufa029000();

    tCloseRedis();
    tDoneLog();
    tCloseDb();
    return (EXIT_SUCCESS);
}

