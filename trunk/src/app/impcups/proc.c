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
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_redis.h"
#include "t_app_conf.h"
#include "t_daemon.h"
#include "t_log.h"
#include "param.h"
#include "dbop.h"

typedef struct {
    char sMsgType[5]; /* 消息类型码       */
    char sProcCode[7]; /* 交易处理码       */
    char sProcCodeEx[3]; /* 扩展的交易处理码 */
    char sNmiCode[4]; /* 网络管理信息码   */
    char sTransCode[7]; /* 交易类型码       */
    char sOldTransCode[7]; /* 原交易类型码     */
    char sOOldTransCode[7]; /* 原原交易类型码   */
} TranTbl;


int g_aiAcom[] = {0, 11, 11, 6, 10, 19, 12, 12, 12, 4, 6, 4, 8, 15, 12, 2, 6, 11, 6, 2, 3, 12, 12, 12, 1, 3, 1, 1, 10, 11, 1, 2, 2, 12, 14};
int g_aiAFee[] = {0, 11, 1, 2, 2, 1, 1, 4, 11, 11, 11, 11, 11, 11, 4, 6, 2, 6, 10, 19, 28, 28, 42, 8, 15, 12, 12, 12, 12, 12, 121, 99};

int GetField(int iNo, char *pcData, char *pcSrc, int *piIsoTable) {
    int i, iOffset = 0;
    /* 从0开始 */
    for (i = 0; i < iNo; ++i) {
        iOffset += piIsoTable[i];
    }

    if (i > 0) {
        iOffset += (i - 1);
    }
    tStrCpy(pcData, pcSrc + iOffset, piIsoTable[i]);
    return 0;
}

TranTbl g_staTranTbl[] = {
    /* 消息ID  处理码    消息类型码  NMI   交易代码  原交易代码  */
    { "0200", "300000", "00", "", "021000", "", ""}, /* POS查余额            */
    { "0200", "000000", "00", "", "020000", "", ""}, /* POS消费              */
    { "0200", "000000", "00", "", "T20000", "", ""}, /* T0消费              */
    { "0220", "000000", "00", "", "022000", "", ""}, /* POS脱机消费              */
    { "0420", "000000", "00", "", "020300", "020000", ""}, /* POS消费冲正          */
    { "0420", "000000", "00", "", "020300", "T20000", ""}, /* T0消费冲正		   */
    { "0200", "200000", "00", "", "020200", "020000", ""}, /* POS消费撤销          */
    { "0420", "200000", "00", "", "020300", "020200", "020000"}, /* POS消费撤销冲正      */
    { "0220", "200000", "00", "", "020100", "020000", ""}, /* POS退货   */
    { "0220", "200000", "69", "", "020100", "022000", ""}, /* POS脱机消费退货*/
    { "0100", "030000", "06", "", "024000", "", ""}, /* POS预授权            */
    { "0420", "030000", "06", "", "020300", "024000", ""}, /* POS预授权冲正        */
    { "0100", "200000", "06", "", "020200", "024000", ""}, /* POS预授权撤销        */
    { "0420", "200000", "06", "", "020300", "020200", "024000"}, /* POS预授权撤销冲正    */
    { "0200", "000000", "06", "", "024100", "024000", ""}, /* POS预授权完成        */
    { "0420", "000000", "06", "", "020300", "024100", "024000"}, /* POS预授权完成冲正    */
    { "0200", "200000", "06", "", "020200", "024100", "024000"}, /* POS预授权完成撤销    */
    { "0420", "200000", "06", "", "020300", "020200", "024100"}, /* POS预授权完成撤销冲正*/
    //{ "0200", "600000", "91", "", "023000", "100000", ""}, /* 电子现金圈存              */
    //{ "0420", "600000", "91", "", "020300", "023000", ""}, /* 电子现金圈存冲正      */
    //{ "0200", "630000", "91", "", "023000", "200000", ""}, /* 电子现金充值              */
    //{ "0420", "630000", "91", "", "020300", "023000", ""}, /* 电子现金现金充值冲正      */
    //{ "0200", "170000", "91", "", "020200", "023000", ""}, /* 电子现金现金充值撤销      */
    //{ "0420", "170000", "91", "", "020300", "020200", ""}, /* 电子现金现金充值撤销冲正  */
    //{ "0200", "620000", "91", "", "023000", "300000", ""}, /* 电子现金非指定账户转账圈存*/
    //{ "0420", "620000", "91", "", "020300", "023000", ""}, /* 电子非指定账户转账圈存冲正      */

    { "0620", "300000", "00", "951", "020400", "021000", ""}, /* POS查余额脚本通知*/
    { "0620", "000000", "00", "951", "020400", "020000", ""}, /* POS消费脚本通知*/
    { "0620", "030000", "06", "951", "020400", "024000", ""}, /* POS预授权脚本通知*/
    { "0620", "600000", "91", "951", "020400", "023000", "100000"}, /* 电子现金圈存脚本通知*/
    { "0620", "630000", "91", "951", "020400", "023000", "200000"}, /* 电子现金圈存脚本通知*/
    { "0620", "650000", "91", "951", "020400", "023000", "300000"}, /* 电子现金圈存脚本通知*/
    { "0620", "170000", "91", "951", "020400", "020200", ""}, /* 电子现金充值撤销脚本通知*/
    /*
    { "0820", "", "00", "372", "020500", "", ""},  IC卡状态上送         *
    { "0800", "", "00", "370", "029600", "", ""}, * IC卡公钥下载         *
    { "0800", "", "00", "380", "029700", "", ""}, * IC卡参数下载         */

    { "0800", "", "00", "370", "029600", "000001", ""}, /* IC卡公钥下载          */
    { "0800", "", "00", "380", "029600", "000002", ""}, /* IC卡参数下载          */
    { "0800", "", "00", "390", "029600", "000003", ""}, /* IC卡黑名单下载          */

    { "0800", "", "00", "371", "029600", "000011", ""}, /* IC卡公钥下载结束          */
    { "0800", "", "00", "381", "029600", "000012", ""}, /* IC卡参数下载结束          */
    { "0800", "", "00", "391", "029600", "000013", ""}, /* IC卡黑名单下载结束          */

    { "0820", "", "00", "372", "020500", "000007", ""}, /* IC卡公钥下载状态上送          */
    { "0820", "", "00", "382", "020500", "000008", ""}, /* IC卡参数下载状态上送          */

    { "0820", "", "", "001", "03H000", "", ""}, /* 平台签到          */
    { "0820", "", "", "001", "039009", "", ""}, /* 平台签到(CUPS)          */
    { "0820", "", "", "002", "039004", "", ""}, /* 平台签退          */
    { "0820", "", "", "002", "039010", "", ""}, /* 平台签退(CUPS)          */
    { "0820", "", "", "301", "039005", "", ""}, /* 平台线路测试          */
    { "0820", "", "", "301", "039006", "", ""}, /* 平台线路测试cups          */
    { "0820", "", "", "201", "039007", "", ""}, /* 银联日切开始cups          */
    { "0820", "", "", "202", "039008", "", ""}, /* 银联日切结束cups          */
    { "0820", "", "", "101", "00H000", "", ""}, /* 密钥重置交易          */
    { "0800", "", "", "101", "009000", "", ""}, /* 密钥重置交易（CUPS）          */
    { "", "", "", "", "", "", ""},
};

int GetTranType(char *pcTransCode, char *pcMsgType, char *pcProcCode) {
    int i;

    /* 查取当前交易类型(根据从发卡机构报文解出的消息ID、处理码、消息类型码、网络管理码) */
    for (i = 0; g_staTranTbl[i].sMsgType[0] != '\0'; i++) {
        if ((!memcmp(g_staTranTbl[i].sMsgType, pcMsgType, strlen(g_staTranTbl[i].sMsgType)))
                && (!memcmp(g_staTranTbl[i].sProcCode, pcProcCode, (g_staTranTbl[i].sProcCode[0] ? 2 : 0))) /* 非空比较前2位 */
                ) {
            strcpy(pcTransCode, g_staTranTbl[i].sTransCode);
            return ( 0);
        }
    }

    return ( -1);
}

int ImpAcom(char *pcChannelId, char *pcDate) {
    char sCurDate[9] = {0}, sPath[256] = {0}, sPath1[256] = {0}, sTmp[1024] = {0}, sData[128];
    FILE * pstFp = NULL, *pLoadfile = NULL;
    ChannelSettleTransDetail stChannnelTransDetail;
    int i = 0;

    //tGetDate( sCurDate, "", -1 );
    //tAddDay(sCurDate, -1 );
    strcpy(sCurDate, pcDate);
    sprintf(sPath, "%s/%s/%s/IND%s01ACOM", getenv("SETTLE_DIR"), pcChannelId, sCurDate, sCurDate + 2);
    sprintf(sPath1, "%s/tmp.dat", getenv("HOME") );
    if ((pstFp = fopen(sPath, "r")) == NULL) {
        tLog(ERROR, "打开文件[%s]失败,errno[%d:%s]", sPath, errno, strerror(errno));
        return -1;
    }
    //写入临时文件用于导入库表
    if((pLoadfile = fopen(sPath1, "w")) == NULL){
        tLog(ERROR, "打开文件[%s]失败,errno[%d:%s]", sPath1, errno, strerror(errno));
        return -1;
    }
    while (!feof(pstFp)) {
        MEMSET(sTmp);
        fgets(sTmp, 1024, pstFp);
        tLog(DEBUG, "[%d][%s]", ++i, sTmp);
        if (sTmp[0] == '\0') {
            tLog(ERROR, "文件[%s]无记录,忽略.", sPath);
            break;
        }

        //     GetField(2, stChannnelTransDetail.sChannelId, sTmp, g_aiAcom );
        //   tTrim(stChannnelTransDetail.sChannelId);

        strcpy(stChannnelTransDetail.sChannelId, pcChannelId);

        strcpy(stChannnelTransDetail.sSettleDate, sCurDate);

        GetField(3, stChannnelTransDetail.sSysTrace, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sSysTrace);

        GetField(4, stChannnelTransDetail.sTransmitTime, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sTransmitTime);

        tStrCpy(stChannnelTransDetail.sTransTime, stChannnelTransDetail.sTransmitTime + 4, 6);
        tStrCpy(stChannnelTransDetail.sTransDate, sCurDate, 4);
        tStrCpy(stChannnelTransDetail.sTransDate + 4, stChannnelTransDetail.sTransmitTime, 4);

        GetField(5, stChannnelTransDetail.sCard1No, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sCard1No);

        GetField(6, sData, sTmp, g_aiAcom);
        tTrim(sData);
        stChannnelTransDetail.dAmount = atol(sData) / 100.00;

        GetField(9, stChannnelTransDetail.sMsgId, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sMsgId);

        GetField(10, stChannnelTransDetail.sProCode, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sProCode);

        GetField(11, stChannnelTransDetail.sChannelMcc, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sChannelMcc);

        GetField(13, stChannnelTransDetail.sChannelMerchId, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sChannelMerchId);

        GetField(12, stChannnelTransDetail.sChannelTermId, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sChannelTermId);

        GetField(14, stChannnelTransDetail.sChannelRrn, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sChannelRrn);

        GetField(15, stChannnelTransDetail.sCondCode, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sCondCode);

        GetField(16, stChannnelTransDetail.sAuthCode, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sAuthCode);

        GetField(17, stChannnelTransDetail.sRespInstId, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sRespInstId);

        GetField(18, stChannnelTransDetail.sOrgSysTrace, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sOrgSysTrace);

        GetField(19, stChannnelTransDetail.sRespCode, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sRespCode);

        GetField(20, stChannnelTransDetail.sInputMode, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sInputMode);

        GetField(22, sData, sTmp, g_aiAcom);
        tTrim(sData);
        stChannnelTransDetail.dIssFee = atol(sData) / 100.00;

        GetField(23, sData, sTmp, g_aiAcom);
        tTrim(sData);
        stChannnelTransDetail.dChannelFee = atol(sData + 1) / 100.00;
        if (sData[0] == 'C')
            stChannnelTransDetail.dChannelFee = -stChannnelTransDetail.dChannelFee;

        GetField(28, stChannnelTransDetail.sOrgTransmitTime, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sOrgTransmitTime);

        GetField(30, &stChannnelTransDetail.cTranArea, sTmp, g_aiAcom);

        GetField(29, stChannnelTransDetail.sIssInstId, sTmp, g_aiAcom);
        tTrim(stChannnelTransDetail.sIssInstId);

        stChannnelTransDetail.cAdjustFlag = 'Y';
        stChannnelTransDetail.cChkFlag = 'N';
        stChannnelTransDetail.dFee = stChannnelTransDetail.dChannelFee + stChannnelTransDetail.dIssFee;

        GetTranType(stChannnelTransDetail.sTransCode, stChannnelTransDetail.sMsgId, stChannnelTransDetail.sProCode);
        strcpy(stChannnelTransDetail.sPayType, "0004");
        stChannnelTransDetail.cCardType = 'X';
        //AddChannleTrans(&stChannnelTransDetail);
        getLoadFile(&stChannnelTransDetail,pLoadfile);
    }

    fclose(pstFp);
    fclose(pLoadfile);
    return 0;
}

int ImpAlfee(char *pcChannelId, char *pcDate) {
    char sCurDate[9] = {0}, sPath[256] = {0}, sTmp[1024] = {0}, sData[128];
    FILE * pstFp = NULL;
    ChannelSettleTransDetail stChannnelTransDetail;
    int i = 0;

    //tGetDate( sCurDate, "", -1 );
    //tAddDay(sCurDate, -1 );
    strcpy(sCurDate, pcDate);
    sprintf(sPath, "%s/%s/%s/IND%s99ALFEE", getenv("SETTLE_DIR"), pcChannelId, sCurDate, sCurDate + 2);

    if ((pstFp = fopen(sPath, "r")) == NULL) {
        tLog(ERROR, "打开文件[%s]失败,errno[%d:%s]", sPath, errno, strerror(errno));
        return -1;
    }
    while (!feof(pstFp)) {
        MEMSET(sTmp);
        fgets(sTmp, 1024, pstFp);
        tLog(DEBUG, "[%d][%s]", ++i, sTmp);
        if (sTmp[0] == '\0') {
            tLog(ERROR, "文件[%s]无记录,忽略.", sPath);
            break;
        }
        GetField(2, stChannnelTransDetail.sChannelId, sTmp, g_aiAFee);
        tTrim(stChannnelTransDetail.sChannelId);

        stChannnelTransDetail.cCardType = '0';
        GetField(3, sData, sTmp, g_aiAFee);
        tTrim(sData);

        if (!strcmp(sData, "01")) {
            stChannnelTransDetail.cCardType = '0';
        } else if (!strcmp(sData, "02") || !strcmp(sData, "03"))
            stChannnelTransDetail.cCardType = '1';

        //tErrLog( DEBUG, "[%d]", stChannnelTransDetail.cCardType );

        GetField(18, stChannnelTransDetail.sOrgSysTrace, sTmp, g_aiAFee);
        tTrim(stChannnelTransDetail.sOrgSysTrace);


        GetField(28, sData, sTmp, g_aiAFee);
        tTrim(sData);
        stChannnelTransDetail.dChannelFee2 = atol(sData + 1) / 100.00;

        if (sData[0] == 'C')
            stChannnelTransDetail.dChannelFee2 = -stChannnelTransDetail.dChannelFee2;

        GetField(25, stChannnelTransDetail.sChannelRrn, sTmp, g_aiAFee);
        tTrim(stChannnelTransDetail.sChannelRrn);
        tLog(DEBUG, "rrn[%s]type[%d]fee[%.2f]"
                , stChannnelTransDetail.sChannelRrn, stChannnelTransDetail.cCardType, stChannnelTransDetail.dChannelFee2);
        UpdChannelFee(pcChannelId, stChannnelTransDetail.sChannelRrn, stChannnelTransDetail.cCardType, stChannnelTransDetail.dChannelFee2, sCurDate);
    }
    tCommit();
    fclose(pstFp);

    return 0;
}

void SndProc(char *pcArgv[]) {

    if(!strcmp(GetSwitch(),"Acom"))
    /* 导入清算文件 */
    ImpAcom(GetCupsId(), GetStartDate());

    /* 导入品牌费用 */
    else if(!strcmp(GetSwitch(),"Alfee"))
    ImpAlfee(GetCupsId(), GetStartDate());

    return;
}