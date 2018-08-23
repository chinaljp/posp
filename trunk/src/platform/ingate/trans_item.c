/*************************************************
** 文 件 名: trans_item.c                       **
** 文件描述: 交易链表的操作                     **
**                                              **
** 作    者: 钟读营                             **
** 创建日期: 2012.12.01                         **
** 修改日期:                                    **
*************************************************/

#include <stdio.h>
#include <string.h>
#include "ingate.h"
#include "trans_item.h"
#include "t_log.h"
#include "t_dlink.h"

int g_int = 0;

/*****************************************************************************
** 函数: InitTransItem( )                                                   **
** 类型: void                                                               **
** 入参：                                                                   **
**       TransItem *pstTransItem    -- 交易链表项指针                       **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       交易链表项初始化                                                   **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
void InitTransItem( TransItem *pstTransItem )
{
    memset( pstTransItem, 0, sizeof(TransItem) );
    pstTransItem->iFd = -1;
    pstTransItem->tTime = 0L;
    pstTransItem->iLen = 0;
    pstTransItem->pcPtr = pstTransItem->caData;
}

/*****************************************************************************
** 函数: RecoverTransItem( )                                                **
** 类型: void                                                               **
** 入参：                                                                   **
**       TransItem *pstTransItem    -- 交易链表项指针                       **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       交易链表项字段恢复                                                 **
** 创建日期:                                                                **
**       2012.12.01                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
void RecoverTransItem( TransItem *pstTransItem )
{
    int     iFd;
    char    sUniqueKey[UNIQUEKEY_SIZE];
    char    sOriginInfo[ORIGININFO_LEN+1];
    char    caSvcMsgInfo[SVCMSGINFO_SIZE];
    time_t  tTime;

    if ( NULL == pstTransItem )
        return;

    iFd = pstTransItem->iFd;
    tTime = pstTransItem->tTime;
    strcpy( sUniqueKey, pstTransItem->sUniqueKey );
    memcpy( sOriginInfo, pstTransItem->sOriginInfo, (ORIGININFO_LEN+1) );
    strcpy( caSvcMsgInfo, pstTransItem->caSvcMsgInfo );
    memset( (char *)pstTransItem, 0, sizeof(TransItem) );

    pstTransItem->iLen = PKT_BCDLEN;
    pstTransItem->pcPtr = pstTransItem->caData;
    pstTransItem->iFd = iFd;
    pstTransItem->tTime = tTime;
    strcpy( pstTransItem->sUniqueKey, sUniqueKey );
    memcpy( pstTransItem->sOriginInfo, sOriginInfo, (ORIGININFO_LEN+1) );
    strcpy( pstTransItem->caSvcMsgInfo, caSvcMsgInfo );
}

/*****************************************************************************
** 函数: CmpTransItemByFd( )                                                **
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
**       交易链表比较函数，通过TransItem.iFd比较大小                        **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
int CmpTransItemByFd( void *pvData1, void *pvData2 )
{
    TransItem *pstTransItem1, *pstTransItem2;

    if (( NULL == pvData1 ) || ( NULL == pvData2 ))
        return CMP_ERR;

    pstTransItem1 = (TransItem *)pvData1;
    pstTransItem2 = (TransItem *)pvData2;

    if ( pstTransItem1->iFd == pstTransItem2->iFd )
    {
        return CMP_E;
    }
    else if ( pstTransItem1->iFd > pstTransItem2->iFd )
    {
        return CMP_G;
    }
    else
    {
        return CMP_L;
    }
}

/*****************************************************************************
** 函数: CmpTransItemByUniqueKey( )                                         **
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
**       交易链表比较函数，通过TransItem.sUniqueKey比较大小                 **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
int CmpTransItemByUniqueKey( void *pvData1, void *pvData2 )
{
    TransItem *pstTransItem1, *pstTransItem2;

    if ( ( NULL == pvData1 ) || ( NULL == pvData2 ))
        return CMP_ERR;

    pstTransItem1 = (TransItem *)pvData1;
    pstTransItem2 = (TransItem *)pvData2;

    return strcmp( pstTransItem1->sUniqueKey, pstTransItem2->sUniqueKey );
}

/*****************************************************************************
** 函数: CreateTransLink( )                                                 **
** 类型: int                                                                **
** 入参：                                                                   **
**       DLINK *pstTransFree  -- 待创建的链表指针，创建完成后将存有         **
**                               链表详细数据                               **
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
int CreateTransLink( DLINK *pstTransFree, int iInitSize )
{
    pstTransFree->pvCmpFunc = CmpTransItemByUniqueKey;
    pstTransFree->iNodeSize = sizeof(TransItem);
    pstTransFree->cAddMode  = ADD_TO_TAIL;
    pstTransFree->cGetMode  = GET_FROM_HEAD;

    return tDLinkCreate( pstTransFree, iInitSize );
}

/*****************************************************************************
** 函数: AddTransLink( )                                                    **
** 类型: DNode *                                                            **
** 入参：                                                                   **
**       TransItem *pstTransItem -- 交易链表项数据指针                      **
**       DLINK *pstTransLink     -- 交易链表指针                            **
**       DLINK *pstTransFree     -- 空闲交易链表指针                        **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       DNode *    -- 指向新增多路复用链表项的指针                         **
**       NULL       -- 没有可用的空闲链表项                                 **
** 功能：                                                                   **
**       从pstTransFree中摘取一个空闲结点，挂到pstTransLink上               **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
DNode *AddTransLink( TransItem *pstTransData, DLINK *pstTransLink, DLINK *pstTransFree, int iTimeOut )
{
    TransItem  *pstTransItem = NULL;
    DNode      *pstDNodeFree = NULL;
    time_t      tTime;

    /* 交易链表中以sUniqueKey为唯一键值不能重复，且最长保留CommItem项time_out秒 */
    time( &tTime );
    if ( tDLinkSearch( pstTransLink, (void *)pstTransData ))
    {
        tLog ( FATAL, "交易链表中唯一键值[%s]重复", pstTransData->sUniqueKey );
        return NULL;
    }

    /* 获取一个空闲的交易链表项 */
    pstDNodeFree = GetFreeTransItem( pstTransFree );
    if ( NULL == pstDNodeFree )
    {
        tLog ( ERROR, "已达最大交易数(%d),键值为[%s]的交易不作处理!",
                  GetMaxTrans(), pstTransData->sUniqueKey );
        TransLinkChk( pstTransLink, "Full TransLink" );
        return NULL;
    }
    pstTransItem = (TransItem *)pstDNodeFree->pvData;

    /* 复制入参交易链表数据到空闲交易链表项，并插入交易链表 */
    memcpy( pstTransItem, pstTransData, sizeof(TransItem) );
    pstTransItem->tTime = tTime;
    pstTransItem->pcPtr = pstTransItem->caData + pstTransItem->iLen;
    tDLinkInsert( pstTransLink, pstDNodeFree );

    return pstDNodeFree;
}

DNode *GetTransLink( char *pcUKey, DLINK *pstTransLink, DLINK *pstTransFree, int iTimeOut )
{
    DNode *pstNode = NULL;
    TransItem  *pstTransItem = NULL;
    time_t      tTime;
    int         iFd;

    for ( pstNode = pstTransLink->pHead; pstNode != NULL; pstNode = pstNode->pstNext )
    {   
        pstTransItem = (TransItem  *)pstNode->pvData;
        if ( !strcmp( pstTransItem->sUniqueKey, pcUKey ) )
            break;
    }

    if ( NULL == pstNode )
    {
        tLog ( ERROR, "未找到交易链表中唯一键值[%s]的链表项.", pcUKey );
        return NULL;
    }

    /*  响应包超时,清除此终端的交易链表项   */
    time( &tTime );
    if ( (tTime - pstTransItem->tTime) > iTimeOut ) 
    {
        iFd = pstTransItem->iFd;
        tLog(ERROR,"TestDebug");
       /* 
        tErrLog ( ERR, "键值[%s]响应包超时,放弃回送[%d][%s:%d]",
                pcUKey, iFd, get_client_ip( iFd ), get_client_port( iFd ) ); 
                */
                
       /**/
        tLog ( ERROR, "键值[%s]响应包超时,放弃回送[%d]",
                pcUKey, iFd ); 
                
        ClearTransLink( pcUKey, pstTransLink, pstTransFree );

        iFd = g_int;
         /*
        tErrLog ( ERR, "键值[%s]响应包超时,放弃回送[%d][%s:%d]",
                pcUKey, iFd, get_client_ip( iFd ), get_client_port( iFd ) );
                */
       /*      */   
         tLog ( ERROR, "键值[%s]响应包超时,放弃回送[%d]",
                pcUKey, iFd ); 
                
        return ( NULL );
    }

    return pstNode;
}

/*****************************************************************************
** 函数: GetFreeTransItem( )                                                **
** 类型: DNode *                                                            **
** 入参：                                                                   **
**       DLINK *pstTransFree  -- 空闲交易链表指针                           **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       DNode *      -- 获取空闲的交易链表项的指针                         **
**       NULL         -- 没有可用的空闲链表项                               **
** 功能：                                                                   **
**       从pstTransFree中摘取一个空闲结点，返回                             **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
DNode *GetFreeTransItem( DLINK *pstTransFree )
{
    return tDLinkPickNode( pstTransFree );
}

/*****************************************************************************
** 函数: ClearTransLink( )                                                  **
** 类型: void                                                               **
** 入参：                                                                   **
**       char *pcUniqueKey    -- 模块自定义的交易链表项关键字               **
**       DLINK *pstTransLink  -- 交易链表指针                               **
**       DLINK *pstTransFree  -- 空闲交易链表指针                           **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       从pstTransLink回收指定pcUniqueKey的一个结点，挂到pstTransFree上    **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
void ClearTransLink( char *pcUniqueKey, DLINK *pstTransLink, DLINK *pstTransFree )
{
    TransItem   stTransItem = {0};
    DNode      *pstDNode = NULL;

    memcpy( stTransItem.sUniqueKey, pcUniqueKey, UNIQUEKEY_SIZE );
    pstDNode = tDLinkSearch( pstTransLink, &stTransItem );
    if ( pstDNode )
    {
        tDLinkRmvNode( pstTransLink, pstDNode );
        if ( pstDNode->pvData )
        {
            InitTransItem( (TransItem *)pstDNode->pvData );
        }
        tDLinkInsert( pstTransFree, pstDNode );
		tLog( INFO, "键值为[%s]的交易链表项删除成功.", stTransItem.sUniqueKey );
    }
	else 
		tLog( INFO, "未成功删除键值[%s]的交易链表项.", stTransItem.sUniqueKey );
}

/*****************************************************************************
** 函数: DeleteTransLink( )                                                 **
** 类型: void                                                               **
** 入参：                                                                   **
**       int iFd 管道描述字                                                 **
**       DLINK *pstTransLink  -- 交易链表指针                               **
**       DLINK *pstTransFree  -- 空闲交易链表指针                           **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       从pstTransLink上回收所有TransItem.iFd与入参iFd相匹配的结点，       **
**       挂到pstTransFree上                                                 **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
void DeleteTransLink( int iFd, DLINK *pstTransLink, DLINK *pstTransFree )
{
    TransItem   stTransItem = {0};
    DNode      *pstDNode    = NULL;
    int       (*pvCmpFuncOrigin)(void*, void*);

    stTransItem.iFd = iFd;
    pvCmpFuncOrigin = pstTransLink->pvCmpFunc;
    pstTransLink->pvCmpFunc = CmpTransItemByFd;

    /* 摘链挂链操作会改变pstDNode的pstPrev和pstNext指针，影响遍历，因此此处使用安全遍历宏 */
    tDLINK_SAFE_SCAN_BEGIN( pstTransLink, pstDNode )
    {
        if ( CMP_E == tDLinkDataCmp( pstTransLink, (void *)pstDNode->pvData, (void *)&stTransItem ) )
        {
            tDLinkRmvNode( pstTransLink, pstDNode );
            if ( pstDNode->pvData )
            {
                InitTransItem( (TransItem *)pstDNode->pvData );
            }
            tDLinkInsert( pstTransFree, pstDNode );
        }
    }
    tDLINK_SAFE_SCAN_END()
    pstTransLink->pvCmpFunc = pvCmpFuncOrigin;
}

/*****************************************************************************
** 函数: FreeTransLink( )                                                   **
** 类型: void                                                               **
** 入参：                                                                   **
**       DLINK *pstTransFree  -- 待释放的链表指针                           **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       交易链表的释放                                                     **
** 创建日期:                                                                **
**       2012.12.03                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
void FreeTransLink( DLINK *pstTransLink )
{
    tDLinkFree( pstTransLink );
}

/*****************************************************************************
** 函数: TransLinkChk( )                                                    **
** 类型: void                                                               **
** 入参：                                                                   **
**       DLINK *pstTransLink  -- 交易链表指针                               **
**       char *pcTip          -- 附加信息串                                 **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       void                                                               **
** 功能：                                                                   **
**       调试用。打印交易链表各链表项。                                     **
** 创建日期:                                                                **
**       2012.12.04                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
void TransLinkChk( DLINK *pstTransLink, char *pcTip )
{
    TransItem  *pstTransItem    = NULL;
    DNode      *pstDNode        = NULL;
	struct tm  *pstTime         = NULL;
    char        sTime[20]       = {0};
    int         i               = 0;

    tLog( DEBUG, "----------------------------------------------");
    tLog( DEBUG, "TRANSLINK CHECK FOR -- %s:", pcTip );
    tLog( DEBUG, "[No.]-[fd][time][len][unique_key][OriginInfo]");
    tDLINK_SCAN_BEGIN( pstTransLink, pstDNode )
    {
        ++i;
        pstTransItem = (TransItem *)pstDNode->pvData;

        /* 解析time_t */
        pstTime = localtime( &pstTransItem->tTime );
        sprintf( sTime, "%04d%02d%02d_%02d%02d%02d", 
            pstTime->tm_year + 1900,
            pstTime->tm_mon + 1,
            pstTime->tm_mday,
            pstTime->tm_hour,
            pstTime->tm_min,
            pstTime->tm_sec );

        /* 记录链表项数据 */
        tLog( DEBUG, " [%04d]-[%02d][%s][%d][%s][%s]",
            i,
            pstTransItem->iFd,
            sTime,
            pstTransItem->iLen,
            pstTransItem->sUniqueKey,
            pstTransItem->sOriginInfo );
    }
    tDLINK_SCAN_END()
    tLog( DEBUG, "CHECK FOR %s ENDED.", pcTip );
    tLog( DEBUG, "----------------------------------------------");
}


