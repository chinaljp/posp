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
 ** ����: tInitIso( )                                                        **
 ** ����: int                                                                **
 ** ���:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 **      unsigned char *pcIsoBuf ( ���8583���ݱ��ĵĻ�����ָ�� )            **
 **      int iIsoBufLen ( ���8583���ĵĻ��������� )                         **
 **      int iAscVar ( 8583�����г�����Ϣ�����ʽ,1ΪASCII,0ΪBCD)           **
 **      IsoTable *pstIsoTable ( ׼��ʹ�õ�8583���ĸ����������������ĵ�ַ )**
 ** ����:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 ** ����ֵ�� ��                                                              **
 ** ���ܣ�ʹ�ò���1������4,��ʼ������8583�����õ����ݽṹ(����0)             **
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
 ** ����: tInitIso( )                                                        **
 ** ����: int                                                                **
 ** ���:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 ** ����:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 ** ����ֵ�� ��                                                              **
 ** ���ܣ� ��ղ���8583�����õ����ݽṹ�е�8583����������Ϣ                  **
 *****************************************************************************/
void tClearIso(IsoData *pstIso) {
    memset(pstIso->pcBuf, 0, pstIso->hMaxLen);
    memset(pstIso->pstFld, 0, sizeof (pstIso->pstFld));
    pstIso->hOff = 0;
}

/*****************************************************************************
 ** ����: tStr2Iso( )                                                        **
 ** ����: int                                                                **
 ** ���:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 **      unsigned char *pcIn8583 ( ��Ŵ�����8583���ݱ��ĵĻ�����ָ�� )    **
 **      int iInLen ( ������8583���ĳ��� )                                 **
 ** ����:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 ** ����ֵ�� �ɹ� - 0,ʧ�� - -1                                              **
 ** ���ܣ���������8583����,��8583���Ŀ���IsoData��,����IsoData�е�8583����   **
 **      ����������,����������ⱨ��������Щ��,��ĳ����Լ��������ڱ����е�  **
 **      λ��,Ϊ�����ȡ��������׼��                                         **
 *****************************************************************************/
int tStr2Iso(IsoData *pstIso, unsigned char *pcIn8583, int iInLen) {
    unsigned char cLenType, cFldType, *pcBitMap, *pcData, *pcEndBuf;
    IsoTable *pstIsoTable;
    int i, j, k, iVarLen, iFldNo, iBitMapSize, iLen, iOff = 0;

    tClearIso(pstIso);
    pcEndBuf = pcIn8583 + iInLen;
    pstIsoTable = pstIso->pstIsoTable;
    /* ���1���������� */
    if (pstIsoTable[0].cType & BCD)
        /* IsoTable[0]�Ǳ��������� */
        iLen = (pstIsoTable[0].hLen + 1) / 2;
    else
        iLen = pstIsoTable[0].hLen;
    memcpy(pstIso->pcBuf, pcIn8583, iLen);
    pstIso->pstFld[0].hFlag = EXIST;
    pstIso->pstFld[0].hLen = pstIsoTable[0].hLen;
    pstIso->pstFld[0].hOff = iOff;
    iOff += iLen;
    /* ȷ��������8583������λͼ���ݵ�λ�� */
    pcBitMap = pcIn8583 + iLen;
    /* ȷ��λͼ���ݵ��ֽ��� */
    if (pcBitMap[0] & 0x80)
        iBitMapSize = 16;
    else
        iBitMapSize = 8;

    /* ȷ��������8583������λԪ���ݵ���ʼλ�� */
    pcData = pcBitMap + iBitMapSize;
    if (pcData > pcEndBuf)
        return ( -1);

    for (i = 0; i < iBitMapSize; i++) {
        /* �������,�Ӵ�����8583��������ȡ��ÿ��λԪ���ݼ��䳤����Ϣ */
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
                /* ����䳤�򳤶���ϢΪASCII���뷽ʽ */
                if (pstIso->cAscVar == ASC_LLVAR) {
                    for (k = 0; k < iVarLen; k++) {
                        iLen = iLen * 10 + *pcData - '0';
                        pcData++;
                    }
                }/* ����䳤�򳤶���ϢΪBCD���뷽ʽ */
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

            /* �����BCD���뷽ʽ */
            if (cFldType & BCD)
                iLen = (iLen + 1) / 2;

            /* ����Ƿ��������� */
            if (cFldType & SYMBOL)
                iLen = iLen + 1;

            if (pcData + iLen > pcEndBuf) {
                tLog(DEBUG, "iLen=%d", iLen);
                return ( -1);
            }
            if (iOff + iLen > pstIso->hMaxLen)
                return ( -3);

            /* ��8583��������ȡ����������� */
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
 ** ����: tIso2Str( )                                                        **
 ** ����: int                                                                **
 ** ���:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 **      unsigned char *pcOut8583 ( �����װ���8583���ݱ��ĵĻ�����ָ�� )   **
 **      int iOutMaxLen ( �����װ���8583���Ļ���������󳤶� )             **
 ** ����:                                                                    **
 **      unsigned char *pcOut8583 ( �����װ���8583���ݱ��ĵĻ�����ָ�� )   **
 ** ����ֵ�� �ɹ� - 0,ʧ�� - -1                                              **
 ** ���ܣ�����IsoData�е�8583��������������,�Լ��洢��IsoData�е�׼����װ��  **
 **      ��������Ϣ(����Щ��,��ĳ����Լ���������IsoData�Ļ������е�ƫ��λ��)**
 **      ,�����װ8583����                                                   **
 *****************************************************************************/
int tIso2Str(IsoData *pstIso, unsigned char *pcOut8583, int iOutMaxLen) {
    unsigned char cLenType, cFldType, *pcBitMap, *pcData, *pcEndBuf, cBitInfo;
    IsoTable *pstIsoTable;
    int i, j, iVarLen, iFldNo, iLen, iBitMapSize = 8;


    pcEndBuf = pcOut8583 + iOutMaxLen;
    pstIsoTable = pstIso->pstIsoTable;

    /* ȷ��λͼ�ֽ��� */
    for (i = 64; i < 128; i++) {
        if (pstIso->pstFld[i].hFlag == EXIST) {
            iBitMapSize = 16;
            break;
        }
    }

    /* ������Ϣ������8583������ */
    if (pstIso->pstIsoTable[0].cType & BCD)
        iLen = (pstIso->pstFld[0].hLen + 1) / 2;
    else
        iLen = pstIso->pstFld[0].hLen;
    memcpy(pcOut8583, pstIso->pcBuf + pstIso->pstFld[0].hOff, iLen);

    /* ȷ��λͼ���ݺ�8583λԪ������8583�����е���ʼλ�� */
    pcBitMap = pcOut8583 + iLen;
    pcData = pcOut8583 + iLen + iBitMapSize;
    if (pcData > pcEndBuf)
        return (-1);

    for (i = 0; i < iBitMapSize; i++) {
        cBitInfo = 0;
        /* �������,��ÿ��λԪ���ݼ��䳤����Ϣд��8583���� */
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

            /* ȷ���䳤��ĳ�����Ϣ */
            iVarLen = (cLenType >> 6) ? ((cLenType >> 6) + pstIso->cAscVar) : 0;

            if ((pcData + iVarLen) > pcEndBuf)
                return (-1);

            /* ���򳤶���Ϣд��8583���� */
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

            /* �����BCD���뷽ʽ */
            if (cFldType & BCD)
                iLen = (iLen + 1) / 2;

            /* ����Ƿ��������� */
            if (cFldType & SYMBOL)
                iLen = iLen + 1;

            if ((pcData + iLen) > pcEndBuf)
                return ( -1);

            /* ������iso->buf�е�������д��8583���� */
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
 ** ����: tIsoSetBit( )                                                      **
 ** ����: int                                                                **
 ** ���:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 **      int iFldNo ( 8583���ĵ����� )                                     **
 **      char *pcStrIn ( �����Ҫ�����8583����ָ��������ݻ�����ָ�� )      **
 **      int iInLen ( ���ݻ����������ݳ��� )                                 **
 ** ����:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 ** ����ֵ�� �ɹ� - 0,ʧ�� - -1                                              **
 ** ���ܣ�������װ8583����,����IsoData�е�8583��������������,����Ҫ�����8583**
 **      ����ָ���������,����ʽת�����洢��IsoData�Ļ�������,����������װ   **
 **      ��������Ϣ(����Щ��,��ĳ����Լ���������IsoData�Ļ������е�ƫ��λ��)**
 **      λ��,Ϊ�����װ8583������׼��                                       **
 *****************************************************************************/
int tIsoSetBit(IsoData *pstIso, int iFldNo, char *pcStrIn, int iInLen) {
    unsigned char cLenType, cFldType, *pcData, sTmp[999];
    IsoTable *pstIsoTable;
    int iLen;


    if (iInLen <= 0)
        return ( 0);

    if (iInLen > 999)
        return ( -1);

    /* Isotable����ű�ȶ�Ӧ��8583���С1,IsoTable[0]�Ǳ���������,IsoTable[1]�����ʺ��� */
    iFldNo = iFldNo - 1;
    if (iFldNo < 0 || iFldNo > 127)
        return ( -1);


    pstIsoTable = pstIso->pstIsoTable;
    cFldType = pstIsoTable[iFldNo].cType;
    cLenType = cFldType & 0xC0;

    /* ȡ����8583�����󳤶�   */
    iLen = pstIsoTable[iFldNo].hLen;

    /* ����Ƿ���������,�򳤶ȼ�ȥ���ų��� */
    if (cFldType & SYMBOL)
        iInLen--;

    /* ����Ǳ䳤�����������ݳ���С��������󳤶� */
    if (cLenType != FIXED && (iInLen < iLen))
        iLen = iInLen;

    /* �����ݳ��Ȱ�����δ��BCDѹ���ĳ���ȡֵ */
    pstIso->pstFld[iFldNo].hFlag = EXIST;
    pstIso->pstFld[iFldNo].hLen = iLen;
    pstIso->pstFld[iFldNo].hOff = pstIso->hOff;
    pcData = pstIso->pcBuf + pstIso->hOff;


    /* ����Ƕ����������� */
    if (cFldType & BINARY) {
        memcpy(pcData, pcStrIn, iLen);
        pstIso->hOff += iLen;
        return ( 0);
    }

    /* ����Ƿ�����,inStr�б��뺬�н���Ƿ��� */
    if (cFldType & SYMBOL) {
        if ((pstIso->hOff + 1) > pstIso->hMaxLen)
            return (-1);
        *(pcData++) = *(pcStrIn++);
        (pstIso->hOff)++;
    }

    /* ����ǿո���䷽ʽ */
    if (cFldType & BLANK)
        memset(sTmp, ' ', iLen);
    else
        memset(sTmp, '0', iLen);

    /* ���������뷽ʽ */
    if (cFldType & LEFT_ALIGN)
        memcpy(sTmp, pcStrIn, iInLen);
    else
        memcpy(sTmp + ((iLen > iInLen) ? (iLen - iInLen) : 0), pcStrIn, iInLen);

    /* �����BCD���뷽ʽ */
    if (cFldType & BCD) {
        if ((pstIso->hOff + (iLen + 1) / 2) > pstIso->hMaxLen)
            return (-1);
        tAsc2Bcd(pcData, sTmp, iLen, (char) !(cFldType & 0x01));
        pstIso->hOff += (iLen + 1) / 2;
    }/* �����ASCII���뷽ʽ */
    else {
        if ((pstIso->hOff + iLen) > pstIso->hMaxLen)
            return (-1);
        memcpy(pcData, sTmp, iLen);
        pstIso->hOff += iLen;
    }

    return (0);
}

/*****************************************************************************
 ** ����: tIsoGetBit( )                                                      **
 ** ����: int                                                                **
 ** ���:                                                                    **
 **      IsoData *pstIso ( ����8583�����õ����ݽṹ�ĵ�ַ )                  **
 **      int iFldNo ( 8583���ĵ����� )                                     **
 **      char *pcStrOut ( ��Ų�����8583����ָ��������ݻ�����ָ�� )       **
 **      int iOutMaxLen ( ������ݵĻ���������������򳤶� )                 **
 ** ����:                                                                    **
 **      char *pcStrOut ( ��Ų�����8583����ָ��������ݻ�����ָ�� )       **
 ** ����ֵ�� �ɹ� - 0,ʧ�� - -1                                              **
 ** ���ܣ������8583����,����IsoData�е�8583��������������,��������Ϣ(���� **
 **      Щ��,��ĳ����Լ���������IsoData�Ļ������е�ƫ��λ��),��ȡ����ָ����**
 **      ������,����ʽת�����洢��pcStrOut�Ļ�������                         **
 *****************************************************************************/
int tIsoGetBit(IsoData *pstIso, int iFldNo, char *pcStrOut, int iOutMaxLen) {
    unsigned char *pcData, cFldType;
    IsoTable *pstIsoTable;
    int iLen, iLenOut;


    /* ��֤GetBit()����ʧ��ʱ,strOutΪ�մ� */
    pcStrOut[0] = '\0';

    /* Isotable����ű�ȶ�Ӧ��8583���С1,IsoTable[0]�Ǳ���������,IsoTable[1]�����ʺ��� */
    iFldNo = iFldNo - 1;
    if (iFldNo < 0 || iFldNo > 127)
        return ( -1);

    if (pstIso->pstFld[iFldNo].hFlag == NON_EXIST)
        return ( 0);

    pstIsoTable = pstIso->pstIsoTable;
    cFldType = pstIsoTable[iFldNo].cType;
    iLen = pstIso->pstFld[iFldNo].hLen;

    pcData = pstIso->pcBuf + pstIso->pstFld[iFldNo].hOff;

    /* �������������ʱ,��8583�򳤶ȶ�1�ֽڷ���+1�ֽ�'\0' */
    if (cFldType & SYMBOL) {
        *(pcStrOut++) = *(pcData++);
        iLenOut = iLen + 1;
    }/* ���������������ʱ,����8583�򳤶� */
    else if (cFldType & BINARY)
        iLenOut = iLen;
        /* �����������������ʱ,��8583�򳤶ȶ�1���ֽ�'\0' */
    else
        iLenOut = iLen;

    if (iLenOut > iOutMaxLen)
        return ( -2);


    /* �����BCD���뷽ʽ */
    if (cFldType & BCD)
        tBcd2Asc((unsigned char *) pcStrOut, pcData, iLen, (char) !(cFldType & 0x01));
        /* �����ASCII���뷽ʽ */
    else
        memcpy(pcStrOut, pcData, iLen);

    pcStrOut[iLen] = '\0';

    return ( iLenOut);
}

int tGetIsoFld(IsoData *pstIso, int iFldNo, char *pcStrOut, int *piLen) {
    /* ��֤GetBit()����ʧ��ʱ,strOutΪ�մ� */
    pcStrOut[0] = '\0';

    /* Isotable����ű�ȶ�Ӧ��8583���С1,IsoTable[0]�Ǳ���������,IsoTable[1]�����ʺ��� */
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


/* ��ܺ��� */
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
        tLog(ERROR, "��־�ļ�%s�򿪴�! ���顣", sLogName);
        return;
    }

    tGetDate(sDate, "/", -1);
    tGetTime(sTime, ":", -1);
    gettimeofday(&tv, NULL);
    fprintf(pstFp, "\n----------%s, ʱ�� %s %s %03d\n", pcTitle, sDate, sTime, (int) (tv.tv_usec / 1000));

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
             * ���Ҷ���ʱ,��ӡ������,����23��bcd��,0x00,0x01,���ӡ��000
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
            fprintf(pstFp, "��[%3d] LLL[%3d] ����[%3d] ����:%s\n", i + 1, iFldLen, iLen, sData);
        else if ((cFldType & LLVAR) == LLVAR)
            fprintf(pstFp, "��[%3d] LLV[%3d] ����[%3d] ����:%s\n", i + 1, iFldLen, iLen, sData);
        else
            fprintf(pstFp, "��[%3d] FIX[%3d] ����[%3d] ����:%s\n", i + 1, iFldLen, iLen, sData);
    }
    fclose(pstFp);
}
