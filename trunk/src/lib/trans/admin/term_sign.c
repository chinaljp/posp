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
#include "tKms.h"

#define KEY32_LEN 32
/******************************************************************************/
/*      函数名:     TermSign()                  	                          */
/*      功能说明:   终端签到  					  */
/*      输入参数:   cJSON *pstTranData                                        */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
#if 0

int TermSign(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL, *pstTermJson = NULL, *pstAgentJson = NULL;
    char sZmkuLmk[KEY32_LEN + 1] = {0};
    char sTmkuZmk[KEY32_LEN + 1] = {0}, sTmkuLmk[KEY32_LEN + 1] = {0}, sZmkChkValue[KEY32_LEN + 1] = {0};
    char sTpkuTmk[KEY32_LEN + 1] = {0}, sTpkuLmk[KEY32_LEN + 1] = {0}, sTpkChkValue[KEY32_LEN + 1] = {0};
    char sTakuTmk[KEY32_LEN + 1] = {0}, sTakuLmk[KEY32_LEN + 1] = {0}, sTakChkValue[KEY32_LEN + 1] = {0};
    char sTdkuTmk[KEY32_LEN + 1] = {0}, sTdkuLmk[KEY32_LEN + 1] = {0}, sTdkChkValue[KEY32_LEN + 1] = {0};
    char sTak_key[48 + 1] = {0}, sTpk_key[48 + 1] = {0}, sTdk_key[48 + 1] = {0}, sTmk_key[48 + 1] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    pstTermJson = GET_JSON_KEY(pstJson, "terminal");
    pstAgentJson = GET_JSON_KEY(pstJson, "agent");

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTermJson, "tmk_key", sTmkuLmk);

    /* 判断是否有值，没有值，查找所属代理商获取tmk */
    if (sTmkuLmk[0] == '\0') {
        GET_STR_KEY(pstAgentJson, "zmk_key", sZmkuLmk);
        /*
       if (GetDefaultZmk(sZmkuLmk) < 0) {
           ErrHanding(pstTransJson, "A7", "获取zmk失败.");
           return -1;
       }
         */
        tLog(DEBUG, "ZmkuLmk[%s].", sZmkuLmk);
        if ((HSM_FirstGenTmk(sZmkuLmk, sTmkuZmk, sTmkuLmk, sZmkChkValue)) != 0) {
            ErrHanding(pstTransJson, "A7", "加密机首次生成商户[%s]终端[%s]Tmk失败.", sMerchId, sTermId);
            return -1;
        }
        strcpy(sTmk_key, sTmkuZmk);
        strcat(sTmk_key, sZmkChkValue);
        SET_STR_KEY(pstTransJson, "tmk_key", sTmk_key);
        SET_STR_KEY(pstTransJson, "tmk_flag", "1");
        tLog(DEBUG, "TmkuZmk[%s],TmkuLmk[%s],ZmkChkValue[%s].", sTmkuZmk, sTmkuLmk, sZmkChkValue);
    } else {
        SET_STR_KEY(pstTransJson, "tmk_flag", "0");
    }

    if (HSM_GenTpk(sTmkuLmk, sTpkuTmk, sTpkuLmk, sTpkChkValue) != 0) {
        ErrHanding(pstTransJson, "A7", "加密机生成商户[%s]终端[%s]TPK失败.", sMerchId, sTermId);
        return -1;
    }
    strcpy(sTpk_key, sTpkuTmk);
    strcat(sTpk_key, sTpkChkValue);
    SET_STR_KEY(pstTransJson, "tpk_key", sTpk_key);
    tLog(DEBUG, "TpkuTmk[%s],TpkuLmk[%s],TpkChkValue[%s].", sTpkuTmk, sTpkuLmk, sTpkChkValue);

    if (HSM_GenTak(sTmkuLmk, sTakuTmk, sTakuLmk, sTakChkValue) != 0) {
        ErrHanding(pstTransJson, "A7", "加密机生成商户[%s]终端[%s]TAK失败.", sMerchId, sTermId);
        return -1;
    }
    strcpy(sTak_key, sTakuTmk);
    strcat(sTak_key, sTakChkValue);
    SET_STR_KEY(pstTransJson, "tak_key", sTak_key);
    tLog(DEBUG, "TakuTmk[%s],TakuLmk[%s],TakChkValue[%s].", sTakuTmk, sTakuLmk, sTakChkValue);

    if (HSM_GenTdk(sTmkuLmk, sTdkuTmk, sTdkuLmk, sTdkChkValue) != 0) {
        ErrHanding(pstTransJson, "A7", "加密机生成商户[%s]终端[%s]TDK失败.", sMerchId, sTermId);
        return -1;
    }
    strcpy(sTdk_key, sTdkuTmk);
    strcat(sTdk_key, sTdkChkValue);
    SET_STR_KEY(pstTransJson, "tdk_key", sTdk_key);
    tLog(DEBUG, "TdkuTmk[%s],TdkuLmk[%s],TdkChkValue[%s].", sTdkuTmk, sTdkuLmk, sTdkChkValue);

    /* 更新终端密钥 */
    tLog(INFO, "更新终端[%s]密钥.", sTermId);
    if (UpdTermKey(sTmkuLmk, sTpkuLmk, sTakuLmk, sTdkuLmk, sMerchId, sTermId) != 0) {
        tLog(ERROR, "更新商户[%s]终端[%s]密钥失败.", sMerchId, sTermId);
        ErrHanding(pstTransJson, "96", "更新商户[%s]终端[%s]密钥失败.", sMerchId, sTermId);
        return -1;
    }

    tLog(INFO, "商户号[%s],终端号[%s]签到成功.", sMerchId, sTermId);
    return 0;
}
#endif

int TermSign(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL, *pstTermJson = NULL, *pstAgentJson = NULL;
    char sZmkuLmk[KEY32_LEN + 1] = {0};
    char sTmkuZmk[KEY32_LEN + 1] = {0}, sTmkuLmk[KEY32_LEN + 1] = {0}, sZmkChkValue[KEY32_LEN + 1] = {0};
    char sTpkuTmk[KEY32_LEN + 1] = {0}, sTpkuLmk[KEY32_LEN + 1] = {0}, sTpkChkValue[KEY32_LEN + 1] = {0};
    char sTakuTmk[KEY32_LEN + 1] = {0}, sTakuLmk[KEY32_LEN + 1] = {0}, sTakChkValue[KEY32_LEN + 1] = {0};
    char sTdkuTmk[KEY32_LEN + 1] = {0}, sTdkuLmk[KEY32_LEN + 1] = {0}, sTdkChkValue[KEY32_LEN + 1] = {0};
    char sTak_key[48 + 1] = {0}, sTpk_key[48 + 1] = {0}, sTdk_key[48 + 1] = {0}, sTmk_key[48 + 1] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    pstTermJson = GET_JSON_KEY(pstJson, "terminal");
    pstAgentJson = GET_JSON_KEY(pstJson, "agent");

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTermJson, "tmk_key", sTmkuLmk);
    /* 判断是否有值，没有值，查找所属代理商获取tmk */
    if (sTmkuLmk[0] == '\0') {
        GET_STR_KEY(pstAgentJson, "zmk_key", sZmkuLmk);
        /*
       if (GetDefaultZmk(sZmkuLmk) < 0) {
           ErrHanding(pstTransJson, "A7", "获取zmk失败.");
           return -1;
       }
         */
        tLog(DEBUG, "ZmkuLmk[%s].", sZmkuLmk);
        snprintf(sTmkuLmk, sizeof (sTmkuLmk), "pos.%s.tmk", sTermId);
        if ((tHsm_Gen_Exp_Zmk(sTmkuLmk, sZmkuLmk, sTmkuZmk, sZmkChkValue)) != 0) {
            ErrHanding(pstTransJson, "A7", "加密机首次生成商户[%s]终端[%s]Tmk失败.", sMerchId, sTermId);
            return -1;
        }
        strcpy(sTmk_key, sTmkuZmk);
        strcat(sTmk_key, sZmkChkValue);
        SET_STR_KEY(pstTransJson, "tmk_key", sTmk_key);
        SET_STR_KEY(pstTransJson, "tmk_flag", "1");
        tLog(DEBUG, "TmkuZmk[%s],TmkuLmk[%s],ZmkChkValue[%s].", sTmkuZmk, sTmkuLmk, sZmkChkValue);
    } else {
        SET_STR_KEY(pstTransJson, "tmk_flag", "0");
    }
    snprintf(sTpkuLmk, sizeof (sTpkuLmk), "pos.%s.zpk", sTermId);
    if (tHsm_Gen_Zpk(sTpkuLmk, sTmkuLmk, sTpkuTmk, sTpkChkValue) != 0) {
        ErrHanding(pstTransJson, "A7", "加密机生成商户[%s]终端[%s]TPK失败.", sMerchId, sTermId);
        return -1;
    }
    strcpy(sTpk_key, sTpkuTmk);
    strcat(sTpk_key, sTpkChkValue);
    SET_STR_KEY(pstTransJson, "tpk_key", sTpk_key);
    tLog(DEBUG, "TpkuTmk[%s],TpkuLmk[%s],TpkChkValue[%s].", sTpkuTmk, sTpkuLmk, sTpkChkValue);
    snprintf(sTakuLmk, sizeof (sTakuLmk), "pos.%s.zak", sTermId);
    if (tHsm_Gen_Zak(sTakuLmk, sTmkuLmk, sTakuTmk, sTakChkValue) != 0) {
        ErrHanding(pstTransJson, "A7", "加密机生成商户[%s]终端[%s]TAK失败.", sMerchId, sTermId);
        return -1;
    }
    strcpy(sTak_key, sTakuTmk);
    strcat(sTak_key, sTakChkValue);
    SET_STR_KEY(pstTransJson, "tak_key", sTak_key);
    tLog(DEBUG, "TakuTmk[%s],TakuLmk[%s],TakChkValue[%s].", sTakuTmk, sTakuLmk, sTakChkValue);
    snprintf(sTdkuLmk, sizeof (sTdkuLmk), "pos.%s.zek", sTermId);
    if (tHsm_Gen_Zek(sTdkuLmk, sTmkuLmk, sTdkuTmk, sTdkChkValue) != 0) {
        ErrHanding(pstTransJson, "A7", "加密机生成商户[%s]终端[%s]TDK失败.", sMerchId, sTermId);
        return -1;
    }
    strcpy(sTdk_key, sTdkuTmk);
    strcat(sTdk_key, sTdkChkValue);
    SET_STR_KEY(pstTransJson, "tdk_key", sTdk_key);
    tLog(DEBUG, "TdkuTmk[%s],TdkuLmk[%s],TdkChkValue[%s].", sTdkuTmk, sTdkuLmk, sTdkChkValue);

    /* 更新终端密钥 */
    tLog(INFO, "更新终端[%s]密钥.", sTermId);
    if (UpdTermKey(sTmkuLmk, sTpkuLmk, sTakuLmk, sTdkuLmk, sMerchId, sTermId) != 0) {
        tLog(ERROR, "更新商户[%s]终端[%s]密钥失败.", sMerchId, sTermId);
        ErrHanding(pstTransJson, "96", "更新商户[%s]终端[%s]密钥失败.", sMerchId, sTermId);
        return -1;
    }

    tLog(INFO, "商户号[%s],终端号[%s]签到成功.", sMerchId, sTermId);
    return 0;
}