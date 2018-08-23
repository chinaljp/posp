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
#include "t_db.h"
#include "t_redis.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_macro.h"

int UpdNoticeStatus( char *pcStatus, char *pcId ) {
    char sSqlStr[256] = { 0 };
    OCI_Resultset *pstRes = NULL;

    snprintf( sSqlStr, sizeof (sSqlStr ), "update B_NOTICE_WAllET set status='%s' where id='%s'", pcStatus, pcId );
    if ( tExecute( &pstRes, sSqlStr ) < 0 ) {
        tLog( ERROR, "96", "更新id[%s]通知状态失败.", pcId );
        return -1;
    }
    tLog( DEBUG, "pstRes=%p", pstRes );
    if ( 0 == tGetAffectedRows ) {
        tReleaseRes( pstRes );
        return -1;
    }
    tReleaseRes( pstRes );
    return 0;
}

int DelNotice( char *pcId ) {
    char sSqlStr[256] = { 0 };
    OCI_Resultset *pstRes = NULL;

    snprintf( sSqlStr, sizeof (sSqlStr ), "delete from B_NOTICE_WAllET  where id='%s'", pcId );
    if ( tExecute( &pstRes, sSqlStr ) < 0 ) {
        tLog( ERROR, "96", "更新id[%s]通知状态失败.", pcId );
        return -1;
    }
    tLog( DEBUG, "pstRes=%p,%d", pstRes, tGetAffectedRows( ) );
    if ( 0 == tGetAffectedRows( ) ) {
        tReleaseRes( pstRes );
        return -1;
    }
    tReleaseRes( pstRes );
    return 0;
}

/* 通知表在处理时，设置为处理中状态 */
int ReqNoticeStatus( cJSON *pstJson, int *piFlag ) {
    char sId[32 + 1] = { 0 };
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY( pstJson, "data" );
    GET_STR_KEY( pstTransJson, "id", sId );
    if ( UpdNoticeStatus( "1", sId ) < 0 ) {
        //ErrHanding(pstTransJson, "96", "无id[%s]的记录.", sId);
        return 0;
    }
    return 0;
}

/* 更新记录通知次数 */
int UpdNoticeCnt( char *pcId ) {
    char sSqlStr[1024] = { 0 };
    char sDate[8 + 1] = { 0 }, sTime[6 + 1] = { 0 };
    OCI_Resultset *pstRes = NULL;

    tGetDate( sDate, "", -1 );
    tGetTime( sTime, "", -1 );
    snprintf( sSqlStr, sizeof (sSqlStr ), "update B_NOTICE_WAllET set notice_cnt=notice_cnt+1"
            ", notice_date='%s', notice_time='%s' "
            "where id='%s'", sDate, sTime, pcId );

    tLog( DEBUG, "sql[%s]", sSqlStr );
    if ( tExecute( &pstRes, sSqlStr ) < 0 ) {
        return -1;
    }
    tLog( DEBUG, "pstRes=%p,%d", pstRes, tGetAffectedRows( ) );
    if ( 0 == tGetAffectedRows( ) ) {
        tLog( WARN, "无id[%s]的通知记录.", pcId );
        tReleaseRes( pstRes );
        return 0;
    }
    tLog( ERROR, "记录[%s]通知失败,次数+1.", pcId );
    tReleaseRes( pstRes );
    return 0;
}

/*添加，修改,结果处理*/
int ResNoticeStatus( cJSON *pstJson, int *piFlag ) {
    char sId[32 + 1] = { 0 }, sRespCode[32 + 1] = { 0 };
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY( pstJson, "data" );
    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "id", sId );
    //notice_wallet和webd都使用这个交易流
    if ( '\0' == sId[0] ) {
        //webd发送的交易
        return 0;
    }
    if ( !memcmp( sRespCode, "00", 2 ) ) {
        if ( DelNotice( sId ) < 0 ) {
            ErrHanding( pstTransJson, sRespCode, "无id[%s]的记录.", sId );
            return 0;
        }
    } else {
        if ( UpdNoticeStatus( "0", sId ) < 0 ) {
            ErrHanding( pstTransJson, sRespCode, "无id[%s]的记录.", sId );
            return 0;
        }
        if ( UpdNoticeCnt( sId ) < 0 ) {
            ErrHanding( pstTransJson, sRespCode, "无id[%s]的记录.", sId );
            return 0;
        }
    }
    return 0;
}

int AddMerchAcctId( char *pcMerchId, char *pcAcctId ) {
    char sSqlStr[1024] = { 0 };
    char sDate[8 + 1] = { 0 }, sTime[6 + 1] = { 0 };
    OCI_Resultset *pstRes = NULL;

    tGetDate( sDate, "", -1 );
    tGetTime( sTime, "", -1 );
    snprintf( sSqlStr, sizeof (sSqlStr ), "update B_MERCH_ACCOUNT set account_uuid='%s'"
            "where merch_id='%s'", pcAcctId, pcMerchId );

    if ( tExecute( &pstRes, sSqlStr ) < 0 ) {
        return -1;
    }
    if ( 0 == tGetAffectedRows( ) ) {
        tLog( WARN, "无商户[%s]的账户信息,无法更新钱包id[%s].", pcMerchId, pcAcctId );
        tReleaseRes( pstRes );
        return 0;
    }
    tReleaseRes( pstRes );
    return 0;
}

int AddAgentAcctId( char *pcAgentId, char *pcAcctId ) {
    char sSqlStr[1024] = { 0 };
    char sDate[8 + 1] = { 0 }, sTime[6 + 1] = { 0 };
    OCI_Resultset *pstRes = NULL;
    tGetDate( sDate, "", -1 );
    tGetTime( sTime, "", -1 );
    snprintf( sSqlStr, sizeof (sSqlStr ), "update B_AGENT set account_uuid='%s'"
            "where agent_id='%s'", pcAcctId, pcAgentId );
    if ( tExecute( &pstRes, sSqlStr ) < 0 ) {
        return -1;
    }
    if ( 0 == tGetAffectedRows( ) ) {
        tLog( WARN, "无代理商[%s]的账户信息,无法更新钱包id[%s].", pcAgentId, pcAcctId );
        tReleaseRes( pstRes );
        return 0;
    }
    tReleaseRes( pstRes );
    return 0;
}

int AddAccountId( cJSON *pstJson, int *piFlag ) {
    char sId[32 + 1] = { 0 }, sRespCode[32 + 1] = { 0 }, sRespData[512] = { 0 }, sUuid[64 + 1] = { 0 };
    char sMerchId[MERCH_ID_LEN + 1] = { 0 };
    cJSON *pstTransJson = NULL, *pstAcctJson = NULL;

    pstTransJson = GET_JSON_KEY( pstJson, "data" );
    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "merchantNo", sMerchId );
    if ( !memcmp( sRespCode, "00", 2 ) ) {
        GET_STR_KEY( pstTransJson, "data", sRespData );
        pstAcctJson = cJSON_Parse( sRespData );
        if ( NULL == pstAcctJson ) {
            ErrHanding( pstTransJson, "96", "获取账户uuid失败,放弃处理." );
            return -1;
        }
        GET_STR_KEY( pstAcctJson, "accountUuid", sUuid );
        if ( AddMerchAcctId( sMerchId, sUuid ) < 0 ) {
            ErrHanding( pstTransJson, "96", "添加商户[%s]账户[%s].", sMerchId, sUuid );
            cJSON_Delete( pstAcctJson );
            return -1;
        } else
            tLog( INFO, "添加商户[%s]账户[%s]成功.", sMerchId, sUuid );
        cJSON_Delete( pstAcctJson );
    }
    return 0;
}

int AddAgentAccountId( cJSON *pstJson, int *piFlag ) {
    char sId[32 + 1] = { 0 }, sRespCode[32 + 1] = { 0 }, sRespData[512] = { 0 }, sUuid[64 + 1] = { 0 };
    char sAgentId[8 + 1] = { 0 };
    cJSON *pstTransJson = NULL, *pstAcctJson = NULL;
    pstTransJson = GET_JSON_KEY( pstJson, "data" );
    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "agentNo", sAgentId );
    if ( !memcmp( sRespCode, "00", 2 ) ) {
        GET_STR_KEY( pstTransJson, "data", sRespData );
        pstAcctJson = cJSON_Parse( sRespData );
        if ( NULL == pstAcctJson ) {
            ErrHanding( pstTransJson, "96", "获取账户uuid失败,放弃处理." );
            return -1;
        }
        GET_STR_KEY( pstAcctJson, "accountUuid", sUuid );
        if ( AddAgentAcctId( sAgentId, sUuid ) < 0 ) {
            ErrHanding( pstTransJson, "96", "添加代理商[%s]账户[%s].", sAgentId, sUuid );
            cJSON_Delete( pstAcctJson );
            return -1;
        } else
            tLog( INFO, "添加代理商[%s]账户[%s]成功.", sAgentId, sUuid );
        cJSON_Delete( pstAcctJson );
    }
    return 0;
}

int UpdPosTrans( char *pcStatus, char *pcRrn, char *pcTransDate ) {
    char sSqlStr[1024] = { 0 };
    char sDate[8 + 1] = { 0 }, sTime[6 + 1] = { 0 };
    OCI_Resultset *pstRes = NULL;

    tGetDate( sDate, "", -1 );
    tGetTime( sTime, "", -1 );
    snprintf( sSqlStr, sizeof (sSqlStr ), "update B_POS_TRANS_DETAIL set settle_flag='%s'"
            "where trans_date='%s' and rrn='%s'", pcStatus, pcTransDate, pcRrn );

    if ( tExecute( &pstRes, sSqlStr ) < 0 ) {
        return -1;
    }
    if ( 0 == tGetAffectedRows( ) ) {
        tLog( ERROR, "钱包通知成功,更新交易[%s:%s]结算状态[%s],失败.", pcTransDate, pcRrn, pcStatus );
        tReleaseRes( pstRes );
        return 0;
    }
    tReleaseRes( pstRes );
    tLog( INFO, "钱包通知成功,更新交易[%s:%s]结算状态[%s].", pcTransDate, pcRrn, pcStatus );
    return 0;
}

/* 更新二维码交易 */
int UpdQrTrans( char *pcStatus, char *pcRrn, char *pcTransDate ) {
    char sSqlStr[1024] = { 0 };
    char sDate[8 + 1] = { 0 }, sTime[6 + 1] = { 0 };
    OCI_Resultset *pstRes = NULL;

    tGetDate( sDate, "", -1 );
    tGetTime( sTime, "", -1 );
    snprintf( sSqlStr, sizeof (sSqlStr ), "update B_INLINE_TARNS_DETAIL set settle_flag='%s'"
            "where trans_date='%s' and rrn='%s'", pcStatus, pcTransDate, pcRrn );

    if ( tExecute( &pstRes, sSqlStr ) < 0 ) {
        return -1;
    }
    if ( 0 == tGetAffectedRows( ) ) {
        tLog( ERROR, "钱包通知成功,更新交易[%s:%s]结算状态[%s],失败.", pcTransDate, pcRrn, pcStatus );
        tReleaseRes( pstRes );
        return 0;
    }
    tReleaseRes( pstRes );
    tLog( INFO, "钱包通知成功,更新交易[%s:%s]结算状态[%s].", pcTransDate, pcRrn, pcStatus );
    return 0;
}

/* 充值 */
int RechargeTransStatus( cJSON *pstJson, int *piFlag ) {
    char sRrn[12 + 1] = { 0 }, sRespCode[32 + 1] = { 0 }, sTransDate[8 + 1] = { 0 }, sTransType[16 + 1] = { 0 };
    char sRespLog[4096 + 1] = { 0 };
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY( pstJson, "data" );
    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_STR_KEY( pstTransJson, "trans_date", sTransDate );
    GET_STR_KEY( pstTransJson, "trans_type", sTransType );
    GET_STR_KEY( pstTransJson, "resp_desc", sRespLog );
    if ( !memcmp( sRespCode, "00", 2 ) ) {
        if ( !memcmp( sTransType, "POS", 3 ) ) {
            if ( UpdPosTrans( "M", sRrn, sTransDate ) < 0 ) {
                ErrHanding( pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog );
                return -1;
            }
        } else {
            if ( UpdQrTrans( "M", sRrn, sTransDate ) < 0 ) {
                ErrHanding( pstTransJson, "96", "无rrn[%s]的二维码交易流水[%s].", sRrn, sRespLog );
                return -1;
            }
        }
    } else if ( !memcmp( sRespCode, "03010027", 8 ) || !memcmp( sRespCode, "03010028", 8 ) ) {
        tLog( ERROR, "交易[%s]重复充值,默认成功.", sRrn );
        if ( !memcmp( sTransType, "POS", 3 ) ) {
            if ( UpdPosTrans( "M", sRrn, sTransDate ) < 0 ) {
                ErrHanding( pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog );
                return -1;
            }
        } else {
            if ( UpdQrTrans( "M", sRrn, sTransDate ) < 0 ) {
                ErrHanding( pstTransJson, "96", "无rrn[%s]的二维码交易流水[%s].", sRrn, sRespLog );
                return -1;
            }
        }
    } else {
        ErrHanding( pstTransJson, "96", "交易[%s]充值失败[%s].", sRrn, sRespLog );
    }
    return 0;
}

/* 日结 */
int CashTransStatus( cJSON *pstJson, int *piFlag ) {
    char sRrn[12 + 1] = { 0 }, sRespCode[32 + 1] = { 0 }, sTransDate[8 + 1] = { 0 }, sTransType[16 + 1] = { 0 };
    char sRespLog[512 + 1] = { 0 };
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY( pstJson, "data" );
    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_STR_KEY( pstTransJson, "trans_date", sTransDate );
    GET_STR_KEY( pstTransJson, "trans_type", sTransType );
    GET_STR_KEY( pstTransJson, "resp_desc", sRespLog );
    if ( !memcmp( sRespCode, "00", 2 ) ) {
        if ( UpdPosTrans( "Y", sRrn, sTransDate ) < 0 ) {
            ErrHanding( pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog );
            return -1;
        }
    } else if ( !memcmp( sRespCode, "03010027", 8 ) ) {
        tLog( ERROR, "交易[%s]重复充值,默认成功.", sRrn );
        if ( UpdPosTrans( "Y", sRrn, sTransDate ) < 0 ) {
            ErrHanding( pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog );
            return -1;
        }
    } else {
        ErrHanding( pstTransJson, "96", "交易[%s]日结失败[%s].", sRrn, sRespLog );
    }
    return 0;
}

/* 转账 */
int TransferStatus( cJSON *pstJson, int *piFlag ) {
    char sRrn[12 + 1] = { 0 }, sRespCode[32 + 1] = { 0 }, sTransDate[8 + 1] = { 0 }, sTransType[16 + 1] = { 0 };
    char sRespLog[512 + 1] = { 0 };
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY( pstJson, "data" );
    GET_STR_KEY( pstTransJson, "resp_code", sRespCode );
    GET_STR_KEY( pstTransJson, "rrn", sRrn );
    GET_STR_KEY( pstTransJson, "trans_date", sTransDate );
    GET_STR_KEY( pstTransJson, "trans_type", sTransType );
    GET_STR_KEY( pstTransJson, "resp_desc", sRespLog );
    if ( !memcmp( sRespCode, "00", 2 ) ) {
        if ( UpdPosTrans( "F", sRrn, sTransDate ) < 0 ) {
            ErrHanding( pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog );
            return -1;
        }
    } else if ( !memcmp( sRespCode, "03010027", 8 ) ) {
        tLog( ERROR, "交易[%s]重复充值,默认成功.", sRrn );
        if ( UpdPosTrans( "F", sRrn, sTransDate ) < 0 ) {
            ErrHanding( pstTransJson, "96", "无rrn[%s]的pos交易流水[%s].", sRrn, sRespLog );
            return -1;
        }
    } else {
        ErrHanding( pstTransJson, "96", "交易[%s]转账失败[%s].", sRrn, sRespLog );
    }
    return 0;
}
