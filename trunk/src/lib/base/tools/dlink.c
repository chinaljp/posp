/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*************************************************
** 文 件 名: dlink.c                            **
** 文件描述: 基本数据结构之双向链表             **
**                                              **
** 作    者: 钟读营                             **
** 创建日期: 2012.11.29                         **
** 修改日期:                                    **
*************************************************/
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_dlink.h"

/*****************************************************************************
** 函数: tDLinkCreate( )                                                    **
** 类型: int                                                                **
** 入参：                                                                   **
**       DLINK *pstDLink -- 待创建的链表指针，创建完成后将存有链表详细数据  **
**       int iInitSize   -- 初始化链表结点个数                              **
**       int iNodeSize   -- 每个链表结点申请的动态内存大小                  **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       0  -- 成功                                                         **
**       -1 -- 内存申请失败                                                 **
** 功能：                                                                   **
**       双向链表的创建                                                     **
** 创建日期:                                                                **
**       2012.11.29                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
int tDLinkCreate( DLINK *pstDLink, int iInitSize )
{
    DNode  *pstDNodeNew = NULL;
    //DNode  *pstNode     = NULL;
    BOOL_T  bTailData   = B_FALSE;
    int     i = 0;

    pstDLink->iLen = 0;
    pstDLink->pHead= NULL;
    pstDLink->pTail = NULL;

    for ( i = 0; i < iInitSize; i++ )
    {
        /* 申请链表结点内存 */
        pstDNodeNew = (DNode *)malloc( sizeof(DNode) );
        if ( NULL == pstDNodeNew )
        {
            /* 内存申请失败，退出前释放前面申请成功的结点空间 */
            break;
        }
        memset( pstDNodeNew, 0, sizeof(DNode) );

        /* 申请链表项保存数据的内存 */
        bTailData = B_FALSE;
        pstDNodeNew->pvData = malloc( pstDLink->iNodeSize );
        if ( NULL == pstDNodeNew->pvData )
        {
            /* 内存申请失败，根据记录的bTailData释放结点内存 */
            break;
        }
        memset( pstDNodeNew->pvData, 0, pstDLink->iNodeSize );
        bTailData = B_TRUE;

        /* 链表插入结点 */
        tDLinkInsert( pstDLink, pstDNodeNew );
    }

    /* 出现内存申请失败时，应释放前面申请成功的内存，然后退出程序 */
    if ( i < iInitSize )
    {
        if ( B_TRUE == bTailData )
        {
            free( pstDNodeNew );
        }
        tDLinkFree( pstDLink );
        return (EXIT_FAILURE);
    }
    pstDLink->iLen = iInitSize;

    return (EXIT_SUCCESS);
}

/*****************************************************************************
** 函数: tDLinkExtend( )                                                    **
** 类型: int                                                                **
** 入参：                                                                   **
**       DLINK *pstDLink -- 待扩展的链表指针                                **
**       int iExtSize    -- 链表扩展结点个数                                **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       0  -- 成功                                                         **
**       -1 -- 内存申请失败                                                 **
** 功能：                                                                   **
**       双向链表的扩展。当初始化大小不够用时，再新申请并添加iExtSize个结点 **
** 创建日期:                                                                **
**       2012.11.29                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
int tDLinkExtend( DLINK *pstDLink, int iExtSize )
{
    DNode  *pstDNodeNew = NULL;
    //DNode  *pstNode     = NULL;
    BOOL_T  bTailData   = B_FALSE;
    int     i;

    for ( i = 0; i < iExtSize; i++ )
    {
        /* 申请链表结点内存 */
        pstDNodeNew = (DNode *)malloc( sizeof(DNode) );
        if ( NULL == pstDNodeNew )
        {
            /* 内存申请失败，退出前释放前面申请成功的结点空间 */
            break;
        }
        memset( pstDNodeNew, 0, sizeof(DNode) );

        /* 申请链表项保存数据的内存 */
        bTailData = B_FALSE;
        pstDNodeNew->pvData = malloc( pstDLink->iNodeSize );
        if ( NULL == pstDNodeNew->pvData )
        {
            /* 内存申请失败，根据记录的bTailData释放结点内存 */
            break;
        }
        memset( pstDNodeNew->pvData, 0, pstDLink->iNodeSize );
        bTailData = B_TRUE;

        /* 链表插入结点 */
        tDLinkInsert( pstDLink, pstDNodeNew );
    }

    /* 出现内存申请失败时，应释放前面申请成功的内存，然后退出程序 */
    if ( i < iExtSize )
    {
        if ( B_TRUE == bTailData )
        {
            free( pstDNodeNew );
        }
        tDLinkFree( pstDLink );
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

/*****************************************************************************
** 函数: tDLinkSearch( )                                                    **
** 类型: DNode *                                                            **
** 入参：                                                                   **
**       DLINK *pstDLink -- 双向链表指针                                    **
**       void *pvData    -- 要查找的数据                                    **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       DNode *  -- 数据相匹配的链表结点                                   **
**       NULL     -- 没有找到匹配结点                                       **
** 功能：                                                                   **
**       双向链表的数据检索，pvData与DNode.pvData进行匹配                   **
** 创建日期:                                                                **
**       2012.11.30                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
DNode *tDLinkSearch( DLINK *pstDLink, void *pvData )
{
    DNode *pstDNode = NULL;

    if ( !pstDLink->pvCmpFunc )
    {
        return NULL;
    }

    tDLINK_SCAN_BEGIN( pstDLink, pstDNode )
    {
        if ( CMP_E == pstDLink->pvCmpFunc( pstDNode->pvData, pvData ) )
        {
            return pstDNode;
        }
    }
    tDLINK_SCAN_END()

    return NULL;
}

/*****************************************************************************
** 函数: tDLinkGetNodeById( )                                               **
** 类型: int                                                                **
** 入参：                                                                   **
**       DLINK *pstDLink -- 双向链表指针                                    **
**       int iIndex      -- 要获取的链表结点序号，计数从0开始               **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       DNode *  -- 数据相匹配的链表结点                                   **
**       NULL     -- 给定的序号非法                                         **
** 功能：                                                                   **
**       获取双向链表第iIndex个链表结点                                     **
** 创建日期:                                                                **
**       2012.11.30                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
DNode *tDLinkGetNodeById( DLINK *pstDLink, int iIndex )
{
    DNode *pstDNode = NULL;

    if (( iIndex < 0 ) || ( iIndex >= pstDLink->iLen ))
    {
        return  NULL;
    }

    tDLINK_SCAN_BEGIN( pstDLink, pstDNode )
    {
        if ( 0 == iIndex-- )
        {
            return pstDNode;
        }
    }
    tDLINK_SCAN_END()

    /* 若此处返回，说明链表结构出现错误! */
    return NULL;
}

/* Begin: removed here by zhongduying on 2013.01.10 for code encapsulation */
/* !原先作为宏定义在.h中，现将不对外公开的宏挪到.c中! */

/* 在链表pstDLink的结点pstDNode前，插入结点pstDNodeNew */
#define tDLinkInsBefore( pstDLink, pstDNode, pstDNodeNew ) \
    { \
        if ( pstDNode == (pstDLink)->pHead ) \
        { \
            (pstDNodeNew)->pstPrev = NULL; \
            (pstDNodeNew)->pstNext = pstDNode; \
            (pstDNode)->pstPrev = pstDNodeNew; \
            (pstDLink)->pHead = pstDNodeNew; \
        } \
        else \
        { \
            (pstDNodeNew)->pstPrev = (pstDNode)->pstPrev; \
            (pstDNodeNew)->pstNext = pstDNode; \
            (pstDNode)->pstPrev->pstNext = pstDNodeNew; \
            (pstDNode)->pstPrev = pstDNodeNew; \
        } \
        ++(pstDLink)->iLen; \
    }

/* 在链表pstDLink的结点pstDNode后，插入结点pstDNodeNew */
#define tDLinkInsAfter( pstDLink, pstDNode, pstDNodeNew ) \
    { \
        if ( pstDNode == (pstDLink)->pTail ) \
        { \
            (pstDNodeNew)->pstNext = NULL; \
            (pstDNodeNew)->pstPrev = pstDNode; \
            (pstDNode)->pstNext = pstDNodeNew; \
            (pstDLink)->pTail = pstDNodeNew; \
        } \
        else \
        { \
            (pstDNodeNew)->pstNext = (pstDNode)->pstNext; \
            (pstDNodeNew)->pstPrev = pstDNode; \
            (pstDNode)->pstNext->pstPrev = pstDNodeNew; \
            (pstDNode)->pstNext = pstDNodeNew; \
        } \
        ++(pstDLink)->iLen; \
    }

/* ( 公开接口 )将结点pstDNode插入链表pstDLink，插入方式由DLINK.cAddMode定义 */
void tDLinkInsert( DLINK *pstDLink, DNode *pstDNodeNew )
{
    if ( tDLinkIsEmpty( pstDLink ))
    {
        (pstDLink)->pHead = (pstDLink)->pTail = (pstDNodeNew);
        (pstDLink)->pHead->pstPrev = NULL;
        (pstDLink)->pTail->pstNext = NULL;
        ++(pstDLink)->iLen;
    }
    else
    {
        if ( ADD_TO_HEAD == (pstDLink)->cAddMode )
        {
            tDLinkInsBefore( pstDLink, (pstDLink)->pHead, pstDNodeNew );
        }
        else
        {
            tDLinkInsAfter( pstDLink, (pstDLink)->pTail, pstDNodeNew );
        }
    }
}

/* 从链表中取得一个结点，取得方式由DLINK.cGetMode定义 */
#define tDLinkGet( pstDLink ) \
        (( GET_FROM_HEAD == (pstDLink)->cGetMode ) ? \
          tDLinkGetHead( pstDLink ): \
          tDLinkGetTail( pstDLink ))

#define tDLinkGetHead( pstDLink )       ( (pstDLink)->pHead )

#define tDLinkGetTail( pstDLink )       ( (pstDLink)->pTail )

/* ( 公开接口 )从链表中取得一个结点，取得方式由DLINK.cGetMode定义 */
DNode *tDLinkGetNode( DLINK *pstDLink )
{
    return  (( !(pstDLink) || tDLinkIsEmpty( pstDLink ) ) ?
              NULL :
              tDLinkGet( pstDLink ));
}

/* 从链表pstDLink摘除头结点pHead */
#define tDLinkRmvHead( pstDLink ) \
    { \
        if ( (pstDLink)->pHead == (pstDLink)->pTail ) \
        { \
            (pstDLink)->pHead = NULL; \
            (pstDLink)->pTail = NULL; \
            (pstDLink)->iLen  = 0; \
        } \
        else \
        { \
            (pstDLink)->pHead = (pstDLink)->pHead->pstNext; \
            (pstDLink)->pHead->pstPrev = NULL; \
            --(pstDLink)->iLen; \
        } \
    }

/* 从链表pstDLink摘除尾结点pTail */
#define tDLinkRmvTail( pstDLink ) \
    { \
        if ( (pstDLink)->pHead == (pstDLink)->pTail ) \
        { \
            (pstDLink)->pHead = NULL; \
            (pstDLink)->pTail = NULL; \
            (pstDLink)->iLen  = 0; \
        } \
        else \
        { \
            (pstDLink)->pTail = (pstDLink)->pTail->pstPrev; \
            (pstDLink)->pTail->pstNext = NULL; \
            --(pstDLink)->iLen; \
        } \
    }

/* 从链表pstDLink摘除一个结点，摘除位置由DLINK.cGetMode定义 */
#define tDLinkRemove( pstDLink ) \
    { \
        if ( GET_FROM_HEAD == (pstDLink)->cGetMode ) \
        { \
            tDLinkRmvHead( pstDLink ); \
        } \
        else \
        { \
            tDLinkRmvTail( pstDLink ); \
        } \
    }

/* ( 公开接口 )从链表pstDLink摘除指定结点pstDNode */
void tDLinkRmvNode( DLINK *pstDLink, DNode *pstDNode )
{
    if ( pstDNode == (pstDLink)->pHead )
    {
        tDLinkRmvHead( pstDLink );
    }
    else if ( pstDNode == (pstDLink)->pTail )
    {
        tDLinkRmvTail( pstDLink );
    }
    else
    {
        (pstDNode)->pstPrev->pstNext = (pstDNode)->pstNext;
        (pstDNode)->pstNext->pstPrev = (pstDNode)->pstPrev;
        --(pstDLink)->iLen;
    }
}

/*****************************************************************************
** 函数: tDLinkPickNode( )                                                  **
** 类型: DNode *                                                            **
** 入参：                                                                   **
**       DLINK *pstDLink -- 双向链表指针                                    **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       DNode *  -- 获取到的链表结点                                       **
**       NULL     -- 无可用结点(链表为空)                                   **
** 功能：                                                                   **
**       从双向链表获取一个链表结点，返回，并从链上摘除                     **
** 创建日期:                                                                **
**       2012.11.30                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
DNode *tDLinkPickNode( DLINK *pstDLink )
{
    DNode *pstDNode = NULL;

    if ( !pstDLink || tDLinkIsEmpty( pstDLink ) )
    {
        return NULL;
    }
    pstDNode = tDLinkGet( pstDLink );
    if ( !pstDNode )
    {
        return NULL;
    }
    tDLinkRemove( pstDLink );

    return pstDNode;
}

/* 释放结点pstDNode的结点内存和数据内存 */
#define tDLinkFreeNode( pstDNode ) \
    if ( (pstDNode)->pvData ) \
    { \
        free( (pstDNode)->pvData ); \
    } \
    free( pstDNode ); \
    pstDNode = NULL; 

/* 从链表pstDLink删除指定结点pstDNode */
#define tDLinkDelNode( pstDLink, pstDNode ) \
    { \
        DNode *pstDNodeOld = pstDNode; \
        tDLinkRmvNode( pstDLink, pstDNode ); \
        tDLinkFreeNode( pstDNodeOld ); \
    }

/*****************************************************************************
** 函数: tDLinkFree( )                                                      **
** 类型: int                                                                **
** 入参：                                                                   **
**       DLINK *pstDLink -- 待释放的链表指针                                **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       0  -- 成功                                                         **
**       -1 -- 失败                                                         **
** 功能：                                                                   **
**       双向链表的释放，拆除链表结构，并释放链表占用的所有内存             **
** 创建日期:                                                                **
**       2012.11.29                                                         **
** 修改日期:                                                                **
**                                                                          **
*****************************************************************************/
int tDLinkFree( DLINK *pstDLink )
{
    DNode *pstDNode = NULL;

    while ( (pstDNode = tDLinkGet(pstDLink)) )
    {
        tDLinkDelNode( pstDLink, pstDNode );
    }
    return (EXIT_SUCCESS);
}

/* End: removed here by zhongduying on 2013.01.10 for code encapsulation */

