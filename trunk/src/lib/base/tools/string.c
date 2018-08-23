/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "t_tools.h"
#include "t_log.h"

static char g_sTmp[2048];

/*****************************************************************************
 ** 函数: tTrim( )                                                           **
 ** 类型: char *                                                             **
 ** 入参:                                                                    **
 **      char *pcStr ( 要处理的字符串 )                                      **
 ** 出参:                                                                    **
 **      char *pcStr ( 处理后的字符串 )                                      **
 ** 返回值： 字符串的起始地址                                                **
 ** 功能：删除字符串前后的空格和缩进键和回车符,pcStr指向的字符串必须         **
 **       以'\0'结尾                                                         **
 *****************************************************************************/
char *tTrim(char *pcStr) {
    char *pcTmp = NULL, *pcTmp1 = NULL;
    int iLen = 0;

    for (pcTmp = pcStr; *pcTmp != '\0'; pcTmp++)
        if (*pcTmp != ' ' && *pcTmp != '\t' &&
                *pcTmp != '\n' && *pcTmp != '\r')
            break;
    if (*pcTmp == '\0') {
        pcStr[0] = '\0';
        return ( pcStr);
    }
    for (pcTmp1 = pcStr + strlen(pcStr) - 1; *pcTmp1 != '\0'; pcTmp1--)
        if (*pcTmp1 != ' ' && *pcTmp1 != '\t' &&
                *pcTmp1 != '\n' && *pcTmp1 != '\r')
            break;

    iLen = pcTmp1 - pcTmp + 1;
    memmove(pcStr, pcTmp, iLen);
    pcStr[iLen] = '\0';

    return ( pcStr);
}

/*****************************************************************************
 ** 函数：tStrCpy( )                                                         **
 ** 类型：void                                                               **
 ** 入参:                                                                    **
 **      void *pvTag ( 目标字符串 )                                          **
 **      void *pvSrc ( 源字符串 )                                            **
 **      int iLen ( 拷贝长度 )                                               **
 ** 出参: 无                                                                 **
 ** 返回值： 无                                                              **
 ** 功能: 将源字符串内容拷贝至目标字符串中,在目标字符串尾自动添加'\0'        **
 *****************************************************************************/
void tStrCpy(void *pvTag, void *pvSrc, int iLen) {
    memcpy(pvTag, pvSrc, iLen);
    ((char *) pvTag)[iLen] = '\0';
}

/*****************************************************************************
 ** 函数：tAsc2Bcd( )                                                        **
 ** 类型：void                                                               **
 ** 入参:                                                                    **
 **      unsigned char *pcAsc  ASCII数据缓冲区 )                             **
 **      int iLen (ASCII数据长度 )                                           **
 **      char cAlign ( 对齐方式 )                                            **
 ** 出参:                                                                    **
 **      unsigned char *pcBcd ( BCD数据缓冲区 )                              **
 ** 返回值： 无                                                              **
 ** 功能: 将ASCII字符串转换成BCD编码的数据                                   **
 *****************************************************************************/
void tAsc2Bcd(unsigned char *pcBcd, unsigned char *pcAsc, int iLen, char cAlign) {
    int i, flag = 0;
    unsigned char ch;

    memset(pcBcd, 0, (iLen + 1) / 2);

    if ((iLen % 2) && cAlign == LEFT_ALIGN)
        flag = 1;

    for (i = 0; i < iLen; i++) {
        if (pcAsc[i] >= 'a')
            ch = pcAsc[i] - 'a' + 10;
        else if (pcAsc[i] >= 'A')
            ch = pcAsc[i] - 'A' + 10;
        else if (pcAsc[i] >= '0')
            ch = pcAsc[i] - '0';
        else
            ch = 0;
        if ((i + flag) % 2)
            pcBcd[(i + flag) / 2] |= (ch & 0x0F);
        else
            pcBcd[(i + flag) / 2] |= (ch << 4);
    }
}

/*****************************************************************************
 ** 函数：tBcd2Asc( )                                                        **
 ** 类型：void                                                               **
 ** 入参:                                                                    **
 **      unsigned char *pcBcd ( BCD数据缓冲区 )                              **
 **      int iLen (ASCII数据长度 )                                           **
 **      char cAlign ( 对齐方式 )                                            **
 ** 出参:                                                                    **
 **      unsigned char *pcAsc  ASCII数据缓冲区 )                             **
 ** 返回值： 无                                                              **
 ** 功能: 将BCD编码的数据转换成ASCII编码的数据                               **
 *****************************************************************************/
void tBcd2Asc(unsigned char *pcAsc, unsigned char *pcBcd, int iLen, char cAlign) {
    int i, flag = 0;

    if ((iLen % 2) && cAlign == LEFT_ALIGN)
        flag = 1;

    for (i = 0; i < iLen; i++) {
        if ((i + flag) % 2)
            pcAsc[i] = pcBcd[(i + flag) / 2] & 0x0F;
        else
            pcAsc[i] = (pcBcd[(i + flag) / 2] >> 4);
        pcAsc[i] += (pcAsc[i] > 9) ? ('A' - 10) : '0';
    }
}

/*****************************************************************************
 ** 函数: tStrPadL( )                                                        **
 ** 类型: void                                                               **
 ** 入参：                                                                   **
 **       char *pcStr            -- 待填充字符串                             **
 **       int iLen               -- 填充完成后字符串的长度                   **
 **       UCHAR cFldAttr         -- 域属性: 包括补零/补空格,左对齐/右对齐    **
 ** 出参:                                                                    **
 **       char *pcStr            -- 填充完成后的字符串                       **
 ** 返回值：                                                                 **
 **       NA                                                                 **
 ** 功能：                                                                   **
 **       字符串对齐补充，填充内容及对齐方式由cFldAttr指定。                 **
 **       ( tTrim的相反功能 )                                                **
 ** 创建日期:                                                                **
 **       2013.02.27 钟读营 函数创建                                         **
 ** 修改日期:                                                                **
 **                                                                          **
 *****************************************************************************/

void tStrPad(char *pcStr, int iLen, UCHAR cFldAttr) {
    int iDataLen, iStep;
    char cPad;

    iDataLen = strlen(pcStr);
    iStep = iLen - iDataLen;
    if (iStep <= 0)
        return;

    if (cFldAttr & BLANK) /* 补空格 */ {
        cPad = ' ';
    } else /* 补零   */ {
        cPad = '0';
    }

    if (cFldAttr & LEFT_ALIGN) /* 左对齐 */ {
        memset(pcStr + iDataLen, cPad, iStep);
        *(pcStr + iLen) = '\0';
    } else /* 右对齐 */ {
        memmove(pcStr + iStep, pcStr, iDataLen + 1);
        memset(pcStr, cPad, iStep);
    }

    return;
}

/*****************************************************************************
 ** 函数: tCutOneField( )                                                    **
 ** 类型: void                                                               **
 ** 入参:                                                                    **
 **      char **ppcStart ( 要处理的字符串起始位置 )                          **
 **      char *pcBuf ( 存放子串的缓冲区指针 )                                **
 ** 出参:                                                                    **
 **      char *pcBuf ( 存放子串的缓冲区指针 )                                **
 ** 返回值： char * 下一子串的首地址                                         **
 ** 功能：以','为分隔符从被处理的字符串中取出一个子串,本函数不会破坏原字符串 **
 **       如果字符串中含有"( )"字符序列当成不可分割的字符串处理              **
 *****************************************************************************/
void tCutOneField(char **ppcStart, char *pcBuf) {
    char *pcCur;


    pcCur = *ppcStart;
    *pcBuf = '\0';

    for (; *pcCur != '\0'; pcCur++) {
        if (*pcCur == ',')
            break;
        if (*pcCur == '(') {
            if ((pcCur = strchr(pcCur + 1, ')')) == NULL) {
                *ppcStart = NULL;
                return;
            }
        }
    }

    memcpy(pcBuf, *ppcStart, pcCur - *ppcStart);
    pcBuf[pcCur - *ppcStart] = '\0';
    tTrim(pcBuf);

    if (*pcCur != '\0')
        *ppcStart = pcCur + 1;
    else
        *ppcStart = NULL;
}

/*****************************************************************************
 ** 函数: tSeperate( )                                                       **
 ** 类型: int                                                                **
 ** 入参:                                                                    **
 **      char *pcStr ( 要处理的字符串 )                                      **
 **      char *pcPtr[] ( 指向拆分后各个字串的的指针数组 )                    **
 **      int  iFieldCnt ( 指针数组元素个数 )                                 **
 **      char cDelim ( 分隔符,可以是任意字符 )                               **
 ** 出参:                                                                    **
 **      char *pcPtr[] ( 指向拆分后各个字串的的指针数组 )                    **
 ** 返回值： 拆分出的字符串个数                                              **
 ** 功能：将字符串按分隔符拆分若干个字符串,本函数会破坏原字符串              **
 *****************************************************************************/
int tSeperate(char *pcStr, char *pcPtr[], int iFieldCnt, char cDelim) {
    int iField = 0;
    char *pcTmp;


    tTrim(pcStr);
    pcPtr[0] = pcStr;
    pcTmp = pcStr;

    while (*pcTmp != '\0' && iField < iFieldCnt - 1) {
        if (*pcTmp == cDelim) {
            *pcTmp = 0x00;
            tTrim(pcPtr[iField]);
            pcPtr[++iField] = pcTmp + 1;
            tTrim(pcTmp + 1);
        }
        pcTmp++;
    }

    return ( iField + 1);
}

/*****************************************************************************
 ** 函数: tGetOneField( )                                                    **
 ** 类型: void                                                               **
 ** 入参:                                                                    **
 **      char **ppcStart ( 要处理的字符串的指针 )                            **
 **      char cDelim ( 分隔符,可以是任意字符 )                               **
 **      char *pcBuf ( 存储一个域信息的缓冲区 )                              **
 ** 出参:                                                                    **
 **      char **ppcStart ( 取完一个域后, ppcStart新的位置 )                  **
 ** 返回值： 字符串中域的个数                                                **
 ** 功能：按分隔符从被处理的字符串中取出一个子串,本函数不会破坏原字符串      **
 ** 修改日期:                                                                **
 **      2013.02.28 钟读营 修改出参注释                                      **
 *****************************************************************************/
void tGetOneField(char **ppcStart, char cDelim, char *pcBuf) {
    char *pcCur;

    pcCur = *ppcStart;
    *pcBuf = '\0';

    for (; *pcCur != '\0'; pcCur++) {
        if (*pcCur == cDelim)
            break;
    }

    memcpy(pcBuf, *ppcStart, pcCur - *ppcStart);
    pcBuf[pcCur - *ppcStart] = '\0';
    if (*pcCur != '\0') {
        *ppcStart = pcCur + 1;
    } else
        *ppcStart = NULL;
}

//字符串转换小写

void tLower(const char* pcStr, char* pcOutStr) {
    char sTemp[1024] = {0};
    int i, iLen = strlen(pcStr);

    for (i = 0; i < iLen; i++) {
        sTemp[i] = (char) (tolower(pcStr[i]));
    }
    tStrCpy(pcOutStr, sTemp, iLen);
}

//字符串转换大写 

void tUpper(const char* pcStr, char* pcOutStr) {
    char sTemp[1024] = {0};
    int i, iLen = strlen(pcStr);
    for (i = 0; i < iLen; i++) {
        sTemp[i] = (char) (toupper(pcStr[i]));
    }
    tStrCpy(pcOutStr, sTemp, iLen);
}

//屏蔽字符串，前6，后4

void tScreenStr(char *pcStr, int iLen, char cChar, int iStart, int iEnd) {
    char sTmp[128 + 1] = {0};
    if (iLen > 128)
        return;
    memset(sTmp, cChar, sizeof (sTmp));
    memcpy(sTmp, pcStr, iStart);
    memcpy(sTmp + iLen - iEnd, pcStr + iLen - iEnd, iEnd);
    tStrCpy(pcStr, sTmp, iLen);
}