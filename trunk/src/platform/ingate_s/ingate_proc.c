#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include "param.h"
#include "t_dlink.h"
#include "t_redis.h"
#include "t_log.h"
#include "t_netlib.h"
#include  "ingate.h"
#include "trans_item.h"
#include "multi_item.h"
#include "t_cjson.h"


extern int g_iQuitLoop;
extern int g_iaPipeFd[2];


DLINK g_stMultiFree, g_stMultiLink;
DLINK g_stTransFree, g_stTransLink;

int g_debug = 0;

void getUniqueKey(char *uniKey) {
    struct timeval time;
    gettimeofday(&time, NULL);
    sprintf(uniKey, "%ld", (long) (time.tv_sec) * 1000000 + time.tv_usec);
}

char *ItemToJson(TransItem *pstMultiItem) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char pcJson[REDISDATA_SIZE] = {0};
    char *pcMsg = NULL;

    unsigned char sData[REDISDATA_SIZE] = {0};

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return NULL;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "����Key-Jsonʧ��.");
        return NULL;
    }
    SET_STR_KEY(pstJson, "svrid", GetSvrId());
    SET_STR_KEY(pstJson, "key", pstMultiItem->sUniqueKey);

    //tBcd2Asc(sData, pstMultiItem->caData + PKT_BCDLEN, (pstMultiItem->iLen - PKT_BCDLEN) * 2, LEFT_ALIGN);
    tStrCpy(sData, pstMultiItem->caData + PKT_BCDLEN, pstMultiItem->iLen - PKT_BCDLEN);
    //tLog(INFO, "tBcd2Asc data[%s]", sData);
    SET_STR_KEY(pstDataJson, "msg", (const char *) sData);
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    pcMsg = cJSON_PrintUnformatted(pstJson);
    //tLog(INFO,"Fuck! pcMsg[%s]", pcMsg);
    //tStrCpy(pcJson, pcMsg, strlen(pcMsg));
    //cJSON_Delete(pstDataJson);
    cJSON_Delete(pstJson);
    return pcMsg;
}

int SendProc() {
    int iLen, iRet;
    char *sSvrId = NULL;
    char sRedisTran[REDISDATA_SIZE] = {0};
    char sLen[4 + 1] = {0};

    sSvrId = GetSvrId();
    /*�رչܵ���*/
    close(g_iaPipeFd[0]);
    while (g_iQuitLoop) {
        memset(sRedisTran, 0, sizeof (sRedisTran));
        iRet = tRecvMsg(sSvrId, sRedisTran + 4, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                break;
            case MSG_TIMEOUT:
                tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", sSvrId, TIMEOUT);
                break;
            case MSG_SUCC:
                tLog(INFO, "����Ӧ����Ϣ��׼��д��ܵ�.");
                //д�ṹ�峤��
                sprintf(sLen, "%04d", iLen);
                memcpy(sRedisTran, sLen, 4);
                tLog(DEBUG, "�ܵ�д������:%s", sRedisTran);
                write(g_iaPipeFd[1], sRedisTran, strlen(sRedisTran));
                break;
        }
    }
    close(g_iaPipeFd[1]);
    return 0;
}

int RecvProc() {
    int iRet, iFd, iMaxFd;
    fd_set stRdSet;
    time_t tLastChkTime, tTime;
    struct timeval stTime;
    DNode *pstDNode;
    MultiItem *pstMultiItem;

    close(g_iaPipeFd[1]);
    CreateMultiLink(&g_stMultiFree, GetMaxMulti());
    CreateMultiLink(&g_stMultiLink, 0);
    CreateTransLink(&g_stTransFree, GetMaxTrans());
    CreateTransLink(&g_stTransLink, 0);

    if (AddPipeFd(&g_stMultiLink, &g_stMultiFree, g_iaPipeFd[0]) < 0)
        return -1;

    if (AddListenFd(&g_stMultiLink, &g_stMultiFree) < 0)
        return -1;

    time(&tLastChkTime);
    while (g_iQuitLoop) {
        iMaxFd = 0;
        stTime.tv_sec = 5;
        stTime.tv_usec = 0;
        FD_ZERO(&stRdSet);
        for (pstDNode = g_stMultiLink.pHead; pstDNode != NULL;) {
            pstMultiItem = (MultiItem *) pstDNode->pvData;
            FD_SET(pstMultiItem->iFd, &stRdSet);

            if (pstMultiItem->iFd > iMaxFd)
                iMaxFd = pstMultiItem->iFd;

            pstDNode = pstDNode->pstNext;
        }

        iRet = select(iMaxFd + 1, &stRdSet, (fd_set *) NULL, (fd_set *) NULL, &stTime);
        if (iRet < 0) {
            if (!g_iQuitLoop)
                tLog(ERROR, "in_module-recv receive exit signal.");
            else {
                tLog(ERROR, "in_module-recv select I/O error.");
                sleep(2);
            }
            continue;
        }
        if (iRet == 0) {
            CheckProc(GetTranTimeout());
            if (COMM_ASYNC == *GetComMode())
                BuildAllAsyncSndLink(&g_stMultiLink, &g_stMultiFree);
            continue;
        }

        time(&tTime);
        if (tTime - tLastChkTime > 5) {
            CheckProc(GetTranTimeout());
            if (COMM_ASYNC == *GetComMode())
                BuildAllAsyncSndLink(&g_stMultiLink, &g_stMultiFree);
            tLastChkTime = tTime;
        }

        for (pstDNode = g_stMultiLink.pHead; pstDNode != NULL; pstDNode = pstDNode->pstNext) {
            pstMultiItem = (MultiItem *) pstDNode->pvData;
            iFd = pstMultiItem->iFd;
            if (FD_ISSET(iFd, &stRdSet)) {
                switch (pstMultiItem->cType) {
                    case TCP_LISTEN:
                        AddTcpwrFd(&g_stMultiLink, &g_stMultiFree, iFd);
                        break;
                    case FIFO:
                        if (PipeProc(pstDNode, GetTranTimeout()) < 0)
                            g_iQuitLoop = 0;
                        break;
                    case TCP_WR:
                        TcpProc(&pstDNode, GetTranTimeout());
                        break;
                }
            }
        }
    }

    for (pstDNode = g_stMultiLink.pHead; pstDNode != NULL; pstDNode = pstDNode->pstNext) {
        pstMultiItem = (MultiItem *) pstDNode->pvData;
        tLog(DEBUG, "close Fd[%d]", pstMultiItem->iFd);
        close(pstMultiItem->iFd);
    }

    tDLinkFree(&g_stMultiFree);
    tDLinkFree(&g_stMultiLink);
    tDLinkFree(&g_stTransFree);
    tDLinkFree(&g_stTransLink);

    return 0;
}

int TcpProc(DNode **ppstDNode, int iTimeOut) {
    int iFd, iRet;
    DNode *pstSndNode = NULL;
    MultiItem *pstMultiItem = NULL, *pstSndMultiItem = NULL;
    TransItem stTransItem;
    char sSvcName[64] = {0};
    char sPeerIp[30] = {0}, sPeerPort[10] = {0}, sLocalPort[10] = {0};
    char sMsg[64] = {0};
    char *pcMsg = NULL, *pcQname = NULL;

    tStrCpy(sSvcName, GetSvrName(), strlen(GetSvrName()));
    pstMultiItem = (MultiItem *) (*ppstDNode)->pvData;
    iFd = pstMultiItem->iFd;
    tLog(DEBUG, "TcpProc Fd[%d]", iFd);
    iRet = RcvData(pstMultiItem);
    switch (iRet) {
        case PACK_LEN_ERR:
            tLog(DEBUG, "�յ�������[%d][%s]���ĳ��ȴ���,�ر�����.", iFd, pstMultiItem->sClientInfo);
            ClearMultiLink(iFd, &g_stMultiLink, &g_stMultiFree);
            DeleteTransLink(iFd, &g_stTransLink, &g_stTransFree);
            return -1;
        case SOCK_CLOSE:
            tLog(INFO, "�յ�FIN��Ϣ,�ر�����[%d][%s].", iFd, pstMultiItem->sClientInfo);
            *ppstDNode = (*ppstDNode)->pstPrev;
            ClearMultiLink(iFd, &g_stMultiLink, &g_stMultiFree);
            DeleteTransLink(iFd, &g_stTransLink, &g_stTransFree);
            return -1;
    }

    if (RD_FIN != pstMultiItem->cRdStatus)
        return 0;

    tLog(DEBUG, "���ձ���[len=%d][%s].", pstMultiItem->iLen, pstMultiItem->sClientInfo);

    sprintf(sMsg, "[%s]����������", GetSvrName());
    PkDump(pstMultiItem->caData, pstMultiItem->iLen, sMsg);

    /*
    iRet = ModuleUnpack( pstMultiItem->caData, pstMultiItem->iLen, (void *)sNetTran );
    if ( iRet < 0 )
    {
        tErrLog( ERR, "�ӿڷ�����[%s]��������Ĵ���[%d].", g_stModuleInfo.sExecName, iRet );
        RecoverMultiItem( pstMultiItem );
        return -1;
    }
     */
    if (COMM_ASYNC == *GetComMode()) {
        sprintf(sLocalPort, "%d", get_server_port(iFd));
        if (ChkAsyncIP(get_server_ip(iFd), sLocalPort, sPeerIp, sPeerPort)) {
            RecoverMultiItem(pstMultiItem);
            return -1;
        }
        pstSndNode = BuildAsyncSndLink(sPeerIp, sPeerPort, &g_stMultiLink, &g_stMultiFree);
        if (NULL == pstSndNode) {
            RecoverMultiItem(pstMultiItem);
            return -1;
        }
        pstSndMultiItem = (MultiItem *) pstSndNode->pvData;
        CopyDataFromMulti(&stTransItem, pstSndMultiItem);
    } else
        CopyDataFromMulti(&stTransItem, pstMultiItem);

    getUniqueKey(stTransItem.sUniqueKey);
    if (NULL == AddTransLink(&stTransItem, &g_stTransLink, &g_stTransFree, GetTranTimeout())) {
        tLog(ERROR, "���Ӽ�ֵ[%s]����������ʧ��[%s].", stTransItem.sUniqueKey, pstMultiItem->sClientInfo);
        RecoverMultiItem(pstMultiItem);
        return -1;
    }
    tLog(DEBUG, "���Ӽ�ֵΪ[%s]��������������ɹ�", stTransItem.sUniqueKey);

    RecoverMultiItem(pstMultiItem);
    pcMsg = ItemToJson(&stTransItem);
    pcQname = GetQname();
    tLog(DEBUG, "sQname[%s],pcMsg[%s]", pcQname, pcMsg);
    iRet = tSendMsg(pcQname, pcMsg, strlen(pcMsg));
    if (pcMsg)
        free(pcMsg);
    switch (iRet) {
        case MSG_ERR:
            tLog(ERROR, "�����м��ʧ��[%d],����[%s],��ֵ[%s],�ͻ�����Ϣ[%s].", \
                iRet, sSvcName, stTransItem.sUniqueKey, pstMultiItem->sClientInfo);
            ClearTransLink(stTransItem.sUniqueKey, &g_stTransLink, &g_stTransFree);
            break;
        case MSG_SUCC:
            tLog(INFO, "������Ϣ�ɹ�.");
            break;
    }
    return 0;
}

int PipeProc(DNode *pstDNode, int iTimeOut) {
    int iFd, iLen;
    cJSON *pstJson = NULL, *pstDataJson = NULL, *pstKeyJson = NULL, *pstRepJson = NULL;
    char sUniqueKey[20] = {0}, sMsg[4096] = {0};
    MultiItem *pstPipeMultiItem = NULL, *pstSndMultiItem = NULL, stSndMultiItem = {0};
    TransItem *pstTransItem = NULL;
    DNode *pstTransNode = NULL;
    char sRedisTran[REDISDATA_SIZE] = {0};
    char sSvcName[64] = {0};
    char sLen[4 + 1] = {0};

    pstPipeMultiItem = (MultiItem *) pstDNode->pvData;
    iFd = pstPipeMultiItem->iFd;

    if (g_debug) {
        sleep(1);
        return 0;
    }

    tLog(DEBUG, "׼�����ܵ�.");
    //������
    iLen = read(iFd, sLen, 4);
    if (iLen <= 0) {
        tLog(ERROR, "���ܵ�����ʧ��[%d:%s].", errno, strerror(errno));
        return ( -1);
    }
    iLen = atoi(sLen);
    tLog(ERROR, "��ȡ�ܵ����ݳ���:%d", iLen);
    iLen = read(iFd, sRedisTran, iLen);
    if (iLen <= 0) {
        tLog(ERROR, "���ܵ�����ʧ��[%d:%s].", errno, strerror(errno));
        return ( -1);
    }

    tLog(DEBUG, "��ȡ�ܵ�����:%s", sRedisTran);

    //ModulePutKeyData( sUniqueKey, sOriginInfo, sNetTran );
    //tStrCpy(sUniqueKey, sRedisTran, UNIQUEKEY_SIZE);
    //
    pstJson = cJSON_Parse(sRedisTran);
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
    /* ��ȡΨһ��ֵ */
    GET_STR_KEY(pstJson, "key", sUniqueKey);
    tLog(INFO, "key[%s]", sUniqueKey);
    pstTransNode = GetTransLink(sUniqueKey, &g_stTransLink, &g_stTransFree, GetTranTimeout());
    if (pstTransNode == NULL) {
        tLog(INFO, "��ֵ[%s]�����������ѱ�ɾ��.", sUniqueKey);
        return 0;
    }
    pstTransItem = (TransItem *) pstTransNode->pvData;

    //��ȡmsg����
    GET_STR_KEY(pstDataJson, "msg", sMsg);
    tLog(DEBUG, "msg[%s]", sMsg);

    pstSndMultiItem = &stSndMultiItem;

    //tAsc2Bcd(pstSndMultiItem->caData + 4, sMsg, strlen(sMsg), LEFT_ALIGN);
    iLen = strlen(sMsg);
    //�鱨��ͷ
    sprintf(pstSndMultiItem->caData, "%04d", iLen);
    tStrCpy(pstSndMultiItem->caData + 4, sMsg, iLen);
    pstTransItem->iLen = iLen + 4;
    tLog(DEBUG, "msg:[%s]", pstSndMultiItem->caData);
    CopyDataFromTrans(pstSndMultiItem, pstTransItem);
    /*
        if ( ModulePack( stSndMultiItem.caData, &stSndMultiItem.iLen, (void *)sNetTran ) < 0 )
        {
            tLog( ERROR, "��ֵ[%s]������[%s]��Ӧ����ʧ��.", sUniqueKey, stSndMultiItem.sClientInfo );
            ClearTransLink( sUniqueKey, &g_stTransLink, &g_stTransFree );
            if ( COMM_SYNC == g_stMainCfg.ucCommMode )
                ClearMultiLink( stSndMultiItem.iFd, &g_stMultiLink, &g_stMultiFree );
            return 0;
        }
     */
    sprintf(sSvcName, "[%s]����Ӧ����", GetSvrName());
    PkDump(stSndMultiItem.caData, stSndMultiItem.iLen, sSvcName);

    if (SndData(&stSndMultiItem) < 0) {
        tLog(ERROR, "��ֵ[%s]������[%s]����Ӧ����ʧ��.", sUniqueKey, stSndMultiItem.sClientInfo);
        DeleteTransLink(stSndMultiItem.iFd, &g_stTransLink, &g_stTransFree);
        if (COMM_SYNC == *GetComMode())
            ClearMultiLink(stSndMultiItem.iFd, &g_stMultiLink, &g_stMultiFree);
        return 0;
    }

    tLog(INFO, "��ֵ[%s]������[%s]����Ӧ���ĳɹ�.", sUniqueKey, stSndMultiItem.sClientInfo);
    ClearTransLink(sUniqueKey, &g_stTransLink, &g_stTransFree);

    /*
        if ( COMM_SYNC == g_stMainCfg.ucCommMode )
            ClearMultiLink( stSndMultiItem.iFd, &g_stMultiLink, &g_stMultiFree );
     */
    return 0;
}

void CheckProc(int iTimeOut) {
    int iTcpTimeOut;
    time_t tTime;
    DNode *pstLinkNode;
    TransItem *pstTransItem;
    MultiItem *pstMultiItem;

    time(&tTime);
    for (pstLinkNode = g_stTransLink.pHead; pstLinkNode != NULL;) {
        pstTransItem = (TransItem *) pstLinkNode->pvData;
        if (tTime - pstTransItem->tTime > iTimeOut) {
            tLog(WARN, "��ֵΪ[%s]�Ľ�����Ӧ����ʱ,��������.", pstTransItem->sUniqueKey);
            ClearTransLink(pstTransItem->sUniqueKey, &g_stTransLink, &g_stTransFree);
        }
        pstLinkNode = pstLinkNode->pstNext;
    }
    tLog(DEBUG, "��齻��������ɹ�.");

    for (pstLinkNode = g_stMultiLink.pHead; pstLinkNode != NULL;) {
        pstMultiItem = (MultiItem *) pstLinkNode->pvData;
        iTcpTimeOut = GetTcpTimeout();
        if (COMM_SYNC == *GetComMode() && \
            tTime - pstMultiItem->tTime > iTcpTimeOut && \
            iTcpTimeOut > 0 && \
            pstMultiItem->cType == TCP_WR) {
            close(pstMultiItem->iFd);
            tLog(WARN, "����·��ʱ��������,���������[%s].", pstMultiItem->sClientInfo);
            ClearMultiLink(pstMultiItem->iFd, &g_stMultiLink, &g_stMultiFree);
        }
        pstLinkNode = pstLinkNode->pstNext;
    }
    tLog(DEBUG, "����·����������ɹ�.");
}
/*
int SendResp( char *pcNetTran  )
{
    SvcMsg  stSvcMsg;

    memset( &stSvcMsg, 0, sizeof(SvcMsg) );
    memcpy( stSvcMsg.sAppMsg, pcNetTran, TRANDATA_SIZE );
    stSvcMsg.iReqId = 1;
    stSvcMsg.iAppMsgLen = TRANDATA_SIZE;
    stSvcMsg.iReqId = 1;
    return( mSendMsg( g_stModuleInfo.sSvrId, RESP_QUEUE, &stSvcMsg, 0, 2 ) );
}
 */
