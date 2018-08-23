#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include "t_tools.h"
#include "t_config.h"
#include "outgate.h"
#include "t_log.h"
#include "t_netlib.h"
#include "param.h"


int BuildAsyncSndLink(MultiItem *pstMultiItem) {
    int iSock;
    char sIpC[20] = {0};
    char sPortC[10] = {0};
    strcat(sIpC, GetIpC());
    strcat(sPortC, GetPortC());
    if(pstMultiItem->cRdStatus!=RD_FIN){
        return 0;
    }
    /* ������������ */
    if ((iSock = tcp_connect(sIpC, sPortC, "", "")) < 0) {
        tLog(ERROR, "�첽ͨѶ��·[%s:%s]����ʧ��."
                , sIpC, sPortC);
        return -1;
    }
    pstMultiItem->cRdStatus=RD_LEN;
    pstMultiItem->iLen = 2; //����Ϊ4�ֽ�asc��
    pstMultiItem->pcPtr = pstMultiItem->caData;
    pstMultiItem->iFd=iSock;
    pstMultiItem->cType=TCP_WR;
    pstMultiItem->cCommMode=COMM_ASYNC;
    
    tLog(ERROR, "�첽ͨѶ��·[%s:%s]�����ɹ�.SocketID=[%d]"
            , sIpC, sPortC, iSock);

    return 0;
}


int AddPipeFd( MultiItem *pstMultiItem, int iPipeFd )
{
    pstMultiItem->iFd=iPipeFd;
    pstMultiItem->cType=FIFO;
    pstMultiItem->cCommMode=COMM_ASYNC;
    return 0;
}


void ClearMultiLink(MultiItem *pstMultiItem) {


    /* �ر�TCPͨѶ�׽��� */
    if (TCP_WR == pstMultiItem->cType) {
        close(pstMultiItem->iFd);
        pstMultiItem->cRdStatus=RD_FIN;
        tLog(NOTICE, "ɾ��iFd[%d]�Ķ�·��������", pstMultiItem->iFd);
        pstMultiItem->iFd=-1;
    }

}

void RecoverMultiItem( MultiItem *pstMultiItem ){
    pstMultiItem->cRdStatus = RD_LEN;  
    pstMultiItem->iLen = 4; //����Ϊ4�ֽ�asc��
    pstMultiItem->pcPtr = pstMultiItem->caData;
}

int KeepAliveSnd( MultiItem *pstMultiItem )
{
    MultiItem stIoLink = *pstMultiItem;
    int     iRet;

    memcpy(stIoLink.caData, "0000", PKT_ASCLEN);
    stIoLink.pcPtr = stIoLink.caData;
    stIoLink.iLen = 4;
    iRet = SndData( &stIoLink );
    if( iRet < 0 )
    {
        tLog( ERROR, "��Զ�[%s]����������ʧ��![%d]"
                    , stIoLink.sClientInfo, iRet );
        return iRet;
    }
    //pstMultiItem->tKASnd = tTime;
    tLog( ERROR, "��Զ�[%s]�����������ɹ�."
                , stIoLink.sClientInfo );

    return 0;
}