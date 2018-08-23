#ifndef _TRANS_TYPE_TAB_H
#define _TRANS_TYPE_TAB_H

typedef struct {
    char    sMsgId[5];          /* 消息ID           */
    char    sProcCode[7];       /* 处理码           */
    char    sMsgType[3];        /* 消息类型码       */
    char    sNmiCode[4];        /* 网络管理信息码   */
    char    sTransCode[7];      /* 交易类型码       */
    char    sOldTransCode[7];   /* 原交易类型码     */
}TranTbl;

typedef struct {
    char    sTransCode[7];          /* 交易码       */
    char    sMsgType[3];            /* 消息类型码   */
    unsigned char *pcUnpackBitmap;  /* 拆包域位图   */
    unsigned char *pcPackBitmap;    /* 组包域位图   */
}TranBitMap;

typedef struct {
    char    sTransCode[7];
    unsigned char *pcMacBitmap;
}TransMacMap;

#endif 
