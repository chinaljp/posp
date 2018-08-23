/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tools.h
 * Author: feng.gaoo
 *
 * Created on 2016年12月4日, 下午5:43
 */

#ifndef TOOLS_H
#define TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

    /****************************/
    /*  常用数据类型            */
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

#define GETMESG(sWay) sWay == 'C'?"IC信用卡":(sWay == 'D'?"IC借记卡":(sWay == 'E'?"磁条借记卡":(sWay == 'F'?"磁条信用卡":(sWay == 'Q'?"快捷消费信用卡":"快捷消费借记卡")))) //add by GJQ at 20180613 检查限额交易流，进行更新限额累计日志打印使用

    /* 字符串操作api */
    /* 为函数tAsc2Bcd( )的入参cAlign定义 */
#define BLANK       0x02        /* 用空格填充   */
#define ZERO        0x00        /* 用字符0填充  */
#define LEFT_ALIGN  0x01            /* 左对齐       */
#define RIGHT_ALIGN 0x00            /* 右对齐       */

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

    /* 四舍五入	 */
    void tMake(double *pdVar, int iBitNum, char cMakeFlag);

    /* time.c */

    /* 时间结构体 */
    typedef struct tagTime {
        int iUSec; /* 微秒 */
        int iMSec; /* 毫秒 */
        int iSec; /* 秒   */
        int iMin; /* 分   */
        int iHour; /* 时   */
        int iDay; /* 日   */
        int iMonth; /* 月   */
        int iYear; /* 年   */
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
    
    /* 查找tag值 */
    int tFindTag( unsigned short tag, unsigned char *value, short *length, unsigned char *buffer, short bufLen);
#ifdef __cplusplus
}
#endif

#endif /* TOOLS_H */

