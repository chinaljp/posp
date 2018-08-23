#ifndef __INGATE_H__
#define __INGATE_H__
#include "t_tools.h"

#define CLIENTINFO_LEN  64
#define LOCALINFO_LEN   64
#define DATA_SIZE  2048

/* ���屨�ĳ�������BCD����ASCII��ʽ��ʾ */
#define LENTYPE_BCD     '0'
#define LENTYPE_ASC     '1'
#define PKT_BCDLEN      2
#define PKT_ASCLEN      4

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

/****************************/
/*  �������ݲ���            */
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

/* ����TCP�� */
typedef enum tagPktType {
    PKT_TYPE_NORMAL = 0, /* 0: ��������      */
    PKT_TYPE_KA, /* 1: KA�����    */
} PktType;

typedef struct {
    int iGrpId; /* �첽ͨѶ�������ʶ   */
    char sInIp[30]; /* �������ӵĶԶ�IP��ַ */
    char sInPort[10]; /* �������ӵĶԶ˶˿ں� */
    char sOutIp[30]; /* �������ӵĶԶ�IP��ַ */
    char sOutPort[10]; /* �������ӵĶԶ˶˿ں� */
    char sListenIp[30]; /* �������ӵı���IP��ַ */
    char sListenPort[10]; /* �������ӵı��˶˿ں� */
} AsyncLink; /* �첽������Ϣ         */


#define CHKFLAG_TRANS       0x0001
#define CHKFLAG_MULTI       0x0002
#define CHKFLAG_ALIVE       0x0004
#define CHKFLAG_ALL         0xFFFF

typedef struct
{
    int     iFd;                            /* socket, pipefd, file������               */
    char    cType;                          /* ��Ϊ���ֺ궨��TCP_LISTEN,FIFO,TCP_WR     */
    char    cRdStatus;                      /* ��Ϊ���ֺ궨��RD_LEN,RD_DATA,RD_FIN      */
    char    cCommMode;                      /* ͨѶģʽCOMM_SYNC,COMM_ASYNC             */
    time_t  tTime;                          /* ��¼�����������MultiItem�е�ʱ��        */
    time_t  tKASnd;                         /* �ϴη���keep-alive����ʱ��,���Ա�������  */
    time_t  tKARcv;                         /* �ϴ��յ�keep-alive����ʱ��,���Ա�������  */
    int     iLen;                           /* ���ĳ���                                 */
    UCHAR   caData[DATA_SIZE];              /* ���ݻ���,��Ž�������,��������           */
    UCHAR   *pcPtr;                         /* ���ݻ�����ָ��,�����ѽ������ݵ�ƫ��λ��  */
    char    sClientInfo[CLIENTINFO_LEN+1];  /* TCP�Զ�socket��Ϣ                        */
    char    sLocalInfo[LOCALINFO_LEN+1];    /* TCP����socket��Ϣ                        */
}MultiItem;

/*************** ��ܺ������� ***************/
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

