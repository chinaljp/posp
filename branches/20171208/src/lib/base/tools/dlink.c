/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*************************************************
** �� �� ��: dlink.c                            **
** �ļ�����: �������ݽṹ֮˫������             **
**                                              **
** ��    ��: �Ӷ�Ӫ                             **
** ��������: 2012.11.29                         **
** �޸�����:                                    **
*************************************************/
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_dlink.h"

/*****************************************************************************
** ����: tDLinkCreate( )                                                    **
** ����: int                                                                **
** ��Σ�                                                                   **
**       DLINK *pstDLink -- ������������ָ�룬������ɺ󽫴���������ϸ����  **
**       int iInitSize   -- ��ʼ�����������                              **
**       int iNodeSize   -- ÿ������������Ķ�̬�ڴ��С                  **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       0  -- �ɹ�                                                         **
**       -1 -- �ڴ�����ʧ��                                                 **
** ���ܣ�                                                                   **
**       ˫������Ĵ���                                                     **
** ��������:                                                                **
**       2012.11.29                                                         **
** �޸�����:                                                                **
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
        /* �����������ڴ� */
        pstDNodeNew = (DNode *)malloc( sizeof(DNode) );
        if ( NULL == pstDNodeNew )
        {
            /* �ڴ�����ʧ�ܣ��˳�ǰ�ͷ�ǰ������ɹ��Ľ��ռ� */
            break;
        }
        memset( pstDNodeNew, 0, sizeof(DNode) );

        /* ��������������ݵ��ڴ� */
        bTailData = B_FALSE;
        pstDNodeNew->pvData = malloc( pstDLink->iNodeSize );
        if ( NULL == pstDNodeNew->pvData )
        {
            /* �ڴ�����ʧ�ܣ����ݼ�¼��bTailData�ͷŽ���ڴ� */
            break;
        }
        memset( pstDNodeNew->pvData, 0, pstDLink->iNodeSize );
        bTailData = B_TRUE;

        /* ��������� */
        tDLinkInsert( pstDLink, pstDNodeNew );
    }

    /* �����ڴ�����ʧ��ʱ��Ӧ�ͷ�ǰ������ɹ����ڴ棬Ȼ���˳����� */
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
** ����: tDLinkExtend( )                                                    **
** ����: int                                                                **
** ��Σ�                                                                   **
**       DLINK *pstDLink -- ����չ������ָ��                                **
**       int iExtSize    -- ������չ������                                **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       0  -- �ɹ�                                                         **
**       -1 -- �ڴ�����ʧ��                                                 **
** ���ܣ�                                                                   **
**       ˫���������չ������ʼ����С������ʱ���������벢���iExtSize����� **
** ��������:                                                                **
**       2012.11.29                                                         **
** �޸�����:                                                                **
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
        /* �����������ڴ� */
        pstDNodeNew = (DNode *)malloc( sizeof(DNode) );
        if ( NULL == pstDNodeNew )
        {
            /* �ڴ�����ʧ�ܣ��˳�ǰ�ͷ�ǰ������ɹ��Ľ��ռ� */
            break;
        }
        memset( pstDNodeNew, 0, sizeof(DNode) );

        /* ��������������ݵ��ڴ� */
        bTailData = B_FALSE;
        pstDNodeNew->pvData = malloc( pstDLink->iNodeSize );
        if ( NULL == pstDNodeNew->pvData )
        {
            /* �ڴ�����ʧ�ܣ����ݼ�¼��bTailData�ͷŽ���ڴ� */
            break;
        }
        memset( pstDNodeNew->pvData, 0, pstDLink->iNodeSize );
        bTailData = B_TRUE;

        /* ��������� */
        tDLinkInsert( pstDLink, pstDNodeNew );
    }

    /* �����ڴ�����ʧ��ʱ��Ӧ�ͷ�ǰ������ɹ����ڴ棬Ȼ���˳����� */
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
** ����: tDLinkSearch( )                                                    **
** ����: DNode *                                                            **
** ��Σ�                                                                   **
**       DLINK *pstDLink -- ˫������ָ��                                    **
**       void *pvData    -- Ҫ���ҵ�����                                    **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       DNode *  -- ������ƥ���������                                   **
**       NULL     -- û���ҵ�ƥ����                                       **
** ���ܣ�                                                                   **
**       ˫����������ݼ�����pvData��DNode.pvData����ƥ��                   **
** ��������:                                                                **
**       2012.11.30                                                         **
** �޸�����:                                                                **
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
** ����: tDLinkGetNodeById( )                                               **
** ����: int                                                                **
** ��Σ�                                                                   **
**       DLINK *pstDLink -- ˫������ָ��                                    **
**       int iIndex      -- Ҫ��ȡ����������ţ�������0��ʼ               **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       DNode *  -- ������ƥ���������                                   **
**       NULL     -- ��������ŷǷ�                                         **
** ���ܣ�                                                                   **
**       ��ȡ˫�������iIndex��������                                     **
** ��������:                                                                **
**       2012.11.30                                                         **
** �޸�����:                                                                **
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

    /* ���˴����أ�˵������ṹ���ִ���! */
    return NULL;
}

/* Begin: removed here by zhongduying on 2013.01.10 for code encapsulation */
/* !ԭ����Ϊ�궨����.h�У��ֽ������⹫���ĺ�Ų��.c��! */

/* ������pstDLink�Ľ��pstDNodeǰ��������pstDNodeNew */
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

/* ������pstDLink�Ľ��pstDNode�󣬲�����pstDNodeNew */
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

/* ( �����ӿ� )�����pstDNode��������pstDLink�����뷽ʽ��DLINK.cAddMode���� */
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

/* ��������ȡ��һ����㣬ȡ�÷�ʽ��DLINK.cGetMode���� */
#define tDLinkGet( pstDLink ) \
        (( GET_FROM_HEAD == (pstDLink)->cGetMode ) ? \
          tDLinkGetHead( pstDLink ): \
          tDLinkGetTail( pstDLink ))

#define tDLinkGetHead( pstDLink )       ( (pstDLink)->pHead )

#define tDLinkGetTail( pstDLink )       ( (pstDLink)->pTail )

/* ( �����ӿ� )��������ȡ��һ����㣬ȡ�÷�ʽ��DLINK.cGetMode���� */
DNode *tDLinkGetNode( DLINK *pstDLink )
{
    return  (( !(pstDLink) || tDLinkIsEmpty( pstDLink ) ) ?
              NULL :
              tDLinkGet( pstDLink ));
}

/* ������pstDLinkժ��ͷ���pHead */
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

/* ������pstDLinkժ��β���pTail */
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

/* ������pstDLinkժ��һ����㣬ժ��λ����DLINK.cGetMode���� */
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

/* ( �����ӿ� )������pstDLinkժ��ָ�����pstDNode */
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
** ����: tDLinkPickNode( )                                                  **
** ����: DNode *                                                            **
** ��Σ�                                                                   **
**       DLINK *pstDLink -- ˫������ָ��                                    **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       DNode *  -- ��ȡ����������                                       **
**       NULL     -- �޿��ý��(����Ϊ��)                                   **
** ���ܣ�                                                                   **
**       ��˫�������ȡһ�������㣬���أ���������ժ��                     **
** ��������:                                                                **
**       2012.11.30                                                         **
** �޸�����:                                                                **
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

/* �ͷŽ��pstDNode�Ľ���ڴ�������ڴ� */
#define tDLinkFreeNode( pstDNode ) \
    if ( (pstDNode)->pvData ) \
    { \
        free( (pstDNode)->pvData ); \
    } \
    free( pstDNode ); \
    pstDNode = NULL; 

/* ������pstDLinkɾ��ָ�����pstDNode */
#define tDLinkDelNode( pstDLink, pstDNode ) \
    { \
        DNode *pstDNodeOld = pstDNode; \
        tDLinkRmvNode( pstDLink, pstDNode ); \
        tDLinkFreeNode( pstDNodeOld ); \
    }

/*****************************************************************************
** ����: tDLinkFree( )                                                      **
** ����: int                                                                **
** ��Σ�                                                                   **
**       DLINK *pstDLink -- ���ͷŵ�����ָ��                                **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       0  -- �ɹ�                                                         **
**       -1 -- ʧ��                                                         **
** ���ܣ�                                                                   **
**       ˫��������ͷţ��������ṹ�����ͷ�����ռ�õ������ڴ�             **
** ��������:                                                                **
**       2012.11.29                                                         **
** �޸�����:                                                                **
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

