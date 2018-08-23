#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "param.h"
#include "t_dlink.h"
#include "t_netlib.h"
#include "multi_item.h"
#include "ingate.h"
#include "t_log.h"



#define		CLIENT_INFO		0
#define		LOCAL_INFO		1
/*****************************************************************************
** 函数: SelectSndLink( )                                                   **
** 类型: void                                                               **
** 入参：                                                                   **
**       DLINK *pstMultiLink  -- 多路复用链表指针                           **
**       DLINK *pstMultiFree  -- 空闲多路复用链表指针                       **
** 出参:                                                                    **
**       MultiItem *pstIoLink -- 选链成功后返回的多路复用I/O链表指针        **
** 返回值：                                                                 **
**       0  -- 找到可用链路                                                 **
**       -1 -- 未找到可用链路                                               **
** 功能：                                                                   **
**       选择一条异步发送链路。选择成功后，从多路复用链表摘下               **
** 创建日期:                                                                **
**       2012.12.04                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
int SelectSndLink( MultiItem *pstIoLink, DLINK *pstMultiLink, DLINK *pstMultiFree )
{
    DNode       *pstDNode       = NULL;

    pstDNode = tDLinkPickNode( pstMultiLink );
    if ((NULL == pstDNode) || (NULL == pstDNode->pvData))
    {
        return ERROR;
    }

    memcpy( pstIoLink, pstDNode->pvData, sizeof(MultiItem) );
    InitMultiItem( (MultiItem *)pstDNode->pvData );
    tDLinkInsert( pstMultiFree, pstDNode );
    return (EXIT_SUCCESS);
}

DNode	*ChkMultiLink( DLINK *pstMultiLink, char *pcIp, char *pcPort, char cInfoType )
{
    char    sInfo[LOCALINFO_LEN+1] = {0};
    DNode 	*pstNode = NULL;
	MultiItem *pstItem = NULL;

	sprintf( sInfo, "%s:%s", pcIp, pcPort );
    for ( pstNode = pstMultiLink->pHead; pstNode != NULL; pstNode = pstNode->pstNext )
	{
		pstItem = (MultiItem *)pstNode->pvData;
		if ( pstItem->cType != TCP_WR )
            continue;
		
		if ( cInfoType == LOCAL_INFO )
        {
            if ( strstr( pstItem->sLocalInfo, sInfo ) != NULL )
				return pstNode;
        }
        else
        {
            if ( strstr( pstItem->sClientInfo, sInfo ) != NULL )
				return pstNode;
        }
	}

	tLog( INFO, "多路复用链表项[%s]已删除.", sInfo );
	return NULL;
}

DNode *GetMultiLinkByFd( int iFd, DLINK *pstMultiLink )
{
    DNode 	*pstNode = NULL;
	MultiItem *pstItem = NULL;

    for ( pstNode = pstMultiLink->pHead; pstNode != NULL; pstNode = pstNode->pstNext )
	{
		pstItem = (MultiItem *)pstNode->pvData;
		if ( pstItem->iFd == iFd )
			break;
	}
	
	if ( NULL == pstNode )
	{
		tLog( INFO, "未找到描述字[%d]的多路复用链表项.", iFd );	
		return NULL;
	}
	tLog( INFO, "已找到描述字[%d]的多路复用链表项.", iFd );	
	return pstNode;
}

int AddPipeFd( DLINK *pstMultiLink, DLINK *pstMultiFree, int iPipeFd )
{
	if ( AddMultiLink( iPipeFd, FIFO, 0, RCV_LINK, pstMultiLink, pstMultiFree ) == NULL )
	{
		tLog ( ERROR, "增加管道描述字至多路复用链表失败!" );
		return ERROR;
	}

	tLog ( INFO, "添加管道描述字[%d]至多路复用链表成功.", iPipeFd );

	return (EXIT_SUCCESS);
}

int BuildSyncListenLink( DLINK *pstMultiLink, DLINK *pstMultiFree )
{
	int i, iFd;
	char 	*pcIp = NULL, *pcPort = NULL; 
	DNode	*pstDNode = NULL;
	MultiItem *pstItem = NULL;
	
	
	for ( i=0; i<1; i++ )
	{
		pcIp = GetIp();
		pcPort = GetPort();
		iFd = tcp_listen( pcIp, pcPort );
        if( iFd < 0 )
		{
            tLog( ERROR, "同步监听IP[%s]TCP端口[%s]失败[%d:%s].", pcIp, pcPort, errno, strerror(errno) );
			return ERROR;
        }
		pstDNode = AddMultiLink( iFd, TCP_LISTEN, COMM_SYNC, RCV_LINK, pstMultiLink, pstMultiFree );
		if ( NULL == pstDNode )
		{
			tLog ( ERROR, "增加TCP同步监听描述字[%d][%s:%s]至多路复用链表失败!", iFd, pcIp, pcPort );
			return ERROR;
		}
		pstItem = (MultiItem *)pstDNode->pvData;
		tLog ( INFO, "增加TCP同步监听描述字[%d][%s:%s]至多路复用链表成功!",\
			iFd, pcIp, pcPort );
	}

	return (EXIT_SUCCESS);
           
}

int BuildAsyncListenLink( DLINK *pstMultiLink, DLINK *pstMultiFree )
{
    /*
	int 	i, iFd;
	char    sListenIp[30];
	char    sListenPort[10];
	AsyncInfo   *pstAsyncInfo;
	DNode	*pstDNode = NULL;
	MultiItem *pstItem = NULL;
	
	pstAsyncInfo = g_stAsyncIoCfg.pstAsyncInfo;
	for ( i=0; i<g_stAsyncIoCfg.iAsyncNum; i++ )
	{
		strcpy( sListenIp, GetIp() );
		strcpy( sListenPort, GetPort() );
		iFd = tcp_listen( sListenIp, sListenPort );
        if( iFd < 0 )
		{
            tLog( ERROR, "异步监听IP[%s]TCP端口[%s]失败[%d:%s].", \
				sListenIp, sListenPort, errno, strerror(errno) );
			return ERROR;
        }
		pstDNode = AddMultiLink( iFd, TCP_LISTEN, COMM_ASYNC, RCV_LINK, pstMultiLink, pstMultiFree );
		if ( NULL == pstDNode )
		{
			tLog ( ERROR, "增加TCP异步监听描述字[%d][%s:%s]至多路复用链表失败!", iFd, sListenIp, sListenPort );
			return ERROR;
		}
		pstItem = (MultiItem *)pstDNode->pvData;
		tLog ( INFO, "增加TCP异步监听描述字[%d][%s:%s]至多路复用链表成功!", \
			iFd, sListenIp, sListenPort );
	}
*/
	return (EXIT_SUCCESS);
}

void BuildAllAsyncSndLink( DLINK *pstMultiLink, DLINK *pstMultiFree )
{
    /*
    int i;
    AsyncInfo *pstAsyncInfo;

    for ( i=0; i<g_stAsyncIoCfg.iAsyncNum; i++ )
	{
    	pstAsyncInfo = &g_stAsyncIoCfg.pstAsyncInfo[i];
		BuildAsyncSndLink( pstAsyncInfo->sOutIp, pstAsyncInfo->sOutPort, pstMultiLink, pstMultiFree ); 
	}
*/
	return;
}

DNode *BuildAsyncSndLink( char *pcClitIp, char *pcClitPort, DLINK *pstMultiLink, DLINK *pstMultiFree )
{
	//char 	cSndRcv = RCV_LINK;
    /*
	int 	iSndSock;
	DNode	*pstMultiNode = NULL;
	MultiItem *pstItem = NULL;
	
	if ( ( pstMultiNode = (DNode *)ChkMultiLink( pstMultiLink, pcClitIp, pcClitPort, CLIENT_INFO ) ) != NULL ) 
		return pstMultiNode;

    if ( ( iSndSock = tcp_connect( pcClitIp, pcClitPort, "", "" ) ) < 0 )
    {
        tLog( ERROR, "建立回送链路[%s:%s]出错.", pcClitIp, pcClitPort );
		return NULL;
    }
	pstMultiNode = AddMultiLink( iSndSock, TCP_WR, g_stMainCfg.ucCommMode, SND_LINK, pstMultiLink, pstMultiFree );
	if ( NULL == pstMultiNode )
	{
		tLog ( ERROR, "增加TCP回送描述字[%d][%s:%s]至多路复用链表失败!", iSndSock, pcClitIp, pcClitPort );
		return NULL;
	}
	pstItem = (MultiItem *)pstMultiNode->pvData;
	tLog ( INFO, "增加TCP回送描述字[%s][%s]至多路复用链表成功!", \
		pstItem->sClientInfo, pstItem->sLocalInfo );

	return pstMultiNode;
     */
    return NULL;
}
/*
DNode *BuildAsyncSndLink( char *pcClitIp, DLINK *pstMultiLink, DLINK *pstMultiFree )
{
	char 	cSndRcv = RCV_LINK;
    char    sOutIp[30] = {0}, sOutPort[10] = {0}, sClitPort[10] = {0}, sInfo[10] = {0};
	int 	iPort, iSndSock;
	DNode	*pstMultiNode = NULL;
	
	if( AuthAsyncIP( pcClitIp, "", sOutIp, sOutPort ) )
		return NULL;

	if ( ( pstMultiNode = (DNode *)ChkMultiLink( pstMultiLink, sOutIp, sOutPort, CLIENT_INFO ) ) != NULL ) 
		return pstMultiNode;

    if ( ( iSndSock = tcp_connect( sOutIp, sOutPort, "", "" ) ) < 0 )
    {
        tErrLog( ERR, "建立回送链路[%s:%s]出错.", sOutIp, sOutPort );
		return NULL;
    }
	pstMultiNode = AddMultiLink( iSndSock, TCP_WR, g_stMainCfg.ucCommMode, SND_LINK, pstMultiLink, pstMultiFree );
	if ( NULL == pstMultiNode )
	{
		tErrLog ( ERR, "增加TCP回送描述字[%d][%s:%s]至多路复用链表失败!", iSndSock, sOutIp, sOutPort );
		return NULL;
	}
	tErrLog ( INFO, "增加TCP回送描述字[%d][%s:%s]至多路复用链表成功!", \
		iSndSock, sOutIp, sOutPort );

	return pstMultiNode;
}
*/

int AddSyncTcpwrLink( int iRcvFd, DLINK *pstMultiLink, DLINK *pstMultiFree )
{
	char 	*pcClitIp = NULL;
	int 	iPort;
	DNode 	*pstDNode = NULL;
	MultiItem *pstItem = NULL;

	pcClitIp = get_client_ip( iRcvFd );
	iPort = get_client_port( iRcvFd );

	if( AuthSyncIP( pcClitIp ) )
		return (EXIT_FAILURE);

	pstDNode = AddMultiLink( iRcvFd, TCP_WR, COMM_SYNC, DUB_LINK, pstMultiLink, pstMultiFree );
	if ( NULL == pstDNode )
	{
		tLog ( ERROR, "增加TCP同步描述字[%d][%s:%d]至多路复用链表失败!", iRcvFd, pcClitIp, iPort );
		return ERROR;
	}
	pstItem = (MultiItem *)pstDNode->pvData;
	tLog ( INFO, "增加TCP同步描述字[%s][%s]至多路复用链表成功!", \
		pstItem->sClientInfo, pstItem->sLocalInfo );

	return (EXIT_SUCCESS);
}

int AddAsyncTcpwrLink( int iRcvFd, DLINK *pstMultiLink, DLINK *pstMultiFree )
{
	char 	cSndRcv = RCV_LINK;
    char    sClitPort[10] = {0}, sInfo[10] = {0};
	char 	*pcClitIp = NULL;
	int 	iPort;
	DNode 	*pstDNode = NULL;
	MultiItem *pstItem = NULL;

	pcClitIp = get_client_ip( iRcvFd );
/*
 *网控器多ip长连接
	if( AuthAsyncIP( pcClitIp, "", sOutIp, sOutPort ) )
		return ERROR;
*/
	iPort = get_client_port( iRcvFd );
	sprintf( sClitPort, "%d", iPort ); 
	if ( ChkMultiLink( pstMultiLink, pcClitIp, sClitPort, CLIENT_INFO ) ) 
		return (EXIT_FAILURE);

	if ( COMM_ASYNC == *GetComMode() )
	{
		cSndRcv = RCV_LINK;
		strcpy( sInfo, "接收" );
	}
	else if ( COMM_DOU_ASYNC == *GetComMode() )
	{
		cSndRcv = DUB_LINK;
		strcpy( sInfo, "接收回送" );
	}

	pstDNode = AddMultiLink( iRcvFd, TCP_WR, *GetComMode(), cSndRcv, pstMultiLink, pstMultiFree );
	if ( NULL == pstDNode )
	{
		tLog ( ERROR, "增加TCP异步[%s]描述字[%d][%s:%s]至多路复用链表失败!", sInfo, iRcvFd, pcClitIp, sClitPort );
		return ERROR;
	}

	pstItem = (MultiItem *)pstDNode->pvData;
	tLog ( INFO, "增加TCP异步[%s]描述字[%s][%s]至多路复用链表成功!", \
		sInfo, pstItem->sClientInfo, pstItem->sLocalInfo );

	return (EXIT_SUCCESS);
}

int AddListenFd( DLINK *pstMultiLink, DLINK *pstMultiFree )
{
	if ( COMM_SYNC  == *GetComMode() )
		return BuildSyncListenLink( pstMultiLink, pstMultiFree );
	else 
		return BuildAsyncListenLink( pstMultiLink, pstMultiFree );

	return ERROR;
}

int AuthSyncIP( const char *pcClientIp )
{
#if 0
    int     i;
	FilterInfo *pstFilterInfo = NULL;

    pstFilterInfo = g_stFilterCfg.pstDenyIp;
    for ( i = 0; i < g_stFilterCfg.iDenyNum; i++, pstFilterInfo++ )
    {
        if ( !memcmp( pstFilterInfo->sIpAddr, pcClientIp, strlen(pstFilterInfo->sIpAddr ) ) )
        {
            tLog( ERROR, "拒绝访问的IP[%s]", pcClientIp );
			return ERROR;
        }
    }

    /* 允许所有IP访问 */
    if ( g_stFilterCfg.iAllowNum == 0 )
    {
        tLog( INFO, "允许访问的IP[%s]", pcClientIp );
		return (EXIT_SUCCESS);
    }

    pstFilterInfo = g_stFilterCfg.pstAllowIp;
    for ( i = 0; i < g_stFilterCfg.iAllowNum; i++, pstFilterInfo++ )
    {
        if ( !memcmp( pstFilterInfo->sIpAddr, pcClientIp, strlen(pstFilterInfo->sIpAddr ) ) )
        {
            tLog( INFO, "允许访问的IP[%s]", pcClientIp );
			return (EXIT_SUCCESS);
        }
    }

    tLog( ERROR, "拒绝访问的IP[%s]", pcClientIp );
	return ERROR;
#endif
        return (EXIT_SUCCESS);
}

int AuthAsyncIP( const char *pcInIp, const char *pcInPort )
{
    /*
	int i;
	AsyncInfo *pstAsyncInfo;

	pstAsyncInfo = g_stAsyncIoCfg.pstAsyncInfo;
	for ( i=0; i<g_stAsyncIoCfg.iAsyncNum; i++ )
	{
		if ( !memcmp( pstAsyncInfo->sInIp, pcInIp, strlen(pcInIp) ) &&
             !memcmp( pstAsyncInfo->sInPort, pcInPort, strlen(pcInPort) ) )
        {
			tLog( INFO, "允许访问的IP[%s]", pcInIp );
            break;
        }
	}
	
   	if ( i == g_stAsyncIoCfg.iAsyncNum )
	{
		tLog( ERROR, "拒绝访问的IP[%s]", pcInIp );
		return ERROR;
	}
*/
	return (EXIT_SUCCESS);
}

int ChkAsyncIP( const char *pcLocalIp, const char *pcLocalPort, char *pcPeerIp, char *pcPeerPort )
{
    /*
	int i;
	AsyncInfo *pstAsyncInfo;

	for ( i=0; i<g_stAsyncIoCfg.iAsyncNum; i++ )
	{
		pstAsyncInfo = g_stAsyncIoCfg.pstAsyncInfo+i;
		if ( !memcmp( pstAsyncInfo->sListenIp, pcLocalIp, strlen(pcLocalIp) ) &&
             !memcmp( pstAsyncInfo->sListenPort, pcLocalPort, strlen(pcLocalPort) ) )
        {
	       	strcpy( pcPeerIp, pstAsyncInfo->sOutIp );
           	strcpy( pcPeerPort, pstAsyncInfo->sOutPort );
			tLog( INFO, "报文通过本地监听IP[%s:%s]接收, 回送IP[%s:%s]", \
					pcLocalIp, pcLocalPort, pcPeerIp, pcPeerPort );
			return (EXIT_SUCCESS);
            break;
        }
	}
	
	tLog( ERROR, "报文通过本地监听IP[%s:%s]接收，监听未配置.", pcLocalIp, pcLocalPort );
     */ 
	return ERROR;
}

void AddTcpwrFd( DLINK *pstMultiLink, DLINK *pstMultiFree, int iListenSock )
{
	//char 	cSndRcv;
    int     iNewSock, iSockLen;
	//DNode	*pstDNode = NULL;
	//AsyncInfo	*pstAsyncInfo = NULL;
    //char    sClitIp[30], sClitPort[10], sOutIp[30], sOutPort[10];;
	struct sockaddr stAddr;
	
	iSockLen = sizeof(struct sockaddr);
	if ( ( iNewSock = accept( iListenSock, &stAddr, (socklen_t *)&iSockLen )) < 0 )
	{
        tLog ( ERROR, "accept描述字[%d]失败[%d:%s].", iListenSock, errno, strerror(errno) );
		return;
    }

	switch( *GetComMode() )
	{
		case COMM_SYNC:
			if ( AddSyncTcpwrLink( iNewSock, pstMultiLink, pstMultiFree ) )
				close( iNewSock );
			break;
		case COMM_ASYNC:
		case COMM_DOU_ASYNC:
			if ( AddAsyncTcpwrLink( iNewSock, pstMultiLink, pstMultiFree ) )
				close( iNewSock );
			break;
	}

	return;
}
