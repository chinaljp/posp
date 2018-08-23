#ifndef _TRANS_TYPE_TAB_H
#define _TRANS_TYPE_TAB_H

typedef struct {
    char sMsgType[5]; /* ��Ϣ������       */
    char sProcCode[7]; /* ���״�����       */
    char sProcCodeEx[3]; /* ��չ�Ľ��״����� */
    char sNmiCode[4]; /* ���������Ϣ��   */
    char sTransCode[7]; /* ����������       */
    char sOldTransCode[7]; /* ԭ����������     */
    char sOOldTransCode[7]; /* ԭԭ����������   */
} TranTbl;

typedef struct {
    char sTransCode[7]; /* ����������       */
    char sOldTransCode[7]; /* ԭ����������     */
    char sOOldTransCode[7]; /* ԭԭ����������   */
    unsigned char *pcUnpackBitmap; /* �����λͼ       */
    unsigned char *pcPackBitmap; /* �����λͼ       */
} TranBitMap;

typedef struct {
    char sTransCode[7];
    unsigned char *pcMacBitmap;
} TransMacMap;

#endif 
