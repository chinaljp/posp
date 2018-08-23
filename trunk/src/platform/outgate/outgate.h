#ifndef __INGATE_H__
#define __INGATE_H__
#include "t_tools.h"

#define CLIENTINFO_LEN  64
#define LOCALINFO_LEN   64
#define DATA_SIZE  2048

/* 定义报文长度域以BCD还是ASCII格式表示 */
#define LENTYPE_BCD     '0'
#define LENTYPE_ASC     '1'
#define PKT_BCDLEN      2
#define PKT_ASCLEN      4

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

/****************************/
/*  常用数据操作            */
/****************************/
#define BIT_TEST( a, b )    ( ( a ) & ( b ) )
#define BIT_SET( a, b )     ( ( a ) |= ( b ) )
#define BIT_RESET( a, b )   ( ( a ) &= ~( b ) )


#define HoldTimer      30
#define KeepAlive      30
#define PACK_LEN_ERR    -9
#define SOCK_CLOSE      -10
#define CMP_G           1
#define CMP_L          -1
#define CMP_ERR        -2
#define SND_MSG_ERR     -7
#define REDISDATA_SIZE 4096

#define UKEY_LEN        64
#define ORIGININFO_LEN  96

/* 特殊TCP包 */
typedef enum tagPktType {
    PKT_TYPE_NORMAL = 0, /* 0: 正常报文      */
    PKT_TYPE_KA, /* 1: KA保活报文    */
} PktType;

typedef struct {
    int iGrpId; /* 异步通讯链接组标识   */
    char sInIp[30]; /* 接入链接的对端IP地址 */
    char sInPort[10]; /* 接入链接的对端端口号 */
    char sOutIp[30]; /* 回送链接的对端IP地址 */
    char sOutPort[10]; /* 回送链接的对端端口号 */
    char sListenIp[30]; /* 侦听链接的本端IP地址 */
    char sListenPort[10]; /* 侦听链接的本端端口号 */
} AsyncLink; /* 异步链接信息         */


#define CHKFLAG_TRANS       0x0001
#define CHKFLAG_MULTI       0x0002
#define CHKFLAG_ALIVE       0x0004
#define CHKFLAG_ALL         0xFFFF

typedef struct
{
    int     iFd;                            /* socket, pipefd, file描述字               */
    char    cType;                          /* 分为四种宏定义TCP_LISTEN,FIFO,TCP_WR     */
    char    cRdStatus;                      /* 分为三种宏定义RD_LEN,RD_DATA,RD_FIN      */
    char    cCommMode;                      /* 通讯模式COMM_SYNC,COMM_ASYNC             */
    time_t  tTime;                          /* 记录此请求包加入MultiItem中的时间        */
    time_t  tKASnd;                         /* 上次发出keep-alive报文时间,用以保持心跳  */
    time_t  tKARcv;                         /* 上次收到keep-alive报文时间,用以保持心跳  */
    int     iLen;                           /* 报文长度                                 */
    UCHAR   caData[DATA_SIZE];              /* 数据缓冲,存放接收数据,发送数据           */
    UCHAR   *pcPtr;                         /* 数据缓冲区指针,标明已接收数据的偏移位置  */
    char    sClientInfo[CLIENTINFO_LEN+1];  /* TCP对端socket信息                        */
    char    sLocalInfo[LOCALINFO_LEN+1];    /* TCP本端socket信息                        */
}MultiItem;

/*************** 框架函数声明 ***************/
int RecvProc();
int SendProc();
int AddPipeFd( MultiItem *pstMultiItem, int iPipeFd );
void CheckTransLink(  );
void CheckAlive(  );
int KeepAliveSnd( MultiItem *pstMultiItem );

int ReqDeal(MultiItem *pstItem, void *pstNetTran);
int RespDeal(MultiItem *pstItem, void *pstNetTran);
int SndData(MultiItem *pstItem);
int RcvData(MultiItem *pstItem);

int PkDump(unsigned char *pcStr, int iLen, char *pcMsg);

int BuildAsyncSndLink(MultiItem *pstMultiItem);
int AuthSyncIP(const char *pcClientIp);
void tSigProc();
void SigQuitSend ( int iSigNo );

#endif /* __MOUDLE_IN_H__ */

