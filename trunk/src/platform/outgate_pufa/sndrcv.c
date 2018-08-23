#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "t_tools.h"
#include "outgate.h"
#include "t_log.h"
#include  "param.h"



jmp_buf oldenv;
void timeout();
int tBufDump(char *pcFile, UCHAR *pcaData, int iLen, char *pcMsg);

/*
 *  SEND MESSAGE PACKET TO CONNECTED ORGANIZATION
 */
int SndData(MultiItem *pstIoLink) {
    int i, iLen;
    unsigned char caBuf[DATA_SIZE];

    tLog(DEBUG, "[SndData] fd=%d, item.len=%d", pstIoLink->iFd, pstIoLink->iLen);

    memset(caBuf, '\0', sizeof (caBuf));
    memcpy((void *) caBuf, (void *) pstIoLink->caData, pstIoLink->iLen);
    if (setjmp(oldenv)) {
        //tLog( ERR, "write [%d] time_out!", pstIoLink->iFd );
        return ( SND_MSG_ERR);
    }
    signal(SIGALRM, timeout);
    alarm(2);

    i = 0;
    while (i != pstIoLink->iLen) {
        iLen = write(pstIoLink->iFd, caBuf + i, pstIoLink->iLen - i);
        if (iLen <= 0) {
            tLog(ERROR, "write error! %d:%s", errno, strerror(errno));
            alarm(0);
            return ( SND_MSG_ERR);
        }
        i += iLen;
    }
    shutdown(pstIoLink->iFd,1);
    alarm(0);

    return (EXIT_SUCCESS);
}

/*
 *  RECEIVE MESSAGE FROM CONNECTED ORGANIZATION
 */
int RcvData(MultiItem *pstIoLink) {
    int iLen = 0, iRet = 0;
    char cPktLenType = GetPktLenType();
    unsigned char *ptr;
    tLog(INFO, "cPktLenType--------------[%c]", cPktLenType);
    tLog(INFO, " pstIoLink->cRdStatus[%d][%d]", pstIoLink->cRdStatus, pstIoLink->iLen);
    iLen = read(pstIoLink->iFd, pstIoLink->pcPtr, pstIoLink->iLen);
    tLog(INFO, "ilen[%d],pstIoLink->iLen=[%d]", iLen, pstIoLink->iLen);
    //if (( iLen < 0 ) || (( iLen == 0 ) && ( pstIoLink->cRdStatus == RD_LEN ))) 
    if (iLen <= 0) {
        //tBufDump("/home/post/log/test", pstIoLink->caData, 50, "�յ�SOCK_CLOSE");
        return SOCK_CLOSE;
    }

    if (iLen < pstIoLink->iLen) {
        pstIoLink->pcPtr += iLen;
        pstIoLink->iLen -= iLen;
        return (EXIT_SUCCESS);
    }

    switch (pstIoLink->cRdStatus) {
        case RD_LEN:
            /* ���������Ϊ��, ��������Ϊ�� */
            if (iLen == 0) {
                //tBufDump("test", pstIoLink->caData, 50, "�յ�SOCK_CLOSE");
                return SOCK_CLOSE;
            }
            tLog(DEBUG, "[RcvData] fd=%d, item.len=%d, read len=%d", pstIoLink->iFd, pstIoLink->iLen, iLen);
            char sLen[4 + 1];

            pstIoLink->pcPtr += iLen;
            ptr = pstIoLink->pcPtr;

            iLen = 0;
            if (LENTYPE_BCD == cPktLenType) {
                ptr = ptr - PKT_BCDLEN;
                iLen = (*ptr) * 256 + *(ptr + 1);
            } else {
                ptr = ptr - PKT_ASCLEN;
                tStrCpy(sLen, ptr, PKT_ASCLEN);
                iLen = atoi(sLen);
            }
            tLog(DEBUG, "PktLenType = [%s], len = [%d]"
                    , (LENTYPE_BCD == cPktLenType) ? "BCD" : "ASCII"
                    , iLen);
#if 0
            /*����������*/
            if (iLen == 0) {
                RecoverMultiItem(pstIoLink);
                iRet = KeepAliveSnd(pstIoLink);
                if (iRet < 0) {
                    tLog(WARN, "��·[%d]����KA����ʧ��, ��·���!", pstIoLink->iFd);
                    return SOCK_CLOSE;
                } else {
                    tLog(WARN, "��·[%d]����KA���ĳɹ�.", pstIoLink->iFd);
                }
                return (EXIT_SUCCESS);
            }
#endif
            /* tBufDump( DEBUG, pGetLogFile, ptr, 50, "FOR RcvData" ); */
            if (iLen >= DATA_SIZE) {
                tLog(ERROR, "packet too long len[%d]", iLen);
                return PACK_LEN_ERR;
            }
            pstIoLink->iLen = iLen;
            pstIoLink->cRdStatus = RD_DATA;
            break;
        case RD_DATA:
            pstIoLink->pcPtr += iLen;
            pstIoLink->cRdStatus = RD_FIN;
            pstIoLink->iLen = pstIoLink->pcPtr - pstIoLink->caData;
            break;
            //return SOCK_CLOSE;
    }
    tLog(INFO, " 1pstIoLink->cRdStatus[%d]", pstIoLink->cRdStatus);
    return (EXIT_SUCCESS);
}

void timeout() {
    signal(SIGALRM, SIG_IGN);
    alarm(0);
    longjmp(oldenv, 1);
}

/*****************************************************************************
 ** ����: tBufDump( )                                                        **
 ** ����: int                                                                **
 ** ��Σ�                                                                   **
 **       FuncGetLogFile *pvFunc -- ����ָ��,���Ի�ȡҪ���������־�ļ�      **
 **       UCHAR          *pcStr  -- Ҫ�����buffer                           **
 **       int             iLen   -- ���buffer����                           **
 **       char           *pcMsg  -- ���buffer�ļ�Ҫ����,�ḽ�ӵ���־��      **
 ** ����:                                                                    **
 **       NA                                                                 **
 ** ����ֵ��                                                                 **
 **       0  --  �������                                                    **
 **       -1 --  ��־�ļ���ʧ��                                            **
 ** ���ܣ�                                                                   **
 **       ��ָ��buffer�Զ�������ʽ�������־�ļ��С�                         **
 ** ��������:                                                                **
 **       2013.04.19 �Ӷ�Ӫ ��������                                         **
 ** �޸�����:                                                                **
 **                                                                          **
 *****************************************************************************/

int tBufDump(char *pcFile, UCHAR *pcaData, int iLen, char *pcMsg) {
    char sTime[14], sDate[14], sLogName[128];
    int i;
    FILE *pstFp;
    struct timeval tv;
    time_t tTime;

    time(&tTime);
    tGetDate(sDate, "/", tTime);
    tGetTime(sTime, ":", tTime);

    pstFp = fopen(pcFile, "a+");

    if (pstFp == NULL) {
        tLog(ERROR, "��־�ļ�%s�򿪴�! ����."
                , pcFile);
        return ( -1);
    }
    gettimeofday(&tv, NULL);

    fprintf(pstFp, "\n----%s, ʱ�� %s %s %d\n", pcMsg, sDate, sTime, (int) (tv.tv_usec / 1000));
    for (i = 0; i < iLen; i++) {
        if (i != 0 && i % 25 == 0)
            fprintf(pstFp, "\n");
        fprintf(pstFp, "%02x ", pcaData[i]);
    }
    fprintf(pstFp, "\n");
    fclose(pstFp);

    return ( 0);
}