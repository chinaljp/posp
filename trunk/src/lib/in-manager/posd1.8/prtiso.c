#include <stdio.h>
#include <string.h> 
#include <sys/time.h>
#include "t_tools.h"
#include "t_extiso.h"
#include "p_inmodule.h"

extern IsoData  g_stIsoData;

void tPrtIsoData( char *pcTitle )
{
    unsigned char cFldType;
    int i, iFldLen, iLen, iLenTmp=0;
    IsoTable    *pstIsoTable;
    Fld         *pstFld;
    char    sTime[14], sDate[14], sLogName[128];
    char    *pcStr, sData[2048], sTmp[1024];
    FILE    *pstFp;
    struct  timeval tv;

    pstIsoTable = g_stIsoData.pstIsoTable;
    pstFld = g_stIsoData.pstFld;

    if ( tGetLogLevel( ) < '2' )
        return;

    pGetLogFile( sLogName );
    pstFp = fopen( sLogName , "a+" );
    if ( pstFp == NULL ) {
        tErrLog( ERR, "日志文件%s打开错! 请检查。", sLogName );
        return;
    }

    tGetDate( sDate, "/", -1 );
    tGetTime( sTime, ":", -1 );
    gettimeofday( &tv, NULL );
    fprintf( pstFp, "\n----------%s, 时间 %s %s %03d\n", pcTitle, sDate, sTime, (int)(tv.tv_usec/1000) );

    for ( i=0; i<128; i++ )
    {
        if ( pstFld[i].hFlag == NON_EXIST )
            continue;
//        if ( i == 1)
//            continue;
//        if ( i == 13)
//            continue;
//        if ( i == 34)
//            continue;
//        if ( i == 35)
//            continue;
         
        pcStr = g_stIsoData.pcBuf+pstFld[i].hOff;
        cFldType = pstIsoTable[i].cType;
        iFldLen = pstIsoTable[i].hLen;
        iLen = pstFld[i].hLen;
        if ( cFldType & SYMBOL )
            iLen += 1;

        if ( cFldType & BINARY )
        {
            tStrCpy( sTmp, pcStr, iLen );
            tBcd2Asc( sData, sTmp, iLen*2, 0 );
            sData[iLen*2] = 0;
        }
        else if ( cFldType & BCD )
        {
            /*
            tStrCpy( sTmp, pcStr, (iLen+1)/2 );
            tBcd2Asc( sData, sTmp, iLen, 0 );
            sData[iLen] = 0;
             */
            /*
             * modify by gaof 2016/05/26
             * 左右对齐时,打印有问题,比如23域bcd码,0x00,0x01,会打印出000
             * */
            iLenTmp = ((iLen+1)/2)<<1;
            tBcd2Asc( sTmp, pcStr, iLenTmp, 0 );
            sTmp[iLenTmp] = 0;
            //tErrLog( DEBUG,"%d,%s", iLenTmp, sTmp );
            if ( cFldType & LEFT_ALIGN )
            {
                tStrCpy(sData, sTmp, iLen );
                sData[iLen] = 0;
            }
            else
            {
                tStrCpy(sData, sTmp+iLenTmp-iLen, iLen );
                sData[iLen] = 0;
                //tErrLog(DEBUG,"data[%s]",sData );
            }
        }
        else 
            tStrCpy( sData, pcStr, iLen );
 
        if ( (cFldType & LLLVAR) == LLLVAR )
            fprintf( pstFp, "域[%3d] LLL[%3d] 长度[%3d] 数据:%s\n", i+1, iFldLen, iLen, sData );
        else if ( (cFldType & LLVAR) == LLVAR )
            fprintf( pstFp, "域[%3d] LLV[%3d] 长度[%3d] 数据:%s\n", i+1, iFldLen, iLen, sData );
        else
            fprintf( pstFp, "域[%3d] FIX[%3d] 长度[%3d] 数据:%s\n", i+1, iFldLen, iLen, sData );
    }
    fclose( pstFp );
}

void PrintHex( char *pcTitle, char *pcData, int iLen )
{
    char sTmp[2048] = {0};
    int i;
    for ( i=0; i<iLen; i++ )
    {
        sprintf( sTmp+i*3, "%02X ", (unsigned char)pcData[i] );
    }
    tErrLog( DEBUG, "%s:%s", pcTitle, sTmp );
}
