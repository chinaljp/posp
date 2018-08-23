#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "ingate.h"
#include "t_log.h"



jmp_buf oldenv;
void timeout();

/*
 *  SEND MESSAGE PACKET TO POS
 */
int SndData(MultiItem *pstItem) {
    int i, iLen;
    unsigned char caBuf[DATA_SIZE];

    memset(caBuf, '\0', sizeof (caBuf));
    memcpy(caBuf, pstItem->caData, pstItem->iLen);

    if (setjmp(oldenv)) {
        /* tErrLog( ERR, "write [%d] time_out!", pstItem->iFd ); */
        return ( SND_MSG_ERR);
    }
    signal(SIGALRM, timeout);
    alarm(2);

    i = 0;
    while (i != pstItem->iLen) {
        iLen = write(pstItem->iFd, caBuf + i, pstItem->iLen - i);
        if (iLen <= 0) {
            tLog(ERROR, "write error! %d:%s", errno, strerror(errno));
            alarm(0);
            signal(SIGALRM, SIG_IGN);
            return ( SND_MSG_ERR);
        }
        i += iLen;
    }
    alarm(0);

    signal(SIGALRM, SIG_IGN);
    return (EXIT_SUCCESS);
}

/*
 *  RECEIVE MESSAGE FROM POS
 */
int RcvData(MultiItem *pstItem) {
    int iLen;
    unsigned char *pcPtr;
    char sLen[PKT_BCDLEN + 1] = {0};

    iLen = read(pstItem->iFd, pstItem->pcPtr, pstItem->iLen);
    if (iLen <= 0)
        return SOCK_CLOSE;

    if (iLen < pstItem->iLen) {
        pstItem->pcPtr += iLen;
        pstItem->iLen -= iLen;
        return (EXIT_SUCCESS);
    }

    switch (pstItem->cRdStatus) {
        case RD_LEN:
            pstItem->pcPtr += iLen;
            pcPtr = pstItem->pcPtr;

            iLen = 0;
            pcPtr = pcPtr - PKT_BCDLEN;
            //iLen = (*pcPtr) * 256 + *(pcPtr + 1);
            tStrCpy(sLen, pcPtr, PKT_BCDLEN);
            iLen = atoi(sLen);
            if (iLen >= DATA_SIZE) {
                tLog(ERROR, "packet too long len[%d]", iLen);
                return PACK_LEN_ERR;
            }

            pstItem->iLen = iLen;
            pstItem->cRdStatus = RD_DATA;
            break;
        case RD_DATA:
            pstItem->pcPtr += iLen;
            pstItem->cRdStatus = RD_FIN;
            pstItem->iLen = pstItem->pcPtr - pstItem->caData;
            break;
    }

    return (EXIT_SUCCESS);
}

void timeout() {
    printf("SIGALRM catched.\n");
    signal(SIGALRM, SIG_IGN);
    alarm(0);
    longjmp(oldenv, 1);
}
