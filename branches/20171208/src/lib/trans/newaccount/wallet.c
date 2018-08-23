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
/* 入账类型和交易码转换 */
PostType g_staPostType[] = {
    {"020000", "1000" },
    {"020003", "2001" },
    {"020002", "2000" },
    {"020023", "1001" },
    {"020001", "2002" },
    /* 预授权 */
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

/* 判断消费的钱包类型 */
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
    /* 判断钱包类型
     * 借记卡消费以及磁条信用卡消费1万以上入待审钱包 
     * 但是借记卡不鉴权
     * trans_type类型:0:D0,1:日结 2:D0待审鉴权，3:D0待审不鉴权
     *  商户在交易鉴权白名单中 且 磁条信用卡交易额小于等于2万（参数） 入D0钱包 0:D0  add by gjq at 20171019
     *  */
    if ( ( sCardType[0] == '1' )&&( sInputMode[1] == '2' ) ) {
        /*add by gjq at 20171019 BENGIN*/
        if ( FindValueByKey( sSingleAmt, "D0_TRANS_AMT" ) < 0 ) {
            tLog( ERROR, "查找key[D0_TRANS_AMT]D0限额值,失败." );
            /* 入待审 */
            sTransType[0] = '2';
        }
        dSingleAmt = atof( sSingleAmt );
        tLog( DEBUG, "交易金额[%f],磁条信用卡D0限额[%f].", dTranAmt, dSingleAmt );
        tLog( INFO, "检查商户消费认证白名单.(d0)" );
        snprintf( sSql, sizeof (sSql ), "SELECT COUNT(1) FROM B_MERCH_D0_TRANS "
                "WHERE MERCH_ID = '%s' AND STATUS = '1'", sMerchId );
        iNum = tQueryCount( sSql );
        if ( ( iNum > 0 && !DBL_CMP( dTranAmt, dSingleAmt ) ) || ( iNum > 0 && DBL_EQ( dTranAmt, dSingleAmt ) ) ) {
            tLog( DEBUG, "商户[%s]已在消费认证白名单中且磁条信用卡交易额[%.02f]小于等于[%.02f]", sMerchId, dTranAmt, dSingleAmt );
            sTransType[0] = '0';
        }/*add by gjq at 20171019 END*/
        else {
            tLog( WARN, "商户[%s]不在消费认证白名单 或者 此次交易磁条信用卡交易额超限.", sMerchId );
            if ( FindValueByKey( sLimitAmt, "D0_SINGLE_CASH_MIDDLE" ) < 0 ) {
                tLog( ERROR, "查找key[D0_SINGLE_CASH_MIDDLE]D0限额值,失败." );
                /* 入待审 */
                sTransType[0] = '2';
            }
            /* 判断交易金额是否大于此值 */
            dLimitAmt = atof( sLimitAmt );
            tLog( DEBUG, "交易金额[%f],磁条信用卡鉴权限额[%f].", dTranAmt, dLimitAmt );
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
    /* 如果是在白名单里，直接D0提现 */
    if ( '2' == sTransType[0] ) {
        tLog( DEBUG, "检查D0卡号白名单." );
        snprintf( sSql, sizeof (sSql ), "select count(1) from b_trans_card_white_list"
                " where account_no='%s'", sCardNo );
        iNum = tQueryCount( sSql );
        if ( iNum > 0 ) {
            tLog( DEBUG, "卡号[%s]已在白名单中,可以直接提现.", sCardNo );
            sTransType[0] = '0';
        } else {
            tLog( DEBUG, "卡号[%s]不在白名单中,提现需要审核.", sCardNo );
        }
    }
    SET_STR_KEY( pstTransJson, "trans_type", sTransType );
    tLog( INFO, "交易[%s]钱包类型[%s:%s].", sRrn, sTransType, sTransType[0] == '0' ? "D0钱包" : "D0待审钱包" );
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
    //组请求报文
    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    /* merch_id是为了打印日志 */
    SET_STR_KEY( pstNetJson, "merch_id", sMerchId );
    SET_STR_KEY( pstNetJson, "trans_type", "POS" ); //用于区分银行卡还是二维码的交易
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY( pstNetJson, "transAmount", dAmount / 100 );
    SET_DOU_KEY( pstNetJson, "settleAmount", dAmount / 100 - dFee );
    SET_STR_KEY( pstNetJson, "sys_trace", sTrace );
    SET_STR_KEY( pstNetJson, "trans_date", sTransDate );
    SET_STR_KEY( pstNetJson, "trans_time", sTransTime );
    SET_STR_KEY( pstNetJson, "settle_date", sSettleDate );
    /* 
     * 钱包类型判断:100、D0钱包 101、D0待审钱包 102、POS T1钱包
     *  借记卡(ic,磁条)-102
     *  信用卡(磁条) 1万以内 100; 1万-5万 101
     *  信用卡(ic) 100
     * 100、D0钱包 
     * 101、D0待审钱包 
     *  */
    if ( sTransType[0] == '0' || sTransType[0] == 'S' || sTransType[0] == '1' ) {
        SET_STR_KEY( pstNetJson, "walletType", "100" ); //钱包类型
    } else if ( sTransType[0] == '2' || sTransType[0] == '3' ) {
        SET_STR_KEY( pstNetJson, "walletType", "101" ); //钱包类型
    }
    GetPostType( sPostType, sTransCode );
    SET_STR_KEY( pstNetJson, "postType", sPostType );
}

/* 普通消费入账 ,失败不处理*/
int AddPosWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    if ( memcmp( sRespCode, "00", 2 ) ) {
        ErrHanding( pstTransJson, sRespCode, "交易失败[%s],不入账.", sRespCode );
        return 0;
    }
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, sRespCode, "创建入账Json失败." );
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
            ErrHanding( pstTransJson, sRespCode, "钱包入账超时,忽略." );
        } else
            ErrHanding( pstTransJson, sRespCode, "钱包入账失败,忽略." );
    } else {
        tLog( INFO, "钱包入账请求发送成功." );
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

    /*add by gjq 退货获取冻结钱包类型操作需要 BEGIN */
    GET_STR_KEY( pstTransJson, "trans_code", sTransCode );
    GET_STR_KEY( pstTransJson, "wallet_flag", sWalletFlag );
    tLog( DEBUG, "trans_code=%s,wallet_flag = [%s]", sTransCode, sWalletFlag );

    if ( !memcmp( sTransCode, "020001", 6 ) && sWalletFlag[0] != '\0' && sWalletFlag[0] != '0' ) {
        /* 退货交易 且 pos 的两个类型的钱包（D0可提现钱包、D0 待入账钱包）其中有一个钱包余额不足时，去冻结余额足够的钱包*/
        if ( sWalletFlag[0] == '1' ) { /*1 - D0可提现钱包余额不足*/
            SET_STR_KEY( pstNetJson, "frozenType", "23" );
            SET_STR_KEY( pstNetJson, "walletType", "111" );
        } else if ( sWalletFlag[0] == '2' ) {/* 2 - D0待入账钱包余额不足 */
            SET_STR_KEY( pstNetJson, "frozenType", "21" );
            SET_STR_KEY( pstNetJson, "walletType", "110" );
        }
    } else {
        /*退货交易且pos 的两个类型的钱包余额充足 或者 非退货交易   则 按照正常流程 选择冻结的钱包类型*/
        tLog( DEBUG, "trans_type=%s", sTransType );
        if ( sTransType[0] == '0' || sTransType[0] == 'S' || sTransType[0] == '1' ) {
            SET_STR_KEY( pstNetJson, "frozenType", "21" );
            SET_STR_KEY( pstNetJson, "walletType", "110" );
        } else if ( sTransType[0] == '2' || sTransType[0] == '3' ) {
            SET_STR_KEY( pstNetJson, "frozenType", "23" );
            SET_STR_KEY( pstNetJson, "walletType", "111" );
        }
    }
    /*add by gjq 退货获取冻结钱包类型操作需要 END */

    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    SET_STR_KEY( pstNetJson, "user_code", sUserCode );
    SET_DOU_KEY( pstNetJson, "amount", dAmount / 100 - dFee );
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    SET_STR_KEY( pstNetJson, "sys_trace", sTrace );
}

/* 撤销，冲正冻结钱包交易,失败需要赋应答码 */
int FrozenWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sMerchId[15 + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sRespCode[2 + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);

    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, "96", "创建冻结Json失败." );
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
            ErrHanding( pstTransJson, "96", "交易[%s]钱包冻结通知超时.", sRrn );
        } else
            ErrHanding( pstTransJson, "96", "交易[%s]钱包冻结通知失败.", sRrn );
        cJSON_Delete( pstNetJson );
        return -1;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
    GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
    if ( memcmp( sRespCode, "00", 2 ) ) {
        ErrHanding( pstTransJson, "96", "钱包冻结失败[%s:%s].", sRespCode, sResvDesc );
    } else
        tLog( INFO, "交易[%s]钱包冻结成功.", sRrn );
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
    GET_STR_KEY( pstTransJson, "wallet_flag", sWalletFlag ); /*add by gjq 退货获取冻结钱包类型操作需要*/
    //组请求报文
    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    SET_STR_KEY( pstNetJson, "trans_type", "POS" ); //用于区分银行卡还是二维码的交易
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY( pstNetJson, "transAmount", dAmount / 100 );
    SET_DOU_KEY( pstNetJson, "settleAmount", dAmount / 100 - dFee );
    SET_STR_KEY( pstNetJson, "sys_trace", sTrace );
    SET_STR_KEY( pstNetJson, "trans_date", sTransDate );
    SET_STR_KEY( pstNetJson, "trans_time", sTransTime );
    SET_STR_KEY( pstNetJson, "settle_date", sSettleDate );
    /* 
     * 钱包类型判断:100、D0钱包 101、D0待审钱包 102、POS T1钱包
     *  借记卡(ic,磁条)-102
     *  信用卡(磁条) 1万以内 100; 1万-5万 101
     *  信用卡(ic) 100
     *  */
    /* add by gjq 退货获取冻结钱包类型操作需要 BENGIN*/
    tLog( DEBUG, "trans_code=%s,wallet_flag = [%s]", sTransCode, sWalletFlag );

    if ( !memcmp( sTransCode, "020001", 6 ) && sWalletFlag[0] != '\0' && sWalletFlag[0] != '0' ) {
        /* 退货交易 且 pos 的两个类型的钱包（D0可提现钱包、D0 待入账钱包）其中有一个钱包余额不足时，去解冻余额足够的钱包*/
        if ( sWalletFlag[0] == '1' ) { /*1 - D0可提现钱包余额不足*/
            SET_STR_KEY( pstNetJson, "walletType", "111" );
        } else if ( sWalletFlag[0] == '2' ) {/* 2 - D0待入账钱包余额不足 */
            SET_STR_KEY( pstNetJson, "walletType", "110" );
        }
    } else {
        /*退货交易且pos 的两个类型的钱包余额充足 或者 非退货交易   则 按照正常流程 选择解冻的钱包类型*/
        tLog( DEBUG, "trans_type=%s", sTransType );
        if ( sTransType[0] == '0' || sTransType[0] == 'S' || sTransType[0] == '1' ) {
            SET_STR_KEY( pstNetJson, "walletType", "110" );
        } else if ( sTransType[0] == '2' || sTransType[0] == '3' ) {
            SET_STR_KEY( pstNetJson, "walletType", "111" );
        }
    }
    /* add by gjq 退货获取冻结钱包类型操作需要 END */

    GetPostType( sPostType, sTransCode );
    SET_STR_KEY( pstNetJson, "postType", sPostType );
}

/* 撤销，冲正交易结果处理，解冻钱包或者恢复钱包,失败不处理 */
int UnFrozenWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, sRespCode, "创建钱包json失败,忽略." );
        return 0;
    }
    if ( !memcmp( sRespCode, "00", 2 ) ) {
        tLog( DEBUG, "交易[%s]成功,解冻入账钱包.", sRrn );
        Net2UnFrozenWallet( pstTransJson, pstNetJson );
        SET_STR_KEY( pstNetJson, "trans_code", "00T600" );
        tGetUniqueKey( stQMsgData.sSvrId );
        snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00T600%s", sRrn );
        stQMsgData.pstDataJson = pstNetJson;
        iRet = tSvcCall( "WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 20 );
        //iRet = tSvcCall("00T600_Q", pstNetJson, &pstRecvJson, 30);
        if ( iRet < 0 ) {
            if ( MSG_TIMEOUT == iRet ) {
                ErrHanding( pstTransJson, sRespCode, "钱包入账超时,忽略." );
            } else
                ErrHanding( pstTransJson, sRespCode, "钱包入账失败,忽略." );
        } else {
            if ( NULL != pstRecvDataJson ) {
                GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
                GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
            }
            if ( memcmp( sRespCode, "00", 2 ) ) {
                ErrHanding( pstTransJson, sRespCode, "钱包解冻入账失败,忽略." );
            } else {
                tLog( INFO, "钱包解冻入账成功." );
            }
        }
        cJSON_Delete( pstNetJson );
        if ( NULL != pstRecvDataJson )
            cJSON_Delete( pstRecvDataJson );
        return 0;
    }
    tLog( INFO, "交易[%s]失败,解冻钱包......", sRrn );
    Net2FrozenWallet( pstTransJson, pstNetJson );
    SET_STR_KEY( pstNetJson, "trans_code", "00T500" );
    tGetUniqueKey( stQMsgData.sSvrId );
    snprintf( stQMsgData.sKey, sizeof (stQMsgData.sKey ), "00T500%s", sRrn );
    stQMsgData.pstDataJson = pstNetJson;
    iRet = tSvcCall( "WALLET_Q", &stQMsgData, stQMsgData.sSvrId, &stPMsgData, 10 );
    //iRet = tSvcCall("00T500_Q", pstNetJson, &pstRecvJson, 30);
    if ( iRet < 0 ) {
        if ( MSG_TIMEOUT == iRet ) {
            ErrHanding( pstTransJson, sRespCode, "钱包入账超时,忽略." );
        } else
            ErrHanding( pstTransJson, sRespCode, "钱包入账失败,忽略." );
        cJSON_Delete( pstNetJson );
        return 0;
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    if ( NULL != pstRecvDataJson ) {
        GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
        GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
    }
    if ( memcmp( sRespCode, "00", 2 ) ) {
        ErrHanding( pstTransJson, sRespCode, "钱包解冻失败,忽略." );
    } else {
        tLog( INFO, "钱包解冻成功." );
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
    //组请求报文
    SET_STR_KEY( pstNetJson, "merchantNo", sMerchId );
    SET_STR_KEY( pstNetJson, "trans_type", "POS" ); //用于区分银行卡还是二维码的交易
    SET_STR_KEY( pstNetJson, "rrn", sRrn );
    //SET_STR_KEY(pstNetJson, "accountUuid", sAcctUuid); //transAmount 
    SET_DOU_KEY( pstNetJson, "transAmount", dAmount / 100 );
    SET_DOU_KEY( pstNetJson, "settleAmount", dAmount / 100 - dFee );
    SET_DOU_KEY( pstNetJson, "amount", dAmount / 100 - dFee );
    SET_STR_KEY( pstNetJson, "sys_trace", sRrn + 6 );
    SET_STR_KEY( pstNetJson, "trans_date", sTransDate );
    SET_STR_KEY( pstNetJson, "trans_time", sTransTime );
    SET_STR_KEY( pstNetJson, "settle_date", sSettleDate );
    SET_STR_KEY( pstNetJson, "walletType", "100" ); //钱包类型
}

/* 日结确认消费提现,失败需要赋应答码 */
int AddD0PosWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;

    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
#if 0
    if ( memcmp( sRespCode, "00", 2 ) ) {
        tLog( DEBUG, "日结确认交易[%s]失败[%s],无钱包通知.", sRrn, sRespCode );
        return 0;
    }
#endif
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, "96", "创建发送Json失败." );
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
            ErrHanding( pstTransJson, "96", "钱包提现超时,忽略." );
        } else
            ErrHanding( pstTransJson, "96", "钱包提现失败,忽略." );
        cJSON_Delete( pstNetJson );
        return -1;
    } else {
        tLog( INFO, "钱包提现请求发送成功." );
    }
    cJSON_Delete( pstNetJson );
    return 0;
}

/*POS退货交易 查询钱包余额(用于选择钱包类型对余额足够的钱包进行冻结操作)确定退货冻结钱包的类型，失败需要赋应答码，（余额不足）add by gjq at 20171010*/
int QueryWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sMerchId[15 + 1] = { 0 }, sRespCode[2 + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    char sRrn[RRN_LEN + 1] = { 0 }, sTrace[6 + 1] = { 0 };
    double dAmount = 0.0, dPosD0Wallet = 0.00, dPosD0PreWallet = 0.00, dFee = 0;

    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, "96", "创建钱包查询Json失败." );
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
            ErrHanding( pstTransJson, "Z1", "交易[%s]钱包查余超时.", sRrn );
        } else {
            ErrHanding( pstTransJson, "Z1", "交易[%s]钱包查余失败.", sRrn );
        }
        cJSON_Delete( pstNetJson );
        /* 查询钱包账户余额超时或者失败，则 直接报错返回 Z1 - 交易失败，请稍后重试*/
        return ( -1 );
    }
    pstRecvDataJson = stPMsgData.pstDataJson;
    GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
    GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
    if ( memcmp( sRespCode, "00", 2 ) ) {
        /*查询钱包余额失败 此函数继续执行 会判出：商户钱包余额不足 系统返回 51-可用余额不足*/
        tLog( ERROR, "查询钱包余额失败[%s:%s].", sRespCode, sResvDesc );
    } else {
        tLog( INFO, "交易[%s]钱包查余成功.", sRrn );
    }

    GET_DOU_KEY( pstTransJson, "amount", dAmount );
    GET_DOU_KEY( pstTransJson, "fee", dFee );
    dAmount /= 100;
    dAmount = dAmount - dFee;
    /*退货交易 退货时需要扣手续费，所以退货冻结金额为原交易的交易金额 - 手续费*/
    tLog( INFO, "退货需要冻结金额 [%.02f]元", dAmount );

    GET_DOU_KEY( pstRecvDataJson, "WalletAmt", dPosD0Wallet );
    GET_DOU_KEY( pstRecvDataJson, "PreWalletAmt", dPosD0PreWallet );
    if ( DBL_CMP( dAmount, dPosD0Wallet ) && DBL_CMP( dAmount, dPosD0PreWallet ) ) {
        tLog( WARN, "商户钱包余额不足不允许本次退货操作" );
        ErrHanding( pstTransJson, "51", "钱包余额不足[%s].", sRrn );
        return ( -1 );
    } else if ( DBL_CMP( dAmount, dPosD0Wallet ) ) {
        tLog( INFO, "POS D0可提现钱包余额不足，dPosD0Wallet[%.02f],本次退货交易应冻结的钱包类型为：111 - POS D0待入账业务冻结钱包.", dPosD0Wallet );
        /* 1 -  D0可提现钱包余额不足，退货冻结账户时，冻结D0待入账钱包 */
        SET_STR_KEY( pstTransJson, "wallet_flag", "1" );
    } else if ( DBL_CMP( dAmount, dPosD0PreWallet ) ) {
        tLog( INFO, "POS D0待入账钱包余额不足，dPosD0PreWallet[%.02f], 本次退货交易应冻结的钱包类型为：110 - POS D0可提现业务冻结钱包.", dPosD0PreWallet );
        /*2 - D0待入账钱包余额不足，退货冻结账户时，冻结D0可提现钱包 */
        SET_STR_KEY( pstTransJson, "wallet_flag", "2" );
    } else {
        tLog( INFO, "钱包余额充足，dPosD0Wallet[%.02f],dPosD0PreWallet[%.02f],本次退货交易 按正常流程取获取应冻结的钱包类型", dPosD0Wallet, dPosD0PreWallet );
        /*0 - 两个钱包余额充足，退货冻结账户时，按正常流程取获取冻结的钱包类型 */
        SET_STR_KEY( pstTransJson, "wallet_flag", "0" );
    }

    cJSON_Delete( pstNetJson );
    if ( NULL != pstRecvDataJson )
        cJSON_Delete( pstRecvDataJson );

    return ( 0 );
}

/* D0钱包类型,日结消费使用 */
int ChkD0WalletType( cJSON *pstTransJson, int *piFlag ) {
    SET_STR_KEY( pstTransJson, "trans_type", "1" );
    return 0;
}

/* app审核通过转账,失败需要赋应答码  */
int AddAppPosWallet( cJSON *pstTransJson, int *piFlag ) {
    cJSON *pstNetJson = NULL, *pstRecvDataJson = NULL;
    char sRespCode[RESP_CODE_LEN + 1] = { 0 }, sRrn[RRN_LEN + 1] = { 0 }, sResvDesc[4096 + 1] = { 0 };
    int iRet = -1;
    MsgData stQMsgData, stPMsgData;
    //DUMP_JSON(pstJson);
    GET_STR_KEY( pstTransJson, "o_resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "o_rrn", sRrn );
    if ( memcmp( sRespCode, "00", 2 ) ) {
        ErrHanding( pstTransJson, "96", "日结交易[%s]失败[%s],无钱包通知.", sRrn, sRespCode );
        return -1;
    }
    /* 发送的交易对象 */
    pstNetJson = cJSON_CreateObject( );
    if ( NULL == pstNetJson ) {
        ErrHanding( pstTransJson, "96", "创建发送Json失败." );
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
            ErrHanding( pstTransJson, "96", "App交易[%s]钱包转账超时.", sRrn );
        } else
            ErrHanding( pstTransJson, "96", "App交易[%s]钱包转账失败.", sRrn );
        cJSON_Delete( pstNetJson );
        return -1;
    } else {
        pstRecvDataJson = stPMsgData.pstDataJson;
        GET_STR_KEY( pstRecvDataJson, "resp_code", sRespCode );
        GET_STR_KEY( pstRecvDataJson, "resp_desc", sResvDesc );
        if ( memcmp( sRespCode, "00", 2 ) ) {
            ErrHanding( pstTransJson, "96", "App交易[%s]钱包转账失败[%s:%s].", sRrn, sRespCode, sResvDesc );
            cJSON_Delete( pstNetJson );
            if ( NULL != pstRecvDataJson )
                cJSON_Delete( pstRecvDataJson );
            return -1;
        } else {
            tLog( INFO, "App交易[%s]钱包转账成功.", sRrn );
        }
    }
    cJSON_Delete( pstNetJson );
    if ( NULL != pstRecvDataJson )
        cJSON_Delete( pstRecvDataJson );
    return 0;
}