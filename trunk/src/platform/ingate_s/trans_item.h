#ifndef __TRANS_TTEM_H__
#define __TRANS_TTEM_H__

/*************************************************
** �� �� ��: trans_item.h                       **
** �ļ�����: ��������Ĳ���                     **
**                                              **
** ��    ��: �Ӷ�Ӫ                             **
** ��������: 2012.12.01                         **
** �޸�����:                                    **
*************************************************/

#include <time.h>
#include "t_tools.h"
#include "multi_item.h"

/* ���ȶ��� */  
#define UNIQUEKEY_SIZE  64
#define SVCMSGINFO_SIZE 84
#define ORIGININFO_LEN  96

typedef struct
{
    int     iFd;                            /* socket, pipefd, file������               */
    time_t  tTime;                          /* ��¼�����������TransItem�е�ʱ��        */
    int     iLen;                           /* ���ĳ���                                 */
    UCHAR   caData[DATA_SIZE];              /* ���ݻ���,��Ž�������,��������           */
    UCHAR   *pcPtr;                         /* ���ݻ�����ָ��,�����ѽ������ݵ�ƫ��λ��  */
    char    sUniqueKey[UNIQUEKEY_SIZE];     /* �ڽ�����������ģ���Զ����Ψһ��ֵ       */
    char    sOriginInfo[ORIGININFO_LEN+1];  /* Ӧ�÷���������ƥ�佻�������Ӧ�����Ϣ   */
    char    caSvcMsgInfo[SVCMSGINFO_SIZE];  /* Ӧ�÷��������͵�����SvcMsg����Ϣ         */
}TransItem;

#define CopyDataFromMulti( pstTransItem, pstMultiItem ) \
    { \
        (pstTransItem)->iFd = (pstMultiItem)->iFd; \
        (pstTransItem)->iLen = (pstMultiItem)->iLen; \
        memcpy((pstTransItem)->caData, (pstMultiItem)->caData, DATA_SIZE); \
        (pstTransItem)->pcPtr = (pstTransItem)->caData; \
    }

void 	InitTransItem( TransItem *pstTransItem );
void 	RecoverTransItem( TransItem *pstTransItem );
int 	CmpTransItemByUniqueKey( void *pvData1, void *pvData2 );
int 	CmpTransItemByFd( void *pvData1, void *pvData2 );

int 	CreateTransLink( DLINK *pstTransFree, int iInitSize );
DNode 	*AddTransLink( TransItem *pstTransData, DLINK *pstTransLink, DLINK *pstTransFree, int iTimeOut );
DNode 	*GetTransLink( char *pcUKey, DLINK *pstTransLink, DLINK *pstTransFree, int iTimeOut );
DNode 	*GetFreeTransItem( DLINK *pstTransFree );
void 	ClearTransLink( char *pcUniqueKey, DLINK *pstTransLink, DLINK *pstTransFree ); /* ɾ��һ���� */
void 	DeleteTransLink( int iFd, DLINK *pstTransLink, DLINK *pstTransFree ); /* ɾ����������� */
void 	FreeTransLink( DLINK *pstTransLink );

void 	TransLinkChk( DLINK *pstTransLink, char *pcTip );

#endif /* __TRANS_TTEM_H__ */

