/*************************************************
** �� �� ��: multi_item.c                       **
** �ļ�����: ��·��������Ĳ���                 **
**                                              **
** ��    ��: �Ӷ�Ӫ                             **
** ��������: 2012.12.01                         **
** �޸�����:                                    **
*************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include "t_tools.h"
#include "t_dlink.h"
#include "ingate.h"
#include "multi_item.h"
#include "t_log.h"

/*****************************************************************************
** ����: InitMultiItem( )                                                   **
** ����: void                                                               **
** ��Σ�                                                                   **
**       MultiItem *pstMultiItem    -- ��·����������ָ��                   **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       ��·�����������ʼ��                                               **
** ��������:                                                                **
**       2012.12.01                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
void InitMultiItem( MultiItem *pstMultiItem )
{
    memset( pstMultiItem, 0, sizeof(MultiItem) );
    pstMultiItem->iFd = -1;
    pstMultiItem->cType = 0;
    pstMultiItem->cRdStatus = 0;
    pstMultiItem->tTime = 0L;
    pstMultiItem->iLen = 0;
    pstMultiItem->pcPtr = pstMultiItem->caData;
}

/*****************************************************************************
** ����: RecoverMultiItem( )                                                **
** ����: void                                                               **
** ��Σ�                                                                   **
**       MultiItem *pstMultiItem    -- ��·����������ָ��                   **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       ��·�����������ֶλָ�                                             **
** ��������:                                                                **
**       2012.12.01                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
void RecoverMultiItem( MultiItem *pstMultiItem )
{
    int     iFd;
    char    cType, cCommMode, cSndRcv;
    char    sClientInfo[CLIENTINFO_LEN+1];
    char    sLocalInfo[LOCALINFO_LEN+1];
    time_t  tTime;

    if ( NULL == pstMultiItem )
        return;

    iFd = pstMultiItem->iFd;
    cType = pstMultiItem->cType;
    tTime = pstMultiItem->tTime;
    cCommMode = pstMultiItem->cCommMode;
    cSndRcv = pstMultiItem->cSndRcv;
    strcpy( sClientInfo, pstMultiItem->sClientInfo );
    strcpy( sLocalInfo, pstMultiItem->sLocalInfo );
    memset( (char *)pstMultiItem, 0, sizeof(MultiItem) );

    pstMultiItem->cRdStatus = RD_LEN;
    pstMultiItem->iLen = PKT_BCDLEN;
    pstMultiItem->pcPtr = pstMultiItem->caData;
    pstMultiItem->iFd = iFd;
    pstMultiItem->cType = cType;
    pstMultiItem->tTime = tTime;
    pstMultiItem->cCommMode = cCommMode;
    pstMultiItem->cSndRcv = cSndRcv;
    strcpy( pstMultiItem->sClientInfo, sClientInfo );
    strcpy( pstMultiItem->sLocalInfo, sLocalInfo );
	tLog( INFO, "��·����������[%s][%s]�ָ�״̬�ɹ�.", sClientInfo, sLocalInfo );
}

/*****************************************************************************
** ����: CmpMultiItem( )                                                    **
** ����: int                                                                **
** ��Σ�                                                                   **
**       void *pvData1  -- ���Ƚϵ�����                                     **
**       void *pvData2  -- ���Ƚϵ�����                                     **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       CMP_L   -- pvData1 < pvData2                                       **
**       CMP_E   -- pvData1 = pvData2                                       **
**       CMP_G   -- pvData1 > pvData2                                       **
**       CMP_ERR -- ������ָ��Ϊ��                                          **
** ���ܣ�                                                                   **
**       ��·��������ȽϺ�����ͨ��MultiItem.iFd�Ƚϴ�С                    **
** ��������:                                                                **
**       2012.12.01                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
int CmpMultiItem( void *pvData1, void *pvData2 )
{
    MultiItem *pstMultiItem1, *pstMultiItem2;

    if (( NULL == pvData1 ) || ( NULL == pvData2 ))
        return CMP_ERR;

    pstMultiItem1 = (MultiItem *)pvData1;
    pstMultiItem2 = (MultiItem *)pvData2;

    if ( pstMultiItem1->iFd == pstMultiItem2->iFd )
    {
        return CMP_E;
    }
    else if ( pstMultiItem1->iFd > pstMultiItem2->iFd )
    {
        return CMP_G;
    }
    else
    {
        return CMP_L;
    }
}

/*****************************************************************************
** ����: CreateMultiLink( )                                                 **
** ����: int                                                                **
** ��Σ�                                                                   **
**       DLINK *pstMultiFree  -- ������������ָ�룬������ɺ󽫴�������     **
**                               ��ϸ����                                   **
**       int iInitSize        -- ��ʼ�����������                         **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       0  -- �ɹ�                                                         **
**       -1 -- �ڴ�����ʧ��                                                 **
** ���ܣ�                                                                   **
**       ��·��������Ĵ���                                                 **
** ��������:                                                                **
**       2012.12.01                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
int CreateMultiLink( DLINK *pstMultiFree, int iInitSize )
{
    pstMultiFree->pvCmpFunc = CmpMultiItem;
	if ( !pstMultiFree->pvCmpFunc )
	{
		tLog( ERROR, "MultiItem �ȽϺ���ΪNULL." );
		return -1;
	}
    pstMultiFree->iNodeSize = sizeof(MultiItem);
    pstMultiFree->cAddMode  = ADD_TO_TAIL;
    pstMultiFree->cGetMode  = GET_FROM_HEAD;

    return tDLinkCreate( pstMultiFree, iInitSize );
}

/*****************************************************************************
** ����: AddMultiLink( )                                                    **
** ����: DNode *                                                            **
** ��Σ�                                                                   **
**       int iFd              -- �ܵ�������                                 **
**       char cType           -- ͨѶ����                                   **
**       char cCommMode       -- ͨѶģʽ��ͬ��/�첽��                      **
**       char cSndRcv         -- ������·���ͣ�����/����/�շ���             **
**       DLINK *pstMultiLink  -- ��·��������ָ��                           **
**       DLINK *pstMultiFree  -- ���ж�·��������ָ��                       **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       DNode *    -- ָ��������·�����������ָ��                         **
**       NULL       -- û�п��õĿ���������                                 **
** ���ܣ�                                                                   **
**       ��pstMultiFree��ժȡһ�����н�㣬�ҵ�pstMultiLink��               **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
DNode *AddMultiLink( int iFd, char cType, char cCommMode, char cSndRcv, DLINK *pstMultiLink, DLINK *pstMultiFree )
{
    MultiItem  *pstMultiItem = NULL;
    DNode      *pstDNodeFree = NULL;
    char        sClientIp[30], sClientPort[10], sSndLocalIp[30], sSndLocalPort[10];
    char        sClitInfo[CLIENTINFO_LEN + 1], sLocalInfo[LOCALINFO_LEN + 1];
    char        sMsg[ERR_MSG_LEN + 1];
    time_t      tTime = 0;

	if ( TCP_WR == cType )
	{
		sprintf( sClientIp, "%s", get_client_ip( iFd ) );
		sprintf( sClientPort, "%d", get_client_port( iFd ) );
		sprintf( sSndLocalIp, "%s", get_server_ip( iFd ) );
		sprintf( sSndLocalPort, "%d", get_server_port( iFd ) );
		sprintf( sClitInfo, "%d:%s:%s", iFd, sClientIp, sClientPort );
		sprintf( sLocalInfo, "%s:%s", sSndLocalIp, sSndLocalPort );
	}
    /*�ȼ�����type��cli_info�ĺϷ���*/
    switch ( cType )
    {
        case TCP_LISTEN:
        case FIFO:
            break;
        case TCP_WR:
            if ( sClitInfo[0] != '\0' ) {
                /* �ص����ܳ������ִ� */
                sClitInfo[CLIENTINFO_LEN] = '\0';
                break;
            }
            if ( sLocalInfo[0] != '\0' ) {
                /* �ص����ܳ������ִ� */
                sLocalInfo[LOCALINFO_LEN] = '\0';
                break;
            }
            tLog( ERROR, "AddMultiLink()����type(%d)ʱ, client_info��local_info��ӦΪNULL!", cType);
            return NULL;
        default:
            /* type��,��֧�� */
            tLog( ERROR, "AddMultiLink()����type(%d)��֧��", cType );
            return NULL;
    }

    pstDNodeFree = GetFreeMultiItem( pstMultiFree );
    if ( NULL == pstDNodeFree )
    {
        sprintf( sMsg, "�Ѵ����������(%d),�ܾ�����%s������!", 
                 GetMaxMulti(), get_client_ip(iFd));
        tLog( ERROR, sMsg );
        return NULL;
    }
    pstMultiItem = (MultiItem *)pstDNodeFree->pvData;

    time( &tTime );
    pstMultiItem->iFd = iFd;
    pstMultiItem->cType = cType;
    pstMultiItem->pcPtr = pstMultiItem->caData;
    pstMultiItem->tTime = tTime;
    pstMultiItem->cCommMode = cCommMode;
    pstMultiItem->cSndRcv = cSndRcv;

    switch ( cType )
    {
        case TCP_LISTEN:
        case FIFO:
            break;
        case TCP_WR:
            pstMultiItem->cRdStatus = RD_LEN;
            pstMultiItem->iLen = PKT_BCDLEN;
            strcpy( pstMultiItem->sClientInfo, sClitInfo );
            strcpy( pstMultiItem->sLocalInfo, sLocalInfo );
            break;
    }
    tDLinkInsert( pstMultiLink, pstDNodeFree );

    return pstDNodeFree;
}

/*****************************************************************************
** ����: GetFreeMultiItem( )                                                **
** ����: DNode *                                                            **
** ��Σ�                                                                   **
**       DLINK *pstMultiFree  -- ���ж�·��������ָ��                       **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       DNode *  -- ��ȡ���еĶ�·�����������ָ��                         **
**       NULL     -- û�п��õĿ���������                                   **
** ���ܣ�                                                                   **
**       ��pstMultiFree��ժȡһ�����н�㣬����                             **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
DNode *GetFreeMultiItem( DLINK *pstMultiFree )
{
    return tDLinkPickNode( pstMultiFree );
}

/*****************************************************************************
** ����: ClearMultiLink( )                                                  **
** ����: void                                                               **
** ��Σ�                                                                   **
**       int iFd              -- �ܵ�������                                 **
**       DLINK *pstMultiLink  -- ��·��������ָ��                           **
**       DLINK *pstMultiFree  -- ���ж�·��������ָ��                       **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       ��pstMultiLink����ָ��iFd��һ����㣬�ҵ�pstMultiFree��            **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
void ClearMultiLink( int iFd, DLINK *pstMultiLink, DLINK *pstMultiFree )
{
    MultiItem   stMultiItem = {0};
    DNode      *pstDNode = NULL;

    stMultiItem.iFd = iFd;
    pstDNode = tDLinkSearch( pstMultiLink, &stMultiItem );
    if ( pstDNode )
    {
		close( iFd );	
        tDLinkRmvNode( pstMultiLink, pstDNode );
        if ( pstDNode->pvData )
        {
            InitMultiItem( (MultiItem *)pstDNode->pvData );
        }
        tDLinkInsert( pstMultiFree, pstDNode );
		tLog( INFO, "������[%d]�Ķ�·����������ɾ���ɹ�.", iFd );
    }
	else
		tLog( INFO, "δ�ɹ�ɾ��Fd[%d]�Ķ�·����������.", iFd );

}

/*****************************************************************************
** ����: FreeMultiLink( )                                                   **
** ����: void                                                               **
** ��Σ�                                                                   **
**       DLINK *pstMultiFree  -- ���ͷŵ�����ָ��                           **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ���·����������ͷ�                                                 **
** ��������:                                                                **
**       2012.12.01                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
void FreeMultiLink( DLINK *pstMultiLink )
{
    tDLinkFree( pstMultiLink );
}

/*****************************************************************************
** ����: MultiLinkChk( )                                                    **
** ����: void                                                               **
** ��Σ�                                                                   **
**       DLINK *pstMultiLink  -- ��·��������ָ��                           **
**       char *pcTip          -- ������Ϣ��                                 **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       �����á���ӡ��·��������������                                 **
** ��������:                                                                **
**       2012.12.04                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
void MultiLinkChk( DLINK *pstMultiLink, char *pcTip )
{
    MultiItem  *pstMultiItem    = NULL;
    DNode      *pstDNode        = NULL;
	struct tm  *pstTime         = NULL;
    char        sType[][20]     = {"tcp_listen"   ,"tcp_wr"       ,"fifo"         };
    char        sRdStatus[][20] = {"rd_stx"       ,"rd_len"       ,"rd_data"      ,"rd_fin"};
    char        sSndRcv[][20]   = {"snd_link"     ,"rcv_link"     ,"dub_link"     };
    char        sCommMode[][20] = {"comm_sync"    ,"comm_async"   };
    char        sTime[20]       = {0};
    int         i               = 0;

    tLog( DEBUG, "-----------------------------------------------------------------------");
    tLog( DEBUG, "MULTILINK CHECK FOR -- %s:", pcTip );
    tLog( DEBUG, "[No.]-[fd][type][RdStatus][SndRcv][CommMode][time][len][client][local]");
    tDLINK_SCAN_BEGIN( pstMultiLink, pstDNode )
    {
        ++i;
        pstMultiItem = (MultiItem *)pstDNode->pvData;

        /* ����time_t */
        pstTime = localtime( &pstMultiItem->tTime );
        sprintf( sTime, "%04d%02d%02d_%02d%02d%02d", 
            pstTime->tm_year + 1900,
            pstTime->tm_mon + 1,
            pstTime->tm_mday,
            pstTime->tm_hour,
            pstTime->tm_min,
            pstTime->tm_sec );

        /* ��¼���������� */
        tLog( DEBUG, " [%04d]-[%02d][%s][%s][%s][%s][%s][%d][%s][%s]",
            i,
            pstMultiItem->iFd,
            sType[(int)pstMultiItem->cType],
            sRdStatus[(int)pstMultiItem->cRdStatus],
            sSndRcv[(int)pstMultiItem->cSndRcv],
            sCommMode[(int)pstMultiItem->cCommMode],
            sTime,
            pstMultiItem->iLen,
            pstMultiItem->sClientInfo,
            pstMultiItem->sLocalInfo );
    }
    tDLINK_SCAN_END()
    tLog( DEBUG, "CHECK FOR %s ENDED.", pcTip );
    tLog( DEBUG, "-----------------------------------------------------------------------");
}


