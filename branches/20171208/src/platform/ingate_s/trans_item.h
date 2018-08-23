#ifndef __TRANS_TTEM_H__
#define __TRANS_TTEM_H__

/*************************************************
** 文 件 名: trans_item.h                       **
** 文件描述: 交易链表的操作                     **
**                                              **
** 作    者: 钟读营                             **
** 创建日期: 2012.12.01                         **
** 修改日期:                                    **
*************************************************/

#include <time.h>
#include "t_tools.h"
#include "multi_item.h"

/* 长度定义 */  
#define UNIQUEKEY_SIZE  64
#define SVCMSGINFO_SIZE 84
#define ORIGININFO_LEN  96

typedef struct
{
    int     iFd;                            /* socket, pipefd, file描述字               */
    time_t  tTime;                          /* 记录此请求包加入TransItem中的时间        */
    int     iLen;                           /* 报文长度                                 */
    UCHAR   caData[DATA_SIZE];              /* 数据缓冲,存放接收数据,发送数据           */
    UCHAR   *pcPtr;                         /* 数据缓冲区指针,标明已接收数据的偏移位置  */
    char    sUniqueKey[UNIQUEKEY_SIZE];     /* 在交易链表中由模块自定义的唯一键值       */
    char    sOriginInfo[ORIGININFO_LEN+1];  /* 应用服务器用于匹配交易请求和应答的信息   */
    char    caSvcMsgInfo[SVCMSGINFO_SIZE];  /* 应用服务器发送的请求SvcMsg的信息         */
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
void 	ClearTransLink( char *pcUniqueKey, DLINK *pstTransLink, DLINK *pstTransFree ); /* 删除一个项 */
void 	DeleteTransLink( int iFd, DLINK *pstTransLink, DLINK *pstTransFree ); /* 删除所有相关项 */
void 	FreeTransLink( DLINK *pstTransLink );

void 	TransLinkChk( DLINK *pstTransLink, char *pcTip );

#endif /* __TRANS_TTEM_H__ */

