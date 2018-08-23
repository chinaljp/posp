/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: feng.gaoo
 *
 * Created on 2016年11月30日, 下午9:00
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "t_db.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_redis.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "t_log.h"
#include "param.h"
#include "dbop.h"

int AddChannleTrans(ChannelSettleTransDetail *pstChannleTrans) {
    char sSql[1024] = {0};
    int iRet = -1;
    ChannelSettleTransDetail stChannleTrans;
    OCI_Resultset *pstRes = NULL;

    sprintf(sSql, "select count(1) from B_CUPS_TRANS_DETAIL "
            "where channel_rrn='%s' and msg_id='%s' and pro_code='%s' and trans_date='%s'"
            , pstChannleTrans->sChannelRrn, pstChannleTrans->sMsgId, pstChannleTrans->sProCode, pstChannleTrans->sTransDate);
    iRet = tQueueCnt(sSql);
    if (iRet > 0) {
        //tLog(DEBUG, "sql['%s'][%d]", sSql, iRet);
        tLog(WARN, "交易日期[%s]rrn[%s]已经导入,忽略.", pstChannleTrans->sTransDate, pstChannleTrans->sChannelRrn);
        return -1;
    }

    memset(&stChannleTrans, 0x00, sizeof (ChannelSettleTransDetail));
    stChannleTrans = *pstChannleTrans;

    MEMSET(sSql);
    sprintf(sSql, "INSERT INTO B_CUPS_TRANS_DETAIL( TRANS_CODE,CHANNEL_ID,SETTLE_DATE"
            ",SYS_TRACE,TRANS_DATE,TRANS_TIME,TRANSMIT_TIME"
            ",CARD1_NO,CARD_TYPE,AMOUNT,FEE"
            ",MSG_ID,PRO_CODE,CHANNEL_MCC"
            ",CHANNEL_MERCH_ID,CHANNEL_TERM_ID,CHANNEL_RRN,AUTH_CODE"
            ",RESP_INST_ID,ORG_SYS_TRACE,RESP_CODE,INPUT_MODE"
            ",ISS_FEE,CHANNEL_FEE,CHANNEL_FEE2"
            ",ORG_TRANSMIT_TIME,COND_CODE,TRAN_AREA,ISS_INST_ID"
            ",PAY_TYPE,CHK_FLAG,ADJUST_FLAG,REMARK) "
            " VALUES( '%s','%s','%s'"
            " ,'%s','%s','%s','%s'"
            " ,'%s','%c',%f,%f"
            " ,'%s','%s','%s'"
            " ,'%s','%s','%s','%s'"
            ",'%s','%s','%s','%s'"
            ",%f,%f,%f"
            ",'%s','%s','%c','%s'"
            ",'%s','%c','%c','%s')",
            stChannleTrans.sTransCode, stChannleTrans.sChannelId, stChannleTrans.sSettleDate
            , stChannleTrans.sSysTrace, stChannleTrans.sTransDate, stChannleTrans.sTransTime, stChannleTrans.sTransmitTime
            , stChannleTrans.sCard1No, stChannleTrans.cCardType, stChannleTrans.dAmount, stChannleTrans.dFee
            , stChannleTrans.sMsgId, stChannleTrans.sProCode, stChannleTrans.sChannelMcc
            , stChannleTrans.sChannelMerchId, stChannleTrans.sChannelTermId, stChannleTrans.sChannelRrn, stChannleTrans.sAuthCode
            , stChannleTrans.sRespInstId, stChannleTrans.sOrgSysTrace, stChannleTrans.sRespCode, stChannleTrans.sInputMode
            , stChannleTrans.dFee, stChannleTrans.dChannelFee, stChannleTrans.dChannelFee2
            , stChannleTrans.sOrgTransmitTime, stChannleTrans.sCondCode, stChannleTrans.cTranArea, stChannleTrans.sIssInstId
            , stChannleTrans.sPayType, stChannleTrans.cChkFlag, stChannleTrans.cAdjustFlag, stChannleTrans.sRemark);
    if (tExecute(&pstRes, sSql) < 0) {
        tLog(ERROR, "插入数据rrn[%s]失败.", stChannleTrans.sChannelRrn);
        tLog(ERROR, "sql[%s]", sSql);
        return -1;
    }

    if (0 == tGetAffectedRows()) {
        tReleaseRes(pstRes);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);

    return 0;
}

int getLoadFile(ChannelSettleTransDetail *pstChannnelTransDetail, FILE *pLoadfile) {
    char sInsertData[2048] = {0};
    ChannelSettleTransDetail stChannleTrans;

    memset(&stChannleTrans, 0x00, sizeof (ChannelSettleTransDetail));
    stChannleTrans = *pstChannnelTransDetail;
    sprintf(sInsertData, "%s,%s,%s"
            ",%s,%s,%s,%s"
            ",%s,%c,%f,%f"
            ",%s,%s,%s"
            ",%s,%s,%s,%s"
            ",%s,%s,%s,%s"
            ",%f,%f,%f"
            ",%s,%s,%c,%s"
            ",%s,%c,%c,%s\n", stChannleTrans.sTransCode, stChannleTrans.sChannelId, stChannleTrans.sSettleDate
            , stChannleTrans.sSysTrace, stChannleTrans.sTransDate, stChannleTrans.sTransTime, stChannleTrans.sTransmitTime
            , stChannleTrans.sCard1No, stChannleTrans.cCardType, stChannleTrans.dAmount, stChannleTrans.dFee
            , stChannleTrans.sMsgId, stChannleTrans.sProCode, stChannleTrans.sChannelMcc
            , stChannleTrans.sChannelMerchId, stChannleTrans.sChannelTermId, stChannleTrans.sChannelRrn, stChannleTrans.sAuthCode
            , stChannleTrans.sRespInstId, stChannleTrans.sOrgSysTrace, stChannleTrans.sRespCode, stChannleTrans.sInputMode
            , stChannleTrans.dIssFee, stChannleTrans.dChannelFee, stChannleTrans.dChannelFee2
            , stChannleTrans.sOrgTransmitTime, stChannleTrans.sCondCode, stChannleTrans.cTranArea, stChannleTrans.sIssInstId
            , stChannleTrans.sPayType, stChannleTrans.cChkFlag, stChannleTrans.cAdjustFlag, stChannleTrans.sRemark);

    tLog(DEBUG, "%s", sInsertData);
    fwrite(sInsertData, strlen(sInsertData), 1, pLoadfile);
    return 0;
}

#if 0

int ReverProc() {

    char sSqlStr[256] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE CHANNEL_SETTLE_TRANS_DETAIL set resp_code='96' "
            "where SYS_TRACE in (select ORG_SYS_TRACE from CHANNEL_SETTLE_TRANS_DETAIL where msg_id='0420')");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "96", "更新id['%s']通知状态失败.", pcId);
        return -1;
    }
    if (0 == tGetAffectedRows) {
        tReleaseRes(pstRes);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);

    return 0;
}
#endif

int UpdChannelFee(char *pcChannelId, char *pcChannleRrn, char cCardType, double dLfee, char *pcSettleDate) {

    char sSqlStr[256] = {0};
    OCI_Resultset *pstRes = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_CUPS_TRANS_DETAIL set FEE=%f+CHANNEL_FEE+ISS_FEE,CHANNEL_FEE2=%f,CARD_TYPE='%c' "
            " where CHANNEL_RRN='%s' and CHANNEL_ID='%s' and SETTLE_DATE='%s' ", dLfee, dLfee, cCardType, pcChannleRrn, pcChannelId, pcSettleDate);
    tLog(ERROR, "sql[%s].", sSqlStr);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "更新rrn[%s]手续费失败.", pcChannleRrn);
        return -1;
    }
    if (0 == tGetAffectedRows()) {
        tReleaseRes(pstRes);
        return -1;
    }
    tCommit();
    tReleaseRes(pstRes);
    return 0;
}
#if 0

int UpdChannelCardType(char *pcChannelId, char *pcChannleRrn, char cCardType) {
    EXEC SQL BEGIN DECLARE SECTION;
    char sRrn[13], sChannelId[9] = {0};
    double dFee = 0;
    char cType;
    EXEC SQL END DECLARE SECTION;

    strcpy(sRrn, pcChannleRrn);
    strcpy(sChannelId, pcChannelId);
    cType = cCardType;

    EXEC SQL UPDATE CHANNEL_SETTLE_TRANS_DETAIL set CARD_TYPE=:cType
    where CHANNEL_RRN=:sRrn and CHANNEL_ID=:sChannelId;

    if (sqlca.sqlcode != 0) {
        tErrLog(ERR, "UpdChannelFee error[%d]!", sqlca.sqlcode);
        SqlError();

        return ( -1);
    }

    return 0;
}

int DBFindCardBinEx(CardBin *pstCardBin, char *pcCardNo) {
    EXEC SQL BEGIN DECLARE SECTION;
    char sTrack[105] = {0};
    CardBin stCardBin;
    char cTrackNo;
    char sCardlen[2 + 1] = {0};
    EXEC SQL END DECLARE SECTION;

    strcpy(sTrack, pcCardNo);
    sprintf(sCardlen, "%d", strlen(sTrack));

    EXEC SQL DECLARE cur_find CURSOR FOR
    SELECT
            CARD_ID,
            CARD_TYPE,
            CARD_NAME,
            ISS_ID,
            CARD_BIN,
            CARD_BIN_LEN,
            CARD_NO_LEN,
            TRACK,
            CARD_POS,
            EXP_POS,
            ACQUIRE_FLAG
    FROM CARD_BIN
    WHERE CARD_NO_LEN = :sCardlen
    AND TRACK in ('2','3')
    AND INSTR( :sTrack, CARD_BIN, CARD_POS, 1 )=CARD_POS
    ORDER BY CARD_BIN_LEN DESC, CARD_NO_LEN DESC;

    EXEC SQL OPEN cur_find;
    if (sqlca.sqlcode != 0) {
        SqlError();
        return -1;
    }
    memset(&stCardBin, 0x00, sizeof (CardBin));
    EXEC SQL FETCH cur_find INTO :stCardBin;
    if (sqlca.sqlcode < 0) {
        EXEC SQL CLOSE cur_find;
        SqlError();
        return -1;
    }
    if (sqlca.sqlcode == 100 || sqlca.sqlcode == 1403) {
        tErrLog(INFO, "未检索到卡BIN信息.");
        EXEC SQL CLOSE cur_find;
        return -1;
    }

    EXEC SQL CLOSE cur_find;

    //tErrLog( INFO, "检索到卡BIN信息.");

    tTrim(stCardBin.sCardId);
    tTrim(stCardBin.sIssId);
    tTrim(stCardBin.sCardName);

    memcpy(pstCardBin, &stCardBin, sizeof (CardBin));

    return 0;
}

int JudgeCard(char *pcChannelId, char *pcDate) {
    int i = 0;
    char sCurDate[9] = {0};
    EXEC SQL BEGIN DECLARE SECTION;
    int iRet = 0;
    ChannelSettleTransDetail stChannnelTransDetail;
    CardBin stCardBin;
    char sChannelId[9] = {0}, sType[3] = {0};
    EXEC SQL END DECLARE SECTION;

    strcpy(sCurDate, pcDate);
    strcpy(sChannelId, pcChannelId);
    tErrLog(DEBUG, "['%s']['%s']", sChannelId, sCurDate);
    EXEC SQL DECLARE JudgeCursor CURSOR FOR SELECT
            TRANS_DATE,
            CHANNEL_RRN,
            CARD1_NO
    FROM  CHANNEL_SETTLE_TRANS_DETAIL
    WHERE  CARD_TYPE='X' AND CHANNEL_ID=:sChannelId
    ORDER BY CHANNEL_MERCH_ID DESC;

    EXEC SQL OPEN JudgeCursor;
    if (sqlca.sqlcode < 0) {
        tErrLog(ERR, "open AcctJournalCursor failed,sqlcode=[%d].", sqlca.sqlcode);
        SqlError();
        EXEC SQL CLOSE JudgeCursor;
        return -1;
    }

    while (1) {

        MEMSET_ST(stChannnelTransDetail);
        EXEC SQL FETCH JudgeCursor into :stChannnelTransDetail.sTransDate, :stChannnelTransDetail.sChannelRrn,
                :stChannnelTransDetail.sCard1No;
        if (sqlca.sqlcode < 0) {
            tErrLog(ERR, "fetch T0SettleInfoCursor failed,sqlcode=[%d].", sqlca.sqlcode);
            SqlError();
            EXEC SQL CLOSE JudgeCursor;
            return -1;
        }

        if (sqlca.sqlcode == 1403 || sqlca.sqlcode == 100) {
            tErrLog(DEBUG, "解析卡号处理完成.");
            break;
        }
        tTrim(stChannnelTransDetail.sCard1No);
        //tErrLog( DEBUG, "卡号['%s']", stChannnelTransDetail.sCard1No );
        if (DBFindCardBinEx(&stCardBin, stChannnelTransDetail.sCard1No) < 0) {
            tErrLog(ERR, "交易['%s']卡号['%s']解析失败.", stChannnelTransDetail.sChannelRrn, stChannnelTransDetail.sCard1No);
            //UpdChannelCardType( pcChannelId, stChannnelTransDetail.sChannelRrn, 'X' );
        } else
            UpdChannelCardType(pcChannelId, stChannnelTransDetail.sChannelRrn, stCardBin.cCardType);
        tCommitWork();
    }

    EXEC SQL CLOSE JudgeCursor;
    tErrLog(DEBUG, "解析卡号完成.");
    return 0;
}
#endif



