/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t_extiso.h
 * Author: feng.gaoo
 *
 * Created on 2017��2��28��, ����8:35
 */

#ifndef T_EXTISO_H
#define T_EXTISO_H

#ifdef __cplusplus
extern "C" {
#endif



    /* ΪIsoData.cAscVar�ֶ�,Init_Iso()��iAscVar��ζ���    */
#define ASC_LLVAR   1           /* 8583�򳤶���Ϣ���뷽ʽ   */
#define BCD_LLVAR   0           /* 8583�򳤶���Ϣ���뷽ʽ   */

    /* ΪFld.hFlag�ֶζ��� */
#define EXIST       1           /* �������     */
#define NON_EXIST   0           /* ���򲻴���   */

    /* ΪIsoTable.cType�ֶζ��� */
#define FIXED       0x00        /* �̶�������   */
#define LLVAR       0x80        /* ��λ�䳤��   */
#define LLLVAR      0xC0        /* ��λ�䳤��   */

#define BINARY      0x10        /* ����������   */
#define OTHER       0x00        /* �Ƕ��������� */

#define BCD         0x08        /* BCD���뷽ʽ  */
#define ASCII       0x00        /* ASCII���뷽ʽ*/

#define SYMBOL      0x04        /* ������       */
#define UNSYMBOL    0x00        /* �Ƿ�����     */

#define BLANK       0x02        /* �ÿո����   */
#define ZERO        0x00        /* ���ַ�0���  */

#define LEFT_ALIGN  0x01        /* �����       */
#define RIGHT_ALIGN 0x00        /* �Ҷ���       */

    /*
     *  IsoTable.type field�Ķ�����λ����:
     *      bit 7��6    :   ��������    00 �̶����ȣ�01 ��λ�䳤��10 ��λ�䳤
     *      bit 5       :   ����
     *      bit 4       :   ��������    1 ���������ݣ�0 �Ƕ���������
     *      bit 3       :   ���뷽ʽ    1 bcd�룬0 ASCII��
     *      bit 2       :   ����������  1 ������0 �Ƿ�����
     *      bit 1       :   �������    1 ��' '��䣬0�á�0�����
     *      bit 0       :   ��������    1 ����룬0�Ҷ���
     *      ���ڶ����ķ���ֵ�����ݣ�һ������룬���Ȳ���ʱ�Ҳ��ո�
     *      ���ڶ�������ֵ��(���͡�ʵ��)���ݣ�һ���Ҷ��룬λ������ʱ���㡣
     *      ���ڱ䳤�����ݣ����ʾΪ����Ԫ����+����Ԫֵ����ʽ��
     *      ����Ҷ��룬���ַ�0���ʺ�����룬�Ҳ��ո�
     */
    typedef struct {
        short hLen; /* ����󳤶�       */
        unsigned char cType; /* �������         */
    } IsoTable; /* 8583�������ṹ   */

    typedef struct {
        short hFlag; /* ������IsoData.pcBuf���Ƿ����*/
        short hLen; /* �������ݵ�ʵ�ʳ���           */
        short hOff; /* ����������IsoData.pcBuf��ƫ��λ��*/
    } Fld; /* ����8583����������IsoData.pcBuf�д洢����� */

    typedef struct {
        unsigned char *pcBuf; /* ����8583����ʱʹ�õĻ�����   */
        short hOff; /* pcBuf�д洢������������ʵ�ʳ���*/
        short hMaxLen; /* pcBuf�����洢����          */
        char cAscVar; /* 8583�䳤��ĳ�����Ϣ���뷽ʽ */
        Fld pstFld[128]; /* 8583������buf�д洢�ֲ�״̬  */
        IsoTable *pstIsoTable; /* ʹ�õ�8583�����Խṹ         */
    } IsoData; /* ����8583����ʱʹ�õĽṹ     */


    void tInitIso(IsoData *pstIso, unsigned char *pcIsoBuf, int iIsoBufLen, int iAscVar, IsoTable *pstIsoTable);
    void tClearIso(IsoData *pstIso);
    int tStr2Iso(IsoData *pstIso, unsigned char *pcIn8583, int inLen);
    int tIso2Str(IsoData *pstIso, unsigned char *pcOut8583, int iOutMaxLen);
    int tIsoSetBit(IsoData *pstIso, int iFldNo, char *pcStrIn, int iInLen);
    int tIsoGetBit(IsoData *pstIso, int iFldNo, char *pcStrOut, int iOutMaxLen);
    int tGetIsoFld(IsoData *pstIso, int iFldNo, char *pcStrOut, int *piLen);
    void tDumpIso(IsoData *pstIsoData, char *pcTitle);

#ifdef __cplusplus
}
#endif

#endif /* T_EXTISO_H */

