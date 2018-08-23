#ifndef _TRANS_TYPE_TAB_H
#define _TRANS_TYPE_TAB_H

typedef struct {
    char sMsgType[5]; /* 消息类型码       */
    char sProcCode[7]; /* 交易处理码       */
    char sProcCodeEx[3]; /* 扩展的交易处理码 */
    char sNmiCode[4]; /* 网络管理信息码   */
    char sTransCode[7]; /* 交易类型码       */
    char sOldTransCode[7]; /* 原交易类型码     */
    char sOOldTransCode[7]; /* 原原交易类型码   */
} TranTbl;

typedef struct {
    char sTransCode[7]; /* 交易类型码       */
    char sOldTransCode[7]; /* 原交易类型码     */
    char sOOldTransCode[7]; /* 原原交易类型码   */
    unsigned char *pcUnpackBitmap; /* 拆包域位图       */
    unsigned char *pcPackBitmap; /* 组包域位图       */
} TranBitMap;

typedef struct {
    char sTransCode[7];
    unsigned char *pcMacBitmap;
} TransMacMap;

#endif 
