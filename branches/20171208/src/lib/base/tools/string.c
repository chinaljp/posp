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
 ** ����: tTrim( )                                                           **
 ** ����: char *                                                             **
 ** ���:                                                                    **
 **      char *pcStr ( Ҫ������ַ��� )                                      **
 ** ����:                                                                    **
 **      char *pcStr ( �������ַ��� )                                      **
 ** ����ֵ�� �ַ�������ʼ��ַ                                                **
 ** ���ܣ�ɾ���ַ���ǰ��Ŀո���������ͻس���,pcStrָ����ַ�������         **
 **       ��'\0'��β                                                         **
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
 ** ������tStrCpy( )                                                         **
 ** ���ͣ�void                                                               **
 ** ���:                                                                    **
 **      void *pvTag ( Ŀ���ַ��� )                                          **
 **      void *pvSrc ( Դ�ַ��� )                                            **
 **      int iLen ( �������� )                                               **
 ** ����: ��                                                                 **
 ** ����ֵ�� ��                                                              **
 ** ����: ��Դ�ַ������ݿ�����Ŀ���ַ�����,��Ŀ���ַ���β�Զ����'\0'        **
 *****************************************************************************/
void tStrCpy(void *pvTag, void *pvSrc, int iLen) {
    memcpy(pvTag, pvSrc, iLen);
    ((char *) pvTag)[iLen] = '\0';
}

/*****************************************************************************
 ** ������tAsc2Bcd( )                                                        **
 ** ���ͣ�void                                                               **
 ** ���:                                                                    **
 **      unsigned char *pcAsc  ASCII���ݻ����� )                             **
 **      int iLen (ASCII���ݳ��� )                                           **
 **      char cAlign ( ���뷽ʽ )                                            **
 ** ����:                                                                    **
 **      unsigned char *pcBcd ( BCD���ݻ����� )                              **
 ** ����ֵ�� ��                                                              **
 ** ����: ��ASCII�ַ���ת����BCD���������                                   **
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
 ** ������tBcd2Asc( )                                                        **
 ** ���ͣ�void                                                               **
 ** ���:                                                                    **
 **      unsigned char *pcBcd ( BCD���ݻ����� )                              **
 **      int iLen (ASCII���ݳ��� )                                           **
 **      char cAlign ( ���뷽ʽ )                                            **
 ** ����:                                                                    **
 **      unsigned char *pcAsc  ASCII���ݻ����� )                             **
 ** ����ֵ�� ��                                                              **
 ** ����: ��BCD���������ת����ASCII���������                               **
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
 ** ����: tStrPadL( )                                                        **
 ** ����: void                                                               **
 ** ��Σ�                                                                   **
 **       char *pcStr            -- ������ַ���                             **
 **       int iLen               -- �����ɺ��ַ����ĳ���                   **
 **       UCHAR cFldAttr         -- ������: ��������/���ո�,�����/�Ҷ���    **
 ** ����:                                                                    **
 **       char *pcStr            -- �����ɺ���ַ���                       **
 ** ����ֵ��                                                                 **
 **       NA                                                                 **
 ** ���ܣ�                                                                   **
 **       �ַ������벹�䣬������ݼ����뷽ʽ��cFldAttrָ����                 **
 **       ( tTrim���෴���� )                                                **
 ** ��������:                                                                **
 **       2013.02.27 �Ӷ�Ӫ ��������                                         **
 ** �޸�����:                                                                **
 **                                                                          **
 *****************************************************************************/

void tStrPad(char *pcStr, int iLen, UCHAR cFldAttr) {
    int iDataLen, iStep;
    char cPad;

    iDataLen = strlen(pcStr);
    iStep = iLen - iDataLen;
    if (iStep <= 0)
        return;

    if (cFldAttr & BLANK) /* ���ո� */ {
        cPad = ' ';
    } else /* ����   */ {
        cPad = '0';
    }

    if (cFldAttr & LEFT_ALIGN) /* ����� */ {
        memset(pcStr + iDataLen, cPad, iStep);
        *(pcStr + iLen) = '\0';
    } else /* �Ҷ��� */ {
        memmove(pcStr + iStep, pcStr, iDataLen + 1);
        memset(pcStr, cPad, iStep);
    }

    return;
}

/*****************************************************************************
 ** ����: tCutOneField( )                                                    **
 ** ����: void                                                               **
 ** ���:                                                                    **
 **      char **ppcStart ( Ҫ������ַ�����ʼλ�� )                          **
 **      char *pcBuf ( ����Ӵ��Ļ�����ָ�� )                                **
 ** ����:                                                                    **
 **      char *pcBuf ( ����Ӵ��Ļ�����ָ�� )                                **
 ** ����ֵ�� char * ��һ�Ӵ����׵�ַ                                         **
 ** ���ܣ���','Ϊ�ָ����ӱ�������ַ�����ȡ��һ���Ӵ�,�����������ƻ�ԭ�ַ��� **
 **       ����ַ����к���"( )"�ַ����е��ɲ��ɷָ���ַ�������              **
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
 ** ����: tSeperate( )                                                       **
 ** ����: int                                                                **
 ** ���:                                                                    **
 **      char *pcStr ( Ҫ������ַ��� )                                      **
 **      char *pcPtr[] ( ָ���ֺ�����ִ��ĵ�ָ������ )                    **
 **      int  iFieldCnt ( ָ������Ԫ�ظ��� )                                 **
 **      char cDelim ( �ָ���,�����������ַ� )                               **
 ** ����:                                                                    **
 **      char *pcPtr[] ( ָ���ֺ�����ִ��ĵ�ָ������ )                    **
 ** ����ֵ�� ��ֳ����ַ�������                                              **
 ** ���ܣ����ַ������ָ���������ɸ��ַ���,���������ƻ�ԭ�ַ���              **
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
 ** ����: tGetOneField( )                                                    **
 ** ����: void                                                               **
 ** ���:                                                                    **
 **      char **ppcStart ( Ҫ������ַ�����ָ�� )                            **
 **      char cDelim ( �ָ���,�����������ַ� )                               **
 **      char *pcBuf ( �洢һ������Ϣ�Ļ����� )                              **
 ** ����:                                                                    **
 **      char **ppcStart ( ȡ��һ�����, ppcStart�µ�λ�� )                  **
 ** ����ֵ�� �ַ�������ĸ���                                                **
 ** ���ܣ����ָ����ӱ�������ַ�����ȡ��һ���Ӵ�,�����������ƻ�ԭ�ַ���      **
 ** �޸�����:                                                                **
 **      2013.02.28 �Ӷ�Ӫ �޸ĳ���ע��                                      **
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

//�ַ���ת��Сд

void tLower(const char* pcStr, char* pcOutStr) {
    char sTemp[1024] = {0};
    int i, iLen = strlen(pcStr);

    for (i = 0; i < iLen; i++) {
        sTemp[i] = (char) (tolower(pcStr[i]));
    }
    tStrCpy(pcOutStr, sTemp, iLen);
}

//�ַ���ת����д 

void tUpper(const char* pcStr, char* pcOutStr) {
    char sTemp[1024] = {0};
    int i, iLen = strlen(pcStr);
    for (i = 0; i < iLen; i++) {
        sTemp[i] = (char) (toupper(pcStr[i]));
    }
    tStrCpy(pcOutStr, sTemp, iLen);
}

//�����ַ�����ǰ6����4

void tScreenStr(char *pcStr, int iLen, char cChar, int iStart, int iEnd) {
    char sTmp[128 + 1] = {0};
    if (iLen > 128)
        return;
    memset(sTmp, cChar, sizeof (sTmp));
    memcpy(sTmp, pcStr, iStart);
    memcpy(sTmp + iLen - iEnd, pcStr + iLen - iEnd, iEnd);
    tStrCpy(pcStr, sTmp, iLen);
}