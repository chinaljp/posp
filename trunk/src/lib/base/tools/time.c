/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <time.h>
#include "t_tools.h"

char g_sTimeErrBuf[256];

/* 闰年判断 */
#define IS_YEAR_LEEP( iYear ) \
    ((( iYear % 4 == 0 ) && ( iYear % 100 != 0 )) || ( iYear % 400 == 0))

/*****************************************************************************
** 函数: tGetDate( )                                                        **
** 类型: void                                                               **
** 入参:                                                                    **
**      char *pcDate ( 存放结果字串地址 )                                   **
**      char *pcFlag ( 日期分隔符,以字符串形式传入 )                        **
**      time_t tTime ( 要转成标准日期格式的原始time,取当前则置为<0任意值 )  **
** 出参：                                                                   **
**      char *pcDate                                                        **
** 返回值：无                                                               **
** 功能：将当前日期按YYYYMMDD格式输出到字符串中,年月日之间以分隔符分开      **
*****************************************************************************/
void    tGetDate( char *pcDate, char *pcFlag, time_t tTime )
{
    time_t  tNow;
    struct tm *pstTime;


    if ( tTime < 0 )
        time( &tNow );
    else
        tNow = tTime;

    pstTime = localtime( &tNow );

    sprintf( pcDate, "%04d%s%02d%s%02d", 
        1900 + pstTime->tm_year, pcFlag, pstTime->tm_mon + 1, pcFlag, pstTime->tm_mday );
}



/*****************************************************************************
** 函数: tGetTime( )                                                        **
** 类型: void                                                               **
** 入参:                                                                    **
**      char *pcTime ( 存放结果字串地址 )                                   **
**      char *pcFlag ( 时间分隔符 )                                         **
**      time_t tTime ( 要转成标准时间格式的原始time值,取当前则置为<0任意值 )**
** 出参:                                                                    **
**      char *pcTime                                                        **
** 返回值：按hhmmss打印当前时间                                             **
** 功能：将当前期按hhmmss格式输出到字符串中,时分秒之间以分隔符分开          **
*****************************************************************************/
void    tGetTime( char *pcTime, char *pcFlag, time_t tTime )
{
    time_t  tNow;
    struct tm *pstTime;


    if ( tTime < 0 )
        time( &tNow );
    else
        tNow = tTime;

    pstTime = localtime( &tNow );

    sprintf( pcTime, "%02d%s%02d%s%02d", 
        pstTime->tm_hour, pcFlag, pstTime->tm_min, pcFlag, pstTime->tm_sec );
}


/*****************************************************************************
** 函数: tGetFullTime( )                                                    **
** 类型: void                                                               **
** 入参:                                                                    **
**      char *pcTime ( 存放结果字串地址 )                                   **
**      char *pcFlag ( 时间分隔符 )                                         **
**      time_t tTime ( 要转成标准时间格式的原始time值,取当前则置为<0任意值 )**
** 出参:                                                                    **
**      char *pcTime                                                        **
** 返回值：按hhmmss打印当前时间                                             **
** 功能：将当前期按hhmmss格式输出到字符串中,时分秒之间以分隔符分开          **
*****************************************************************************/
void    tGetFullTime( char *pcFullTime, char *pcFlag, time_t tTime )
{
    time_t  tNow;
    struct tm *pstTime;


    if ( tTime < 0 )
        time( &tNow );
    else
        tNow = tTime;

    pstTime = localtime( &tNow );

    sprintf( pcFullTime, "%04d%s%02d%s%02d%02d%s%02d%s%02d", 
            1900 + pstTime->tm_year, pcFlag, pstTime->tm_mon + 1, pcFlag, 
            pstTime->tm_mday, pstTime->tm_hour, pcFlag, pstTime->tm_min, 
            pcFlag, pstTime->tm_sec );
}


/*****************************************************************************
** 函数: tHaveDays( )                                                       **
** 类型: long                                                               **
** 入参:                                                                    **
**      long lDate1 ( 起始日期 )                                            **
**      long lDate2 ( 终止日期 )                                            **
** 出参：  无                                                               **
** 返回值：两个日期间天数之差值若iDate1>iDate2, 则为负数;                   ** 
**         否则, 为正.                                                      **
**         iDate为以整数形式表示的"YYYYMMDD"。                              **
**         如：int 19801217 表示1980年12月17日                              **
** 功能：  计算两个日期间的天数                                             **
*****************************************************************************/
long tHaveDays( long iDate1,  long iDate2 )
{
    char    cFlag;
    long    iMon1, iMon2, iDays = 0;
    long    i, iLeap = 0, iYear;
    long    iDayTab[2][13] = {
                { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
                { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
            };


    if ( iDate1 > iDate2 )
    {
        long iYy;
        iYy = iDate2;
        iDate2 = iDate1;
        iDate1 = iYy;
        cFlag = 1;
    }
    else
        cFlag = 0;

    iMon1 = iDate1/100;
    iMon2 = iDate2/100;

    for ( i = iMon1; i < iMon2; )
    {
        iYear = i / 100;
        iLeap = IS_YEAR_LEEP( iYear );
        iDays+= iDayTab[iLeap][i % 100];

        if ( i % 100 == 12 )
            i = ( i / 100 + 1 ) * 100 + 1;
        else
            i++;
    }

    iDays+= iDate2 % 100 - iDate1 % 100;

    if ( cFlag )
        return( -iDays );

    return( iDays );
}

/*****************************************************************************
** 函数: tGetMonthDays( )                                                   **
** 类型: int                                                                **
** 入参：                                                                   **
**       Time *pstTime   -- 以结构体Time格式表示的时间                      **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       long            -- 返回指定月份的天数                              **
** 功能：                                                                   **
**       计算指定日期所在月的天数                                           **
**                                                                          **
*****************************************************************************/
int tGetMonthDays( Time *pstTime )
{
    int iLeap;
    int iDayTab[2][13] = 
        {
            { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
            { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
        };

    iLeap = IS_YEAR_LEEP( pstTime->iYear );

    return ( iDayTab[iLeap][pstTime->iMonth] );
}

/*****************************************************************************
** 函数: tDateCount( )                                                      **
** 类型: long                                                               **
** 入参：                                                                   **
**       char *pcDate1   -- 以YYYYMMDD格式表示的日期                        **
**       char *pcDate2   -- 以YYYYMMDD格式表示的日期                        **
** 出参:                                                                    **
**       NA                                                                 **
** 返回值：                                                                 **
**       long   -- 从日期Date1到Date2所经历的天数                           **
**       0xFFFF -- Date1到Date2间隔 > 1年                                   **
** 功能：                                                                   **
**       计算从日期Date1到Date2所经历的天数                                 **
**                                                                          **
*****************************************************************************/
long tDateCount( char *pcDate1, char *pcDate2 )
{
    char cFlag;
    char sDateTmp[8+1]  = {0};
    char sDate1[8+1]    = {0};
    char sDate2[8+1]    = {0};
    char sYear1[4+1]    = {0};
    char sYear2[4+1]    = {0};
    char sMonDay1[4+1]  = {0};
    char sMonDay2[4+1]  = {0};
    long lDate1 = 0, lDate2 = 0, lYear1 = 0, lYear2 = 0, lMonDay1 = 0, lMonDay2 = 0;
    long lDayCount      = 0;
    int  i;

    if (( NULL == pcDate1 ) || ( NULL == pcDate1 ))
        return 0xFF;

    tStrCpy( sDate1, pcDate1, 8 );
    tStrCpy( sDate2, pcDate2, 8 );

    if ( !strcmp( sDate1, sDate2 ) )
    {
        tStrCpy( sDateTmp, sDate1, 8 );
        tStrCpy( sDate1, sDate2, 8 );
        tStrCpy( sDate2, sDateTmp, 8 );
        cFlag = 1;
    }
    else
        cFlag = 0;

    strncpy( sYear1, sDate1, 4 );
    strncpy( sYear2, sDate2, 4 );
    strncpy( sMonDay1, sDate1+4, 4 );
    strncpy( sMonDay2, sDate2+4, 4 );

    lYear1   = atol( sYear1 );
    lYear2   = atol( sYear2 );
    lMonDay1 = atol( sMonDay1 );
    lMonDay2 = atol( sMonDay2 );

    /* Date1与Date2间隔超过1年 */
    if ( lYear2 - lYear1 > 1 )
        return 0xFFFF;

    for ( i = (int)lYear1; i < (int)lYear2; i++ )
    {
        if ( IS_YEAR_LEEP( i ) )
        {
            lDayCount += 366;
        }
        else
        {
            lDayCount += 365;
        }
    }

    lDate1 = lYear1 * 10000 + lMonDay1;
    lDate2 = lYear2 * 10000 + lMonDay2;

    lDayCount +=  tHaveDays( lDate1, lDate2 );

    if ( cFlag )
        return ( -lDayCount );
    else
        return lDayCount;
}

/*****************************************************************************
** 函数: tHaveSecs( )                                                       **
** 类型: int                                                                **
** 入参:                                                                    **
**      int iTime1 ( 起始时间 )                                             **
**      int iTime2 ( 终止时间 )                                             **
** 出参：  无                                                               **
** 返回值：一天中两个时间之差值若iTime1>iTime2, 则为负数;                   ** 
**         否则, 为正.                                                      **
** 功能：  计算同一日期两个时间之间的秒数.                                  **
*****************************************************************************/
int     tHaveSecs( int iTime1, int iTime2 )
{
    long    lSec1, lSec2;


    lSec1 = ( iTime1 / 10000 % 100 ) * 3600 + ( iTime1 / 100 % 100 ) * 60 + iTime1 % 100;
    lSec2 = ( iTime2 / 10000 % 100 ) * 3600 + ( iTime2 / 100 % 100 ) * 60+ iTime2 % 100;

    return( lSec2 - lSec1 );
}


/*****************************************************************************
** 函数: tTime_AddDay( )                                                    **
** 类型: void                                                               **
** 入参:                                                                    **
**      char *psDate ( 原始日期 )                                           **
**      int iDays ( 原始日期相对应的某一天的差值 )                          **
** 出参:  char *psDate ( 原始日期相对应的某一天Buf指针)                     **
** 返回值：无                                                               ** 
** 功能：将任一原始日期前n天日期输出到字符串中                              **
*****************************************************************************/
void    tAddDay( char *psDate, int iDays )
{
    char      sTmp[9];
    int       iYear, iMonth, iDay;
    time_t    lSecond;
    struct tm stTm, *pstTm;


    iYear = (int)( atol( psDate ) ) / 10000;
    strcpy( sTmp, psDate + 4 );
    iMonth = atoi( sTmp ) / 100;
    strcpy( sTmp, psDate + 6 );
    iDay = atoi( sTmp );
    
    stTm.tm_year = iYear - 1900;
    stTm.tm_mon = iMonth - 1;
    stTm.tm_mday = iDay;
    stTm.tm_hour = 0;
    stTm.tm_min = 0;
    stTm.tm_sec = 0;
    stTm.tm_isdst = -1;
    stTm.tm_isdst = 0;

    lSecond = mktime( &stTm );
    lSecond += ( 3600 * 24 ) * iDays;

    pstTm = localtime( &lSecond );
    iYear = pstTm->tm_year + 1900;
    iMonth = pstTm->tm_mon + 1;
    iDay = pstTm->tm_mday;

    sprintf( psDate, "%04d%02d%02d", iYear, iMonth, iDay );
}

/*****************************************************************************
** 函数: tAddMonth( )                                                       **
** 类型: void                                                               **
** 入参:                                                                    **
**      char *pcDate ( 原始日期 )                                           **
**      int iMonths ( 原始日期相对应的某一月的差值 )                        **
** 出参:  char *pcDate ( 原始日期相对应的某一月Buf指针)                     **
** 返回值：无                                                               ** 
** 功能：将任一原始日期前n月日期输出到字符串中                              **
*****************************************************************************/
void    tAddMonth( char *pcDate, int iMonths )
{
    char      sTmp[9];
    int       iYear, iMonth, iDay, iLeap = 0, iTmp = 0; 
    int     iDayTab[2][13] = {
                { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
                { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
            };

    iYear = (int)( atol( pcDate ) ) / 10000;
    strcpy( sTmp, pcDate + 4 );
    iMonth = atoi( sTmp ) / 100;
    strcpy( sTmp, pcDate + 6 );
    iDay = atoi( sTmp );

    iTmp = abs(iMonths) / 12;
    if ( iMonths < 0 )
        iYear -= iTmp;
    else 
        iYear += iTmp;
    
    iTmp = abs(iMonths) % 12;
    if ( iMonths < 0 )
    {
        if ( (iMonth - iTmp) > 0 )
            iMonth -= iTmp;
        else 
        {
            iYear--;
            iMonth = 12 + iMonth - iTmp;
        }
    }
    else
    {
        if ( (iMonth + iTmp) < 13 )
            iMonth += iTmp;
        else 
        {
            iYear++;
            iMonth = iMonth + iTmp - 12; 
        }
    }
    iLeap = IS_YEAR_LEEP( iYear );
    if ( iDay > iDayTab[iLeap][iMonth] )
        iDay = iDayTab[iLeap][iMonth];

    sprintf( pcDate, "%04d%02d%02d", iYear, iMonth, iDay );
}


/*****************************************************************************
** 函数:    tSleep( )                                                       **
** 类型:    void                                                            **
** 入参:                                                                    **
**      int iMsecond ( 休眠的时间,微秒级 )                                  **
** 出参:  无                                                                **
** 返回值：无                                                               ** 
** 功能：将进程休眠指定的时间                                               **
*****************************************************************************/
void    tSleep( int iMsecond )
{
    struct timeval  stTime;

    
    stTime.tv_sec = 0L;
    stTime.tv_usec = (long)iMsecond * 1000;

    select( 0, NULL, NULL, NULL, &stTime );
}

/*****************************************************************************
** 函数:    tIsMonthEnd( )                                                  **
** 类型:    int                                                             **
** 入参:                                                                    **
**      char *psDate ( 日期 )                                               **
** 出参:  无                                                                **
** 返回值：成功返回0，否则返回-1                                            ** 
** 功能：判断日期是否为月末最后一天                                         **
*****************************************************************************/
int     tIsMonthEnd( char *pcDate )
{
    char      sTmp[9];
    int       iYear, iMonth, iDay, iLeap; 
    int     iDayTab[2][13] = {
                { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
                { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
            };

    iYear = (int)( atol( pcDate ) ) / 10000;
    strcpy( sTmp, pcDate + 4 );
    iMonth = atoi( sTmp ) / 100;
    strcpy( sTmp, pcDate + 6 );
    iDay = atoi( sTmp );

    iLeap = IS_YEAR_LEEP( iYear );
    if ( iDay <  iDayTab[iLeap][iMonth] )
        return -1;

    return 0;
}

/*****************************************************************************
** 函数:    tIsYearEnd( )                                                   **
** 类型:    int                                                             **
** 入参:                                                                    **
**      char *psDate ( 日期 )                                               **
** 出参:  无                                                                **
** 返回值：成功返回0，否则返回-1                                            ** 
** 功能：判断日期是否为年末最后一天                                         **
*****************************************************************************/
int     tIsYearEnd( char *pcDate )
{
    char      sTmp[9];
    int       iYear, iMonth, iDay; 

    iYear = (int)( atol( pcDate ) ) / 10000;
    strcpy( sTmp, pcDate + 4 );
    iMonth = atoi( sTmp ) / 100;
    strcpy( sTmp, pcDate + 6 );
    iDay = atoi( sTmp );

    if ( iMonth == 12 && iDay == 31 )
        return 0;

    return -1;
}

/*****************************************************************************
** 函数:    tGetWeek( )                                                     **
** 类型:    int                                                             **
** 入参:                                                                    **
**      char *pcDate ( 日期 )                                               **
** 出参:  无                                                                **
** 返回值：成功返回星期几（1-7），否则返回-1                                ** 
** 功能：获取当前日期是星期几                                               **
*****************************************************************************/
int tGetWeek( char *pcDate )
{
    char      sTmp[9];
    int       iYear, iMonth, iDay; 
    time_t    lSecond;
    struct tm stTm, *pstTm;

    iYear = (int)( atol( pcDate ) ) / 10000;
    strcpy( sTmp, pcDate + 4 );
    iMonth = atoi( sTmp ) / 100;
    strcpy( sTmp, pcDate + 6 );
    iDay = atoi( sTmp );

    stTm.tm_year = iYear - 1900;
    stTm.tm_mon = iMonth - 1;
    stTm.tm_mday = iDay;
    stTm.tm_hour = 0;
    stTm.tm_min = 0;
    stTm.tm_sec = 0;
    stTm.tm_isdst = -1;
    stTm.tm_isdst = 0;

    lSecond = mktime( &stTm );
    pstTm = localtime( &lSecond );
    if ( pstTm->tm_wday == 0 )
        return 7;
    return pstTm->tm_wday;
}


/*****************************************************************************
** 函数:    tGetWeek( )                                                     **
** 类型:    void                                                            **
** 入参:                                                                    **
**      char *pcTimeStamp ( 格式为: YYYYMMDDHHMISS 的时间戳 )                **
**      int pciSec ( 秒数 )                                                 **
** 出参: char *pcNextTimeStamp （加上指定秒数： pciSec 后的时间戳）           **
** 功能：获取一个 字符串类型的时间戳加上指定秒数 后的字符串时间戳               **
*****************************************************************************/
void tAddTime(char *pcTimeStamp, int pciSec, char *pcNextTimeStamp) {
    struct tm stTime;
    struct tm *p;
    time_t iSec;
    char    sYear[4+1]  = {0};
    char    sMonth[2+1] = {0};
    char    sDay[2+1]   = {0};
    char    sHour[2+1]  = {0};
    char    sMin[2+1]   = {0};
    char    sSec[2+1]   = {0};
    
    /*从时间戳pcTimeStamp 中拆分出 年月日 时分秒*/
    tStrCpy( sYear , pcTimeStamp,    4 );
    tStrCpy( sMonth, pcTimeStamp+4,  2 );
    tStrCpy( sDay  , pcTimeStamp+6,  2 );
    tStrCpy( sHour , pcTimeStamp+8,  2 );
    tStrCpy( sMin  , pcTimeStamp+10, 2 );
    tStrCpy( sSec  , pcTimeStamp+12, 2 );
    
    /*将拆分出来的年月日时分秒 赋值到 库函数自带的时间结构体：stTime中*/
    stTime.tm_year = atoi( sYear  )-1900;
    stTime.tm_mon  = atoi( sMonth )-1;
    stTime.tm_mday = atoi( sDay   );
    stTime.tm_hour = atoi( sHour  );
    stTime.tm_min  = atoi( sMin   );
    stTime.tm_sec  = atoi( sSec   );
    
    /*将时间（转成秒） 加上 指定秒数*/
    iSec = mktime(&stTime);
    iSec += pciSec;
    /*将秒数 转换为 时间结构体*/
    p = localtime(&iSec);
    /*时间结构体转换为 字符串时间戳*/
    sprintf(pcNextTimeStamp,"%04d%02d%02d%02d%02d%02d", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}