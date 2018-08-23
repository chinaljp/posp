#ifndef __INGATE_H__
#define __INGATE_H__
#include "multi_item.h"


#define PACK_LEN_ERR    -9
#define SOCK_CLOSE      -10
#define CMP_G           1
#define CMP_L          -1
#define CMP_ERR        -2
#define SND_MSG_ERR     -7
#define REDISDATA_SIZE 4096

/*************** ¿ò¼Üº¯ÊýÉùÃ÷ ***************/
int     RecvProc();
int     SendProc();
int     TcpProc ( DNode **ppstDNode, int iTimeOut );
int     PipeProc( DNode *pstDNode, int iTimeOut );
void    CheckProc( int iTimeOut );


int     ReqDeal( MultiItem *pstItem, void *pstNetTran );
int     RespDeal( MultiItem *pstItem, void *pstNetTran );
int     SndData( MultiItem *pstItem );
int     RcvData( MultiItem *pstItem );

int     PkDump( unsigned char *pcStr, int iLen, char *pcMsg );

DNode   *GetMultiLinkByFd( int iFd, DLINK *pstMultiLink );
DNode   *BuildAsyncSndLink( char *pcClitIp, char *pcClitPort, DLINK *pstMultiLink, DLINK *pstMultiFree );
void    BuildAllAsyncSndLink( DLINK *pstMultiLink, DLINK *pstMultiFree );
int     ChkAsyncIP( const char *pcLocalIp, const char *pcLocalPort, char *pcPeerIp, char *pcPeerPort );
int     AddPipeFd( DLINK *pstMultiLink, DLINK *pstMultiFree, int iPipeFd );
int     AddListenFd( DLINK *pstMultiLink, DLINK *pstMultiFree );
void    AddTcpwrFd( DLINK *pstMultiLink, DLINK *pstMultiFree, int iListenSock );
int     AuthSyncIP( const char *pcClientIp );
void    tSigProc();

#endif /* __MOUDLE_IN_H__ */

