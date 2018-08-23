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

/* �����ж� */
#define IS_YEAR_LEEP( iYear ) \
    ((( iYear % 4 == 0 ) && ( iYear % 100 != 0 )) || ( iYear % 400 == 0))

/*****************************************************************************
** ����: tGetDate( )                                                        **
** ����: void                                                               **
** ���:                                                                    **
**      char *pcDate ( ��Ž���ִ���ַ )                                   **
**      char *pcFlag ( ���ڷָ���,���ַ�����ʽ���� )                        **
**      time_t tTime ( Ҫת�ɱ�׼���ڸ�ʽ��ԭʼtime,ȡ��ǰ����Ϊ<0����ֵ )  **
** ���Σ�                                                                   **
**      char *pcDate                                                        **
** ����ֵ����                                                               **
** ���ܣ�����ǰ���ڰ�YYYYMMDD��ʽ������ַ�����,������֮���Էָ����ֿ�      **
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
** ����: tGetTime( )                                                        **
** ����: void                                                               **
** ���:                                                                    **
**      char *pcTime ( ��Ž���ִ���ַ )                                   **
**      char *pcFlag ( ʱ��ָ��� )                                         **
**      time_t tTime ( Ҫת�ɱ�׼ʱ���ʽ��ԭʼtimeֵ,ȡ��ǰ����Ϊ<0����ֵ )**
** ����:                                                                    **
**      char *pcTime                                                        **
** ����ֵ����hhmmss��ӡ��ǰʱ��                                             **
** ���ܣ�����ǰ�ڰ�hhmmss��ʽ������ַ�����,ʱ����֮���Էָ����ֿ�          **
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
** ����: tGetFullTime( )                                                    **
** ����: void                                                               **
** ���:                                                                    **
**      char *pcTime ( ��Ž���ִ���ַ )                                   **
**      char *pcFlag ( ʱ��ָ��� )                                         **
**      time_t tTime ( Ҫת�ɱ�׼ʱ���ʽ��ԭʼtimeֵ,ȡ��ǰ����Ϊ<0����ֵ )**
** ����:                                                                    **
**      char *pcTime                                                        **
** ����ֵ����hhmmss��ӡ��ǰʱ��                                             **
** ���ܣ�����ǰ�ڰ�hhmmss��ʽ������ַ�����,ʱ����֮���Էָ����ֿ�          **
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
** ����: tHaveDays( )                                                       **
** ����: long                                                               **
** ���:                                                                    **
**      long lDate1 ( ��ʼ���� )                                            **
**      long lDate2 ( ��ֹ���� )                                            **
** ���Σ�  ��                                                               **
** ����ֵ���������ڼ�����֮��ֵ��iDate1>iDate2, ��Ϊ����;                   ** 
**         ����, Ϊ��.                                                      **
**         iDateΪ��������ʽ��ʾ��"YYYYMMDD"��                              **
**         �磺int 19801217 ��ʾ1980��12��17��                              **
** ���ܣ�  �����������ڼ������                                             **
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
** ����: tGetMonthDays( )                                                   **
** ����: int                                                                **
** ��Σ�                                                                   **
**       Time *pstTime   -- �Խṹ��Time��ʽ��ʾ��ʱ��                      **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       long            -- ����ָ���·ݵ�����                              **
** ���ܣ�                                                                   **
**       ����ָ�����������µ�����                                           **
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
** ����: tDateCount( )                                                      **
** ����: long                                                               **
** ��Σ�                                                                   **
**       char *pcDate1   -- ��YYYYMMDD��ʽ��ʾ������                        **
**       char *pcDate2   -- ��YYYYMMDD��ʽ��ʾ������                        **
** ����:                                                                    **
**       NA                                                                 **
** ����ֵ��                                                                 **
**       long   -- ������Date1��Date2������������                           **
**       0xFFFF -- Date1��Date2��� > 1��                                   **
** ���ܣ�                                                                   **
**       ���������Date1��Date2������������                                 **
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

    /* Date1��Date2�������1�� */
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
** ����: tHaveSecs( )                                                       **
** ����: int                                                                **
** ���:                                                                    **
**      int iTime1 ( ��ʼʱ�� )                                             **
**      int iTime2 ( ��ֹʱ�� )                                             **
** ���Σ�  ��                                                               **
** ����ֵ��һ��������ʱ��֮��ֵ��iTime1>iTime2, ��Ϊ����;                   ** 
**         ����, Ϊ��.                                                      **
** ���ܣ�  ����ͬһ��������ʱ��֮�������.                                  **
*****************************************************************************/
int     tHaveSecs( int iTime1, int iTime2 )
{
    long    lSec1, lSec2;


    lSec1 = ( iTime1 / 10000 % 100 ) * 3600 + ( iTime1 / 100 % 100 ) * 60 + iTime1 % 100;
    lSec2 = ( iTime2 / 10000 % 100 ) * 3600 + ( iTime2 / 100 % 100 ) * 60+ iTime2 % 100;

    return( lSec2 - lSec1 );
}


/*****************************************************************************
** ����: tTime_AddDay( )                                                    **
** ����: void                                                               **
** ���:                                                                    **
**      char *psDate ( ԭʼ���� )                                           **
**      int iDays ( ԭʼ�������Ӧ��ĳһ��Ĳ�ֵ )                          **
** ����:  char *psDate ( ԭʼ�������Ӧ��ĳһ��Bufָ��)                     **
** ����ֵ����                                                               ** 
** ���ܣ�����һԭʼ����ǰn������������ַ�����                              **
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
** ����: tAddMonth( )                                                       **
** ����: void                                                               **
** ���:                                                                    **
**      char *pcDate ( ԭʼ���� )                                           **
**      int iMonths ( ԭʼ�������Ӧ��ĳһ�µĲ�ֵ )                        **
** ����:  char *pcDate ( ԭʼ�������Ӧ��ĳһ��Bufָ��)                     **
** ����ֵ����                                                               ** 
** ���ܣ�����һԭʼ����ǰn������������ַ�����                              **
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
** ����:    tSleep( )                                                       **
** ����:    void                                                            **
** ���:                                                                    **
**      int iMsecond ( ���ߵ�ʱ��,΢�뼶 )                                  **
** ����:  ��                                                                **
** ����ֵ����                                                               ** 
** ���ܣ�����������ָ����ʱ��                                               **
*****************************************************************************/
void    tSleep( int iMsecond )
{
    struct timeval  stTime;

    
    stTime.tv_sec = 0L;
    stTime.tv_usec = (long)iMsecond * 1000;

    select( 0, NULL, NULL, NULL, &stTime );
}

/*****************************************************************************
** ����:    tIsMonthEnd( )                                                  **
** ����:    int                                                             **
** ���:                                                                    **
**      char *psDate ( ���� )                                               **
** ����:  ��                                                                **
** ����ֵ���ɹ�����0�����򷵻�-1                                            ** 
** ���ܣ��ж������Ƿ�Ϊ��ĩ���һ��                                         **
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
** ����:    tIsYearEnd( )                                                   **
** ����:    int                                                             **
** ���:                                                                    **
**      char *psDate ( ���� )                                               **
** ����:  ��                                                                **
** ����ֵ���ɹ�����0�����򷵻�-1                                            ** 
** ���ܣ��ж������Ƿ�Ϊ��ĩ���һ��                                         **
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
** ����:    tGetWeek( )                                                     **
** ����:    int                                                             **
** ���:                                                                    **
**      char *pcDate ( ���� )                                               **
** ����:  ��                                                                **
** ����ֵ���ɹ��������ڼ���1-7�������򷵻�-1                                ** 
** ���ܣ���ȡ��ǰ���������ڼ�                                               **
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
** ����:    tGetWeek( )                                                     **
** ����:    void                                                            **
** ���:                                                                    **
**      char *pcTimeStamp ( ��ʽΪ: YYYYMMDDHHMISS ��ʱ��� )                **
**      int pciSec ( ���� )                                                 **
** ����: char *pcNextTimeStamp ������ָ�������� pciSec ���ʱ�����           **
** ���ܣ���ȡһ�� �ַ������͵�ʱ�������ָ������ ����ַ���ʱ���               **
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
    
    /*��ʱ���pcTimeStamp �в�ֳ� ������ ʱ����*/
    tStrCpy( sYear , pcTimeStamp,    4 );
    tStrCpy( sMonth, pcTimeStamp+4,  2 );
    tStrCpy( sDay  , pcTimeStamp+6,  2 );
    tStrCpy( sHour , pcTimeStamp+8,  2 );
    tStrCpy( sMin  , pcTimeStamp+10, 2 );
    tStrCpy( sSec  , pcTimeStamp+12, 2 );
    
    /*����ֳ�����������ʱ���� ��ֵ�� �⺯���Դ���ʱ��ṹ�壺stTime��*/
    stTime.tm_year = atoi( sYear  )-1900;
    stTime.tm_mon  = atoi( sMonth )-1;
    stTime.tm_mday = atoi( sDay   );
    stTime.tm_hour = atoi( sHour  );
    stTime.tm_min  = atoi( sMin   );
    stTime.tm_sec  = atoi( sSec   );
    
    /*��ʱ�䣨ת���룩 ���� ָ������*/
    iSec = mktime(&stTime);
    iSec += pciSec;
    /*������ ת��Ϊ ʱ��ṹ��*/
    p = localtime(&iSec);
    /*ʱ��ṹ��ת��Ϊ �ַ���ʱ���*/
    sprintf(pcNextTimeStamp,"%04d%02d%02d%02d%02d%02d", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}