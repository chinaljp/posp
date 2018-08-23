/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_redis.h"
#include "t_cjson.h"
#include "t_macro.h"

typedef struct {
    char sTransCode[6 + 1];
    char sPostType[4 + 1];
} PostType;
/* �������ͺͽ�����ת�� */
PostType g_staPostType[] = {
    {"020000", "1000" },
    {"020003", "2001" },
    {"020002", "2000" },
    {"020023", "1001" },
    {"020001", "2002" },
    /* Ԥ��Ȩ */
    {"024100", "1010" },
    {"024102", "2010" },
    {"024103", "2011" },
    {"024123", "1011" },

    {"T20000", "1000" },
    {"T20003", "2001" },

    {"U20000", "1000" },
    {"M20000", "1000" },
    {"M20003", "2001" },
    {"M20002", "2000" },
    {"M20023", "1001" },
    {"M20001", "2002" }
};

void GetPostType( char *pcPostType, char *pcTransCode ) {
    int i;
    for ( i = 0; i < 128; i++ ) {
        if ( !strcmp( g_staPostType[i].sTransCode, pcTransCode ) ) {
            strcpy( pcPostType, g_staPostType[i].sPostType );
        }
    }
}

/* �ж����ѵ�Ǯ������ */
int ChkWalletType( cJSON *pstTransJson, int *piFlag ) {
    char sCardType[2] = { 0 }, sInputMode[4] = { 0 }, sTransType[1 + 1] = { 0 }, sRrn[13] = { 0 };
    char sSql[512] = { 0 }, sCardNo[64] = { 0 };
    int iNum = 0, iRet = 0;
    double dTranAmt = 0.0, dLimitAmt = 0.0;
    char sLimitAmt[32 + 1] = { 0 };
    char sMerchId[15 + 1] = { 0 }, sSingleAmt[32 + 1] = { 0 };
    double dSingleAmt = 0.0;

    GET_DOU_KEY( pstTransJson, "amount", dTranAmt );
    GET_STR_KEY( pstTransJson, "card_type", sCardType );
    GET_STR_KEY( pstTransJson, "input_mode", sInputMode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_STR_KEY( pstTransJson, "card_no", sCardNo );
    dTranAmt = dTranAmt / 100;
    /*add by gjq at 20171019 BENGIN*/
    GET_STR_KEY( pstTransJson, "merch_id", sMerchId );
    /*add by gjq at 20171019 END*/
    /* �ж�Ǯ������
     * ��ǿ������Լ��������ÿ�����1�����������Ǯ�� 
     * ���ǽ�ǿ�����Ȩ
     * trans_type����:0:D0,1:�ս� 2:D0�����Ȩ��3:D0���󲻼�Ȩ
     *  �̻��ڽ��׼�Ȩ�������� �� �������ÿ����׶�С�ڵ���2�򣨲����� ��D0Ǯ�� 0:D0  add by gjq at 20171019
     *  */
    if ( ( sCardType[0] == '1' )&&( sInputMode[1] == '2' ) ) {
        /*add by gjq at 20171019 BENGIN*/
        if ( FindValueByKey( sSingleAmt, "D0_TRANS_AMT" ) < 0 ) {
            tLog( ERROR, "����key[D0_TRANS_AMT]D0�޶�ֵ,ʧ��." );
            /* ����� */
            sTransType[0] = '2';
        }
        dSingleAmt = atof( sSingleAmt );
        tLog( DEBUG, "���׽��[%f],�������ÿ�D0�޶�[%f].", dTranAmt, dSingleAmt );
        tLog( INFO, "����̻�������֤������.(d0)" );
        snprintf( sSql, sizeof (sSql ), "SELECT COUNT(1) FROM B_MERCH_D0_TRANS "
                "WHERE MERCH_ID = '%s' AND STATUS = '1'", sMerchId );
        iNum = tQueryCount( sSql );
        if ( ( iNum > 0 && !DBL_CMP( dTranAmt, dSingleAmt ) ) || ( iNum > 0 && DBL_EQ( dTranAmt, dSingleAmt ) ) ) {
            tLog( DEBUG, "�̻�[%s]����������֤���������Ҵ������ÿ����׶�[%.02f]С�ڵ���[%.02f]", sMerchId, dTranAmt, dSingleAmt );
            sTransType[0] = '0';
        }/*add by gjq at 20171019 END*/
        else {
            tLog( WARN, "�̻�[%s]����������֤������ ���� �˴ν��״������ÿ����׶��.", sMerchId );
            if ( FindValueByKey( sLimitAmt, "D0_SINGLE_CASH_MIDDLE" ) < 0 ) {
                tLog( ERROR, "����key[D0_SINGLE_CASH_MIDDLE]D0�޶�ֵ,ʧ��." );
                /* ����� */
                sTransType[0] = '2';
            }
            /* �жϽ��׽���Ƿ���ڴ�ֵ */
            dLimitAmt = atof( sLimitAmt );
            tLog( DEBUG, "���׽��[%f],�������ÿ���Ȩ�޶�[%f].", dTranAmt, dLimitAmt );
            if ( DBL_CMP( dTranAmt, dLimitAmt ) ) {
                sTransType[0] = '2';
            } else {
                sTransType[0] = '0';
            }
        }
    } else if ( sCardType[0] == '0' ) {
        sTransType[0] = '3';
    } else {
        sTransType[0] = '0';
    }
    /* ������ڰ������ֱ��D0���� */
    if ( '2' == sTransType[0] ) {
        tLog( DEBUG, "���D0���Ű�����." );
        snprintf( sSql, sizeof (sSql ), "select count(1) from b_trans_card_white_list"
                " where account_no='%s'", sCardNo );
        iNum = tQueryCount( sSql );
        if ( iNum > 0 ) {
            tLog( DEBUG, "����[%s]���ڰ�������,����ֱ������.", sCardNo );
            sTransType[0] = '0';
        } else {
            tLog( DEBUG, "����[%s]���ڰ�������,������Ҫ���.", sCardNo );
        }
    }
    SET_STR_KEY( pstTransJson, "trans_type", sTransType );
    tLog( INFO, "����[%s]Ǯ������[%s:%s].", sRrn, sTransType, sTransType[0] == '0' ? "D0Ǯ��" : "D0����Ǯ��" );
    return 0;
}

void Net2Wallet( cJSON *pstTransJson, cJSON *pstNetJson ) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = { 0 }, sSettleDate[8 + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sTrace[6 + 1] = { 0 };
    char sTransDate[8 + 1] = { 0 }, sTransTime[6 + 1] = { 0 }, sTransType[1 + 1] = { 0 }, sTransCode[6 + 1] = { 0 };
    char sCardType[1 + 1] = { 0 }, sInputMode[3 + 1] = { 0 }, sPostType[4 + 1] = { 0 };

    GET_STR_KEY( pstTransJson, "merch_id", sMerchId );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_DOU_KEY( pstTransJson, "amount", dAmount );
    GET_DOU_KEY( pstTransJson, "fee", dFee );
    GET_STR_KEY( pstTransJson, "sys_trace", sTrace );
    GET_STR_KEY( pstTransJson, "trans_date", sTransDate );
    GET_STR_KEY( pstTransJson, "trans_time", sTransTime );
    GET_STR_KEY( pstTransJson, "settle_date", sSettleDate );
    GET_STR_KEY( pstTransJson, "trans_type", sTransType );
    GET_STR_KEY( pstTransJson, "card_type", sCardType );
    GET_STR_KEY( pstTransJson, "input_mode", sInputMode );
    GET_STR_KEY( pstTransJson, "trans_code", sTransCode );
    //��������
    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    /* merch_id��Ϊ�˴�ӡ��־ */
    SET_STR_KEY( pstNetJson, "merch_id", sMerchId );
    SET_STR_KEY( pstNetJson, "trans_type", "POS" ); //�����������п����Ƕ�ά��Ľ���
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY( pstNetJson, "transAmount", dAmount / 100 );
    SET_DOU_KEY( pstNetJson, "settleAmount", dAmount / 100 - dFee );
    SET_STR_KEY( pstNetJson, "sys_trace", sTrace );
    SET_STR_KEY( pstNetJson, "trans_date", sTransDate );
    SET_STR_KEY( pstNetJson, "trans_time", sTransTime );
    SET_STR_KEY( pstNetJson, "settle_date", sSettleDate );
    /* 
     * Ǯ�������ж�:100��D0Ǯ�� 101��D0����Ǯ�� 102��POS T1Ǯ��
     *  ��ǿ�(ic,����)-102
     *  ���ÿ�(����) 1������ 100; 1��-5�� 101
     *  ���ÿ�(ic) 100
     * 100��D0Ǯ�� 
     * 101��D0����Ǯ�� 
     *  */
    if ( sTransType[0] == '0' || sTransType[0] == 'S' || sTransType[0] == '1' ) {
        SET_STR_KEY( pstNetJson, "walletType", "100" ); //Ǯ������
    } else if ( sTransType[0] == '2' || sTransType[0] == '3' ) {
        SET_STR_KEY( pstNetJson, "walletType", "101" ); //Ǯ������
    }
    GetPostType( sPostType, sTransCode );
    SET_STR_KEY( pstNetJson, "postType", sPostType );
}

/* ��ͨ�������� ,ʧ�ܲ�����*/
int AddPosWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    if ( memcmp( sRespCode, "00", 2 ) ) {
        ErrHanding( pstTransJson, sRespCode, "����ʧ��[%s],������.", sRespCode );
        return 0;
    }
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, sRespCode, "��������Jsonʧ��." );
        return 0;
    }
    Net2Wallet( pstTransJson, pstNetJson );
    SET_STR_KEY( pstNetJson, "trans_code", "00T600" );

    MEMSET_ST( stQMsgData );
    snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00T600_%s", sRrn );
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcACall( "WALLET_Q", &stQMsgData );
    //iRet = tSvcACall("00T600_Q", &stQMsgData);
    if ( iRet < 0 ) {
        if ( MSG_TIMEOUT == iRet ) {
            ErrHanding( pstTransJson, sRespCode, "Ǯ�����˳�ʱ,����." );
        } else
            ErrHanding( pstTransJson, sRespCode, "Ǯ������ʧ��,����." );
    } else {
        tLog( INFO, "Ǯ�����������ͳɹ�." );
    }
    cJSON_Delete( pstNetJson );
    return 0;
}

void Net2FrozenWallet( cJSON *pstTransJson, cJSON *pstNetJson ) {
    char sMerchId[15 + 1] = { 0 }, sUserCode[15 + 1] = { 0 }, sTransType[1 + 1] = { 0 };
    char sRrn[12 + 1] = { 0 }, sTrace[7] = { 0 }, sTransCode[6 + 1] = { 0 }, sWalletFlag[1 + 1] = { 0 };
    double dAmount = 0.0, dFee = 0.0;
    GET_STR_KEY( pstTransJson, "merch_id", sMerchId );
    GET_STR_KEY( pstTransJson, "user_code", sUserCode );
    GET_STR_KEY( pstTransJson, "trans_type", sTransType );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_DOU_KEY( pstTransJson, "amount", dAmount );
    GET_DOU_KEY( pstTransJson, "fee", dFee );
    GET_STR_KEY( pstTransJson, "sys_trace", sTrace );

    /*add by gjq �˻���ȡ����Ǯ�����Ͳ�����Ҫ BEGIN */
    GET_STR_KEY( pstTransJson, "trans_code", sTransCode );
    GET_STR_KEY( pstTransJson, "wallet_flag", sWalletFlag );
    tLog( DEBUG, "trans_code=%s,wallet_flag = [%s]", sTransCode, sWalletFlag );

    if ( !memcmp( sTransCode, "020001", 6 ) && sWalletFlag[0] != '\0' && sWalletFlag[0] != '0' ) {
        /* �˻����� �� pos ���������͵�Ǯ����D0������Ǯ����D0 ������Ǯ����������һ��Ǯ������ʱ��ȥ��������㹻��Ǯ��*/
        if ( sWalletFlag[0] == '1' ) { /*1 - D0������Ǯ������*/
            SET_STR_KEY( pstNetJson, "frozenType", "23" );
            SET_STR_KEY( pstNetJson, "walletType", "111" );
        } else if ( sWalletFlag[0] == '2' ) {/* 2 - D0������Ǯ������ */
            SET_STR_KEY( pstNetJson, "frozenType", "21" );
            SET_STR_KEY( pstNetJson, "walletType", "110" );
        }
    } else {
        /*�˻�������pos ���������͵�Ǯ�������� ���� ���˻�����   �� ������������ ѡ�񶳽��Ǯ������*/
        tLog( DEBUG, "trans_type=%s", sTransType );
        if ( sTransType[0] == '0' || sTransType[0] == 'S' || sTransType[0] == '1' ) {
            SET_STR_KEY( pstNetJson, "frozenType", "21" );
            SET_STR_KEY( pstNetJson, "walletType", "110" );
        } else if ( sTransType[0] == '2' || sTransType[0] == '3' ) {
            SET_STR_KEY( pstNetJson, "frozenType", "23" );
            SET_STR_KEY( pstNetJson, "walletType", "111" );
        }
    }
    /*add by gjq �˻���ȡ����Ǯ�����Ͳ�����Ҫ END */

    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    SET_STR_KEY( pstNetJson, "user_code", sUserCode );
    SET_DOU_KEY( pstNetJson, "amount", dAmount / 100 - dFee );
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    SET_STR_KEY( pstNetJson, "sys_trace", sTrace );
}

/* ��������������Ǯ������,ʧ����Ҫ��Ӧ���� */
int FrozenWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sMerchId[15 + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sRespCode[2 + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);

    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, "96", "��������Jsonʧ��." );
        return -1;
    }
    Net2FrozenWallet( pstTransJson, pstNetJson );
    SET_STR_KEY( pstNetJson, "trans_code", "00T400" );

    tGetUniqueKey( stQMsgData.sSvrId );
    snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00T400%s", sRrn );
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall( "WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10 );
    // iRet = tSvcCall("00T400_Q", pstNetJson, &pstRecvJson, 30);
    if ( iRet < 0 ) {
        if ( MSG_TIMEOUT == iRet ) {
            ErrHanding( pstTransJson, "96", "����[%s]Ǯ������֪ͨ��ʱ.", sRrn );
        } else
            ErrHanding( pstTransJson, "96", "����[%s]Ǯ������֪ͨʧ��.", sRrn );
        cJSON_Delete( pstNetJson );
        return -1;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
    GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
    if ( memcmp( sRespCode, "00", 2 ) ) {
        ErrHanding( pstTransJson, "96", "Ǯ������ʧ��[%s:%s].", sRespCode, sResvDesc );
    } else
        tLog( INFO, "����[%s]Ǯ������ɹ�.", sRrn );
    cJSON_Delete( pstNetJson );
    if ( NULL != pstRecvDataJson )
        cJSON_Delete( pstRecvDataJson );

    return 0;
}

void Net2UnFrozenWallet( cJSON *pstTransJson, cJSON *pstNetJson ) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = { 0 }, sSettleDate[8 + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sTrace[6 + 1] = { 0 };
    char sTransDate[8 + 1] = { 0 }, sTransTime[6 + 1] = { 0 }, sTransType[1 + 1] = { 0 }, sTransCode[6 + 1] = { 0 };
    char sCardType[1 + 1] = { 0 }, sInputMode[3 + 1] = { 0 }, sPostType[4 + 1] = { 0 };
    char sWalletFlag[1 + 1] = { 0 };

    GET_STR_KEY( pstTransJson, "merch_id", sMerchId );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_DOU_KEY( pstTransJson, "amount", dAmount );
    GET_DOU_KEY( pstTransJson, "fee", dFee );
    GET_STR_KEY( pstTransJson, "sys_trace", sTrace );
    GET_STR_KEY( pstTransJson, "trans_date", sTransDate );
    GET_STR_KEY( pstTransJson, "trans_time", sTransTime );
    GET_STR_KEY( pstTransJson, "settle_date", sSettleDate );
    GET_STR_KEY( pstTransJson, "trans_type", sTransType );
    GET_STR_KEY( pstTransJson, "card_type", sCardType );
    GET_STR_KEY( pstTransJson, "input_mode", sInputMode );
    GET_STR_KEY( pstTransJson, "trans_code", sTransCode );
    GET_STR_KEY( pstTransJson, "wallet_flag", sWalletFlag ); /*add by gjq �˻���ȡ����Ǯ�����Ͳ�����Ҫ*/
    //��������
    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    SET_STR_KEY( pstNetJson, "trans_type", "POS" ); //�����������п����Ƕ�ά��Ľ���
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY( pstNetJson, "transAmount", dAmount / 100 );
    SET_DOU_KEY( pstNetJson, "settleAmount", dAmount / 100 - dFee );
    SET_STR_KEY( pstNetJson, "sys_trace", sTrace );
    SET_STR_KEY( pstNetJson, "trans_date", sTransDate );
    SET_STR_KEY( pstNetJson, "trans_time", sTransTime );
    SET_STR_KEY( pstNetJson, "settle_date", sSettleDate );
    /* 
     * Ǯ�������ж�:100��D0Ǯ�� 101��D0����Ǯ�� 102��POS T1Ǯ��
     *  ��ǿ�(ic,����)-102
     *  ���ÿ�(����) 1������ 100; 1��-5�� 101
     *  ���ÿ�(ic) 100
     *  */
    /* add by gjq �˻���ȡ����Ǯ�����Ͳ�����Ҫ BENGIN*/
    tLog( DEBUG, "trans_code=%s,wallet_flag = [%s]", sTransCode, sWalletFlag );

    if ( !memcmp( sTransCode, "020001", 6 ) && sWalletFlag[0] != '\0' && sWalletFlag[0] != '0' ) {
        /* �˻����� �� pos ���������͵�Ǯ����D0������Ǯ����D0 ������Ǯ����������һ��Ǯ������ʱ��ȥ�ⶳ����㹻��Ǯ��*/
        if ( sWalletFlag[0] == '1' ) { /*1 - D0������Ǯ������*/
            SET_STR_KEY( pstNetJson, "walletType", "111" );
        } else if ( sWalletFlag[0] == '2' ) {/* 2 - D0������Ǯ������ */
            SET_STR_KEY( pstNetJson, "walletType", "110" );
        }
    } else {
        /*�˻�������pos ���������͵�Ǯ�������� ���� ���˻�����   �� ������������ ѡ��ⶳ��Ǯ������*/
        tLog( DEBUG, "trans_type=%s", sTransType );
        if ( sTransType[0] == '0' || sTransType[0] == 'S' || sTransType[0] == '1' ) {
            SET_STR_KEY( pstNetJson, "walletType", "110" );
        } else if ( sTransType[0] == '2' || sTransType[0] == '3' ) {
            SET_STR_KEY( pstNetJson, "walletType", "111" );
        }
    }
    /* add by gjq �˻���ȡ����Ǯ�����Ͳ�����Ҫ END */

    GetPostType( sPostType, sTransCode );
    SET_STR_KEY( pstNetJson, "postType", sPostType );
}

/* �������������׽�������ⶳǮ�����߻ָ�Ǯ��,ʧ�ܲ����� */
int UnFrozenWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, sRespCode, "����Ǯ��jsonʧ��,����." );
        return 0;
    }
    if ( !memcmp( sRespCode, "00", 2 ) ) {
        tLog( DEBUG, "����[%s]�ɹ�,�ⶳ����Ǯ��.", sRrn );
        Net2UnFrozenWallet( pstTransJson, pstNetJson );
        SET_STR_KEY( pstNetJson, "trans_code", "00T600" );
        tGetUniqueKey( stQMsgData.sSvrId );
        snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00T600%s", sRrn );
        stQMsgData.pstDataJson = pstNetJson;
        iRet = tSvcCall( "WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 20 );
        //iRet = tSvcCall("00T600_Q", pstNetJson, &pstRecvJson, 30);
        if ( iRet < 0 ) {
            if ( MSG_TIMEOUT == iRet ) {
                ErrHanding( pstTransJson, sRespCode, "Ǯ�����˳�ʱ,����." );
            } else
                ErrHanding( pstTransJson, sRespCode, "Ǯ������ʧ��,����." );
        } else {
            if ( NULL != pstRecvDataJson ) {
                GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
                GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
            }
            if ( memcmp( sRespCode, "00", 2 ) ) {
                ErrHanding( pstTransJson, sRespCode, "Ǯ���ⶳ����ʧ��,����." );
            } else {
                tLog( INFO, "Ǯ���ⶳ���˳ɹ�." );
            }
        }
        cJSON_Delete( pstNetJson );
        if ( NULL != pstRecvDataJson )
            cJSON_Delete( pstRecvDataJson );
        return 0;
    }
    tLog( INFO, "����[%s]ʧ��,�ⶳǮ��......", sRrn );
    Net2FrozenWallet( pstTransJson, pstNetJson );
    SET_STR_KEY( pstNetJson, "trans_code", "00T500" );
    tGetUniqueKey( stQMsgData.sSvrId );
    snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00T500%s", sRrn );
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall( "WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10 );
    //iRet = tSvcCall("00T500_Q", pstNetJson, &pstRecvJson, 30);
    if ( iRet < 0 ) {
        if ( MSG_TIMEOUT == iRet ) {
            ErrHanding( pstTransJson, sRespCode, "Ǯ�����˳�ʱ,����." );
        } else
            ErrHanding( pstTransJson, sRespCode, "Ǯ������ʧ��,����." );
        cJSON_Delete( pstNetJson );
        return 0;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    if ( NULL != pstRecvDataJson ) {
        GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
        GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
    }
    if ( memcmp( sRespCode, "00", 2 ) ) {
        ErrHanding( pstTransJson, sRespCode, "Ǯ���ⶳʧ��,����." );
    } else {
        tLog( INFO, "Ǯ���ⶳ�ɹ�." );
    }
    cJSON_Delete( pstNetJson );
    if ( NULL != pstRecvDataJson )
        cJSON_Delete( pstRecvDataJson );
    return 0;
}

void Net2D0Wallet( cJSON *pstTransJson, cJSON *pstNetJson ) {
    double dFee = 0.0, dAmount = 0.0;
    char sMerchId[15 + 1] = { 0 }, sSettleDate[8 + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sTrace[6 + 1] = { 0 };
    char sTransDate[8 + 1] = { 0 }, sTransTime[6 + 1] = { 0 }, sTransType[1 + 1] = { 0 };
    char sCardType[1 + 1] = { 0 }, sInputMode[3 + 1] = { 0 };

    GET_STR_KEY( pstTransJson, "merch_id", sMerchId );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_DOU_KEY( pstTransJson, "amount", dAmount );
    GET_DOU_KEY( pstTransJson, "fee", dFee );
    GET_STR_KEY( pstTransJson, "sys_trace", sTrace );
    GET_STR_KEY( pstTransJson, "trans_date", sTransDate );
    GET_STR_KEY( pstTransJson, "trans_time", sTransTime );
    GET_STR_KEY( pstTransJson, "settle_date", sSettleDate );
    GET_STR_KEY( pstTransJson, "trans_type", sTransType );

    tLog( DEBUG, "amount=%f", dAmount );

    tLog( DEBUG, "trans_type=%s", sTransType );
    //��������
    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    SET_STR_KEY( pstNetJson, "trans_type", "POS" ); //�����������п����Ƕ�ά��Ľ���
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY( pstNetJson, "transAmount", dAmount / 100 );
    SET_DOU_KEY( pstNetJson, "settleAmount", dAmount / 100 - dFee );
    SET_DOU_KEY( pstNetJson, "amount", dAmount / 100 - dFee );
    SET_STR_KEY( pstNetJson, "sys_trace", sRrn + 6 );
    SET_STR_KEY( pstNetJson, "trans_date", sTransDate );
    SET_STR_KEY( pstNetJson, "trans_time", sTransTime );
    SET_STR_KEY( pstNetJson, "settle_date", sSettleDate );
    SET_STR_KEY( pstNetJson, "walletType", "100" ); //Ǯ������
}

/* �ս�ȷ����������,ʧ����Ҫ��Ӧ���� */
int AddD0PosWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
#if 0
    if ( memcmp( sRespCode, "00", 2 ) ) {
        tLog( DEBUG, "�ս�ȷ�Ͻ���[%s]ʧ��[%s],��Ǯ��֪ͨ.", sRrn, sRespCode );
        return 0;
    }
#endif
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, "96", "��������Jsonʧ��." );
        return -1;
    }
    Net2D0Wallet( pstTransJson, pstNetJson );
    SET_STR_KEY( pstNetJson, "trans_code", "00T700" );

    MEMSET_ST( stQMsgData );
    snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00T700_%s", sRrn );
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcACall( "WALLET_Q", &stQMsgData );
    //iRet = tSvcACall("00T600_Q", &stQMsgData);
    if ( iRet < 0 ) {
        if ( MSG_TIMEOUT == iRet ) {
            ErrHanding( pstTransJson, "96", "Ǯ�����ֳ�ʱ,����." );
        } else
            ErrHanding( pstTransJson, "96", "Ǯ������ʧ��,����." );
        cJSON_Delete( pstNetJson );
        return -1;
    } else {
        tLog( INFO, "Ǯ�����������ͳɹ�." );
    }
    cJSON_Delete( pstNetJson );
    return 0;
}

/*POS�˻����� ��ѯǮ�����(����ѡ��Ǯ�����Ͷ�����㹻��Ǯ�����ж������)ȷ���˻�����Ǯ�������ͣ�ʧ����Ҫ��Ӧ���룬�����㣩add by gjq at 20171010*/
int QueryWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sMerchId[15 + 1] = { 0 }, sRespCode[2 + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    char sRrn[RRN_LEN + 1] = { 0 }, sTrace[6 + 1] = { 0 };
    double dAmount = 0.0, dPosD0Wallet = 0.00, dPosD0PreWallet = 0.00, dFee = 0;

    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, "96", "����Ǯ����ѯJsonʧ��." );
        return -1;
    }

    GET_STR_KEY( pstTransJson, "merch_id", sMerchId );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_STR_KEY( pstTransJson, "sys_trace", sTrace );

    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    SET_STR_KEY( pstNetJson, "sys_trace", sTrace );
    SET_STR_KEY( pstNetJson, "trans_code", "00AQ00" );

    tGetUniqueKey( stQMsgData.sSvrId );
    snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00AQ00_%s", sRrn );
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall( "WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10 );
    if ( iRet < 0 ) {
        if ( MSG_TIMEOUT == iRet ) {
            ErrHanding( pstTransJson, "Z1", "����[%s]Ǯ�����೬ʱ.", sRrn );
        } else {
            ErrHanding( pstTransJson, "Z1", "����[%s]Ǯ������ʧ��.", sRrn );
        }
        cJSON_Delete( pstNetJson );
        /* ��ѯǮ���˻���ʱ����ʧ�ܣ��� ֱ�ӱ����� Z1 - ����ʧ�ܣ����Ժ�����*/
        return ( -1 );
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
    GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
    if ( memcmp( sRespCode, "00", 2 ) ) {
        /*��ѯǮ�����ʧ�� �˺�������ִ�� ���г����̻�Ǯ������ ϵͳ���� 51-��������*/
        tLog( ERROR, "��ѯǮ�����ʧ��[%s:%s].", sRespCode, sResvDesc );
    } else {
        tLog( INFO, "����[%s]Ǯ������ɹ�.", sRrn );
    }

    GET_DOU_KEY( pstTransJson, "amount", dAmount );
    GET_DOU_KEY( pstTransJson, "fee", dFee );
    dAmount /= 100;
    dAmount = dAmount - dFee;
    /*�˻����� �˻�ʱ��Ҫ�������ѣ������˻�������Ϊԭ���׵Ľ��׽�� - ������*/
    tLog( INFO, "�˻���Ҫ������ [%.02f]Ԫ", dAmount );

    GET_DOU_KEY( pstRecvDataJson, "WalletAmt", dPosD0Wallet );
    GET_DOU_KEY( pstRecvDataJson, "PreWalletAmt", dPosD0PreWallet );
    if ( DBL_CMP( dAmount, dPosD0Wallet ) && DBL_CMP( dAmount, dPosD0PreWallet ) ) {
        tLog( WARN, "�̻�Ǯ�����㲻�������˻�����" );
        ErrHanding( pstTransJson, "51", "Ǯ������[%s].", sRrn );
        return ( -1 );
    } else if ( DBL_CMP( dAmount, dPosD0Wallet ) ) {
        tLog( INFO, "POS D0������Ǯ�����㣬dPosD0Wallet[%.02f],�����˻�����Ӧ�����Ǯ������Ϊ��111 - POS D0������ҵ�񶳽�Ǯ��.", dPosD0Wallet );
        /* 1 -  D0������Ǯ�����㣬�˻������˻�ʱ������D0������Ǯ�� */
        SET_STR_KEY( pstTransJson, "wallet_flag", "1" );
    } else if ( DBL_CMP( dAmount, dPosD0PreWallet ) ) {
        tLog( INFO, "POS D0������Ǯ�����㣬dPosD0PreWallet[%.02f], �����˻�����Ӧ�����Ǯ������Ϊ��110 - POS D0������ҵ�񶳽�Ǯ��.", dPosD0PreWallet );
        /*2 - D0������Ǯ�����㣬�˻������˻�ʱ������D0������Ǯ�� */
        SET_STR_KEY( pstTransJson, "wallet_flag", "2" );
    } else {
        tLog( INFO, "Ǯ�������㣬dPosD0Wallet[%.02f],dPosD0PreWallet[%.02f],�����˻����� ����������ȡ��ȡӦ�����Ǯ������", dPosD0Wallet, dPosD0PreWallet );
        /*0 - ����Ǯ�������㣬�˻������˻�ʱ������������ȡ��ȡ�����Ǯ������ */
        SET_STR_KEY( pstTransJson, "wallet_flag", "0" );
    }

    cJSON_Delete( pstNetJson );
    if ( NULL != pstRecvDataJson )
        cJSON_Delete( pstRecvDataJson );

    return ( 0 );
}

/* D0Ǯ������,�ս�����ʹ�� */
int ChkD0WalletType( cJSON *pstTransJson, int *piFlag ) {
    SET_STR_KEY( pstTransJson, "trans_type", "1" );
    return 0;
}

/* app���ͨ��ת��,ʧ����Ҫ��Ӧ����  */
int AddAppPosWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);
    GET_STR_KEY( pstTransJson, "o_resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "o_rrn", sRrn );
    if ( memcmp( sRespCode, "00", 2 ) ) {
        ErrHanding( pstTransJson, "96", "�սύ��[%s]ʧ��[%s],��Ǯ��֪ͨ.", sRrn, sRespCode );
        return -1;
    }
    /* ���͵Ľ��׶��� */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, "96", "��������Jsonʧ��." );
        return -1;
    }
    Net2D0Wallet( pstTransJson, pstNetJson );
    SET_STR_KEY( pstNetJson, "trans_code", "00T900" );
    tGetUniqueKey( stQMsgData.sSvrId );
    snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00T900%s", sRrn );
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall( "00T900_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10 );
    //iRet = tSvcCall("00T900_Q", pstNetJson, &pstRecvJson, 30);
    if ( iRet < 0 ) {
        if ( MSG_TIMEOUT == iRet ) {
            ErrHanding( pstTransJson, "96", "App����[%s]Ǯ��ת�˳�ʱ.", sRrn );
        } else
            ErrHanding( pstTransJson, "96", "App����[%s]Ǯ��ת��ʧ��.", sRrn );
        cJSON_Delete( pstNetJson );
        return -1;
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
        GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
        if ( memcmp( sRespCode, "00", 2 ) ) {
            ErrHanding( pstTransJson, "96", "App����[%s]Ǯ��ת��ʧ��[%s:%s].", sRrn, sRespCode, sResvDesc );
            cJSON_Delete( pstNetJson );
            if ( NULL != pstRecvDataJson )
                cJSON_Delete( pstRecvDataJson );
            return -1;
        } else {
            tLog( INFO, "App����[%s]Ǯ��ת�˳ɹ�.", sRrn );
        }
    }
    cJSON_Delete( pstNetJson );
    if ( NULL != pstRecvDataJson )
        cJSON_Delete( pstRecvDataJson );
    return 0;
}