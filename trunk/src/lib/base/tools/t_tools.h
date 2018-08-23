/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tools.h
 * Author: feng.gaoo
 *
 * Created on 2016��12��4��, ����5:43
 */

#ifndef TOOLS_H
#define TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

    /****************************/
    /*  ������������            */
    /****************************/
    typedef int BOOL_T;
    typedef unsigned char UCHAR;
    typedef unsigned long ULONG;

#define  EPSILON    0.000001
#define  DBL_ZERO(a) ((a) < EPSILON) && ((a) > -EPSILON)
#define  DBL_EQ(a,b) (((a)-(b)) < EPSILON &&((a)-(b)) > -EPSILON)
#define  DBL_CMP(a,b) (((a)-(b)) > EPSILON )

#define MEMSET(s)     memset(s,0,sizeof(s))
#define MEMSET_P(p,size)     memset(p,0,sizeof(size))
#define MEMSET_D(d)     d=0.00L
#define MEMSET_ST(s)     memset(&s,0,sizeof(s))

#define GETMESG(sWay) sWay == 'C'?"IC���ÿ�":(sWay == 'D'?"IC��ǿ�":(sWay == 'E'?"������ǿ�":(sWay == 'F'?"�������ÿ�":(sWay == 'Q'?"����������ÿ�":"������ѽ�ǿ�")))) //add by GJQ at 20180613 ����޶���������и����޶��ۼ���־��ӡʹ��

    /* �ַ�������api */
    /* Ϊ����tAsc2Bcd( )�����cAlign���� */
#define BLANK       0x02        /* �ÿո����   */
#define ZERO        0x00        /* ���ַ�0���  */
#define LEFT_ALIGN  0x01            /* �����       */
#define RIGHT_ALIGN 0x00            /* �Ҷ���       */

    char *tTrim(char *pcStr);
    void tStrCpy(void *pvTag, void *pvSrc, int iLen);
    void tAsc2Bcd(unsigned char *pcBcd, unsigned char *pcAsc, int iLen, char cAlign);
    void tBcd2Asc(unsigned char *pcAsc, unsigned char *pcBcd, int iLen, char cAlign);
    void tCutOneField(char **ppcStart, char *pcBuf);
    int tSeperate(char *pcStr, char *pcPtr[], int iFieldCnt, char cDelim);
    void tGetOneField(char **ppcStart, char cDelim, char *pcBuf);
    void tLower(const char* pcStr, char* pcOutStr);
    void tUpper(const char* pcStr, char* pcOutStr);
    void tScreenStr(char *pcStr, int iLen, char cChar, int iStart, int iEnd);

    /* ��������	 */
    void tMake(double *pdVar, int iBitNum, char cMakeFlag);

    /* time.c */

    /* ʱ��ṹ�� */
    typedef struct tagTime {
        int iUSec; /* ΢�� */
        int iMSec; /* ���� */
        int iSec; /* ��   */
        int iMin; /* ��   */
        int iHour; /* ʱ   */
        int iDay; /* ��   */
        int iMonth; /* ��   */
        int iYear; /* ��   */
    } Time;
    void tGetDate(char *pcDate, char *pcFlag, time_t tTime);
    void tGetTime(char *pcTime, char *pcFlag, time_t tTime);
    void tGetFullTime(char *pcFullTime, char *pcFlag, time_t tTime);
    long tHaveDays(long iDate1, long iDate2);
    int tGetMonthDays(Time *pstTime);
    long tDateCount(char *pcDate1, char *pcDate2);
    int tHaveSecs(int iTime1, int iTime2);
    void tAddDay(char *psDate, int iDays);
    void tAddMonth(char *pcDate, int iMonths);
    void tSleep(int iMsecond);
    int tIsMonthEnd(char *pcDate);
    int tIsYearEnd(char *pcDate);
    int tGetWeek(char *pcDate);
    
    /* ����tagֵ */
    int tFindTag( unsigned short tag, unsigned char *value, short *length, unsigned char *buffer, short bufLen);
#ifdef __cplusplus
}
#endif

#endif /* TOOLS_H */

