#ifndef __MULTI_ITEM_H__
#define __MULTI_ITEM_H__

/*************************************************
** �� �� ��: multi_item.h                       **
** �ļ�����: ��·��������Ĳ���                 **
**                                              **
** ��    ��: �Ӷ�Ӫ                             **
** ��������: 2012.12.01                         **
** �޸�����:                                    **
*************************************************/

#include <time.h>
#include "t_dlink.h"
#include "t_netlib.h"


#define CLIENTINFO_LEN  64
#define LOCALINFO_LEN   64
#define PKT_BCDLEN      2
#define ERR_MSG_LEN     128

#define DATA_SIZE  2048
/*  ��������CommItem��cTypeͨѶ����    */  
#define TCP_LISTEN  0           /* �����׽���       */
#define TCP_WR      1           /* ͨѶ�õ��׽���   */
#define FIFO        2           /* ���ӹܵ�������   */

/*  ��������CommItem��cRdStatus��д״̬    */  
#define RD_STX      0           /* �����ĵ�STX״̬  */
#define RD_LEN      1           /* �����ĳ���״̬   */
#define RD_DATA     2           /* ����������״̬   */
#define RD_FIN      3           /* �����ݽ���״̬   */

/*  ����CommItem��cCommModeͨѶģʽ      */  
#define COMM_SYNC       '0'           /* ͬ��ͨѶģʽ         */
#define COMM_ASYNC      '1'           /* �첽����ͨѶģʽ     */  
#define COMM_DOU_ASYNC  '2'           /* �첽˫��ͨѶģʽ     */  

/*  ����CommItem��cSndRcv       */  
#define SND_LINK    0x01        /* ���ͱ�����·     */
#define RCV_LINK    0x02        /* ���ձ�����·     */
#define DUB_LINK    (SND_LINK | RCV_LINK) 
                                /* �շ�������·     */ 

typedef struct
{
    int     iFd;                            /* socket, pipefd, file������               */
    char    cType;                          /* ��Ϊ���ֺ궨��TCP_LISTEN,FIFO,TCP_WR     */
    char    cRdStatus;                      /* ��Ϊ���ֺ궨��RD_LEN,RD_DATA,RD_FIN      */
    char    cSndRcv;                        /* ��Ϊ���ֶ���SND_LINK,RCV_LINK,DUB_LINK   */
    char    cCommMode;                      /* ͨѶģʽCOMM_SYNC,COMM_ASYNC             */
    time_t  tTime;                          /* ��¼�����������MultiItem�е�ʱ��        */
    int     iLen;                           /* ���ĳ���                                 */
    unsigned char   caData[DATA_SIZE];              /* ���ݻ���,��Ž�������,��������           */
    unsigned char   *pcPtr;                         /* ���ݻ�����ָ��,�����ѽ������ݵ�ƫ��λ��  */
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
void    ClearMultiLink( int iFd, DLINK *pstMultiLink, DLINK *pstMultiFree ); /* ɾ��һ�� */
void    FreeMultiLink( DLINK *pstMultiLink );

void    MultiLinkChk( DLINK *pstMultiLink, char *pcTip );
int     GetMaxMulti();
int     GetMaxTrans();

#endif /* __MULTI_ITEM_H__ */

