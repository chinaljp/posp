/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t_extiso.h
 * Author: feng.gaoo
 *
 * Created on 2017年2月28日, 下午8:35
 */

#ifndef T_EXTISO_H
#define T_EXTISO_H

#ifdef __cplusplus
extern "C" {
#endif



    /* 为IsoData.cAscVar字段,Init_Iso()的iAscVar入参定义    */
#define ASC_LLVAR   1           /* 8583域长度信息编码方式   */
#define BCD_LLVAR   0           /* 8583域长度信息编码方式   */

    /* 为Fld.hFlag字段定义 */
#define EXIST       1           /* 该域存在     */
#define NON_EXIST   0           /* 该域不存在   */

    /* 为IsoTable.cType字段定义 */
#define FIXED       0x00        /* 固定长度域   */
#define LLVAR       0x80        /* 两位变长域   */
#define LLLVAR      0xC0        /* 三位变长域   */

#define BINARY      0x10        /* 二进制数据   */
#define OTHER       0x00        /* 非二进制数据 */

#define BCD         0x08        /* BCD编码方式  */
#define ASCII       0x00        /* ASCII编码方式*/

#define SYMBOL      0x04        /* 符号域       */
#define UNSYMBOL    0x00        /* 非符号域     */

#define BLANK       0x02        /* 用空格填充   */
#define ZERO        0x00        /* 用字符0填充  */

#define LEFT_ALIGN  0x01        /* 左对齐       */
#define RIGHT_ALIGN 0x00        /* 右对齐       */

    /*
     *  IsoTable.type field的二进制位含义:
     *      bit 7、6    :   长度类型    00 固定长度，01 两位变长，10 三位变长
     *      bit 5       :   保留
     *      bit 4       :   数据类型    1 二进制数据，0 非二进制数据
     *      bit 3       :   编码方式    1 bcd码，0 ASCII码
     *      bit 2       :   符号域属性  1 符号域，0 非符号域
     *      bit 1       :   填充属性    1 用' '填充，0用‘0’填充
     *      bit 0       :   对齐属性    1 左对齐，0右对齐
     *      对于定长的非数值型数据，一律左对齐，长度不足时右补空格。
     *      对于定长的数值型(整型、实型)数据，一律右对齐，位数不足时左补零。
     *      对于变长的数据，则表示为数据元长度+数据元值的形式。
     *      金额右对齐，左补字符0，帐号左对齐，右补空格
     */
    typedef struct {
        short hLen; /* 域最大长度       */
        unsigned char cType; /* 域的属性         */
    } IsoTable; /* 8583域描述结构   */

    typedef struct {
        short hFlag; /* 该域在IsoData.pcBuf中是否存在*/
        short hLen; /* 该域数据的实际长度           */
        short hOff; /* 该域数据在IsoData.pcBuf中偏移位置*/
    } Fld; /* 描述8583各域数据在IsoData.pcBuf中存储的情况 */

    typedef struct {
        unsigned char *pcBuf; /* 拆组8583报文时使用的缓冲区   */
        short hOff; /* pcBuf中存储的所有域数据实际长度*/
        short hMaxLen; /* pcBuf的最大存储长度          */
        char cAscVar; /* 8583变长域的长度信息编码方式 */
        Fld pstFld[128]; /* 8583各域在buf中存储分布状态  */
        IsoTable *pstIsoTable; /* 使用的8583域属性结构         */
    } IsoData; /* 拆组8583报文时使用的结构     */


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

