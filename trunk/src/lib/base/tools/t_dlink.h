#ifndef DLINK_H
#define DLINK_H
/* 为dlink.c定义 */
/* Begin: added by zhongduying on 2013.01.10 for dlink */

#define B_TRUE          1
#define B_FALSE         0
#define CMP_E           0

/* 定义链表初始化长度和每次扩展长度     */
#define DLINK_INIT_SIZE     128
#define DLINK_EXT_SIZE      128

/*********************************************
** 通过定义cAddMode和cGetMode可让链表呈现   **
** 队列、链栈等数据结构的效果               **
**                                          **
** 队列: cAddMode = ADD_TO_TAIL             **
**       cGetMode = GET_FROM_HEAD           **
** 链栈: cAddMode = ADD_TO_HEAD             **
**       cGetMode = GET_FROM_HEAD           **
*********************************************/

/* 定义两种新加结点模式(DLINK.cAddMode) */
#define ADD_TO_HEAD         1
#define ADD_TO_TAIL         0

/* 定义两种删除结点模式(DLINK.cGetMode) */
#define GET_FROM_HEAD       0
#define GET_FROM_TAIL       1


/******    链表结构    ******/
typedef struct tagDNode
{
    struct tagDNode *pstPrev;
    struct tagDNode *pstNext;
    void            *pvData;
}DNode;

typedef struct tagDLINK
{  
    DNode   *pHead;
    DNode   *pTail;
    int    (*pvCmpFunc)(void*, void*);
    char     cAddMode;
    char     cGetMode;
    int      iNodeSize;
    int      iLen;
}DLINK;

/******    基本函数    ******/
int tDLinkCreate( DLINK *pstDLink, int iInitSize );
DNode *tDLinkSearch( DLINK *pstDLink, void *pvData );
void tDLinkInsert( DLINK *pstDLink, DNode *pstDNodeNew );
DNode *tDLinkGetNodeById( DLINK *pstDLink, int iIndex );
DNode *tDLinkGetNode( DLINK *pstDLink );
void tDLinkRmvNode( DLINK *pstDLink, DNode *pstDNode );
DNode *tDLinkPickNode( DLINK *pstDLink );
int tDLinkFree( DLINK *pstDLink );

/* 初始化长度iInitSize用尽时，可继续扩展内存 */
int tDLinkExtend( DLINK *pstDLink, int iExtSize );


/******    操作宏    ******/
/**  !使用操作宏前请预先判断指针是否为空! **/

/* 比较两个结点的数据是否相同 */
#define tDLinkDataCmp( pstDLink, pvData1, pvData2 ) \
    (( NULL == (pstDLink)->pvCmpFunc ) ? \
     (CMP_ERR) : \
     ((pstDLink)->pvCmpFunc((pvData1), (pvData2))))

/* 判断链表pstDLink是否为空 */
#define tDLinkIsEmpty( pstDLink ) \
    (( NULL == (pstDLink)->pHead ) || ( NULL == (pstDLink)->pTail ))

/* 遍历链表pstDLink */
#define tDLINK_SCAN_BEGIN( pstDLink, pstDNode ) \
    for ( pstDNode = (pstDLink)->pHead; pstDNode != NULL; pstDNode = (pstDNode)->pstNext ) \
    { \

#define tDLINK_SCAN_END() \
    } \

/* 循环遍历链表pstDLink: 从pstDNodeStart开始，遍历DLINK.iLen个结点 */
#define tDLINK_SCANLOOP_BEGIN( pstDLink, pstDNode, pstDNodeStart ) \
    { \
        int iLoop = 0; \
        for ( pstDNode = (pstDNodeStart); pstDNode != NULL; ) \
        { \
            if ( iLoop++ == (pstDLink)->iLen ) \
            { \
                break; \
            } \

#define tDLINK_SCANLOOP_END( pstDLink ) \
            if ( NULL == (pstDNode)->pstNext ) \
            { \
                pstDNode = (pstDLink)->pHead; \
            } \
            else \
            { \
                pstDNode = (pstDNode)->pstNext; \
            } \
        } \
    }

/* 遍历链表pstDLink(针对pstDNode可能发生的拆链、挂链进行保护) */
#define tDLINK_SAFE_SCAN_BEGIN( pstDLink, pstDNode ) \
    for ( pstDNode = (pstDLink)->pHead; pstDNode != NULL; ) \
    { \
        DNode *pstDNodeBak = (pstDNode)->pstNext; \

#define tDLINK_SAFE_SCAN_END() \
        pstDNode = pstDNodeBak; \
    } \

#define tDLINK_SAFE_GETBAK() \
    (pstDNodeBak)

#define tDLINK_SAFE_REFRESH_NEXT( pstDNodeNext ) \
    pstDNodeBak = (pstDNodeNext); \

#define tDLINK_SAFE_CONTINUE() \
    pstDNode = pstDNodeBak; \
    continue;

/* 反向遍历链表pstDLink */
#define tDLINK_SCANBACK_BEGIN( pstDLink, pstDNode ) \
    for ( pstDNode = (pstDLink)->pTail; pstDNode != NULL; pstDNode = (pstDNode)->pstPrev ) \
    { \

#define tDLINK_SCANBACK_END() \
    } \

/* 反向遍历链表pstDLink(针对pstDNode可能发生的拆链、挂链进行保护) */
#define tDLINK_SAFE_SCANBACK_BEGIN( pstDLink, pstDNode ) \
    for ( pstDNode = (pstDLink)->pTail; pstDNode != NULL; ) \
    { \
        DNode *pstDNodeBak = (pstDNode)->pstPrev; \

#define tDLINK_SAFE_SCANBACK_END() \
        pstDNode = pstDNodeBak; \
    } \

#define tDLINK_SAFEBACK_GETBAK() \
    (pstDNodeBak)

#define tDLINK_SAFEBACK_REFRESH_PREV( pstDNodePrev ) \
    pstDNodeBak = (pstDNodePrev); \

#define tDLINK_SAFEBACK_CONTINUE() \
    pstDNode = pstDNodeBak; \
    continue;

/* End: added by zhongduying on 2013.01.10 for dlink */

#endif
