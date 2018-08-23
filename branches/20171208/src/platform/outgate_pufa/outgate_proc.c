#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "outgate.h"
#include "param.h"

extern int g_iQuitLoop;
extern int g_iaPipeFd[2];
#define MAXITEM 100

void getUniqueKey(char *uniKey) {
    struct timeval time;
    gettimeofday(&time, NULL);
    sprintf(uniKey, "%ld", (long) (time.tv_sec) * 1000000 + time.tv_usec);
}

/*
char *ItemToJson(MultiItem *pstMultiItem) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
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

    tBcd2Asc(sData, pstMultiItem->caData + 2, (pstMultiItem->iLen - 2) * 2, LEFT_ALIGN);
    tLog(INFO, "tBcd2Asc data[%s]", sData);
    SET_STR_KEY(pstDataJson, "msg", (const char *) sData);
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    pcMsg = cJSON_PrintUnformatted(pstJson);
    cJSON_Delete(pstDataJson);
    cJSON_Delete(pstJson);
    return pcMsg;
}
 */

char *MutiItemToJson(MultiItem *pstMultiItem) {
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    char pcJson[REDISDATA_SIZE] = {0};
    char *pcMsg = NULL, sUniqueKey[20] = {0};
    unsigned char sData[REDISDATA_SIZE] = {0};
    char cPktLenType = GetPktLenType();

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "����Jsonʧ��.");
        return NULL;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "����Key-Jsonʧ��.");
        cJSON_Delete(pstJson);
        return NULL;
    }
    getUniqueKey(sUniqueKey);
    SET_STR_KEY(pstJson, "svrid", GetSvrId());
    SET_STR_KEY(pstJson, "key", sUniqueKey);
    if (LENTYPE_BCD == cPktLenType) {
        tBcd2Asc(sData, pstMultiItem->caData + PKT_BCDLEN, (pstMultiItem->iLen - PKT_BCDLEN) * 2, LEFT_ALIGN);
    } else {
        tBcd2Asc(sData, pstMultiItem->caData + PKT_ASCLEN, (pstMultiItem->iLen - PKT_ASCLEN) * 2, LEFT_ALIGN);
    }

    tLog(DEBUG, "tBcd2Asc data[%s]", sData);
    SET_STR_KEY(pstDataJson, "msg", (const char *) sData);
    SET_JSON_KEY(pstJson, "data", pstDataJson);
    pcMsg = cJSON_PrintUnformatted(pstJson);
    //tLog(INFO,"pcMsg[%s]", pcMsg);
    //tStrCpy(pcJson, pcMsg, strlen(pcMsg));
    //cJSON_Delete(pstDataJson);
    cJSON_Delete(pstJson);
    return pcMsg;
}

/*  ���ڷ���Send����. ��ȡ����ģ����������, д����ڷ����ӹܵ�. */
int SendProc() {
    int iRet, iLen;
    char sSvrName[20];
    char sRedisTran[REDISDATA_SIZE] = {0};
    char sLen[4 + 1] = {0};

    tStrCpy(sSvrName, GetSvrId(), strlen(GetSvrId()));
    /* �رչܵ��� */
    close(g_iaPipeFd[0]);
    //signal( SIGUSR1, SigQuitSend );

    while (g_iQuitLoop) {
        memset(sRedisTran, 0, sizeof (sRedisTran));
        iRet = tRecvMsg(sSvrName, sRedisTran + 4, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "��ȡ��Ϣʧ��.");
                break;
            case MSG_TIMEOUT:
                tLog(DEBUG, "[%s]�ȴ���Ϣ��ʱ[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                tLog(INFO, "[%s]���ڷ������������Ϣ��׼��д��ܵ�.", sSvrName);
                //д�ṹ�峤��
                sprintf(sLen, "%04d", iLen);
                memcpy(sRedisTran, sLen, 4);
                tLog(DEBUG, "�ܵ�д������:%s", sRedisTran);
                write(g_iaPipeFd[1], (char *) sRedisTran, iLen + 4);
                tLog(INFO, "д��ܵ��ɹ�");
                break;
        }

    }

    /* �رչܵ�д */
    close(g_iaPipeFd[1]);
    return ( 0);
}

/*  ���ڷ���PIPE����: ����pipe�е�NetTran�ṹ,�����Ҷ�·���������ж�Ӧ��,   **
    ͨ�����������MultiItem��fd���͸���������,�����뽻��������              */
int PipeProc(MultiItem stMultiItem[], int iTimeOut) {
    int iFd, iLen, iRet, iSock, i, iCurItem;
    time_t tTime;
    char sRedisTran[REDISDATA_SIZE] = {0}, sMsg[REDISDATA_SIZE] = {0}, sLen[4 + 1] = {0}, sTmp[4 + 1] = {0};
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    MultiItem stSndLink;
    char sSvrName[20] = {0};
    char sKey[20] = {0};
    char cPktLenType = GetPktLenType();

    strcat(sSvrName, GetSvrName());
    iFd = stMultiItem[0].iFd;
    /* ���ܵ�ʧ��,�����˳� */
    tLog(DEBUG, "׼�����ܵ�.");
    //������
    iLen = read(iFd, sLen, 4);
    if (iLen <= 0) {
        tLog(ERROR, "���ܵ�����ʧ��[%d:%s].", errno, strerror(errno));
        return ( -1);
    }
    tLog(INFO, "��ȡ�ܵ����ݳ���ASCII:%s", sLen);
    iLen = atoi(sLen);
    tLog(INFO, "��ȡ�ܵ����ݳ���:%d", iLen);
    iLen = read(iFd, sRedisTran, iLen);
    //iLen = read(iFd, (char *) sRedisTran, sizeof (sRedisTran));
    if (iLen <= 0) {
        tLog(ERROR, "���ܵ�����ʧ��[%d].", errno);
        return ( -1);
    }
    tLog(DEBUG, "��ȡ�ܵ�����:%s-����[%d]", sRedisTran, iLen);
    /* ���¶�·���������ֵ����ͨѶʱ�� */
    time(&tTime);
    stMultiItem[1].tTime = tTime;

    /* ��ѡ�񵽵ķ�����·��������ʹ�ÿ�����㷢�����ݡ� */
    /* �������·ͬʱ���ڽ�������ʱ�����ݳ�ͻ��         */
    pstJson = cJSON_Parse(sRedisTran);
    if (NULL == pstJson) {
        tLog(ERROR, "��ȡ��Ϣʧ��,��������.");
        return -1;
    }
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstJson, "key", sKey);
    tLog(ERROR, "��ֵ[%s].", sKey);
    if (NULL == pstDataJson) {
        tLog(ERROR, "��ȡdata����ʧ��,��Ϣ��������.");
        cJSON_Delete(pstJson);
        return -1;
    }
    GET_STR_KEY(pstDataJson, "msg", sMsg);
    cJSON_Delete(pstJson);


    /*���ͨѶ�����Ƿ���ڣ��������������½���*/
    for (i = 0; i < MAXITEM; i++) {
        if (stMultiItem[i].cRdStatus == RD_FIN) {
            (void) BuildAsyncSndLink(&stMultiItem[i]);
            iCurItem = i;
            memcpy(&stSndLink, &stMultiItem[i], sizeof (MultiItem));
            break;
        }

    }

    //ASCII��ĳ�BCD�벢���ϱ���ͷ
    if (LENTYPE_BCD == cPktLenType) {
        tAsc2Bcd(&(stSndLink.caData[PKT_BCDLEN]), sMsg, strlen(sMsg), LEFT_ALIGN);
    } else {
        tAsc2Bcd(&(stSndLink.caData[PKT_ASCLEN]), sMsg, strlen(sMsg), LEFT_ALIGN);
    }

    //�鱨��ͷ
    iLen = strlen(sMsg) / 2;
    if (LENTYPE_BCD == cPktLenType) {
        sprintf(sLen, "%04x", iLen);
        tAsc2Bcd(sTmp, sLen, 4, RIGHT_ALIGN);
        memcpy(stSndLink.caData, sTmp, PKT_BCDLEN);
        stSndLink.iLen = iLen + PKT_BCDLEN;

    } else {
        sprintf(sLen, "%04d", iLen);
        memcpy(stSndLink.caData, sLen, PKT_ASCLEN);
        stSndLink.iLen = iLen + PKT_ASCLEN;
    }


    /* �ӿ�.so�д�ӡ
    sprintf( sMsg,"[%s]���͵�������", GetExecName() );
    PkDump( stSndLink.caData, stSndLink.iLen, sMsg );
     */


    /* ���������ĳ������ͨѶ��������TCP_WR���ͣ�   */
    /* ɾ���������ֵ����н����������Լ���·����������   */
#ifdef _DEBUG
    MultiItemChk(&stSndLink, "SndDev IoLink");
#endif
    iSock = stSndLink.iFd;
    iRet = SndData(&stSndLink);
    if (iRet < 0) {
        tLog(ERROR, "�ӿ�ģ��[%s]���ڷ�����������ʧ��.[%s][%d]"
                , sSvrName, stSndLink.sClientInfo, iSock);
        /* Modified by zhongduying on 2012.12.28 for load balance */
        /* �������ʧ�ܣ��ر��׽��֣����Ӷ�·���ñ���ɾ�����(����ͬ���첽��ɾ��) */
        if (TCP_WR == stSndLink.cType) {

            /* ���iFd��Ӧ�Ķ�·�����б��� */
            ClearMultiLink(&stMultiItem[iCurItem]);
            return ( 0);
        }
    }
    PkDump(stSndLink.caData, stSndLink.iLen, "���ڷ��ͱ���");
    tLog(ERROR, "�ӿ�ģ��[%s]���ڷ����������ĳɹ�.[%d]"
            , sSvrName, iSock);

#ifdef _DEBUG
    //MultiLinkChk( &g_stMultiLink, "After SndDev" );
    //TransLinkChk( &g_stTransLink, "After SndDev" );
#endif

    return ( 0);
}

int TcpProc(MultiItem *pstMultiItem, int iTimeOut) {
    int iRet, iFd;
    time_t tTime;
    char sSvcName[20] = {0}, *pcAscMsg = NULL;


    /* MultiItem�е�data����������,len������� */
    time(&tTime);
    pstMultiItem->tTime = tTime; /* ���¶�·��������������ͨѶʱ�� */
    pstMultiItem->tKASnd = tTime; /* ���±���ķ��ͼ��ʱ��         */
    pstMultiItem->tKARcv = tTime; /* ���±���Ľ��ռ��ʱ��         */
    iFd = pstMultiItem->iFd;
    iRet = RcvData(pstMultiItem);
    switch (iRet) {
        case PACK_LEN_ERR:
            tLog(DEBUG, "�յ�������[%d][%s]���ĳ��ȴ���,�ر�����.", iFd, pstMultiItem->sClientInfo);
            ClearMultiLink(pstMultiItem);
            return ( 0);

        case SOCK_CLOSE:
            tLog(DEBUG, "�յ�FIN��Ϣ,�ر�������[%d]", iFd);
            /*���iFd��Ӧ�Ķ�·�����б��� */
            ClearMultiLink(pstMultiItem);
            return ( 0);
    }

    /* ������׽���û�н�����ȫ������,���ؼ������� */
    if (pstMultiItem->cRdStatus != RD_FIN) {
        return ( 0);
    }
    tLog(ERROR, "TcpProc������ڷ���Ӧ����");
    strcat(sSvcName, GetQname());
    /* ��дӦ����Ϣ */
    //memcpy( pstSvcMsg, pstTransItem->caSvcMsgInfo, SVCMSGINFO_SIZE );
    PkDump(pstMultiItem->caData, pstMultiItem->iLen, "���ڷ���Ӧ����");
    pcAscMsg = MutiItemToJson(pstMultiItem);
    /* ���Ľ�����ɺ����ö�·����������Ķ�д״̬ */
    //RecoverMultiItem(pstMultiItem);
    //�ر�����
    ClearMultiLink(pstMultiItem);
    /* ���͸����ñ��ӿڷ�������Ӧ�÷����� */
    iRet = tSendMsg(sSvcName, (char *) pcAscMsg, strlen(pcAscMsg));
    if (pcAscMsg)
        free(pcAscMsg);
    switch (iRet) {
        case MSG_ERR:
            tLog(ERROR, "�����м��ʧ��[%d],����[%s],�ͻ�����Ϣ[%s].", \
                iRet, GetSvrName(), pstMultiItem->sClientInfo);
            break;
        case MSG_SUCC:
            tLog(INFO, "������Ϣ�ɹ�.");
            break;
    }
    /* �����ͬ�������ӣ��ر��׽��֣����Ӷ�·���ñ���ɾ����� */


    return 0;
}

int RecvProc(void) {
    int iRet, iFd, i, iMaxFd;
    fd_set stRdSet;
    time_t tLastChkTime, tTime;
    struct timeval stTime;
    MultiItem stMultiItem[MAXITEM];

    close(g_iaPipeFd[1]);
    AddPipeFd(&stMultiItem[0], g_iaPipeFd[0]);
    //��ʼ��ͨ��״̬Ϊ�ر�
    for (i = 1; i < MAXITEM; i++) {
        stMultiItem[i].cRdStatus = RD_FIN;
        stMultiItem[i].iFd = -1;
        stMultiItem[i].cType = TCP_LISTEN;
    }

    //(void) BuildAsyncSndLink(&stMultiItem[1]);
    time(&tLastChkTime);

    while (g_iQuitLoop) {
        iMaxFd = 0;
        stTime.tv_sec = 5;
        stTime.tv_usec = 0;
        FD_ZERO(&stRdSet);
        tLog(INFO, "stMultiItem[0].iFd[%d],stMultiItem[1].iFd[%d]", stMultiItem[0].iFd, stMultiItem[1].iFd);
        for (i = 0; i < MAXITEM; i++) {
            if (stMultiItem[i].cRdStatus != RD_FIN)
                FD_SET(stMultiItem[i].iFd, &stRdSet);
            if (stMultiItem[i].iFd > iMaxFd)
                iMaxFd = stMultiItem[i].iFd;
        }
        iRet = select(iMaxFd + 1, &stRdSet, (fd_set *) NULL, (fd_set *) NULL, &stTime);
        if (iRet < 0) {
            if (!g_iQuitLoop)
                tLog(ERROR, "%s-Recv receive quit signal.", GetSvrName());
            else {
                tLog(ERROR, "%s-Recv select I/O error.", GetSvrName());
                sleep(2);
            }
            continue;
        }
        /* select�ڹ涨��ʱ����û�о�����������,��齻�׼���·���������� */
        if (0 == iRet) {
            /*
            if (COMM_ASYNC == *GetComMode()) {
                (void) BuildAsyncSndLink(&stMultiItem[1]);
            }
             */
            /* �ж��Ƿ񵽴���redis����״̬��ʱ�� */
            time(&tTime);
            if (tTime - tLastChkTime > 5) {
                tLastChkTime = tTime;

                /* ���redis����״̬ */
                tCheckStatus();
            }
            continue;
        }



        /* ���Ѿ����Ķ�·���������ַ�����IO���� */
        for (i = 0; i < MAXITEM; i++) {
            iFd = stMultiItem[i].iFd;
            if (FD_ISSET(iFd, &stRdSet)) {
                tLog(INFO, "�ļ�������%d,pstMultiItem->cType[%d]", iFd, stMultiItem[i].cType);
                switch (stMultiItem[i].cType) {
                    case FIFO:
                        /* MultiLinkChk( &g_stMultiLink, "FIFO fd_isset" ); */
                        iRet = PipeProc(stMultiItem, GetTranTimeout());
                        tLog(INFO, "iRet[%d]", iRet);
                        if (iRet < 0)
                            return -1;
                        break;
                    case TCP_WR:
                        TcpProc(&stMultiItem[i], GetTranTimeout());
                        /* MultiLinkChk( &g_stMultiLink, "TCP_WR fd_isset" ); */
                        break;
                }
            }

        }
    }
    /* �رն�·�����������ͨѶ������ */
    for (i = 0; i < MAXITEM; i++) {
        close(stMultiItem[i].iFd);
        //close(stMultiItem[1].iFd);
    }
    return 0;
}
