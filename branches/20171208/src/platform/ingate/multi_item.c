/*************************************************
** 文 件 名: multi_item.c                       **
** 文件描述: 多路复用链表的操作                 **
**                                              **
** 作    者: 钟读营                             **
** 创建日期: 2012.12.01                         **
** 修改日期:                                    **
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
** 函数: InitMultiItem( )                                                   **
** 类型: void                                                               **
** 入参：                                                                   **
**       MultiItem *pstMultiItem    -- 多路复用链表项指针                   **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       多路复用链表项初始化                                               **
** 创建日期:                                                                **
**       2012.12.01                                                         **
** 修改日期:                                                                **
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
** 函数: RecoverMultiItem( )                                                **
** 类型: void                                                               **
** 入参：                                                                   **
**       MultiItem *pstMultiItem    -- 多路复用链表项指针                   **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       多路复用链表项字段恢复                                             **
** 创建日期:                                                                **
**       2012.12.01                                                         **
** 修改日期:                                                                **
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
	tLog( INFO, "多路复用链表项[%s][%s]恢复状态成功.", sClientInfo, sLocalInfo );
}

/*****************************************************************************
** 函数: CmpMultiItem( )                                                    **
** 类型: int                                                                **
** 入参：                                                                   **
**       void *pvData1  -- 待比较的数据                                     **
**       void *pvData2  -- 待比较的数据                                     **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       CMP_L   -- pvData1 < pvData2                                       **
**       CMP_E   -- pvData1 = pvData2                                       **
**       CMP_G   -- pvData1 > pvData2                                       **
**       CMP_ERR -- 有数据指针为空                                          **
** 功能：                                                                   **
**       多路复用链表比较函数，通过MultiItem.iFd比较大小                    **
** 创建日期:                                                                **
**       2012.12.01                                                         **
** 修改日期:                                                                **
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
** 函数: CreateMultiLink( )                                                 **
** 类型: int                                                                **
** 入参：                                                                   **
**       DLINK *pstMultiFree  -- 待创建的链表指针，创建完成后将存有链表     **
**                               详细数据                                   **
**       int iInitSize        -- 初始化链表结点个数                         **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       0  -- 成功                                                         **
**       -1 -- 内存申请失败                                                 **
** 功能：                                                                   **
**       多路复用链表的创建                                                 **
** 创建日期:                                                                **
**       2012.12.01                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
int CreateMultiLink( DLINK *pstMultiFree, int iInitSize )
{
    pstMultiFree->pvCmpFunc = CmpMultiItem;
	if ( !pstMultiFree->pvCmpFunc )
	{
		tLog( ERROR, "MultiItem 比较函数为NULL." );
		return -1;
	}
    pstMultiFree->iNodeSize = sizeof(MultiItem);
    pstMultiFree->cAddMode  = ADD_TO_TAIL;
    pstMultiFree->cGetMode  = GET_FROM_HEAD;

    return tDLinkCreate( pstMultiFree, iInitSize );
}

/*****************************************************************************
** 函数: AddMultiLink( )                                                    **
** 类型: DNode *                                                            **
** 入参：                                                                   **
**       int iFd              -- 管道描述字                                 **
**       char cType           -- 通讯类型                                   **
**       char cCommMode       -- 通讯模式（同步/异步）                      **
**       char cSndRcv         -- 报文链路类型（发送/接收/收发）             **
**       DLINK *pstMultiLink  -- 多路复用链表指针                           **
**       DLINK *pstMultiFree  -- 空闲多路复用链表指针                       **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       DNode *    -- 指向新增多路复用链表项的指针                         **
**       NULL       -- 没有可用的空闲链表项                                 **
** 功能：                                                                   **
**       从pstMultiFree中摘取一个空闲结点，挂到pstMultiLink上               **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
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
    /*先检查参数type及cli_info的合法性*/
    switch ( cType )
    {
        case TCP_LISTEN:
        case FIFO:
            break;
        case TCP_WR:
            if ( sClitInfo[0] != '\0' ) {
                /* 截掉可能超长的字串 */
                sClitInfo[CLIENTINFO_LEN] = '\0';
                break;
            }
            if ( sLocalInfo[0] != '\0' ) {
                /* 截掉可能超长的字串 */
                sLocalInfo[LOCALINFO_LEN] = '\0';
                break;
            }
            tLog( ERROR, "AddMultiLink()参数type(%d)时, client_info和local_info不应为NULL!", cType);
            return NULL;
        default:
            /* type错,不支持 */
            tLog( ERROR, "AddMultiLink()参数type(%d)不支持", cType );
            return NULL;
    }

    pstDNodeFree = GetFreeMultiItem( pstMultiFree );
    if ( NULL == pstDNodeFree )
    {
        sprintf( sMsg, "已达最大链接数(%d),拒绝来自%s的链接!", 
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
** 函数: GetFreeMultiItem( )                                                **
** 类型: DNode *                                                            **
** 入参：                                                                   **
**       DLINK *pstMultiFree  -- 空闲多路复用链表指针                       **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       DNode *  -- 获取空闲的多路复用链表项的指针                         **
**       NULL     -- 没有可用的空闲链表项                                   **
** 功能：                                                                   **
**       从pstMultiFree中摘取一个空闲结点，返回                             **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
DNode *GetFreeMultiItem( DLINK *pstMultiFree )
{
    return tDLinkPickNode( pstMultiFree );
}

/*****************************************************************************
** 函数: ClearMultiLink( )                                                  **
** 类型: void                                                               **
** 入参：                                                                   **
**       int iFd              -- 管道描述字                                 **
**       DLINK *pstMultiLink  -- 多路复用链表指针                           **
**       DLINK *pstMultiFree  -- 空闲多路复用链表指针                       **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       从pstMultiLink回收指定iFd的一个结点，挂到pstMultiFree上            **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
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
		tLog( INFO, "描述字[%d]的多路复用链表项删除成功.", iFd );
    }
	else
		tLog( INFO, "未成功删除Fd[%d]的多路复用链表项.", iFd );

}

/*****************************************************************************
** 函数: FreeMultiLink( )                                                   **
** 类型: void                                                               **
** 入参：                                                                   **
**       DLINK *pstMultiFree  -- 待释放的链表指针                           **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：多路复用链表的释放                                                 **
** 创建日期:                                                                **
**       2012.12.01                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
void FreeMultiLink( DLINK *pstMultiLink )
{
    tDLinkFree( pstMultiLink );
}

/*****************************************************************************
** 函数: MultiLinkChk( )                                                    **
** 类型: void                                                               **
** 入参：                                                                   **
**       DLINK *pstMultiLink  -- 多路复用链表指针                           **
**       char *pcTip          -- 附加信息串                                 **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       调试用。打印多路复用链表各链表项。                                 **
** 创建日期:                                                                **
**       2012.12.04                                                         **
** 修改日期:                                                                **
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

        /* 解析time_t */
        pstTime = localtime( &pstMultiItem->tTime );
        sprintf( sTime, "%04d%02d%02d_%02d%02d%02d", 
            pstTime->tm_year + 1900,
            pstTime->tm_mon + 1,
            pstTime->tm_mday,
            pstTime->tm_hour,
            pstTime->tm_min,
            pstTime->tm_sec );

        /* 记录链表项数据 */
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


