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
        tLog(ERROR, "创建Json失败.");
        return NULL;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "创建Key-Json失败.");
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

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建Json失败.");
        return NULL;
    }
    pstDataJson = cJSON_CreateObject();
    if (NULL == pstDataJson) {
        tLog(ERROR, "创建Key-Json失败.");
        cJSON_Delete(pstJson);
        return NULL;
    }
    getUniqueKey(sUniqueKey);
    SET_STR_KEY(pstJson, "svrid", GetSvrId());
    SET_STR_KEY(pstJson, "key", sUniqueKey);

    tBcd2Asc(sData, pstMultiItem->caData + 4, (pstMultiItem->iLen - 4) * 2, LEFT_ALIGN);
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

/*  出口方向Send进程. 读取出口模块的请求队列, 写入出口方向父子管道. */
int SendProc() {
    int iRet, iLen;
    char sSvrName[20];
    char sRedisTran[REDISDATA_SIZE] = {0};
    char sLen[4 + 1] = {0};

    tStrCpy(sSvrName, GetSvrId(), strlen(GetSvrId()));
    /* 关闭管道读 */
    close(g_iaPipeFd[0]);
    //signal( SIGUSR1, SigQuitSend );

    while (g_iQuitLoop) {
        memset(sRedisTran, 0, sizeof (sRedisTran));
        iRet = tRecvMsg(sSvrName, sRedisTran + 4, &iLen, TIMEOUT);
        switch (iRet) {
            case MSG_ERR:
                tLog(ERROR, "获取消息失败.");
                break;
            case MSG_TIMEOUT:
                tLog(DEBUG, "[%s]等待消息超时[%d].", sSvrName, TIMEOUT);
                break;
            case MSG_SUCC:
                tLog(INFO, "[%s]出口方向接收请求消息，准备写入管道.", sSvrName);
                //写结构体长度
                sprintf(sLen, "%04d", iLen);
                memcpy(sRedisTran, sLen, 4);
                tLog(DEBUG, "管道写入内容:%s", sRedisTran);
                write(g_iaPipeFd[1], (char *) sRedisTran, iLen + 4);
                tLog(INFO, "写入管道成功");
                break;
        }

    }

    /* 关闭管道写 */
    close(g_iaPipeFd[1]);
    return ( 0);
}

/*  出口方向PIPE处理: 接收pipe中的NetTran结构,并查找多路复用链表中对应项,   **
    通过该链表项的MultiItem的fd发送给外联机构,并插入交易链表项              */
int PipeProc(MultiItem stMultiItem[], int iTimeOut) {
    int iFd, iLen, iRet, iSock;
    time_t tTime;
    char sRedisTran[REDISDATA_SIZE] = {0}, sMsg[REDISDATA_SIZE] = {0}, sLen[4 + 1] = {0};
    cJSON *pstJson = NULL, *pstDataJson = NULL;
    MultiItem stSndLink;
    char sSvrName[20] = {0};
    char sKey[20] = {0};

    strcat(sSvrName, GetSvrName());
    iFd = stMultiItem[0].iFd;
    /* 读管道失败,进程退出 */
    tLog(DEBUG, "准备读管道.");
    //读长度
    iLen = read(iFd, sLen, 4);
    if (iLen <= 0) {
        tLog(ERROR, "读管道长度失败[%d:%s].", errno, strerror(errno));
        return ( -1);
    }
    iLen = atoi(sLen);
    tLog(ERROR, "读取管道内容长度:%d", iLen);
    iLen = read(iFd, sRedisTran, iLen);
    //iLen = read(iFd, (char *) sRedisTran, sizeof (sRedisTran));
    if (iLen <= 0) {
        tLog(ERROR, "读管道内容失败[%d].", errno);
        return ( -1);
    }
    tLog(DEBUG, "读取管道内容:%s-长度[%d]", sRedisTran, iLen);
    /* 更新多路复用描述字的最近通讯时间 */
    time(&tTime);
    stMultiItem[1].tTime = tTime;

    /* 将选择到的发送链路做拷贝，使用拷贝结点发送数据。 */
    /* 避免该链路同时用于接收数据时有数据冲突。         */
    pstJson = cJSON_Parse(sRedisTran);
    if (NULL == pstJson) {
        tLog(ERROR, "获取消息失败,放弃处理.");
        return -1;
    }
    pstDataJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstJson, "key", sKey);
    tLog(ERROR, "键值[%s].", sKey);
    if (NULL == pstDataJson) {
        tLog(ERROR, "获取data数据失败,消息放弃处理.");
        cJSON_Delete(pstJson);
        return -1;
    }
    GET_STR_KEY(pstDataJson, "msg", sMsg);
    cJSON_Delete(pstJson);


    /*检查通讯链接是否存在，若不存在则重新建链*/
    (void) BuildAsyncSndLink(&stMultiItem[1]);
    memcpy(&stSndLink, &stMultiItem[1], sizeof (MultiItem));
    //ASCII码改成BCD码并加上报文头
    tAsc2Bcd(&(stSndLink.caData[4]), sMsg, strlen(sMsg), LEFT_ALIGN);
    //组报文头
    iLen = strlen(sMsg) / 2;
    sprintf(sLen, "%04d", iLen);
    memcpy(stSndLink.caData, sLen, 4);
    stSndLink.iLen = iLen + 4;
    /* 接口.so中打印
    sprintf( sMsg,"[%s]发送的请求报文", GetExecName() );
    PkDump( stSndLink.caData, stSndLink.iLen, sMsg );
     */


    /* 发送请求报文出错，如果通讯描述字是TCP_WR类型，   */
    /* 删除该描述字的所有交易链表项以及多路复用链表项   */
#ifdef _DEBUG
    MultiItemChk(&stSndLink, "SndDev IoLink");
#endif
    iSock = stSndLink.iFd;
    iRet = SndData(&stSndLink);
    if (iRet < 0) {
        tLog(ERROR, "接口模块[%s]出口方向发送请求报文失败.[%s][%d]"
                , sSvrName, stSndLink.sClientInfo, iSock);
        /* Modified by zhongduying on 2012.12.28 for load balance */
        /* 如果发送失败，关闭套接字，并从多路复用表中删除结点(不管同步异步均删除) */
        if (TCP_WR == stSndLink.cType) {

            /* 清除iFd对应的多路复用列表项 */
            ClearMultiLink(&stMultiItem[1]);
            return ( 0);
        }
    }
    PkDump(stSndLink.caData, stSndLink.iLen, "出口发送报文");
    tLog(ERROR, "接口模块[%s]出口方向发送请求报文成功.[%d]"
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


    /* MultiItem中的data保存请求报文,len保存包长 */
    time(&tTime);
    pstMultiItem->tTime = tTime; /* 更新多路复用链表项的最近通讯时间 */
    pstMultiItem->tKASnd = tTime; /* 更新保活报文发送检查时间         */
    pstMultiItem->tKARcv = tTime; /* 更新保活报文接收检查时间         */
    iFd = pstMultiItem->iFd;
    iRet = RcvData(pstMultiItem);
    switch (iRet) {
        case PACK_LEN_ERR:
            tLog(DEBUG, "收到描述字[%d][%s]报文长度错误,关闭连接.", iFd, pstMultiItem->sClientInfo);
            ClearMultiLink(pstMultiItem);
            return ( 0);

        case SOCK_CLOSE:
            tLog(DEBUG, "收到FIN信息,关闭描述字[%d]", iFd);
            /*清除iFd对应的多路复用列表项 */
            ClearMultiLink(pstMultiItem);
            return ( 0);
    }

    /* 如果本套接字没有接收完全部数据,返回继续接收 */
    if (pstMultiItem->cRdStatus != RD_FIN) {
        return ( 0);
    }
    tLog(ERROR, "TcpProc处理出口方向应答报文");
    strcat(sSvcName, GetQname());
    /* 填写应答信息 */
    //memcpy( pstSvcMsg, pstTransItem->caSvcMsgInfo, SVCMSGINFO_SIZE );
    PkDump(pstMultiItem->caData, pstMultiItem->iLen, "出口方向应答报文");
    pcAscMsg = MutiItemToJson(pstMultiItem);
    /* 报文接收完成后重置多路复用链表项的读写状态 */
    RecoverMultiItem(pstMultiItem);
    tLog(INFO, "BEFORE tSendMsg ......");
    /* 回送给调用本接口服务器的应用服务器 */
    iRet = tSendMsg(sSvcName, (char *) pcAscMsg, strlen(pcAscMsg));
    tLog(INFO, "AFTER tSendMsg ......");
    if (pcAscMsg)
        free(pcAscMsg);
    switch (iRet) {
        case MSG_ERR:
            tLog(ERROR, "调用中间件失败[%d],服务[%s],客户端信息[%s].", \
                iRet, GetSvrName(), pstMultiItem->sClientInfo);
            break;
        case MSG_SUCC:
            tLog(INFO, "发送消息成功.");
            break;
    }
    /* 如果是同步短链接，关闭套接字，并从多路复用表中删除结点 */


    return 0;
}

int RecvProc(void) {
    int iRet, iFd, i, iMaxFd;
    fd_set stRdSet;
    time_t tLastChkTime, tTime;
    struct timeval stTime;
    MultiItem stMultiItem[2];

    close(g_iaPipeFd[1]);
    AddPipeFd(&stMultiItem[0], g_iaPipeFd[0]);
    //初始化通信状态为关闭
    stMultiItem[1].cRdStatus = RD_FIN;
    (void) BuildAsyncSndLink(&stMultiItem[1]);
    time(&tLastChkTime);

    while (g_iQuitLoop) {
        iMaxFd = 0;
        stTime.tv_sec = 5;
        stTime.tv_usec = 0;
        FD_ZERO(&stRdSet);
        tLog(INFO, "stMultiItem[0].iFd[%d],stMultiItem[1].iFd[%d]", stMultiItem[0].iFd, stMultiItem[1].iFd);
        for (i = 0; i < 2; i++) {
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
        /* select在规定的时间内没有就绪的描述字,检查交易及多路复用链表项 */
        if (0 == iRet) {
            if (COMM_ASYNC == *GetComMode()) {
                (void) BuildAsyncSndLink(&stMultiItem[1]);
            }
            /* 检查redis连接状态 */
            tCheckStatus();
            continue;
        }

        /* 判断是否到达检查交易及多路复用链表项的时间 */
        time(&tTime);
        if (tTime - tLastChkTime > 5) {
            tLastChkTime = tTime;

            /* 建立异步通讯链路 */
            if (COMM_ASYNC == *GetComMode()) {
                (void) BuildAsyncSndLink(&stMultiItem[1]);
            }
        }
        /* 对已就绪的多路复用描述字分类做IO处理 */
        for (i = 0; i < 2; i++) {
            iFd = stMultiItem[i].iFd;
            if (FD_ISSET(iFd, &stRdSet)) {
                tLog(INFO, "文件描述符%d,pstMultiItem->cType[%d]", iFd, stMultiItem[i].cType);
                switch (stMultiItem[i].cType) {
                    case FIFO:
                        /* MultiLinkChk( &g_stMultiLink, "FIFO fd_isset" ); */
                        iRet = PipeProc(stMultiItem, GetTranTimeout());
                        tLog(INFO, "iRet[%d]", iRet);
                        if (iRet < 0)
                            return -1;
                        break;
                    case TCP_WR:
                        TcpProc(&stMultiItem[1], GetTranTimeout());
                        /* MultiLinkChk( &g_stMultiLink, "TCP_WR fd_isset" ); */
                        break;
                }
            }
        }
    }

    /* 关闭多路复用链表项的通讯描述字 */
    close(stMultiItem[0].iFd);
    close(stMultiItem[1].iFd);
    return 0;
}
