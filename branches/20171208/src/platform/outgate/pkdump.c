#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "t_tools.h"
#include "t_log.h"

/* 
 *  二进制形式记录交易数据包
 */
int PkDump(unsigned char *pcStr, int iLen, char *pcMsg) {
    char sTime[14], sDate[14], sLogName[128];
    int i;
    FILE *pstFp;
    time_t tTime;
    struct timeval tv;

    /*
        if ( tGetLogLevel( ) < '2' )
            return;
     */
    time(&tTime);
    tGetDate(sDate, "-", -1);
    snprintf(sLogName, sizeof (sLogName), "%s/log/%s.%s", getenv("HOME"), "cupsd", sDate);
    
    tGetDate(sDate, "/", tTime);
    tGetTime(sTime, ":", tTime);
    
    pstFp = fopen(sLogName, "a+");
    if (pstFp == NULL) {
        tLog(ERROR, "日志文件%s打开错! 请检查[%d:%s].", sLogName, errno, strerror(errno));
        return ( -1);
    }
    gettimeofday(&tv, NULL);

    fprintf(pstFp, "\n----%s, 时间 %s %s %d\n", pcMsg, sDate, sTime, (int) tv.tv_usec / 1000);
    for (i = 0; i < iLen; i++) {
        if (i != 0 && i % 25 == 0)
            fprintf(pstFp, "\n");
        fprintf(pstFp, "%02x ", pcStr[i]);
    }
    fprintf(pstFp, "\n");
    fclose(pstFp);

    return ( 0);
}
