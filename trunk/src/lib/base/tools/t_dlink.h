#ifndef DLINK_H
#define DLINK_H
/* Ϊdlink.c���� */
/* Begin: added by zhongduying on 2013.01.10 for dlink */

#define B_TRUE          1
#define B_FALSE         0
#define CMP_E           0

/* ���������ʼ�����Ⱥ�ÿ����չ����     */
#define DLINK_INIT_SIZE     128
#define DLINK_EXT_SIZE      128

/*********************************************
** ͨ������cAddMode��cGetMode�����������   **
** ���С���ջ�����ݽṹ��Ч��               **
**                                          **
** ����: cAddMode = ADD_TO_TAIL             **
**       cGetMode = GET_FROM_HEAD           **
** ��ջ: cAddMode = ADD_TO_HEAD             **
**       cGetMode = GET_FROM_HEAD           **
*********************************************/

/* ���������¼ӽ��ģʽ(DLINK.cAddMode) */
#define ADD_TO_HEAD         1
#define ADD_TO_TAIL         0

/* ��������ɾ�����ģʽ(DLINK.cGetMode) */
#define GET_FROM_HEAD       0
#define GET_FROM_TAIL       1


/******    ����ṹ    ******/
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

/******    ��������    ******/
int tDLinkCreate( DLINK *pstDLink, int iInitSize );
DNode *tDLinkSearch( DLINK *pstDLink, void *pvData );
void tDLinkInsert( DLINK *pstDLink, DNode *pstDNodeNew );
DNode *tDLinkGetNodeById( DLINK *pstDLink, int iIndex );
DNode *tDLinkGetNode( DLINK *pstDLink );
void tDLinkRmvNode( DLINK *pstDLink, DNode *pstDNode );
DNode *tDLinkPickNode( DLINK *pstDLink );
int tDLinkFree( DLINK *pstDLink );

/* ��ʼ������iInitSize�þ�ʱ���ɼ�����չ�ڴ� */
int tDLinkExtend( DLINK *pstDLink, int iExtSize );


/******    ������    ******/
/**  !ʹ�ò�����ǰ��Ԥ���ж�ָ���Ƿ�Ϊ��! **/

/* �Ƚ��������������Ƿ���ͬ */
#define tDLinkDataCmp( pstDLink, pvData1, pvData2 ) \
    (( NULL == (pstDLink)->pvCmpFunc ) ? \
     (CMP_ERR) : \
     ((pstDLink)->pvCmpFunc((pvData1), (pvData2))))

/* �ж�����pstDLink�Ƿ�Ϊ�� */
#define tDLinkIsEmpty( pstDLink ) \
    (( NULL == (pstDLink)->pHead ) || ( NULL == (pstDLink)->pTail ))

/* ��������pstDLink */
#define tDLINK_SCAN_BEGIN( pstDLink, pstDNode ) \
    for ( pstDNode = (pstDLink)->pHead; pstDNode != NULL; pstDNode = (pstDNode)->pstNext ) \
    { \

#define tDLINK_SCAN_END() \
    } \

/* ѭ����������pstDLink: ��pstDNodeStart��ʼ������DLINK.iLen����� */
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

/* ��������pstDLink(���pstDNode���ܷ����Ĳ������������б���) */
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

/* �����������pstDLink */
#define tDLINK_SCANBACK_BEGIN( pstDLink, pstDNode ) \
    for ( pstDNode = (pstDLink)->pTail; pstDNode != NULL; pstDNode = (pstDNode)->pstPrev ) \
    { \

#define tDLINK_SCANBACK_END() \
    } \

/* �����������pstDLink(���pstDNode���ܷ����Ĳ������������б���) */
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
