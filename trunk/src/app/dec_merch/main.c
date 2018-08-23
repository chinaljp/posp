/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: pangpang
 *
 * Created on 2017年5月22日, 下午10:32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unionAPI.h"
#include "t_app_conf.h"
#include "t_tools.h"
#include "tKms.h"
#include "t_db.h"
#define KEY32_LEN 32

/* 获取 */
int FindValueByKey(char *pcValue, char *pcKey) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key='%s'", pcKey);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        printf("查找Key[%s]的值失败.", pcKey);
        return -1;
    }
    if (NULL == pstRes) {
        printf("sql[%s]结果为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcValue);
        tTrim(pcValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        printf("无Key[%s]的值.", pcKey);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

/* 批量解密 */
int batchMerchDec() {
    char sSqlStr[512] = {0}, sSql[512] = {0};
    char sAccountId[128 + 1] = {0}, sAccountIdDec[128 + 1] = {0};
    char sAccountName[128 + 1] = {0}, sAccountNameDec[128 + 1] = {0};
    char sAccountMobile[128 + 1] = {0}, sAccountMobileDec[128 + 1] = {0};
    char sLegName[128 + 1] = {0}, sLegNameDec[128 + 1] = {0};
    char sLegCertNo[128 + 1] = {0}, sLegCertNoDec[128 + 1] = {0};
    char sLegMobile[128 + 1] = {0}, sLegMobileDec[128 + 1] = {0};
    char sContact[128 + 1] = {0}, sContactDec[128 + 1] = {0};
    char sContactCertNo[128 + 1] = {0}, sContactCertNoDec[128 + 1] = {0};
    char sCardId[128 + 1] = {0}, sCardIdDec[128 + 1] = {0};
    char sMobile[128 + 1] = {0}, sMobileDec[128 + 1] = {0};
    char sTestName[128 + 1] = {0}, sMerchId[15 + 1] = {0};
    int iNum = 0;
    OCI_Resultset *pstRes = NULL;

    if (FindValueByKey(sTestName, "kms.encdata.key") < 0) {
        printf("查找key[kms.key.key]值,失败.");
        tCloseDb();
        return -1;
    }
    snprintf(sSqlStr, sizeof (sSqlStr), "select merch_id,ACCOUNT_ID_encrypt,ACCOUNT_NAME_encrypt"
            ",account_mobile_encrypt,LEGAL_NAME_ENCRYPT,LEGAL_CERT_NO_ENCRYPT,LEGAL_MOBILE_ENCRYPT"
            ",CONTACTS_ENCRYPT,CONTACTS_CERT_NO_ENCRYPT,ID_CARD_ENCRYPT,MOBILE_ENCRYPT "
            " from b_show where ACCOUNT_ID is  null and LEGAL_NAME is null "
            " and LEGAL_CERT_NO is null and LEGAL_MOBILE is null and CONTACTS is null"
            " and CONTACTS_CERT_NO is null and MOBILE is null and ACCOUNT_NAME is null"
            " and ACCOUNT_MOBILE is null and ID_CARD is null"
            " and  rownum<250");

    if (tExecute(&pstRes, sSqlStr) < 0) {
        printf("tExecute err.");
        tCloseDb();
        return -1;
    }
    if (NULL == pstRes) {
        printf("sql[%s]结果为NULL.", sSqlStr);
        tCloseDb();
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        memset(sMerchId, 0, sizeof (sMerchId));
        memset(sAccountIdDec, 0, sizeof (sAccountIdDec));
        memset(sAccountId, 0, sizeof (sAccountId));
        memset(sAccountNameDec, 0, sizeof (sAccountNameDec));
        memset(sAccountName, 0, sizeof (sAccountName));
        memset(sAccountMobileDec, 0, sizeof (sAccountMobileDec));
        memset(sAccountMobile, 0, sizeof (sAccountMobile));
        memset(sLegNameDec, 0, sizeof (sLegNameDec));
        memset(sLegName, 0, sizeof (sLegName));
        memset(sLegCertNoDec, 0, sizeof (sLegCertNoDec));
        memset(sLegCertNo, 0, sizeof (sLegCertNo));
        memset(sLegMobileDec, 0, sizeof (sLegMobileDec));
        memset(sLegMobile, 0, sizeof (sLegMobile));
        memset(sContactDec, 0, sizeof (sContactDec));
        memset(sContact, 0, sizeof (sContact));
        memset(sContactCertNoDec, 0, sizeof (sContactCertNoDec));
        memset(sContactCertNo, 0, sizeof (sContactCertNo));
        memset(sCardIdDec, 0, sizeof (sCardIdDec));
        memset(sCardId, 0, sizeof (sCardId));
        memset(sMobileDec, 0, sizeof (sMobileDec));
        memset(sMobile, 0, sizeof (sMobile));
        STRV(pstRes, 1, sMerchId);
        STRV(pstRes, 2, sAccountIdDec);
        STRV(pstRes, 3, sAccountNameDec);
        STRV(pstRes, 4, sAccountMobileDec);
        STRV(pstRes, 5, sLegNameDec);
        STRV(pstRes, 6, sLegCertNoDec);
        STRV(pstRes, 7, sLegMobileDec);
        STRV(pstRes, 8, sContactDec);
        STRV(pstRes, 9, sContactCertNoDec);
        STRV(pstRes, 10, sCardIdDec);
        STRV(pstRes, 11, sMobileDec);

        if (sAccountIdDec[0] == '\0' || tHsm_Dec_Asc_Data(sAccountId, sTestName, sAccountIdDec) < 0) {
            printf("AccountIdDec[%s] tHsm_Dec_Asc_Data err\n", sAccountIdDec);
        }
        if (sAccountNameDec[0] == '\0' || tHsm_Dec_Asc_Data(sAccountName, sTestName, sAccountNameDec) < 0) {
            printf("sAccountNameDec[%s] tHsm_Dec_Asc_Data err\n", sAccountNameDec);
        }
        if (sAccountMobileDec[0] == '\0' || tHsm_Dec_Asc_Data(sAccountMobile, sTestName, sAccountMobileDec) < 0) {
            printf("sAccountMobileDec[%s] tHsm_Dec_Asc_Data err\n", sAccountMobileDec);
        }
        if (sLegNameDec[0] == '\0' || tHsm_Dec_Asc_Data(sLegName, sTestName, sLegNameDec) < 0) {
            printf("sLegNameDec[%s] tHsm_Dec_Asc_Data err\n", sLegNameDec);
        }
        if (sLegCertNoDec[0] == '\0' || tHsm_Dec_Asc_Data(sLegCertNo, sTestName, sLegCertNoDec) < 0) {
            printf("sLegNameDec[%s] tHsm_Dec_Asc_Data err\n", sLegCertNoDec);
        }
        if (sLegMobileDec[0] == '\0' || tHsm_Dec_Asc_Data(sLegMobile, sTestName, sLegMobileDec) < 0) {
            printf("sLegMobileDec[%s] tHsm_Dec_Asc_Data err\n", sLegMobileDec);
        }
        if (sContactDec[0] == '\0' || tHsm_Dec_Asc_Data(sContact, sTestName, sContactDec) < 0) {
            printf("sContactDec[%s] tHsm_Dec_Asc_Data err\n", sContactDec);
        }
        if (sContactCertNoDec[0] == '\0' || tHsm_Dec_Asc_Data(sContactCertNo, sTestName, sContactCertNoDec) < 0) {
            printf("sContactCertNoDec[%s] tHsm_Dec_Asc_Data err\n", sContactCertNoDec);
        }
        if (sCardIdDec[0] == '\0' || tHsm_Dec_Asc_Data(sCardId, sTestName, sCardIdDec) < 0) {
            printf("sTelDec[%s] tHsm_Dec_Asc_Data err\n", sCardIdDec);
        }
        if (sMobileDec[0] == '\0' || tHsm_Dec_Asc_Data(sMobile, sTestName, sMobileDec) < 0) {
            printf("sMobileDec[%s] tHsm_Dec_Asc_Data err\n", sMobileDec);
        }

        snprintf(sSql, sizeof (sSql), "update b_show set ACCOUNT_ID='%s',ACCOUNT_NAME='%s'"
                ",account_mobile='%s',LEGAL_NAME='%s',LEGAL_CERT_NO='%s',LEGAL_MOBILE='%s'"
                ",CONTACTS='%s',CONTACTS_CERT_NO='%s',ID_CARD='%s',MOBILE='%s' where merch_id='%s'"
                , sAccountId, sAccountName, sAccountMobile, sLegName, sLegCertNo, sLegMobile
                , sContact, sContactCertNo, sCardId, sMobile, sMerchId);
        printf("sql[%s]\n", sSql);
        iNum = tExecuteUpd(sSql);
        printf("merch_id=[%s][%s][%s][%s][%d]\n", sMerchId, sAccountId, sMobile, sCardId, iNum);
        tCommit();
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        printf("无值.");
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    printf("end.\n");
    return 0;
}

/*
 * 
 */
int main(int argc, char** argv) {
    int iRet;
    char sKeyValue[32 + 1] = {0}, sCheckValue[33] = {0}, sKeyValue2[33] = {0}, sTmk[128] = {0}, sBuf[128] = {0};
    char sZmkuLmk[33] = {0}, sTmkuLmk[33] = {0}, sTmp[303];
    //EncDec(argc, argv);

    tOpenDb();
    while (!batchMerchDec()) {
        tCloseDb();
        if (tOpenDb() < 0) {
            printf("数据库连接失败.\n");
            break;
        }
    }
    tCloseDb();
    return (EXIT_SUCCESS);
}

