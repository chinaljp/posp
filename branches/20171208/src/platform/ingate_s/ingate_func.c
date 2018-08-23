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
** ����: SelectSndLink( )                                                   **
** ����: void                                                               **
** ��Σ�                                                                   **
**       DLINK *pstMultiLink  -- ��·��������ָ��                           **
**       DLINK *pstMultiFree  -- ���ж�·��������ָ��                       **
** ����:                                                                    **
**       MultiItem *pstIoLink -- ѡ���ɹ��󷵻صĶ�·����I/O����ָ��        **
** ����ֵ��                                                                 **
**       0  -- �ҵ�������·                                                 **
**       -1 -- δ�ҵ�������·                                               **
** ���ܣ�                                                                   **
**       ѡ��һ���첽������·��ѡ��ɹ��󣬴Ӷ�·��������ժ��               **
** ��������:                                                                **
**       2012.12.04                                                         **
** �޸�����:                                                                **
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

	tLog( INFO, "��·����������[%s]��ɾ��.", sInfo );
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
		tLog( INFO, "δ�ҵ�������[%d]�Ķ�·����������.", iFd );	
		return NULL;
	}
	tLog( INFO, "���ҵ�������[%d]�Ķ�·����������.", iFd );	
	return pstNode;
}

int AddPipeFd( DLINK *pstMultiLink, DLINK *pstMultiFree, int iPipeFd )
{
	if ( AddMultiLink( iPipeFd, FIFO, 0, RCV_LINK, pstMultiLink, pstMultiFree ) == NULL )
	{
		tLog ( ERROR, "���ӹܵ�����������·��������ʧ��!" );
		return ERROR;
	}

	tLog ( INFO, "��ӹܵ�������[%d]����·��������ɹ�.", iPipeFd );

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
            tLog( ERROR, "ͬ������IP[%s]TCP�˿�[%s]ʧ��[%d:%s].", pcIp, pcPort, errno, strerror(errno) );
			return ERROR;
        }
		pstDNode = AddMultiLink( iFd, TCP_LISTEN, COMM_SYNC, RCV_LINK, pstMultiLink, pstMultiFree );
		if ( NULL == pstDNode )
		{
			tLog ( ERROR, "����TCPͬ������������[%d][%s:%s]����·��������ʧ��!", iFd, pcIp, pcPort );
			return ERROR;
		}
		pstItem = (MultiItem *)pstDNode->pvData;
		tLog ( INFO, "����TCPͬ������������[%d][%s:%s]����·��������ɹ�!",\
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
            tLog( ERROR, "�첽����IP[%s]TCP�˿�[%s]ʧ��[%d:%s].", \
				sListenIp, sListenPort, errno, strerror(errno) );
			return ERROR;
        }
		pstDNode = AddMultiLink( iFd, TCP_LISTEN, COMM_ASYNC, RCV_LINK, pstMultiLink, pstMultiFree );
		if ( NULL == pstDNode )
		{
			tLog ( ERROR, "����TCP�첽����������[%d][%s:%s]����·��������ʧ��!", iFd, sListenIp, sListenPort );
			return ERROR;
		}
		pstItem = (MultiItem *)pstDNode->pvData;
		tLog ( INFO, "����TCP�첽����������[%d][%s:%s]����·��������ɹ�!", \
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
        tLog( ERROR, "����������·[%s:%s]����.", pcClitIp, pcClitPort );
		return NULL;
    }
	pstMultiNode = AddMultiLink( iSndSock, TCP_WR, g_stMainCfg.ucCommMode, SND_LINK, pstMultiLink, pstMultiFree );
	if ( NULL == pstMultiNode )
	{
		tLog ( ERROR, "����TCP����������[%d][%s:%s]����·��������ʧ��!", iSndSock, pcClitIp, pcClitPort );
		return NULL;
	}
	pstItem = (MultiItem *)pstMultiNode->pvData;
	tLog ( INFO, "����TCP����������[%s][%s]����·��������ɹ�!", \
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
        tErrLog( ERR, "����������·[%s:%s]����.", sOutIp, sOutPort );
		return NULL;
    }
	pstMultiNode = AddMultiLink( iSndSock, TCP_WR, g_stMainCfg.ucCommMode, SND_LINK, pstMultiLink, pstMultiFree );
	if ( NULL == pstMultiNode )
	{
		tErrLog ( ERR, "����TCP����������[%d][%s:%s]����·��������ʧ��!", iSndSock, sOutIp, sOutPort );
		return NULL;
	}
	tErrLog ( INFO, "����TCP����������[%d][%s:%s]����·��������ɹ�!", \
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
		tLog ( ERROR, "����TCPͬ��������[%d][%s:%d]����·��������ʧ��!", iRcvFd, pcClitIp, iPort );
		return ERROR;
	}
	pstItem = (MultiItem *)pstDNode->pvData;
	tLog ( INFO, "����TCPͬ��������[%s][%s]����·��������ɹ�!", \
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
 *��������ip������
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
		strcpy( sInfo, "����" );
	}
	else if ( COMM_DOU_ASYNC == *GetComMode() )
	{
		cSndRcv = DUB_LINK;
		strcpy( sInfo, "���ջ���" );
	}

	pstDNode = AddMultiLink( iRcvFd, TCP_WR, *GetComMode(), cSndRcv, pstMultiLink, pstMultiFree );
	if ( NULL == pstDNode )
	{
		tLog ( ERROR, "����TCP�첽[%s]������[%d][%s:%s]����·��������ʧ��!", sInfo, iRcvFd, pcClitIp, sClitPort );
		return ERROR;
	}

	pstItem = (MultiItem *)pstDNode->pvData;
	tLog ( INFO, "����TCP�첽[%s]������[%s][%s]����·��������ɹ�!", \
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
            tLog( ERROR, "�ܾ����ʵ�IP[%s]", pcClientIp );
			return ERROR;
        }
    }

    /* ��������IP���� */
    if ( g_stFilterCfg.iAllowNum == 0 )
    {
        tLog( INFO, "������ʵ�IP[%s]", pcClientIp );
		return (EXIT_SUCCESS);
    }

    pstFilterInfo = g_stFilterCfg.pstAllowIp;
    for ( i = 0; i < g_stFilterCfg.iAllowNum; i++, pstFilterInfo++ )
    {
        if ( !memcmp( pstFilterInfo->sIpAddr, pcClientIp, strlen(pstFilterInfo->sIpAddr ) ) )
        {
            tLog( INFO, "������ʵ�IP[%s]", pcClientIp );
			return (EXIT_SUCCESS);
        }
    }

    tLog( ERROR, "�ܾ����ʵ�IP[%s]", pcClientIp );
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
			tLog( INFO, "������ʵ�IP[%s]", pcInIp );
            break;
        }
	}
	
   	if ( i == g_stAsyncIoCfg.iAsyncNum )
	{
		tLog( ERROR, "�ܾ����ʵ�IP[%s]", pcInIp );
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
			tLog( INFO, "����ͨ�����ؼ���IP[%s:%s]����, ����IP[%s:%s]", \
					pcLocalIp, pcLocalPort, pcPeerIp, pcPeerPort );
			return (EXIT_SUCCESS);
            break;
        }
	}
	
	tLog( ERROR, "����ͨ�����ؼ���IP[%s:%s]���գ�����δ����.", pcLocalIp, pcLocalPort );
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
        tLog ( ERROR, "accept������[%d]ʧ��[%d:%s].", iListenSock, errno, strerror(errno) );
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
