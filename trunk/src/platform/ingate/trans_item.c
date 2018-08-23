/*************************************************
** �� �� ��: trans_item.c                       **
** �ļ�����: ��������Ĳ���                     **
**                                              **
** ��    ��: �Ӷ�Ӫ                             **
** ��������: 2012.12.01                         **
** �޸�����:                                    **
*************************************************/

#include <stdio.h>
#include <string.h>
#include "ingate.h"
#include "trans_item.h"
#include "t_log.h"
#include "t_dlink.h"

int g_int = 0;

/*****************************************************************************
** ����: InitTransItem( )                                                   **
** ����: void                                                               **
** ��Σ�                                                                   **
**       TransItem *pstTransItem    -- ����������ָ��                       **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       �����������ʼ��                                                   **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
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
** ����: RecoverTransItem( )                                                **
** ����: void                                                               **
** ��Σ�                                                                   **
**       TransItem *pstTransItem    -- ����������ָ��                       **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       �����������ֶλָ�                                                 **
** ��������:                                                                **
**       2012.12.01                                                         **
** �޸�����:                                                                **
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
** ����: CmpTransItemByFd( )                                                **
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
**       ��������ȽϺ�����ͨ��TransItem.iFd�Ƚϴ�С                        **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
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
** ����: CmpTransItemByUniqueKey( )                                         **
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
**       ��������ȽϺ�����ͨ��TransItem.sUniqueKey�Ƚϴ�С                 **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
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
** ����: CreateTransLink( )                                                 **
** ����: int                                                                **
** ��Σ�                                                                   **
**       DLINK *pstTransFree  -- ������������ָ�룬������ɺ󽫴���         **
**                               ������ϸ����                               **
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
int CreateTransLink( DLINK *pstTransFree, int iInitSize )
{
    pstTransFree->pvCmpFunc = CmpTransItemByUniqueKey;
    pstTransFree->iNodeSize = sizeof(TransItem);
    pstTransFree->cAddMode  = ADD_TO_TAIL;
    pstTransFree->cGetMode  = GET_FROM_HEAD;

    return tDLinkCreate( pstTransFree, iInitSize );
}

/*****************************************************************************
** ����: AddTransLink( )                                                    **
** ����: DNode *                                                            **
** ��Σ�                                                                   **
**       TransItem *pstTransItem -- ��������������ָ��                      **
**       DLINK *pstTransLink     -- ��������ָ��                            **
**       DLINK *pstTransFree     -- ���н�������ָ��                        **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       DNode *    -- ָ��������·�����������ָ��                         **
**       NULL       -- û�п��õĿ���������                                 **
** ���ܣ�                                                                   **
**       ��pstTransFree��ժȡһ�����н�㣬�ҵ�pstTransLink��               **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
DNode *AddTransLink( TransItem *pstTransData, DLINK *pstTransLink, DLINK *pstTransFree, int iTimeOut )
{
    TransItem  *pstTransItem = NULL;
    DNode      *pstDNodeFree = NULL;
    time_t      tTime;

    /* ������������sUniqueKeyΪΨһ��ֵ�����ظ����������CommItem��time_out�� */
    time( &tTime );
    if ( tDLinkSearch( pstTransLink, (void *)pstTransData ))
    {
        tLog ( FATAL, "����������Ψһ��ֵ[%s]�ظ�", pstTransData->sUniqueKey );
        return NULL;
    }

    /* ��ȡһ�����еĽ��������� */
    pstDNodeFree = GetFreeTransItem( pstTransFree );
    if ( NULL == pstDNodeFree )
    {
        tLog ( ERROR, "�Ѵ��������(%d),��ֵΪ[%s]�Ľ��ײ�������!",
                  GetMaxTrans(), pstTransData->sUniqueKey );
        TransLinkChk( pstTransLink, "Full TransLink" );
        return NULL;
    }
    pstTransItem = (TransItem *)pstDNodeFree->pvData;

    /* ������ν����������ݵ����н�������������뽻������ */
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
        tLog ( ERROR, "δ�ҵ�����������Ψһ��ֵ[%s]��������.", pcUKey );
        return NULL;
    }

    /*  ��Ӧ����ʱ,������ն˵Ľ���������   */
    time( &tTime );
    if ( (tTime - pstTransItem->tTime) > iTimeOut ) 
    {
        iFd = pstTransItem->iFd;
        tLog(ERROR,"TestDebug");
       /* 
        tErrLog ( ERR, "��ֵ[%s]��Ӧ����ʱ,��������[%d][%s:%d]",
                pcUKey, iFd, get_client_ip( iFd ), get_client_port( iFd ) ); 
                */
                
       /**/
        tLog ( ERROR, "��ֵ[%s]��Ӧ����ʱ,��������[%d]",
                pcUKey, iFd ); 
                
        ClearTransLink( pcUKey, pstTransLink, pstTransFree );

        iFd = g_int;
         /*
        tErrLog ( ERR, "��ֵ[%s]��Ӧ����ʱ,��������[%d][%s:%d]",
                pcUKey, iFd, get_client_ip( iFd ), get_client_port( iFd ) );
                */
       /*      */   
         tLog ( ERROR, "��ֵ[%s]��Ӧ����ʱ,��������[%d]",
                pcUKey, iFd ); 
                
        return ( NULL );
    }

    return pstNode;
}

/*****************************************************************************
** ����: GetFreeTransItem( )                                                **
** ����: DNode *                                                            **
** ��Σ�                                                                   **
**       DLINK *pstTransFree  -- ���н�������ָ��                           **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       DNode *      -- ��ȡ���еĽ����������ָ��                         **
**       NULL         -- û�п��õĿ���������                               **
** ���ܣ�                                                                   **
**       ��pstTransFree��ժȡһ�����н�㣬����                             **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
DNode *GetFreeTransItem( DLINK *pstTransFree )
{
    return tDLinkPickNode( pstTransFree );
}

/*****************************************************************************
** ����: ClearTransLink( )                                                  **
** ����: void                                                               **
** ��Σ�                                                                   **
**       char *pcUniqueKey    -- ģ���Զ���Ľ���������ؼ���               **
**       DLINK *pstTransLink  -- ��������ָ��                               **
**       DLINK *pstTransFree  -- ���н�������ָ��                           **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       ��pstTransLink����ָ��pcUniqueKey��һ����㣬�ҵ�pstTransFree��    **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
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
		tLog( INFO, "��ֵΪ[%s]�Ľ���������ɾ���ɹ�.", stTransItem.sUniqueKey );
    }
	else 
		tLog( INFO, "δ�ɹ�ɾ����ֵ[%s]�Ľ���������.", stTransItem.sUniqueKey );
}

/*****************************************************************************
** ����: DeleteTransLink( )                                                 **
** ����: void                                                               **
** ��Σ�                                                                   **
**       int iFd �ܵ�������                                                 **
**       DLINK *pstTransLink  -- ��������ָ��                               **
**       DLINK *pstTransFree  -- ���н�������ָ��                           **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       ��pstTransLink�ϻ�������TransItem.iFd�����iFd��ƥ��Ľ�㣬       **
**       �ҵ�pstTransFree��                                                 **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
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

    /* ժ������������ı�pstDNode��pstPrev��pstNextָ�룬Ӱ���������˴˴�ʹ�ð�ȫ������ */
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
** ����: FreeTransLink( )                                                   **
** ����: void                                                               **
** ��Σ�                                                                   **
**       DLINK *pstTransFree  -- ���ͷŵ�����ָ��                           **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       ����������ͷ�                                                     **
** ��������:                                                                **
**       2012.12.03                                                         **
** �޸�����:                                                                **
**                                                                          **
*****************************************************************************/
void FreeTransLink( DLINK *pstTransLink )
{
    tDLinkFree( pstTransLink );
}

/*****************************************************************************
** ����: TransLinkChk( )                                                    **
** ����: void                                                               **
** ��Σ�                                                                   **
**       DLINK *pstTransLink  -- ��������ָ��                               **
**       char *pcTip          -- ������Ϣ��                                 **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       void                                                               **
** ���ܣ�                                                                   **
**       �����á���ӡ��������������                                     **
** ��������:                                                                **
**       2012.12.04                                                         **
** �޸�����:                                                                **
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

        /* ����time_t */
        pstTime = localtime( &pstTransItem->tTime );
        sprintf( sTime, "%04d%02d%02d_%02d%02d%02d", 
            pstTime->tm_year + 1900,
            pstTime->tm_mon + 1,
            pstTime->tm_mday,
            pstTime->tm_hour,
            pstTime->tm_min,
            pstTime->tm_sec );

        /* ��¼���������� */
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


