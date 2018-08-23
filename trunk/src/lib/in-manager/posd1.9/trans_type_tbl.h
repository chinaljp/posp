#ifndef _TRANS_TYPE_TAB_H
#define _TRANS_TYPE_TAB_H

typedef struct {
    char    sMsgId[5];          /* ��ϢID           */
    char    sProcCode[7];       /* ������           */
    char    sMsgType[3];        /* ��Ϣ������       */
    char    sNmiCode[4];        /* ���������Ϣ��   */
    char    sTransCode[7];      /* ����������       */
    char    sOldTransCode[7];   /* ԭ����������     */
}TranTbl;

typedef struct {
    char    sTransCode[7];          /* ������       */
    char    sMsgType[3];            /* ��Ϣ������   */
    unsigned char *pcUnpackBitmap;  /* �����λͼ   */
    unsigned char *pcPackBitmap;    /* �����λͼ   */
}TranBitMap;

typedef struct {
    char    sTransCode[7];
    unsigned char *pcMacBitmap;
}TransMacMap;

#define MAC_CHK_ERR -1
#define MAC_GEN_ERR -2
#define TRANS_ERR  -3

#endif 
