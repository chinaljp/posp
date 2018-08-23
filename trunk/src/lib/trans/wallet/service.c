/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <zlog.h>
#include "t_redis.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_macro.h"

/* ��ܵĺ��� */
int SendRepMsg( cJSON *pstJson );
char *GetSvrId( );

/* ģ���ʼ���ӿ�:�����Զ���������в����������Ի���ʼ�� */
int AppServerInit( int iArgc, char *pcArgv[] ) {
    return 0;
}

/* ģ���˳�����ӿ�:��Գ�ʼ���еĲ����������� */
int AppServerDone( void ) {
    return 0;
}

/* ת������Ҫ����pcKey��������Ȳ���Ҫת��������pcKey=NULL 
 * ֻ�������л��
 */
int AppGetKey( char *pcKey, size_t tKey, cJSON *pstJson ) {
    cJSON *pstDataJson = NULL;
    //char sSysTrace[TRACE_NO_LEN + 1] = {0};
    char sRrn[12 + 1] = { 0 };

    pstDataJson = GET_JSON_KEY( pstJson, "data" );
    if ( NULL == pstDataJson ) {
        tLog( ERROR, "��ȡdata����ʧ��,��Ϣ��������." );
        return -1;
    }
    //GET_STR_KEY(pstDataJson, "sys_trace", sSysTrace);
    //  GET_STR_KEY(pstDataJson, "trans_date", sTransDate);
    GET_STR_KEY( pstDataJson, "rrn", sRrn );

    snprintf( pcKey, tKey, "WALLET_%s", sRrn );
    tLog( DEBUG, "Key[%s]", pcKey );
    return 1;
}

int GetSvrName( char *pcSvrId, size_t tSvrId, cJSON *pstJson ) {
    char sSvrId[SVRID_LEN + 1] = { 0 }, sTransCode[6 + 1] = { 0 };
    cJSON *pstDataJson = NULL;

    pstDataJson = GET_JSON_KEY( pstJson, "data" );
    if ( NULL == pstDataJson ) {
        tLog( ERROR, "��ȡdata����ʧ��,��Ϣ��������." );
        return -1;
    }
    GET_STR_KEY( pstDataJson, "trans_code", sTransCode );
    if ( NULL != strstr( "00T400,00T500,00T600,00T700,00T800,00T900,00AQ00", sTransCode ) ) {
        strcpy( pcSvrId, "TRANSD_Q" );
    } else {
        strcpy( pcSvrId, "ACCTD_Q" );
    }

    return 0;
}

/* �մ��� */
int NullProc( cJSON *pstJson, int *piFlag ) {
    return 0;
}

/* ���ش�����Ӧ�����Ϣcopy��data�� */
int RespPro( cJSON *pstJson, int *piFlag ) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL, *pstSubDataJson = NULL;
    char sTransCode[6 + 1] = { 0 }, sData[2048] = { 0 }, sUuid[128 + 1] = { 0 };
    double dPosFrozenAmt = { 0 }, dQrFrozenAmt = { 0 };
    double dPosThawAmt = { 0 }, dQrThawAmt = { 0 };
    double dPosDeductAmt = { 0 }, dQrDeductAmt = { 0 };
    char sRespCode[16 + 1] = { 0 }, sRespDesc[4096 + 1] = { 0 };
    double dFAmount = 0, dTAmount = 0, dDeAmount = 0;
    char *pcStr = NULL;

    pstDataJson = GET_JSON_KEY( pstJson, "data" );
    pstRepJson = GET_JSON_KEY( pstJson, "response" );

    COPY_JSON( pstDataJson, pstRepJson );
    DUMP_JSON( pstDataJson, pcStr )
#if 0
            /*
                posFrozenTotalAmount : String,		// POS�Ѷ����ܽ��
             * 			qrCodeFrozenTotalAmount : String,	//��ά���Ѷ����ܽ��
             * 			planFrozenAmount : String,			//�ƻ�������
             * 			posThawAmount : String,			// POS�ѽⶳ���
             * 			qrCodeThawAmount : String,		//��ά���ѽⶳ���
             * 	 * 			posDeductAmount : String,			// POS�ѿ۳����
             * 			qrCodeDeductAmount : String,		//��ά���ѿ۳����

             */
            GET_STR_KEY( pstDataJson, "trans_code", sTransCode );
    if ( NULL != strstr( "00TF00", sTransCode ) ) {
        GET_STR_KEY( pstDataJson, "resp_code", sRespCode );
        if ( memcmp( sRespCode, "00", 2 ) ) {
            tLog( ERROR, "����ʧ��[%s].", sRespCode );
            return 0;
        }
        GET_STR_KEY( pstDataJson, "data", sData );
        tLog( DEBUG, "data[%s]", sData );
        pstSubDataJson = cJSON_Parse( sData );
        if ( NULL == pstSubDataJson ) {
            tLog( ERROR, "��data����." );
            return 0;
        }
        GET_DOU_KEY( pstSubDataJson, "posFrozenTotalAmount", dPosFrozenAmt ); //������
        GET_DOU_KEY( pstSubDataJson, "qrCodeFrozenTotalAmount", dQrFrozenAmt ); //������
        GET_DOU_KEY( pstSubDataJson, "posThawAmount", dPosThawAmt ); //�ѽⶳ���
        GET_DOU_KEY( pstSubDataJson, "qrCodeThawAmount", dQrThawAmt ); //�ѽⶳ���
        GET_DOU_KEY( pstSubDataJson, "posDeductAmount", dPosDeductAmt ); //�ѿ۳����
        GET_DOU_KEY( pstSubDataJson, "qrCodeDeductAmount", dQrDeductAmt ); //���ѿ۳����

        dFAmount = dPosFrozenAmt + dQrFrozenAmt;
        dTAmount = dPosThawAmt + dQrThawAmt;
        dDeAmount = dPosDeductAmt + dQrDeductAmt;

        SET_DOU_KEY( pstDataJson, "frozenTotalAmount", dFAmount );
        SET_DOU_KEY( pstDataJson, "thawAmount", dTAmount );
        SET_DOU_KEY( pstDataJson, "deductAmount", dDeAmount );
        cJSON_Delete( pstSubDataJson );
    } else if ( NULL != strstr( "00T000", sTransCode ) ) {
        GET_STR_KEY( pstDataJson, "resp_code", sRespCode );
        if ( memcmp( sRespCode, "00", 2 ) ) {
            tLog( ERROR, "����ʧ��[%s].", sRespCode );

            return 0;
        }
        GET_STR_KEY( pstDataJson, "data", sData );
        tLog( DEBUG, "data[%s]", sData );
        pstSubDataJson = cJSON_Parse( sData );
        if ( NULL == pstSubDataJson ) {
            tLog( ERROR, "��data����." );
            return 0;
        }
        GET_STR_KEY( pstSubDataJson, "accountUuid", sUuid );
        SET_STR_KEY( pstDataJson, "accountUuid", sUuid );
        cJSON_Delete( pstSubDataJson );
    }
#endif
    return 0;
}

/* ��ѯǮ���˻���� ���ش��� */
int RespQueryPro( cJSON *pstJson, int *piFlag ) {
    cJSON *pstDataJson = NULL, *pstRepJson = NULL, *pstSubDataJson = NULL;
    char sRespCode[2 + 1] = { 0 }, sRespDesc[256] = { 0 }, sData[1024] = { 0 };
    double dWallet = 0, dFrozenWallet = 0;
    double dPosD0Wallet = 0, dPosD0BusinessFrozenWallet = 0, dPosD0PreWallet = 0, dPosD0BusinessFrozenPreWallet = 0, dPosT1Wallet = 0, dPosT1BusinessFrozenWallet = 0;
    double dQrCodeD0Wallet = 0, dQrCodeD0BusinessFrozenWallet = 0, dQrCodeD0PreWallet = 0, dQrCodeD0BusinessFrozenPreWallet = 0, dQrCodeT1Wallet = 0, dQrCodeT1BusinessFrozenWallet = 0;

    pstDataJson = GET_JSON_KEY( pstJson, "data" );
    pstRepJson = GET_JSON_KEY( pstJson, "response" );

    GET_STR_KEY( pstRepJson, "resp_code", sRespCode );
    GET_STR_KEY( pstRepJson, "resp_desc", sRespDesc );
    GET_STR_KEY( pstRepJson, "data", sData );
    tLog( DEBUG, "data[%s]", sData );

    pstSubDataJson = cJSON_Parse( sData );
    if ( NULL == pstSubDataJson ) {
        tLog( ERROR, "��data����." );
        return 0;
    }
    //POS D0������Ǯ��
    GET_DOU_KEY( pstSubDataJson, "posD0Wallet", dPosD0Wallet );
    SET_DOU_KEY( pstDataJson, "posD0Wallet", dPosD0Wallet );
    //D0ҵ�񶳽�Ǯ��
    GET_DOU_KEY( pstSubDataJson, "posD0BusinessFrozenWallet", dPosD0BusinessFrozenWallet );
    SET_DOU_KEY( pstDataJson, "posD0BusinessFrozenWallet", dPosD0BusinessFrozenWallet );
    //D0 ������Ǯ��
    GET_DOU_KEY( pstSubDataJson, "posD0PreWallet", dPosD0PreWallet );
    SET_DOU_KEY( pstDataJson, "posD0PreWallet", dPosD0PreWallet );
    //D0������ҵ�񶳽�Ǯ��
    GET_DOU_KEY( pstSubDataJson, "posD0BusinessFrozenPreWallet", dPosD0BusinessFrozenPreWallet );
    SET_DOU_KEY( pstDataJson, "posD0BusinessFrozenPreWallet", dPosD0BusinessFrozenPreWallet );
    //POS T1������Ǯ��
    GET_DOU_KEY( pstSubDataJson, "posT1Wallet", dPosT1Wallet );
    SET_DOU_KEY( pstDataJson, "posT1Wallet", dPosT1Wallet );
    //POS T1ҵ�񶳽�Ǯ��
    GET_DOU_KEY( pstSubDataJson, "posT1BusinessFrozenWallet", dPosT1BusinessFrozenWallet );
    SET_DOU_KEY( pstDataJson, "posT1BusinessFrozenWallet", dPosT1BusinessFrozenWallet );
    //��ά��D0������Ǯ��
    GET_DOU_KEY( pstSubDataJson, "qrCodeD0Wallet", dQrCodeD0Wallet );
    SET_DOU_KEY( pstDataJson, "qrCodeD0Wallet", dQrCodeD0Wallet );
    //��ά��D0ҵ�񶳽�Ǯ��
    GET_DOU_KEY( pstSubDataJson, "qrCodeD0BusinessFrozenWallet", dQrCodeD0BusinessFrozenWallet );
    SET_DOU_KEY( pstDataJson, "qrCodeD0BusinessFrozenWallet", dQrCodeD0BusinessFrozenWallet );
    //��ά��D0������Ǯ��
    GET_DOU_KEY( pstSubDataJson, "qrCodeD0PreWallet", dQrCodeD0PreWallet );
    SET_DOU_KEY( pstDataJson, "qrCodeD0PreWallet", dQrCodeD0PreWallet );
    //��ά��D0������ҵ�񶳽�Ǯ��
    GET_DOU_KEY( pstSubDataJson, "qrCodeD0BusinessFrozenPreWallet", dQrCodeD0BusinessFrozenPreWallet );
    SET_DOU_KEY( pstDataJson, "qrCodeD0BusinessFrozenPreWallet", dQrCodeD0BusinessFrozenPreWallet );
    //��ά��T1Ǯ��
    GET_DOU_KEY( pstSubDataJson, "qrCodeT1Wallet", dQrCodeT1Wallet );
    SET_DOU_KEY( pstDataJson, "qrCodeT1Wallet", dQrCodeT1Wallet );
    //��ά��T1ҵ�񶳽�Ǯ��
    GET_DOU_KEY( pstSubDataJson, "qrCodeT1BusinessFrozenWallet", dQrCodeT1BusinessFrozenWallet );
    SET_DOU_KEY( pstDataJson, "qrCodeT1BusinessFrozenWallet", dQrCodeT1BusinessFrozenWallet );
    //������Ǯ�����
    GET_DOU_KEY( pstSubDataJson, "wallet", dWallet );
    SET_DOU_KEY( pstDataJson, "wallet", dWallet );
    //�����̶�����
    GET_DOU_KEY( pstSubDataJson, "frozenWallet", dFrozenWallet );
    SET_DOU_KEY( pstDataJson, "frozenWallet", dFrozenWallet );

    SET_STR_KEY( pstDataJson, "resp_code", sRespCode );
    SET_STR_KEY( pstDataJson, "resp_desc", sRespDesc );
    //�����˻�ʹ��
    SET_DOU_KEY( pstDataJson, "WalletAmt", dPosD0Wallet );
    SET_DOU_KEY( pstDataJson, "PreWalletAmt", dPosD0PreWallet );

    cJSON_Delete( pstSubDataJson );

    return ( 0 );
}
