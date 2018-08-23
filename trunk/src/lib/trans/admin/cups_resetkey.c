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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "resp_code.h"
#include "secapi.h"
#include "merch.h"
#include "t_macro.h"

#define KEY32_LEN 32
/******************************************************************************/
/*      ������:     CupsResetKey()                  	                          */
/*      ����˵��:   ����������Կ  					  */
/*      �������:   cJSON *pstTranData                                        */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int CupsResetKey(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sPinuZmk[33] = {0}, sMacuZmk[17] = {0}, sPinChk[9] = {0}, sMacChk[9] = {0}, sTmp[64] = {0};
    char sPinuLmk[33] = {0}, sMacuLmk[17] = {0};
    char sZmkuLmk[17] = {0};
    char sBcdZero[17] = {0}, sSecureCtrl[16 + 1] = {0};
    char sKeyName[5], *pcKeyAsc;
    char sErr[128] = {0}, cSignType = '0';

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "secure_ctrl", sSecureCtrl);


    if (GetChannelKey(sZmkuLmk, "ZMK") < 0) {
        ErrHanding(pstTransJson, "96", "ȡ����ZMK[%s]ʧ��.", sZmkuLmk);
        return -1;
    }
    tLog(DEBUG, "Zmk[%s].", sZmkuLmk);
    /* ����PIN KEY(18�ֽ�,3DES) */
    if ('1' == sSecureCtrl[0]) {
        GET_STR_KEY(pstTransJson, "data1", sPinuZmk);
        tLog(DEBUG, "PinKey[%s],Chk[%s].", sPinuZmk, sPinChk);

        // if (HSM_TranZpk_Tmk2Lmk(sZmkuLmk, sPinuZmk, sPinuLmk, "00000000") < 0) {
        strcpy(sPinuLmk, "pos.cups.zpk");
        if (tHsm_Imp_Key(sPinuLmk, sPinuZmk, "", sZmkuLmk)) {
            ErrHanding(pstTransJson, "A7", "����ZPKתLMK����ʧ��[%s]ʧ��.", sPinuZmk);
            return -1;
        }
        strcpy(sKeyName, "ZPK");
        //pcKeyAsc = sPinuLmk;
        if (UpdChannelKey(sPinuLmk, sKeyName) < 0) {
            ErrHanding(pstTransJson, "96", "����ZPK_LMK[%s]����ʧ��.", sPinuLmk);
            return -1;
        }
        /* ����pin��Կ����Ҫʹ������Կ����mac�������ȴ洢��һ��Ҫ��ִ��pin���� */
        strcpy(sPinuLmk, "pos.cups3.zak");
        if (tHsm_Imp_Key(sPinuLmk, sPinuZmk, "", sZmkuLmk)) {
            ErrHanding(pstTransJson, "A7", "����ZAKתLMK����ʧ��[%s]ʧ��.", sPinuZmk);
            return -1;
        }
        strcpy(sKeyName, "ZAK");
        //pcKeyAsc = sMacuLmk;
        if (UpdChannelKey(sPinuLmk, sKeyName) < 0) {
            ErrHanding(pstTransJson, "96", "����ZAK_LMK[%s]����ʧ��.", sPinuLmk);
            return -1;
        }
        /*
                //cups ������Կʹ��pik����128��
                tLog(DEBUG, "��ʼת��pikΪlmk���ܵ�����.");
                if (HSM_TranZak_CUPSLmk(sZmkuLmk, sPinuZmk, sPinuLmk, "00000000") < 0) {
                    ErrHanding(pstTransJson, "A7", "����PAKתLMK����ʧ��[%s]ʧ��.", sPinuZmk);
                    return -1;
                }
                strcpy(sKeyName, "PAK");
                pcKeyAsc = sPinuLmk;
                if (UpdChannelKey(pcKeyAsc, sKeyName) < 0) {
                    ErrHanding(pstTransJson, "96", "����PAK_LMK[%s]����ʧ��.", pcKeyAsc);
                    return -1;
                }*/
    }/* ����MAC KEY(8�ֽ�,DES) */
    else if ('2' == sSecureCtrl[0]) {
        GET_STR_KEY(pstTransJson, "secure_data", sMacuZmk);
        tLog(DEBUG, "MacKey[%s].", sMacuZmk);

        // if (HSM_TranZak_Tmk2Lmk(sZmkuLmk, sMacuZmk, sMacuLmk, "00000000") < 0) {
        strcpy(sPinuLmk, "pos.cups.zak");
        if (tHsm_Imp_Key(sPinuLmk, sMacuZmk, "", sZmkuLmk)) {
            ErrHanding(pstTransJson, "A7", "����ZAKתLMK����ʧ��[%s]ʧ��.", sMacuZmk);
            return -1;
        }
        strcpy(sKeyName, "ZAK");
        //pcKeyAsc = sMacuLmk;
        if (UpdChannelKey(sPinuLmk, sKeyName) < 0) {
            ErrHanding(pstTransJson, "96", "����ZAK_LMK[%s]����ʧ��.", pcKeyAsc);
            return -1;
        }
    }

    /*
        tErrLog(DEBUG, "merchid[%s],termid[%s].", pstNetTran->sChannelMerchId, pstNetTran->sChannelTermId);
        if (DBUpdChannelTermKey(pstNetTran->sChannelId, pstNetTran->sChannelMerchId, pstNetTran->sChannelTermId
                , pstNetTran->sChannelBatchNo, pcKeyAsc, sKeyName) < 0) {
            SndMoniMsg(ERR, "��������(����)[%s]�̻�[%s]�ն�[%s]%sʧ��."
                    , pstNetTran->sChannelId, pstNetTran->sChannelMerchId, pstNetTran->sChannelTermId, sKeyName);
            SetRespCode(pstNetTran, "96", pstTranData->stSysParam.sInstId);
            return -1;
        }
     */
    return 0;
}