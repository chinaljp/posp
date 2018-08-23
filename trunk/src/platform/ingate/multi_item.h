#ifndef __MULTI_ITEM_H__
#define __MULTI_ITEM_H__

/*************************************************
** 文 件 名: multi_item.h                       **
** 文件描述: 多路复用链表的操作                 **
**                                              **
** 作    者: 钟读营                             **
** 创建日期: 2012.12.01                         **
** 修改日期:                                    **
*************************************************/

#include <time.h>
#include "t_dlink.h"
#include "t_netlib.h"


#define CLIENTINFO_LEN  64
#define LOCALINFO_LEN   64
#define PKT_BCDLEN      2
#define ERR_MSG_LEN     128

#define DATA_SIZE  2048
/*  定义四种CommItem的cType通讯类型    */  
#define TCP_LISTEN  0           /* 监听套接字       */
#define TCP_WR      1           /* 通讯用的套接字   */
#define FIFO        2           /* 父子管道描述字   */

/*  定义三种CommItem的cRdStatus读写状态    */  
#define RD_STX      0           /* 读报文的STX状态  */
#define RD_LEN      1           /* 读报文长度状态   */
#define RD_DATA     2           /* 读报文数据状态   */
#define RD_FIN      3           /* 读数据结束状态   */

/*  定义CommItem的cCommMode通讯模式      */  
#define COMM_SYNC       '0'           /* 同步通讯模式         */
#define COMM_ASYNC      '1'           /* 异步单向通讯模式     */  
#define COMM_DOU_ASYNC  '2'           /* 异步双向通讯模式     */  

/*  定义CommItem的cSndRcv       */  
#define SND_LINK    0x01        /* 发送报文链路     */
#define RCV_LINK    0x02        /* 接收报文链路     */
#define DUB_LINK    (SND_LINK | RCV_LINK) 
                                /* 收发报文链路     */ 

typedef struct
{
    int     iFd;                            /* socket, pipefd, file描述字               */
    char    cType;                          /* 分为四种宏定义TCP_LISTEN,FIFO,TCP_WR     */
    char    cRdStatus;                      /* 分为三种宏定义RD_LEN,RD_DATA,RD_FIN      */
    char    cSndRcv;                        /* 分为三种定义SND_LINK,RCV_LINK,DUB_LINK   */
    char    cCommMode;                      /* 通讯模式COMM_SYNC,COMM_ASYNC             */
    time_t  tTime;                          /* 记录此请求包加入MultiItem中的时间        */
    int     iLen;                           /* 报文长度                                 */
    unsigned char   caData[DATA_SIZE];              /* 数据缓冲,存放接收数据,发送数据           */
    unsigned char   *pcPtr;                         /* 数据缓冲区指针,标明已接收数据的偏移位置  */
    char    sClientInfo[CLIENTINFO_LEN+1];  /* client socket infomation                 */
    char    sLocalInfo[LOCALINFO_LEN+1];    /* local socket infomation                  */
}MultiItem;

#define CopyDataFromTrans( pstMultiItem, pstTransItem ) \
    { \
        (pstMultiItem)->iFd = (pstTransItem)->iFd; \
        (pstMultiItem)->iLen = (pstTransItem)->iLen; \
        sprintf( (pstMultiItem)->sClientInfo, "%d:%s:%d", \
            (pstMultiItem)->iFd, get_client_ip((pstMultiItem)->iFd), get_client_port((pstMultiItem)->iFd) ); \
        sprintf( pstMultiItem->sLocalInfo, "%s:%d", \
            get_server_ip((pstMultiItem)->iFd), get_server_port((pstMultiItem)->iFd) ); \
    }

void    InitMultiItem( MultiItem *pstMultiItem );
void    RecoverMultiItem( MultiItem *pstMultiItem );
int     CmpMultiItem( void *pvData1, void *pvData2 );

int     CreateMultiLink( DLINK *pstMultiFree, int iInitSize );
DNode   *AddMultiLink( int iFd, char cType, char cCommMode, char cSndRcv, DLINK *pstMultiLink, DLINK *pstMultiFree );
DNode   *GetFreeMultiItem( DLINK *pstMultiFree );
void    ClearMultiLink( int iFd, DLINK *pstMultiLink, DLINK *pstMultiFree ); /* 删除一项 */
void    FreeMultiLink( DLINK *pstMultiLink );

void    MultiLinkChk( DLINK *pstMultiLink, char *pcTip );
int     GetMaxMulti();
int     GetMaxTrans();

#endif /* __MULTI_ITEM_H__ */

