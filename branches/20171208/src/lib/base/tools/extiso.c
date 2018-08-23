/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "t_extiso.h"
#include "t_log.h"
#include "t_tools.h"

extern IsoTable iso_87[];

/*****************************************************************************
 ** 函数: tInitIso( )                                                        **
 ** 类型: int                                                                **
 ** 入参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 **      unsigned char *pcIsoBuf ( 存放8583数据报文的缓冲区指针 )            **
 **      int iIsoBufLen ( 存放8583报文的缓冲区长度 )                         **
 **      int iAscVar ( 8583报文中长度信息编码格式,1为ASCII,0为BCD)           **
 **      IsoTable *pstIsoTable ( 准备使用的8583报文各个域的属性描述表的地址 )**
 ** 出参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 ** 返回值： 无                                                              **
 ** 功能：使用参数1至参数4,初始化拆组8583数据用的数据结构(参数0)             **
 *****************************************************************************/
void tInitIso(IsoData *pstIso, unsigned char *pcIsoBuf, int iIsoBufLen, int iAscVar, IsoTable *pstIsoTable) {
    memset(pstIso->pstFld, 0, sizeof (pstIso->pstFld));
    pstIso->hOff = 0;
    pstIso->pcBuf = pcIsoBuf;
    pstIso->hMaxLen = iIsoBufLen;
    pstIso->cAscVar = iAscVar;
    if (pstIsoTable != NULL)
        pstIso->pstIsoTable = pstIsoTable;
    else
        pstIso->pstIsoTable = iso_87;
}

/*****************************************************************************
 ** 函数: tInitIso( )                                                        **
 ** 类型: int                                                                **
 ** 入参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 ** 出参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 ** 返回值： 无                                                              **
 ** 功能： 清空拆组8583数据用的数据结构中的8583报文数据信息                  **
 *****************************************************************************/
void tClearIso(IsoData *pstIso) {
    memset(pstIso->pcBuf, 0, pstIso->hMaxLen);
    memset(pstIso->pstFld, 0, sizeof (pstIso->pstFld));
    pstIso->hOff = 0;
}

/*****************************************************************************
 ** 函数: tStr2Iso( )                                                        **
 ** 类型: int                                                                **
 ** 入参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 **      unsigned char *pcIn8583 ( 存放待拆解的8583数据报文的缓冲区指针 )    **
 **      int iInLen ( 待拆解的8583报文长度 )                                 **
 ** 出参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 ** 返回值： 成功 - 0,失败 - -1                                              **
 ** 功能：初步解析8583报文,将8583报文拷入IsoData中,根据IsoData中的8583报文   **
 **      域属性描述,解析出待拆解报文中有哪些域,域的长度以及域数据在报文中的  **
 **      位置,为逐个提取域数据做准备                                         **
 *****************************************************************************/
int tStr2Iso(IsoData *pstIso, unsigned char *pcIn8583, int iInLen) {
    unsigned char cLenType, cFldType, *pcBitMap, *pcData, *pcEndBuf;
    IsoTable *pstIsoTable;
    int i, j, k, iVarLen, iFldNo, iBitMapSize, iLen, iOff = 0;

    tClearIso(pstIso);
    pcEndBuf = pcIn8583 + iInLen;
    pstIsoTable = pstIso->pstIsoTable;
    /* 拆解1号域报文类型 */
    if (pstIsoTable[0].cType & BCD)
        /* IsoTable[0]是报文类型域 */
        iLen = (pstIsoTable[0].hLen + 1) / 2;
    else
        iLen = pstIsoTable[0].hLen;
    memcpy(pstIso->pcBuf, pcIn8583, iLen);
    pstIso->pstFld[0].hFlag = EXIST;
    pstIso->pstFld[0].hLen = pstIsoTable[0].hLen;
    pstIso->pstFld[0].hOff = iOff;
    iOff += iLen;
    /* 确定待拆解的8583报文中位图数据的位置 */
    pcBitMap = pcIn8583 + iLen;
    /* 确定位图数据的字节数 */
    if (pcBitMap[0] & 0x80)
        iBitMapSize = 16;
    else
        iBitMapSize = 8;

    /* 确定待拆解的8583报文中位元数据的起始位置 */
    pcData = pcBitMap + iBitMapSize;
    if (pcData > pcEndBuf)
        return ( -1);

    for (i = 0; i < iBitMapSize; i++) {
        /* 逐个域处理,从待拆解的8583报文中提取除每个位元数据及其长度信息 */
        for (j = 7; j >= 0; j--) {
            iFldNo = (i + 1) * 8 - j - 1;
            if ((pcBitMap[i] & (1 << j)) == 0 || iFldNo == 0) {
                continue;
            }
//            tLog(DEBUG, " GetBit FldNo=%d, Len=%d, Attr=%02x",
//                    iFldNo, pstIsoTable[iFldNo].hLen, pstIsoTable[iFldNo].cType);
            cFldType = pstIsoTable[iFldNo].cType;
            cLenType = cFldType & 0xC0;


            iVarLen = (cLenType >> 6) ? (cLenType >> 6) : 0;
            if (pcData + iVarLen > pcEndBuf) {
                tLog(DEBUG, "VarLen=%d", iVarLen);
                return ( -1);
            }
            if (cLenType != FIXED) {
                iLen = 0;
                /* 如果变长域长度信息为ASCII编码方式 */
                if (pstIso->cAscVar == ASC_LLVAR) {
                    for (k = 0; k < iVarLen; k++) {
                        iLen = iLen * 10 + *pcData - '0';
                        pcData++;
                    }
                }/* 如果变长域长度信息为BCD编码方式 */
                else {
                    for (k = 0; k < (iVarLen + 1) / 2; k++) {
                        iLen = iLen * 100 + *pcData - (*pcData >> 4) * 6;
                        pcData++;
                    }
                }

                if (iLen < 0 || iLen > pstIsoTable[iFldNo].hLen) {
                    return ( -2);
                }
            } else
                iLen = pstIsoTable[iFldNo].hLen;

            pstIso->pstFld[iFldNo].hLen = iLen;
            pstIso->pstFld[iFldNo].hOff = iOff;

            /* 如果是BCD编码方式 */
            if (cFldType & BCD)
                iLen = (iLen + 1) / 2;

            /* 如果是符号数据域 */
            if (cFldType & SYMBOL)
                iLen = iLen + 1;

            if (pcData + iLen > pcEndBuf) {
                tLog(DEBUG, "iLen=%d", iLen);
                return ( -1);
            }
            if (iOff + iLen > pstIso->hMaxLen)
                return ( -3);

            /* 从8583报文中提取出该域的数据 */
            memcpy(pstIso->pcBuf + iOff, pcData, iLen);
            iOff += iLen;
            pcData += iLen;

            pstIso->pstFld[iFldNo].hFlag = EXIST;
        }
    }

    pstIso->hOff = iOff;

    return ( 0);
}

/*****************************************************************************
 ** 函数: tIso2Str( )                                                        **
 ** 类型: int                                                                **
 ** 入参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 **      unsigned char *pcOut8583 ( 存放组装后的8583数据报文的缓冲区指针 )   **
 **      int iOutMaxLen ( 存放组装后的8583报文缓冲区的最大长度 )             **
 ** 出参:                                                                    **
 **      unsigned char *pcOut8583 ( 存放组装后的8583数据报文的缓冲区指针 )   **
 ** 返回值： 成功 - 0,失败 - -1                                              **
 ** 功能：根据IsoData中的8583报文域属性描述,以及存储在IsoData中的准备组装的  **
 **      域数据信息(有哪些域,域的长度以及域数据在IsoData的缓冲区中的偏移位置)**
 **      ,最后组装8583报文                                                   **
 *****************************************************************************/
int tIso2Str(IsoData *pstIso, unsigned char *pcOut8583, int iOutMaxLen) {
    unsigned char cLenType, cFldType, *pcBitMap, *pcData, *pcEndBuf, cBitInfo;
    IsoTable *pstIsoTable;
    int i, j, iVarLen, iFldNo, iLen, iBitMapSize = 8;


    pcEndBuf = pcOut8583 + iOutMaxLen;
    pstIsoTable = pstIso->pstIsoTable;

    /* 确定位图字节数 */
    for (i = 64; i < 128; i++) {
        if (pstIso->pstFld[i].hFlag == EXIST) {
            iBitMapSize = 16;
            break;
        }
    }

    /* 拷贝消息类型至8583报文中 */
    if (pstIso->pstIsoTable[0].cType & BCD)
        iLen = (pstIso->pstFld[0].hLen + 1) / 2;
    else
        iLen = pstIso->pstFld[0].hLen;
    memcpy(pcOut8583, pstIso->pcBuf + pstIso->pstFld[0].hOff, iLen);

    /* 确定位图数据和8583位元数据在8583报文中的起始位置 */
    pcBitMap = pcOut8583 + iLen;
    pcData = pcOut8583 + iLen + iBitMapSize;
    if (pcData > pcEndBuf)
        return (-1);

    for (i = 0; i < iBitMapSize; i++) {
        cBitInfo = 0;
        /* 逐个域处理,将每个位元数据及其长度信息写入8583报文 */
        for (j = 7; j >= 0; j--) {
            iFldNo = (i + 1) * 8 - j - 1;
            if (pstIso->pstFld[iFldNo].hFlag == NON_EXIST || iFldNo == 0)
                continue;

            //tLog(DEBUG, " SetBit FldNo=%d, Len=%d, Attr=%02x",
              //      iFldNo, pstIsoTable[iFldNo].hLen, pstIsoTable[iFldNo].cType);
            cBitInfo |= (1 << j);
            iLen = pstIso->pstFld[iFldNo].hLen;
            cFldType = pstIsoTable[iFldNo].cType;
            cLenType = cFldType & 0xC0;

            /* 确定变长域的长度信息 */
            iVarLen = (cLenType >> 6) ? ((cLenType >> 6) + pstIso->cAscVar) : 0;

            if ((pcData + iVarLen) > pcEndBuf)
                return (-1);

            /* 将域长度信息写入8583报文 */
            switch (cLenType) {
                case LLLVAR:
                    if (pstIso->cAscVar == ASC_LLVAR) {
                        *(pcData++) = iLen / 100 + '0';
                        *(pcData++) = iLen % 100 / 10 + '0';
                        *(pcData++) = iLen % 10 + '0';
                    } else {
                        *(pcData++) = iLen / 100;
                        *(pcData++) = iLen % 100 / 10 * 16 + iLen % 10;
                    }
                    break;
                case LLVAR:
                    if (pstIso->cAscVar == ASC_LLVAR) {
                        *(pcData++) = iLen / 10 + '0';
                        *(pcData++) = iLen % 10 + '0';
                    } else
                        *(pcData++) = iLen / 10 * 16 + iLen % 10;
                    break;
            }

            /* 如果是BCD编码方式 */
            if (cFldType & BCD)
                iLen = (iLen + 1) / 2;

            /* 如果是符号数据域 */
            if (cFldType & SYMBOL)
                iLen = iLen + 1;

            if ((pcData + iLen) > pcEndBuf)
                return ( -1);

            /* 将存在iso->buf中的域数据写入8583报文 */
            memcpy(pcData, pstIso->pcBuf + pstIso->pstFld[iFldNo].hOff, iLen);
            pcData += iLen;
        }
        pcBitMap[i] = cBitInfo;
    }

    if (iBitMapSize == 16)
        pcBitMap[0] |= 0x80;

    return ( pcData - pcOut8583);
}

/*****************************************************************************
 ** 函数: tIsoSetBit( )                                                      **
 ** 类型: int                                                                **
 ** 入参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 **      int iFldNo ( 8583报文的域编号 )                                     **
 **      char *pcStrIn ( 存放需要打包到8583报文指定域的数据缓冲区指针 )      **
 **      int iInLen ( 数据缓冲区中数据长度 )                                 **
 ** 出参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 ** 返回值： 成功 - 0,失败 - -1                                              **
 ** 功能：初步组装8583报文,根据IsoData中的8583报文域属性描述,将需要打包到8583**
 **      报文指定域的数据,做格式转换并存储在IsoData的缓冲区中,并产生待组装   **
 **      域数据信息(有哪些域,域的长度以及域数据在IsoData的缓冲区中的偏移位置)**
 **      位置,为最后组装8583报文做准备                                       **
 *****************************************************************************/
int tIsoSetBit(IsoData *pstIso, int iFldNo, char *pcStrIn, int iInLen) {
    unsigned char cLenType, cFldType, *pcData, sTmp[999];
    IsoTable *pstIsoTable;
    int iLen;


    if (iInLen <= 0)
        return ( 0);

    if (iInLen > 999)
        return ( -1);

    /* Isotable数组脚标比对应的8583域号小1,IsoTable[0]是报文类型域,IsoTable[1]是主帐号域 */
    iFldNo = iFldNo - 1;
    if (iFldNo < 0 || iFldNo > 127)
        return ( -1);


    pstIsoTable = pstIso->pstIsoTable;
    cFldType = pstIsoTable[iFldNo].cType;
    cLenType = cFldType & 0xC0;

    /* 取出该8583域的最大长度   */
    iLen = pstIsoTable[iFldNo].hLen;

    /* 如果是符号数据域,域长度减去符号长度 */
    if (cFldType & SYMBOL)
        iInLen--;

    /* 如果是变长域且输入数据长度小于域定义最大长度 */
    if (cLenType != FIXED && (iInLen < iLen))
        iLen = iInLen;

    /* 域数据长度按数据未做BCD压缩的长度取值 */
    pstIso->pstFld[iFldNo].hFlag = EXIST;
    pstIso->pstFld[iFldNo].hLen = iLen;
    pstIso->pstFld[iFldNo].hOff = pstIso->hOff;
    pcData = pstIso->pcBuf + pstIso->hOff;


    /* 如果是二进制数据域 */
    if (cFldType & BINARY) {
        memcpy(pcData, pcStrIn, iLen);
        pstIso->hOff += iLen;
        return ( 0);
    }

    /* 如果是符号域,inStr中必须含有借贷记符号 */
    if (cFldType & SYMBOL) {
        if ((pstIso->hOff + 1) > pstIso->hMaxLen)
            return (-1);
        *(pcData++) = *(pcStrIn++);
        (pstIso->hOff)++;
    }

    /* 如果是空格填充方式 */
    if (cFldType & BLANK)
        memset(sTmp, ' ', iLen);
    else
        memset(sTmp, '0', iLen);

    /* 如果是左对齐方式 */
    if (cFldType & LEFT_ALIGN)
        memcpy(sTmp, pcStrIn, iInLen);
    else
        memcpy(sTmp + ((iLen > iInLen) ? (iLen - iInLen) : 0), pcStrIn, iInLen);

    /* 如果是BCD编码方式 */
    if (cFldType & BCD) {
        if ((pstIso->hOff + (iLen + 1) / 2) > pstIso->hMaxLen)
            return (-1);
        tAsc2Bcd(pcData, sTmp, iLen, (char) !(cFldType & 0x01));
        pstIso->hOff += (iLen + 1) / 2;
    }/* 如果是ASCII编码方式 */
    else {
        if ((pstIso->hOff + iLen) > pstIso->hMaxLen)
            return (-1);
        memcpy(pcData, sTmp, iLen);
        pstIso->hOff += iLen;
    }

    return (0);
}

/*****************************************************************************
 ** 函数: tIsoGetBit( )                                                      **
 ** 类型: int                                                                **
 ** 入参:                                                                    **
 **      IsoData *pstIso ( 拆组8583数据用的数据结构的地址 )                  **
 **      int iFldNo ( 8583报文的域编号 )                                     **
 **      char *pcStrOut ( 存放拆解出的8583报文指定域的数据缓冲区指针 )       **
 **      int iOutMaxLen ( 存放数据的缓冲区的最大数据域长度 )                 **
 ** 出参:                                                                    **
 **      char *pcStrOut ( 存放拆解出的8583报文指定域的数据缓冲区指针 )       **
 ** 返回值： 成功 - 0,失败 - -1                                              **
 ** 功能：最后拆解8583报文,根据IsoData中的8583报文域属性描述,域数据信息(有哪 **
 **      些域,域的长度以及域数据在IsoData的缓冲区中的偏移位置),提取报文指定域**
 **      的数据,做格式转换并存储在pcStrOut的缓冲区中                         **
 *****************************************************************************/
int tIsoGetBit(IsoData *pstIso, int iFldNo, char *pcStrOut, int iOutMaxLen) {
    unsigned char *pcData, cFldType;
    IsoTable *pstIsoTable;
    int iLen, iLenOut;


    /* 保证GetBit()调用失败时,strOut为空串 */
    pcStrOut[0] = '\0';

    /* Isotable数组脚标比对应的8583域号小1,IsoTable[0]是报文类型域,IsoTable[1]是主帐号域 */
    iFldNo = iFldNo - 1;
    if (iFldNo < 0 || iFldNo > 127)
        return ( -1);

    if (pstIso->pstFld[iFldNo].hFlag == NON_EXIST)
        return ( 0);

    pstIsoTable = pstIso->pstIsoTable;
    cFldType = pstIsoTable[iFldNo].cType;
    iLen = pstIso->pstFld[iFldNo].hLen;

    pcData = pstIso->pcBuf + pstIso->pstFld[iFldNo].hOff;

    /* 符号数据域输出时,比8583域长度多1字节符号+1字节'\0' */
    if (cFldType & SYMBOL) {
        *(pcStrOut++) = *(pcData++);
        iLenOut = iLen + 1;
    }/* 二进制数据域输出时,等于8583域长度 */
    else if (cFldType & BINARY)
        iLenOut = iLen;
        /* 其它类型数据域输出时,比8583域长度多1个字节'\0' */
    else
        iLenOut = iLen;

    if (iLenOut > iOutMaxLen)
        return ( -2);


    /* 如果是BCD编码方式 */
    if (cFldType & BCD)
        tBcd2Asc((unsigned char *) pcStrOut, pcData, iLen, (char) !(cFldType & 0x01));
        /* 如果是ASCII编码方式 */
    else
        memcpy(pcStrOut, pcData, iLen);

    pcStrOut[iLen] = '\0';

    return ( iLenOut);
}

int tGetIsoFld(IsoData *pstIso, int iFldNo, char *pcStrOut, int *piLen) {
    /* 保证GetBit()调用失败时,strOut为空串 */
    pcStrOut[0] = '\0';

    /* Isotable数组脚标比对应的8583域号小1,IsoTable[0]是报文类型域,IsoTable[1]是主帐号域 */
    iFldNo = iFldNo - 1;
    if (iFldNo < 0 || iFldNo > 127)
        return ( -1);

    if (pstIso->pstFld[iFldNo].hFlag == NON_EXIST)
        return ( 0);

    if (pstIso->pstFld[iFldNo].hLen > *piLen)
        return ( -2);

    memcpy(pcStrOut, pstIso->pcBuf + pstIso->pstFld[iFldNo].hOff, pstIso->pstFld[iFldNo].hLen);
    *piLen = pstIso->pstFld[iFldNo].hLen;

    return ( 1);
}


/* 框架函数 */
extern char *GetLogName();

void tDumpIso(IsoData *pstIsoData, char *pcTitle) {
    unsigned char cFldType;
    int i, iFldLen = 0, iLen = 0, iLenTmp = 0;
    IsoTable *pstIsoTable = NULL;
    Fld *pstFld = NULL;
    char sTime[14] = {0}, sDate[14] = {0}, sLogName[64] = {0};
    char *pcStr, sData[2048] = {0}, sTmp[1024] = {0};
    FILE *pstFp = NULL;
    struct timeval tv;

    pstIsoTable = pstIsoData->pstIsoTable;
    pstFld = pstIsoData->pstFld;

    //tGetDate(sDate, "-", -1);
    //snprintf(sLogName, sizeof (sLogName), "%s/log/%s.%s", getenv("HOME"), GetLogName(), sDate);
    tGetDate(sDate, "", -1);
    snprintf(sLogName, sizeof (sLogName), "%s/%s.%s", getenv("PACK_LOG"), GetLogName(), sDate);
  
    pstFp = fopen(sLogName, "a+");
    if (pstFp == NULL) {
        tLog(ERROR, "日志文件%s打开错! 请检查。", sLogName);
        return;
    }

    tGetDate(sDate, "/", -1);
    tGetTime(sTime, ":", -1);
    gettimeofday(&tv, NULL);
    fprintf(pstFp, "\n----------%s, 时间 %s %s %03d\n", pcTitle, sDate, sTime, (int) (tv.tv_usec / 1000));

    for (i = 0; i < 128; i++) {
        if (pstFld[i].hFlag == NON_EXIST)
            continue;
        //        if ( i == 1)
        //            continue;
        //        if ( i == 13)
        //            continue;
        //        if ( i == 34)
        //            continue;
        //        if ( i == 35)
        //            continue;

        pcStr = (char *) pstIsoData->pcBuf + pstFld[i].hOff;
        cFldType = pstIsoTable[i].cType;
        iFldLen = pstIsoTable[i].hLen;
        iLen = pstFld[i].hLen;
        if (cFldType & SYMBOL)
            iLen += 1;

        if (cFldType & BINARY) {
            tStrCpy(sTmp, pcStr, iLen);
            tBcd2Asc((UCHAR*) sData, (UCHAR*) sTmp, iLen * 2, 0);
            sData[iLen * 2] = 0;
        } else if (cFldType & BCD) {
            /*
            tStrCpy( sTmp, pcStr, (iLen+1)/2 );
            tBcd2Asc( sData, sTmp, iLen, 0 );
            sData[iLen] = 0;
             */
            /*
             * modify by gaof 2016/05/26
             * 左右对齐时,打印有问题,比如23域bcd码,0x00,0x01,会打印出000
             * */
            iLenTmp = ((iLen + 1) / 2) << 1;
            tBcd2Asc((UCHAR*) sTmp, (UCHAR*) pcStr, iLenTmp, 0);
            sTmp[iLenTmp] = 0;
            //tErrLog( DEBUG,"%d,%s", iLenTmp, sTmp );
            if (cFldType & LEFT_ALIGN) {
                tStrCpy(sData, sTmp, iLen);
                sData[iLen] = 0;
            } else {
                tStrCpy(sData, sTmp + iLenTmp - iLen, iLen);
                sData[iLen] = 0;
                //tErrLog(DEBUG,"data[%s]",sData );
            }
        } else
            tStrCpy(sData, pcStr, iLen);

        if ((cFldType & LLLVAR) == LLLVAR)
            fprintf(pstFp, "域[%3d] LLL[%3d] 长度[%3d] 数据:%s\n", i + 1, iFldLen, iLen, sData);
        else if ((cFldType & LLVAR) == LLVAR)
            fprintf(pstFp, "域[%3d] LLV[%3d] 长度[%3d] 数据:%s\n", i + 1, iFldLen, iLen, sData);
        else
            fprintf(pstFp, "域[%3d] FIX[%3d] 长度[%3d] 数据:%s\n", i + 1, iFldLen, iLen, sData);
    }
    fclose(pstFp);
}
