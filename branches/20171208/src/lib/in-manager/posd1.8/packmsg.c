/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "t_redis.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_extiso.h"
#include "trans_type_tbl.h"
#include "secapi.h"         /* �ӽ���   */
#include "trans_detail.h"
#include "trans_code.h"
#include "t_macro.h"
#include "trace.h"
#include "err.h"
#include "s_param.h"
#include "tKms.h"

/* dbop.pc */
int GetTermKey(char *pcMerchId, char *pcTermId, char *pcTak, char *pcTdk);

extern IsoData g_stIsoData;
char *g_pcMsg = NULL;
int g_iMsgLen = 0;

TranTbl g_staTranTbl[] = {
    /* ��ϢID  ������    ��Ϣ������  NMI   ���״���  ԭ���״���  */
    { "0200", "310000", "01", "000", "021000", ""}, /* POS�����            */
    { "0200", "000000", "22", "000", "020000", ""}, /* POS����              */
    { "0400", "000000", "22", "000", "020003", ""}, /* POS���ѳ���          */
    { "0200", "200000", "23", "000", "020002", ""}, /* POS���ѳ���          */
    { "0400", "200000", "23", "000", "020023", ""}, /* POS���ѳ�������      */
    { "0220", "200000", "25", "000", "020001", ""}, /* POS�˻�              */
    { "0200", "800000", "22", "000", "022100", ""}, /* ����ǩ���� */
    
    { "0100", "030000", "10", "000", "024000", ""}, /* POSԤ��Ȩ            */
    { "0400", "030000", "10", "000", "024003", ""}, /* POSԤ��Ȩ����        */
    { "0100", "200000", "11", "000", "024002", ""}, /* POSԤ��Ȩ����        */
    { "0400", "200000", "11", "000", "024023", ""}, /* POSԤ��Ȩ��������    */
    { "0200", "000000", "20", "000", "024100", ""}, /* POSԤ��Ȩ���        */
    { "0400", "000000", "20", "000", "024103", ""}, /* POSԤ��Ȩ��ɳ���    */
    { "0200", "200000", "21", "000", "024102", ""}, /* POSԤ��Ȩ��ɳ���    */
    { "0400", "200000", "21", "000", "024123", ""}, /* POSԤ��Ȩ��ɳ�������*/
    
    { "0200", "000000", "36", "000", "U20000", ""},   /* POS�ѻ�����        */
    { "0220", "200000", "27", "000", "U20001", ""},   /* POS�ѻ������˻�     */
    
    { "0200", "311000", "01", "000", "M21000", ""},   /* ������ѯ          */
    { "0200", "100000", "22", "000", "M20000", ""},   /* ��������          */ 
    { "0400", "100000", "22", "000", "M20003", ""},  /* �������ѳ���      */
    { "0200", "210000", "23", "000", "M20002", ""},  /* �������ѳ���      */
    { "0400", "210000", "23", "000", "M20023", ""},  /* �������ѳ�������  */
    
    { "0200", "280000", "22", "000", "T20000", ""},   /* T0����             */
    { "0400", "280000", "22", "000", "T20003", ""},   /* T0���ѳ���          */    
    { "0620", "280000", "22", "000", "T20010", ""},   /* �ս�����֪ͨ         */

    { "0200", "330000", "50", "000", "02A000", ""}, /*  ʵ����֤    */
    { "0200", "340000", "50", "000", "02A100", ""}, /* ���ս���     */
    
        /* ΢�� */
    { "0200", "710000", "60", "000", "02W100", "002"}, /* ΢����ɨ          */
    { "0200", "700000", "60", "000", "02W200", "002"}, /* ΢�ű�ɨ֧��           */     
    { "0200", "730000", "60", "000", "02W300", "002"}, /* ΢�Ų�ѯ          */
    { "0200", "720000", "60", "000", "02W600", "002"},    /* ����         */
    
    /* ���� */
    { "0200", "710000", "60", "000", "02Y100", "004"}, /* ������ɨ          */
    { "0200", "700000", "60", "000", "02Y200", "004"}, /* ������ɨ֧��           */     
    { "0200", "730000", "60", "000", "02Y300", "004"}, /* ������ѯ          */ 
    { "0200", "720000", "60", "000", "02Y600", "004"},    /* ��������         */
    
    /* ֧���� */
    { "0200", "710000", "60", "000", "02B100", "003"}, /* ֧������ɨ          */
    { "0200", "700000", "60", "000", "02B200", "003"}, /* ֧������ɨ֧��           */     
    { "0200", "730000", "60", "000", "02B300", "003"}, /* ֧������ѯ          */   
    { "0200", "720000", "60", "000", "02B600", "003"},    /* ����         */

//    { "0200", "740000", "60", "000", "S00004", ""}, /* �˿�      */
//    { "0200", "750000", "60", "000", "S00005", ""}, /* �˿��ѯ      */
//    { "0200", "720000", "60", "000", "S00006", ""}, /* ����         */   

    { "0620", "", "", "951", "020400", "021000"}, /* POS�ű�֪ͨ*/
    { "0620", "", "", "951", "020400", "023000"}, /* POS�ű�֪ͨ*/
    { "0620", "", "", "951", "020400", "020000"}, /* POS�ű�֪ͨ*/
    { "0620", "", "", "951", "020400", "024000"}, /* POS�ű�֪ͨ*/
    { "0620", "", "", "951", "020400", "024100"}, /* POS�ű�֪ͨ*/
    { "0620", "", "", "951", "020400", "026000"}, /* POS�ű�֪ͨ*/
    { "0620", "", "", "951", "020400", "027000"}, /* POS�ű�֪ͨ*/
    
    { "0200", "800000", "22", "000", "022100", ""},      /* ����ǩ���� */    
    
    /* �����ཻ�� */
    { "0800", "", "00", "003", "029000", ""}, /* POSǩ��              */
    { "0800", "", "00", "101", "029300", ""}, /* POS��ʼ��            */
    { "0800", "", "00", "360", "029400", ""}, /* POS��������          */
    { "0820", "", "07", "301", "029500", ""}, /* POS����ǩ���ϴ�      */
    //{ "0000", "", "F0", "000", "033700", ""}, /* ������֪ͨ(������) */

    { "0800", "", "00", "370", "028100", ""}, /* IC����Կ����          */
    { "0800", "", "00", "380", "028200", ""}, /* IC����������          */
    { "0800", "", "00", "390", "028300", ""}, /* IC������������          */
    { "0800", "", "00", "394", "028400", ""}, /* �ǽӲ�����������          */
    { "0800", "", "00", "396", "028500", ""}, /* ����������bin����          */
    { "0800", "", "00", "398", "028600", ""}, /* ���ܿ�bin����������          */

    { "0800", "", "00", "371", "028110", ""}, /* IC����Կ���ؽ���          */
    { "0800", "", "00", "381", "028210", ""}, /* IC���������ؽ���          */
    { "0800", "", "00", "391", "028310", ""}, /* IC�����������ؽ���          */
    { "0800", "", "00", "395", "028410", ""}, /* �ǽӲ����������ؽ���          */
    { "0800", "", "00", "397", "028510", ""}, /* ����������bin���ؽ���          */
    { "0800", "", "00", "399", "028610", ""}, /* ���ܿ�bin���������ؽ���          */

    { "0820", "", "00", "372", "027800", ""}, /* IC����Կ����״̬����          */
    { "0820", "", "00", "382", "027900", ""}, /* IC����������״̬����          */

    { "", "", "", "", "", ""},
};

/*************
 **  ������  **
 *************/

/* POS����� */
static unsigned char g_caInqueryUnpackMap[] = {\
2, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 59, 60, 62, 64, 255};
static unsigned char g_caInqueryPackMap[] = {\
2, 11, 12, 13, 14, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 54, 55, 60, 63 ,64, 255};



/* POS���� */
static unsigned char g_caPurchaseUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 57, 59, 60, 62, 63, 64, 255};
static unsigned char g_caPurchasePackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55,59, 60, 63, 64, 255};
/* POS�ѻ����� */
static unsigned char g_caPbocUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 59, 60, 62, 63, 64, 255};
static unsigned char g_caPbocPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 60, 63, 64, 255};


/* ��ָ���˻�Ȧ�� */
static unsigned char g_caPurchaseFZDUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 59, 60, 62, 63, 64, 255};
static unsigned char g_caPurchaseFZDPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32,35,36, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55,59, 60, 63, 64, 255};

/* POS���ѳ��� */
static unsigned char g_caUndoPurchaseUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 63, 64, 255};
static unsigned char g_caUndoPurchasePackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};

/* POS�˻� */
static unsigned char g_caRefundUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 63, 64, 255};
static unsigned char g_caRefundPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};

/* POSԤ��Ȩ */
static unsigned char g_caPreAuthoUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 41, 42, 35, 36, 49, 52, 53, 55, 57, 59, 60, 63, 64, 255};
static unsigned char g_caPreAuthoPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55, 59, 60, 63, 64, 255};

/* POSԤ��Ȩ���� */
static unsigned char g_caUndoPreAuthoUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 63, 64, 255};
static unsigned char g_caUndoPreAuthoPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};

/* POSԤ��Ȩ��� */
static unsigned char g_caPreAuthoCompUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 38, 41, 42, 35, 36, 49, 52, 53, 55, 59, 60, 61, 63, 64, 255};
static unsigned char g_caPreAuthoCompPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55, 59, 60, 63, 64, 255};

/* POSԤ��Ȩ��ɳ��� */
static unsigned char g_caUndoPreAuthoCompUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 26, 32, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 63, 63, 255};
static unsigned char g_caUndoPreAuthoCompPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};

/* POS�������� */
static unsigned char g_caCorrectUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 32, 38, 39, 41, 42, 49, 53, 55, 60, 61, 62, 64, 255};
static unsigned char g_caCorrectPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 55, 60, 64, 255};

/* POS���� */
static unsigned char g_caPurchaseRevUnpackMap[] = {\
2, 4, 11, 14, 22, 23, 25, 32, 38, 39, 41, 42, 49, 53, 55, 60, 62, 64, 255};
static unsigned char g_caPurchaseRevPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 55, 60, 64, 255};

/* POS�ű�֪ͨ */
static unsigned char g_caJbtzUnpackMap[] = {\
2, 4, 11, 22, 23, 32, 37, 41, 42, 49, 55, 60, 61, 64, 255};
static unsigned char g_caJbtzPackMap[] = {\
2, 11, 12, 13, 15, 22, 23, 32, 38, 39, 41, 42, 49, 55, 60,  64, 255};

/* ����ǩ����*/
static unsigned char g_caReceiptsUnpackMap[] = {\
11,14, 37, 41, 42, 60,62, 64, 255};
static unsigned char g_caReceiptsPackMap[] = {\
5,11, 12, 13, 14,32, 39, 41, 42, 60, 64, 255};


/* ������ѯ */
static unsigned char g_caOrderUnpackMap[] = {\
6, 11, 41, 42, 60,62, 64, 255};
static unsigned char g_caOrderPackMap[] = {\
6, 11, 12, 13, 37, 39, 41, 42, 44, 54, 60, 64, 255};


/* �������� */
static unsigned char g_caorderPurchaseUnpackMap[] = {\
2, 4, 6, 11, 14, 22, 23, 25, 26, 32 ,41, 42, 35, 36, 49, 52, 53, 55, 57, 59, 60,62, 63, 64, 255};
static unsigned char g_caorderPurchasePackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 54, 55,59, 60,63, 64, 255};

/* �������ѳ��� */
static unsigned char g_caorderPurchaseRevUnpackMap[] = {\
2, 4, 6, 11, 14, 22, 23, 25, 32, 38, 39, 41, 42, 49, 53, 55, 60,62, 64, 255};
static unsigned char g_caorderPurchaseRevPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 55, 60, 64, 255};


/* �������ѳ��� */
static unsigned char g_caUndorderUnpackMap[] = {\
2, 4, 6, 11, 14, 22, 23, 25, 26, 32, 37, 38, 41, 42, 35, 36, 49, 52, 53, 55, 60, 61, 62, 63, 64, 255};
static unsigned char g_caUndorderPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 38, 39, 41, 42, 44, 49, 53, 55, 59, 60, 63, 64, 255};


/* �������ѳ������� */
static unsigned char g_caorderCorrectUnpackMap[] = {\
2, 4, 6, 11, 14, 22, 23, 25, 32, 38, 39, 41, 42, 49, 53, 55, 60, 61, 62,64, 255};
static unsigned char g_caorderCorrectPackMap[] = {\
2, 4, 11, 12, 13, 14, 15, 23, 25, 32, 37, 39, 41, 42, 44, 49, 53, 55, 60, 64, 255};

/* POS��Ȩ */
static unsigned char g_AuthenUnpackMap[] = {\
2, 11, 41, 42, 53, 62, 63, 64, 255};

static unsigned char g_AuthenPackMap[] = {\
2, 11, 12, 13, 37, 39, 41, 42, 44, 63, 64, 255};

/* �ս��Ȳ�ѯ */
static unsigned char g_QuotaUnpackMap[] = {\
11, 41, 42, 64, 255};
static unsigned char g_QuotaPackMap[] = {\
11, 12, 13, 37, 39, 41, 42, 54, 63, 64, 255};

/* ��ɨ���� */
static unsigned char g_UnActivescanpackMap[] = {\
4, 6, 11, 14, 32, 41, 42, 44, 49, 58, 62, 63, 64, 255};
static unsigned char g_ActivescanpackMap[] = {\
4, 6, 11, 12, 13, 14, 32, 37, 39, 41, 42, 44, 49, 63, 64, 255};

/* ��ɨ���� */
static unsigned char g_UnPassivescanpackMap[] = {\
4, 7, 11, 14, 22, 32, 41, 42, 44, 49, 58, 62, 63, 64, 255};
static unsigned char g_PassivescanpackMap[] = {\
4, 5, 6, 11, 12, 13, 14, 32, 37, 39, 41, 42, 44, 49, 63, 64, 255};

/* ��ѯ */
static unsigned char g_UnQueryresultpackMap[] ={\
6, 11, 32, 41, 42, 44, 63, 64, 255};
static unsigned char g_QueryresultpackMap[] = {\
4, 6, 11, 12, 13, 32, 37, 39, 41, 42, 44, 63, 64, 255};

/*��ά�볷��*/
static unsigned char g_UnTdRevokepackMap[] = {\
4, 11, 32, 37, 41, 42, 44, 49, 60, 61, 63, 64, 255};
static unsigned char g_TdRevokepackMap[] = {\
4, 11, 12, 13, 32, 37, 39, 41, 42, 44, 49, 60, 63, 64, 255};

/* �˿� */
static unsigned char g_UnRefundpackMap[] = {\
4, 6, 11, 32, 37, 41, 42, 44, 49, 58, 60, 63, 64, 255};
static unsigned char g_RefundpackMap[] ={\
4, 11, 12, 13, 32, 37, 39, 41, 42, 44, 49, 60, 63, 64, 255};

/* �˿��ѯ */
static unsigned char g_UnQueryRefundpackMap[] ={\
4, 6, 11, 37, 41, 42, 44, 60, 61, 63, 64, 255};
static unsigned char g_QueryRefundpackMap[] ={\
4, 6, 11, 12, 13, 37, 39, 41, 42, 44, 60, 63, 64, 255};

/* �������� */
static unsigned char g_UnRevokepackMap[] = {\
4, 6, 11, 32, 37, 41, 42, 44, 49, 58, 60, 63, 64, 255};
static unsigned char g_RevokepackMap[] = {\
4, 6, 11, 12, 13, 32, 37, 39, 41, 42, 44, 49, 60, 63, 64, 255};


/*************
 **  ������  **
 *************/

/* POSǩ�� */
static unsigned char g_caLoginUnpackMap[] = {\
11, 32,41, 42, 60, 63, 255};
static unsigned char g_caLoginPackMap[] = {\
11, 12, 13, 32, 37, 39, 41, 42, 60, 62, 255};

/* POS״̬���� */
static unsigned char g_caZtssUnpackMap[] = {\
 37, 41, 42, 60, 62, 255};
static unsigned char g_caZtssPackMap[] = {\
 12, 13, 39, 41, 42, 60, 62, 255};

/* POS��ʼ�� */
static unsigned char g_caInitUnpackMap[] = {\
11, 59, 60, 255};
static unsigned char g_caInitPackMap[] = {\
11, 12, 13, 32, 37, 39, 41, 42, 60, 255};

/* POS�������� */
static unsigned char g_caArgumentDownloadUnpackMap[] = {\
41, 42, 60, 255};
static unsigned char g_caArgumentDownloadPackMap[] = {\
12, 13, 37, 39, 41, 42, 60, 62, 255};

/* POS����ǩ���ϴ� */
static unsigned char g_caElecSigUploadUnpackMap[] = {\
2, 4, 11, 15, 37, 41, 42, 60, 62, 64, 255};
static unsigned char g_caElecSigUploadPackMap[] = {\
11, 37, 39, 41, 42, 60, 64, 255};


TranBitMap g_staBitMap[] = {
    /* ������   ��Ϣ������ �����λͼ                      �����λͼ                   */
    { "021000", "01", g_caInqueryUnpackMap, g_caInqueryPackMap}, /* POS�����            */
    { "020000", "22", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* POS����              */
    { "020003", "22", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* POS���ѳ���          */
    { "020002", "23", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* POS���ѳ���          */
    { "020023", "23", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* POS���ѳ�������      */
    { "020001", "25", g_caRefundUnpackMap, g_caRefundPackMap}, /* POS�˻�              */

    { "024000", "10", g_caPreAuthoUnpackMap, g_caPreAuthoPackMap}, /* POSԤ��Ȩ            */
    { "024003", "10", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* POSԤ��Ȩ����        */
    { "024002", "11", g_caUndoPreAuthoUnpackMap, g_caUndoPreAuthoPackMap}, /* POSԤ��Ȩ����        */
    { "024023", "11", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* POSԤ��Ȩ��������    */
    { "024100", "20", g_caPreAuthoCompUnpackMap, g_caPreAuthoCompPackMap}, /* POSԤ��Ȩ���        */
    { "024103", "20", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* POSԤ��Ȩ��ɳ���    */
    { "024102", "21", g_caUndoPreAuthoCompUnpackMap, g_caUndoPreAuthoCompPackMap}, /* POSԤ��Ȩ��ɳ���    */
    { "024123", "21", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* POSԤ��Ȩ��ɳ�������*/

    { "U20000", "36", g_caPbocUnpackMap, g_caPbocPackMap}, /* POS�ѻ�����              */
    { "U20001", "27", g_caRefundUnpackMap, g_caRefundPackMap}, /* POS�ѻ������˻�              */

    { "T20000", "22", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* T0���� */
    { "T20003", "22", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* T0���ѳ��� */
    { "T20010", "22", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* T0�ս�֪ͨ          */

    { "M21000", "01", g_caOrderUnpackMap, g_caOrderPackMap}, /* ������ѯ          */
    { "M20000", "22", g_caorderPurchaseUnpackMap, g_caorderPurchasePackMap}, /* ��������     */
    { "M20002", "23", g_caUndorderUnpackMap, g_caUndorderPackMap}, /* �������ѳ���  */
    { "M20003", "22", g_caorderPurchaseRevUnpackMap, g_caorderPurchaseRevPackMap}, /* �������ѳ���  */
    { "M20023", "23", g_caorderCorrectUnpackMap, g_caorderCorrectPackMap}, /* �������ѳ������� */

    { "02A000", "50", g_AuthenUnpackMap, g_AuthenPackMap}, /* POS��Ȩ */
    { "02A100", "50", g_QuotaUnpackMap, g_QuotaPackMap}, /* �ս��޶��ѯ  */

    { "02Y100", "60", g_UnActivescanpackMap, g_ActivescanpackMap}, /* ������ɨģʽ */
    { "02Y200", "60", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* ������ɨģʽ */
    { "02Y300", "60", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* ����֧����ѯ */
    { "02Y600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap}, /*��ά����������*/
    
    { "02B100", "60", g_UnActivescanpackMap, g_ActivescanpackMap}, /* ֧������ɨģʽ */
    { "02B200", "60", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* ֧������ɨģʽ */
    { "02B300", "60", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* ֧����֧����ѯ */
    { "02B600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap},      /*��ά��֧��������*/

    { "02W100", "60", g_UnActivescanpackMap, g_ActivescanpackMap}, /* ΢����ɨģʽ */
    { "02W200", "60", g_UnPassivescanpackMap, g_PassivescanpackMap}, /* ΢�ű�ɨģʽ */
    { "02W300", "60", g_UnQueryresultpackMap, g_QueryresultpackMap}, /* ΢��֧����ѯ */
    { "02W600", "60", g_UnTdRevokepackMap, g_TdRevokepackMap},      /*��ά��΢�ų���*/
    
    { "S00004", "60", g_UnRefundpackMap, g_RefundpackMap}, /* �˿� */
    { "S00005", "60", g_UnQueryRefundpackMap, g_QueryRefundpackMap}, /* �˿��ѯ */
    { "S00006", "60", g_UnRevokepackMap, g_RevokepackMap}, /* �������� */

    { "023000", "45", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* �����ֽ�Ȧ��              */
    { "020300", "45", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* �����ֽ�Ȧ�����          */
    { "023000", "46", g_caPurchaseUnpackMap, g_caPurchasePackMap}, /* �����ֽ��ֽ��ֵ              */
    { "020300", "46", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* �����ֽ��ֽ��ֵ����         */
    { "023000", "47", g_caPurchaseFZDUnpackMap, g_caPurchaseFZDPackMap}, /* �����ֽ��ָ���˻�Ȧ��              */
    { "020300", "47", g_caPurchaseRevUnpackMap, g_caPurchaseRevPackMap}, /* �����ֽ��ָ���˻�Ȧ�����          */
    { "020200", "51", g_caUndoPurchaseUnpackMap, g_caUndoPurchasePackMap}, /* �����ֽ��ֽ��ֵ����       */
    { "020300", "51", g_caCorrectUnpackMap, g_caCorrectPackMap}, /* �����ֽ��ֽ��ֵ��������      */
    { "020400", "00", g_caJbtzUnpackMap, g_caJbtzPackMap}, /* IC���ű�֪ͨ              */

    { "029000", "00", g_caLoginUnpackMap, g_caLoginPackMap}, /* POSǩ��              */
    { "029300", "00", g_caInitUnpackMap, g_caInitPackMap}, /* POS��ʼ��            */
    { "029400", "00", g_caArgumentDownloadUnpackMap, g_caArgumentDownloadPackMap}, /* POS��������          */
    { "029500", "07", g_caElecSigUploadUnpackMap, g_caElecSigUploadPackMap}, /* POS����ǩ���ϴ�      */
    { "022100", "22", g_caReceiptsUnpackMap, g_caReceiptsPackMap}, /* ����ǩ���� */

    { "028100", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC����Կ����              */
    { "028110", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC����Կ���ؽ���              */
    { "028200", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC����������              */
    { "028210", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC���������ؽ���              */
    { "028300", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC������������              */
    { "028310", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC�����������ؽ���              */
    { "028400", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* �ǽӲ�����������              */
    { "028410", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* �ǽӲ����������ؽ���              */
    { "028500", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* ����������bin����              */
    { "028510", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* ����������bin���ؽ���              */
    { "028600", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* ���ܿ�bin����������              */
    { "028610", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* ���ܿ�bin���������ؽ���              */

    { "027800", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC����Կ����״̬����              */
    { "027900", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC����������״̬����              */

    //    { "020500", "00", g_caZtssUnpackMap, g_caZtssPackMap}, /* IC��״̬����(ʹ��029800,�������)              */
    { "", "", NULL, NULL}
};

/*
 * ����POS���ĵ���Ϣ���͡������롢��Ϣ�����롢���������Ϣ�룬�ڶ�������жϲ��ҽ�������
 */
int GetTranType(cJSON *pstJson, char *pcMsgId, char *pcProcCode, char *pcMsgType, char *pcNmiCode, char *pcOldTransCode) {
    int i;

    /* ��ȡ��ǰ��������(���ݴӷ����������Ľ������ϢID�������롢��Ϣ�����롢���������) */
    for (i = 0; g_staTranTbl[i].sMsgId[0] != '\0'; i++)
    {
        if (!memcmp(g_staTranTbl[i].sMsgId, pcMsgId, strlen(g_staTranTbl[i].sMsgId))
                && !memcmp(g_staTranTbl[i].sProcCode, pcProcCode, strlen(g_staTranTbl[i].sProcCode))
                && !memcmp(g_staTranTbl[i].sMsgType, pcMsgType, strlen(g_staTranTbl[i].sMsgType))
                && !memcmp(g_staTranTbl[i].sNmiCode, pcNmiCode, strlen(g_staTranTbl[i].sNmiCode))
                && !memcmp(g_staTranTbl[i].sOldTransCode, pcOldTransCode, strlen(pcOldTransCode)))
        {
            SET_STR_KEY(pstJson, "trans_code", g_staTranTbl[i].sTransCode);
            return ( 0);
        }
    }
    return ( -1);
}

/*
 * �����ڲ����ĵĽ��״��롢ԭ���״��룬�ڶ�����в�����ϢID�������롢��Ϣ�����롢���������
 */
int GetMsgData(char *pcTransCode, char *pcOldTransCode, char *pcMsgId, char *pcProcCode, char *pcMsgType, char *pcNmiCode) {
    int i;

    for (i = 0; g_staTranTbl[i].sMsgId[0] != '\0'; i++)
    {
        if (!memcmp(g_staTranTbl[i].sTransCode, pcTransCode, 6))
        {
            strcpy(pcMsgId, g_staTranTbl[i].sMsgId);
            strcpy(pcProcCode, g_staTranTbl[i].sProcCode);
            strcpy(pcMsgType, g_staTranTbl[i].sMsgType);
            strcpy(pcNmiCode, g_staTranTbl[i].sNmiCode);

            return ( 0);
        }
    }
    return ( -1);
}

int DecTrack(char *pcClearTrack, char *pcDecTrack, int iDecTrackLen, char *pcTdkuLmk) {
    int iRet = -1, iLen = 0;
    char sBlock[17] = {0}, sTmp[17] = {0};

    tLog(DEBUG, "Track Data[%d][%s]", iDecTrackLen, pcDecTrack);
    tStrCpy(pcClearTrack, pcDecTrack, iDecTrackLen);
    /* ������ż��������+1*/
    iLen = iDecTrackLen % 2 ? iDecTrackLen + 1 : iDecTrackLen;
    /* �Ӻ����ڶ�λ��ǰȡ8�ֽ�*/
    tLog(DEBUG, "Track Len[%d]", iLen);
    memcpy(sBlock, pcDecTrack + iLen - 18, 16);
    //iRet = HSM_Dec_Track(sTmp, sBlock, 16, pcTdkuLmk);
    iRet = tHsm_Dec_Data(sTmp, pcTdkuLmk, sBlock);
    if (iRet < 0)
        return -1;
    tLog(DEBUG, "Track Clear[%s]", sTmp);
    memcpy(pcClearTrack + iLen - 18, sTmp, 16);

    return 0;
}

/* unpack */
int GetFld(int iBitNo, cJSON *pstJson) {
    char sNumBuf[3], sBuf1[2048], sBuf2[2048], caMsgBuf[1024], sTak[33] = {0}, caMac[8], caTmp[8], sTdk[33] = {0};
    int iLen = 0, iRet = -1;
    char sBuf[1024] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0}, sInputMode[3 + 1] = {0};
    char sErr[128] = {0}, sCtype[3] = {0}, sTrack2[38] = {0}, sTrack3[105] = {0}, sAcqId[11 + 1] = {0};
    char track2Buf[37];
    int pos = 0;

    memset(sNumBuf, 0, sizeof (sNumBuf));
    memset(sBuf, 0, sizeof (sBuf));
    memset(caTmp, 0, sizeof (caTmp));
    memset(caMsgBuf, 0, sizeof (caMsgBuf));
    memset(track2Buf, 0, sizeof (track2Buf));
    switch (iBitNo)
    {
        case 2: /* ���˺� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet <= 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_no", sBuf);
            break;
        case 4: /* ���׽�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_DOU_KEY(pstJson, "amount", atol(sBuf));
            break;
        case 6:/* ������ */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /* �÷�һ: ������Ϊ������ؽ���ʱ��������sPrivateData�� */
            if (!memcmp(sTransCode, "MM", 1))
            {
                SET_STR_KEY(pstJson, "merch_order_no", sBuf);
            } else if (!memcmp(sTransCode, "02W100", 6) || !memcmp(sTransCode, "02B100", 6) || !memcmp(sTransCode, "02Y100", 6) ) 
			{
                SET_STR_KEY(pstJson, "seed", sBuf);
            } else
            {
                SET_STR_KEY(pstJson, "qr_order_no", sBuf);
            }
            break;
        case 7: /* �̻������� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "merch_order_no", sBuf);
            tLog(DEBUG, "�̻�������[%s]", sBuf);
            break;
        case 11: /* �ն���ˮ�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "trace_no", sBuf);
            break;
        case 14: /* ����Ч�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_exp_date", sBuf);
            break;
        case 22: /* ��������뷽ʽ�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "input_mode", sBuf);
            break;
        case 23: /* ��Ƭ���к� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            tLog(DEBUG, "23 card_sn[%s]", sBuf);
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "card_sn", sBuf);
            break;
        case 25: /* ����������� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "cond_code", sBuf);
            break;
        case 26: /* �����PIN��ȡ�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "pin_code", sBuf);
            break;
        case 32: /* ���������ʶ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            tTrim(sBuf);
            SET_STR_KEY(pstJson, "acq_id", sBuf);
            break;
        case 35: /* 2�ŵ����� */
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            if (GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
            {
                SET_STR_KEY(pstJson, "resp_code", "A7");
                tLog(ERROR, "��ȡ�ն�[%s:%s]����Կ����.", sMerchId, sTermId);
                return ( -1);
            }
            tLog(DEBUG, "TDK[%s]", sTdk);
            tLog(DEBUG, "TAK[%s]", sTak);
            //�л���û�м��ܣ����⴦��һ��
            GET_STR_KEY(pstJson, "acq_id", sAcqId);
            if (NULL == strstr(sAcqId, "49000780"))
            {
                DecTrack(sTrack2, sBuf, strlen(sBuf), sTdk);
            } else
            {
                strcpy(sTrack2, sBuf);
            }
            SET_STR_KEY(pstJson, "track2", sTrack2);
            tLog(DEBUG, "Track2[%s]", sTrack2);
            if (iRet != 0)
            {
                pos = 0;
                memset(track2Buf, 0, sizeof (track2Buf));
                memcpy(track2Buf, sTrack2, 20);
                while (*(track2Buf + pos) != 'D' && *(track2Buf + pos) != '\0')
                {
                    if (0 == isdigit(track2Buf[pos]))
                        return -1;
                    pos++;
                }
                *(track2Buf + pos) = '\0';
                SET_STR_KEY(pstJson, "card_no", track2Buf);
                tLog(DEBUG, "track card_no[%s]", track2Buf);
            }
            break;
        case 36: /* 3�ŵ����� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            if (sBuf[0] == '\0')
                return 0;
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);

            if (sTdk[0] == '\0' && GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
            {
                SET_STR_KEY(pstJson, "resp_code", "A7");
                tLog(ERROR, "��ȡ�ն�[%s:%s]����Կ����.", sMerchId, sTermId);
                return ( -1);
            }
            tLog(DEBUG, "TDK[%s]", sTdk);
            tLog(DEBUG, "TAK[%s]", sTak);
            DecTrack(sTrack3, sBuf, strlen(sBuf), sTdk);
            SET_STR_KEY(pstJson, "track3", sTrack3);
            break;
        case 37: /* �÷�1: �յ�ϵͳ��ˮ��(ԭ������ˮ��) */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "o_rrn", sBuf);
            break;
        case 38: /* ��Ȩ�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "auth_code", sBuf);
            break;
        case 39: /* Ӧ���� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            //SET_STR_KEY(pstJson, "resp_code", sBuf);
            /* ֻ�г�������39�򣬵���cups����Ҫ���� */
            break;
        case 41: /* �ն˱�� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "term_id", sBuf);
            break;
        case 42: /* �̻���� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "merch_id", sBuf);
            break;
        case 44: /* ֧������ 001���� 002΢�� 003֧���� */
            /* ����㴦�� */
            //            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            //            if (iRet < 0)
            //                return ( iRet);
            //            SET_STR_KEY(pstJson, "input_mode", sBuf);
            break;
        case 49: /* ���׻��Ҵ��� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "ccy_code", sBuf);
            break;
        case 52: /* �ֿ������� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, caTmp, sizeof (caTmp));
            if (iRet < 0)
                return ( iRet);
            GET_STR_KEY(pstJson, "input_mode", sInputMode);
            if (sInputMode[2] == '1')
            {
                tBcd2Asc((UCHAR*) sBuf1, (UCHAR*) caTmp, 16, LEFT_ALIGN);
                sBuf1[16] = '\0';
                SET_STR_KEY(pstJson, "pin", sBuf1);
            }
            break;
        case 53: /* ��ȫ������Ϣ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "secure_ctrl", sBuf);
            break;
        case 55: /* IC�������� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_INT_KEY(pstJson, "ic_len", iRet);
            tBcd2Asc((UCHAR*) sBuf1, (UCHAR*) sBuf, iRet << 1, LEFT_ALIGN);
            sBuf1[iRet << 1] = '\0';
            SET_STR_KEY(pstJson, "ic_data", sBuf1);
            tLog(DEBUG, "55 Ic[%s].", sBuf1);
            break;

        case 57: /* �ն�����Ӳ�����кż��������� */
            /*
               �ն�������֤��� 03 
               �豸���� 04 
               �ն����к� 05 
               ����������� 06 
               Ӳ�����к��������� 07 
               Ӧ�ó���汾�� 08              
             */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "pos_info", sBuf);
            tLog(DEBUG, "pos_info[%s].", sBuf);
            break;

        case 58: /* �������� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "order_header", sBuf);
            tLog(DEBUG, "��������[%s].", sBuf);
            break;
        case 59: /* �Զ������÷�1: �ն˳�����Ϣ20�����к� */
            /* �ն˳�����Ϣ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /*
                        ����Ԫ���� N3
                                59.1 �ն˳��ұ�ʶ N20
                                59.2 �ն����к�sn N30
                                59.3 ��վ��Ϣ N10
                                59.4 pos����汾�� N2
                                59.4�����Ͱ汾�Ŷ��壬�����ն����ͺ�ͨѶ��ʽ�Ĳ�ͬ���Զ������£�
                                CDMAͨѶ��51 GPRSͨѶ��52 ���ж��ֻ�����ʱ���塣
             */
            /* ������Ϣ20 */
            tStrCpy(sBuf1, sBuf, 20);
            tTrim(sBuf1);
            SET_STR_KEY(pstJson, "term_fatory", sBuf1);
            /* ���к�30 */
            tStrCpy(sBuf1, sBuf + 20, 30);
            tTrim(sBuf1);
            SET_STR_KEY(pstJson, "term_sn", sBuf1);
            /* 
             * 59.3��59.4�ݲ�����
             *  */
            break;
        case 60: /* �Զ�����60: ��Ϣ�����롢���κš����������Ϣ�� */
            /* �����ModuleUnpack�ﴦ�� */
            /*
             * ����Ԫ���� N3
             * 60.1 ��Ϣ������ N2
             * 60.2 ���κ� N6
             * 60.3 ���������Ϣ�� N3
             * 60.4 �ն˶�ȡ���� N1
             * 60.5 ����PBOC�� / ���Ǳ�׼��IC���������� N1
             */
            break;
        case 61: /* ԭʼ������Ϣ: ԭʼ�������κ�6λ��ԭʼ����POS��ˮ��λ��ԭʼ��������4λ */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /*
                        ����Ԫ���� N3
                                61.1 ԭʼ�������κ� N6
                                61.2 ԭʼ����POS��ˮ�� N6
                                61.3 ԭʼ�������� N4
                                61.4ԭ������Ȩ��ʽ N2
                                61.5ԭ������Ȩ�������� N11
             */
            tStrCpy(sBuf1, sBuf, 6);
            SET_STR_KEY(pstJson, "o_batch_no", sBuf1);
            tStrCpy(sBuf1, sBuf + 6, 6);
            SET_STR_KEY(pstJson, "o_trace_no", sBuf1);
            tStrCpy(sBuf1, sBuf + 6 + 6, 4);
            SET_STR_KEY(pstJson, "o_trans_date", sBuf1);
            break;
        case 62: /* �Զ����� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            /* �÷�1: IC��״̬���� */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            tLog(DEBUG, "trans_code=%s", sTransCode);
            if (!memcmp(sTransCode, "027800", 6)
                    || !memcmp(sTransCode, "027900", 6))
            {
                SET_STR_KEY(pstJson, "62_req_data", sBuf);
            }/* �÷�2: IC����Կ������������������ */
            else if (!memcmp(sTransCode, "028100", 6)
                    || !memcmp(sTransCode, "028200", 6)
                    || !memcmp(sTransCode, "028300", 6)
                    || !memcmp(sTransCode, "028400", 6)
                    || !memcmp(sTransCode, "028500", 6)
                    || !memcmp(sTransCode, "028600", 6)
                    || !memcmp(sTransCode, "029500", 6))
            {
                SET_INT_KEY(pstJson, "62_req_len", iRet);
                tBcd2Asc((UCHAR*) sBuf1, (UCHAR*) sBuf, iRet << 1, LEFT_ALIGN);
                SET_STR_KEY(pstJson, "62_req_data", sBuf1);
            } else if (!memcmp(sTransCode, "02A000", 6))
            {
                tStrCpy(sCtype, sBuf, 2);
                if (!memcmp(sCtype, "01", 2))
                {
                    tStrCpy(sBuf2, sBuf + 2, 18);
                    SET_STR_KEY(pstJson, "cer_no", sBuf2);
                    tStrCpy(sBuf2, sBuf + 20, 11);
                    tTrim(sBuf2);
                    SET_STR_KEY(pstJson, "mobile", sBuf2);
                    SET_STR_KEY(pstJson, "account_name", sBuf + 31);
                } else
                {
                    return -1;
                }
            } else if (!memcmp(sTransCode, "MM", 1))
            {
                SET_STR_KEY(pstJson, "merch_order_no", sBuf);
            } else
            {
                SET_STR_KEY(pstJson, "private_data", sBuf);
            }
            break;
        case 63: /* �Զ������÷�1: ����Ա���� */
            iRet = tIsoGetBit(&g_stIsoData, iBitNo, sBuf, sizeof (sBuf));
            if (iRet < 0)
                return ( iRet);
            SET_STR_KEY(pstJson, "teller_no", sBuf);
            break;
        case 64: /* MAC */

            memcpy(caMac, g_pcMsg + g_iMsgLen - 8, 8);
            //tLog(DEBUG, "caMac[%s]", caMac);
            /* ��ԭ���ĵ�MAC Buffer */
            memcpy(caMsgBuf, g_pcMsg, g_iMsgLen - 8);
            iLen = g_iMsgLen - 8;
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);

            if (sTak[0] == '\0' && GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
            {
                tLog(ERROR, sErr, "��ȡ�ն�[%s:%s]����Կ����.", sMerchId, sTermId);
                return ( MAC_CHK_ERR);
            }
            tLog(DEBUG, "TDK[%s]", sTdk);
            tLog(DEBUG, "TAK[%s]", sTak);
            //if ((iRet = HSM_GenMac(sTak, (UCHAR*) caMsgBuf, iLen, (UCHAR*) caTmp)) < 0) {
            if ((iRet = tHsm_Gen_Pos_Mac(caTmp, sTak, caMsgBuf, iLen)) < 0)
            {
                tLog(ERROR, "���ü��ܻ�����[%d].", iRet);
                return -1;
            }
            tDumpHex("����", caMac, 8);
            tDumpHex("����", caTmp, 8);
            if (memcmp(caTmp, caMac, 8))
            {
                SET_STR_KEY(pstJson, "resp_code", "A0");
                {
                    /*��ӡmacֵ,�ü��*/
                }
                tLog(ERROR, "У��MAC����.");
                return -1;
            }
            break;
        default:
            tLog(ERROR, "���ı�����[%d]������", iBitNo);

            break;
    }
    return ( 0);
}

/* pack */
int SetFld(int iBitNo, cJSON *pstJson) {
    char caMsgBuf[MSG_MAX_LEN], caMac[8], sTmp[512] = {0};
    char sBuf[1024] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    char sTak[48 + 1] = {0}, sTdk[48 + 1] = {0}, sTpk[48 + 1] = {0}, sTmk[48 + 1] = {0}, sRespCode[3] = {0};
    char sTransCode[7] = {0}, sAmtType[3] = {0}, sAmtType2[3] = {0}, sAcctType[3] = {0};
    char sCcyCode[4] = {0}, sBalanceSign[2] = {0}, sUsableLimit[13] = {0}, sTotalLimit[12] = {0}, sBalancel[12] = {0};
    char sTmkFlag[1 + 1] = {0};
    double dAmount = 0.0;
    int iLen, iRet;

    switch (iBitNo)
    {
        case 2: /* ���˺� */
            GET_STR_KEY(pstJson, "card_no", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 4: /* ���׽�� */
            GET_DOU_KEY(pstJson, "amount", dAmount);
            snprintf(sTmp, sizeof (sTmp), "%012.f", dAmount);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, 12));
        case 5: /* ������url */
            GET_STR_KEY(pstJson, "code_url", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 6: /* ������ */
            GET_STR_KEY(pstJson, "qr_order_no", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 11: /* �ն���ˮ�� */
            GET_STR_KEY(pstJson, "trace_no", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 12: /* ����ʱ�� */
            GET_STR_KEY(pstJson, "trans_time", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 13: /* �������� */
            GET_STR_KEY(pstJson, "trans_date", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf + 4, 4));
        case 14: /* ����Ч�� */
            GET_STR_KEY(pstJson, "card_exp_date", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 15: /* �������� */
            GET_STR_KEY(pstJson, "settle_date", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf + 4, 4));
        case 23: /* ��Ƭ���к� */
            GET_STR_KEY(pstJson, "card_sn", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 25: /* ����������� */
            GET_STR_KEY(pstJson, "cond_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 32: /* �յ�������ʶ */
            /*GET_STR_KEY(pstJson, "agent_id", sBuf); motofy by gjq 32��Ϊ�ط���
             * ��� agent_id  ΪNULL ��ȡ��acq_id�� ��ֵ����
             */
            GET_STR_KEY(pstJson, "agent_id", sBuf);
            if (NULL == sBuf || sBuf[0] == '\0')
            {
                GET_STR_KEY(pstJson, "acq_id", sBuf);
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 37: /* �÷�1: �յ�ϵͳ��ˮ�� */
            GET_STR_KEY(pstJson, "rrn", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 38: /* ��Ȩ�� */
            GET_STR_KEY(pstJson, "auth_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 39: /* Ӧ���� */
            GET_STR_KEY(pstJson, "resp_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 41: /* �ն˱�� */
            GET_STR_KEY(pstJson, "term_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 42: /* �̻���� */
            GET_STR_KEY(pstJson, "merch_id", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 44: /* ����/�յ��ṹ */
            GET_STR_KEY(pstJson, "iss_id", sBuf);
            snprintf(sTmp, sizeof (sTmp), "%-11s%-11s", sBuf, "49000000");
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, strlen(sTmp)));
        case 49: /* ���׻��Ҵ��� */
            GET_STR_KEY(pstJson, "ccy_code", sBuf);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 53: /* ��ȫ������Ϣ */
            GET_STR_KEY(pstJson, "secure_ctrl", sBuf);
            sBuf[2] = '0';
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));
        case 54: /* ���ӽ�� */
            /*
             * �÷�һ���˻��������
            LENGTH n3 ����Ԫ����
            AVAILABLE - BALANCE - AMOUNT �˻��ϵĿ�������¼����Ϊ��
                    ACCOUNT - TYPE n2 �˻�����
                    AMOUNT - TYPE n2 ��������
                    CURRENCY - CODE an3 ���Ҵ���
                    AMOUNT SIGN an1 ������
                    AMOUNT n12 ���
             * �÷������ۼ���Ȩ���
             * �÷������ս���
             * 2�����ݣ���һ��������ڶ����ܶ��
             * ÿ������Ԫ
             * ACCOUNT - TYPE n2 �˻�����
                AMOUNT - TYPE n2 ��������
                CURRENCY - CODE an3 ���Ҵ���
                AMOUNT SIGN an1 ������
                AMOUNT n12 ���
             */

            GET_STR_KEY(pstJson, "resp_code", sBuf);
            if (NULL == sBuf)
            {
                return 0;
            }
            if (memcmp(sBuf, "00", 2))
                return 0;
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            GET_STR_KEY(pstJson, "amt_type", sAmtType);
            GET_STR_KEY(pstJson, "acct_type", sAcctType);
            GET_STR_KEY(pstJson, "ccy_code", sCcyCode);
            GET_STR_KEY(pstJson, "balance_sign", sBalanceSign);
            GET_STR_KEY(pstJson, "balance1", sBalancel);
            /* t0��Ȳ�ѯ */
            if (!memcmp(sTransCode, "02A100", 6))
            {
                GET_STR_KEY(pstJson, "amt_type2", sAmtType2);
                GET_STR_KEY(pstJson, "usable_limit", sUsableLimit);
                GET_STR_KEY(pstJson, "total_limit", sTotalLimit);
                sprintf(sBuf, "%2s%2s%3s%s%12s", sAmtType, sAcctType, \
			  sCcyCode, sBalanceSign, sUsableLimit);
                iLen = strlen(sBuf);
                sprintf(sBuf + iLen, "%2s%2s%3s%s%12s", sAmtType2, sAcctType,  \
				sCcyCode, sBalanceSign, sTotalLimit);
            } else
            {
                sprintf(sBuf, "%2s%2s%3s%s%12s", sAcctType, sAmtType, \
                        sCcyCode, sBalanceSign, sBalancel);
            }


            return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, strlen(sBuf)));

        case 55: /* IC�������� */
            GET_STR_KEY(pstJson, "ic_data", sBuf);
            if ('\0' == sBuf[0])
                return 0;
            iLen = strlen(sBuf);
            tAsc2Bcd((UCHAR*) sTmp, (UCHAR*) sBuf, iLen, LEFT_ALIGN);
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, iLen / 2));
        case 59: /* �Զ������÷�2: ������ */
        {
            /* ��̴��� */
        }
            return 0;
        case 60: /* �Զ�����60: ��Ϣ�����롢���κš����������Ϣ�� */
            /* �����ModulePack�ﴦ�� */
            return 0;
        case 62: /* �Զ����� */
            GET_STR_KEY(pstJson, "trans_code", sTransCode);
            tLog(DEBUG, "trans_code[%s]", sTransCode);
            /* �÷�1: �ն���Կ */
            if (!memcmp(sTransCode, "029000", 6))
            {
                GET_STR_KEY(pstJson, "resp_code", sRespCode);
                if (0 != memcmp(sRespCode, "00", 2))
                    return ( 0);
                GET_STR_KEY(pstJson, "tpk_key", sTpk);
                GET_STR_KEY(pstJson, "tak_key", sTak);
                GET_STR_KEY(pstJson, "tdk_key", sTdk);
                GET_STR_KEY(pstJson, "tmk_key", sTmk);
                GET_STR_KEY(pstJson, "tmk_flag", sTmkFlag);

                tLog(DEBUG, "tak[%s]tpk[%s]tdk[%s]tmk[%s]", sTak, sTpk, sTdk, sTmk);
                tStrCpy(sBuf, "01", 2);
                tStrCpy(sBuf + 2, sTak, 48);
                tStrCpy(sBuf + 50, "02", 2);
                tStrCpy(sBuf + 52, sTpk, 48);
                tStrCpy(sBuf + 100, "03", 2);
                tStrCpy(sBuf + 102, sTdk, 48);
                if (sTmkFlag[0] == '1')
                {
                    tStrCpy(sBuf + 150, "00", 2);
                    tStrCpy(sBuf + 152, sTmk, 48);
                }
                tLog(ERROR, "sKey[%s]", sBuf);
                iLen = strlen(sBuf);
                tAsc2Bcd((UCHAR*) caMsgBuf, (UCHAR*) sBuf, iLen, LEFT_ALIGN);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, caMsgBuf, (iLen + 1) / 2));
                return 0;
            }/* �÷�2: IC��״̬���� */
            else if (!memcmp(sTransCode, "027800", 6)
                    || !memcmp(sTransCode, "027900", 6)
                    || !memcmp(sTransCode, "028100", 6)
                    || !memcmp(sTransCode, "028200", 6)
                    || !memcmp(sTransCode, "028300", 6)
                    )
            {
                GET_INT_KEY(pstJson, "62_len", iLen);
                GET_STR_KEY(pstJson, "62_data", sBuf);
                tLog(DEBUG, "[%d][%s].", iLen, sBuf);
                tAsc2Bcd((UCHAR*) sTmp, (UCHAR*) sBuf, iLen, LEFT_ALIGN);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, iLen / 2));
            } else if (!memcmp(sTransCode, "028500", 6)
                    || !memcmp(sTransCode, "028600", 6)
                    || !memcmp(sTransCode, "028400", 6))
            {
                GET_INT_KEY(pstJson, "62_len", iLen);
                GET_STR_KEY(pstJson, "62_data", sBuf);
                tLog(DEBUG, "[%d][%s].", iLen, sBuf);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, iLen));
            } else if (!memcmp(sTransCode, "029400", 6))
            {
                /* ��������-�˵����� 
                ���� ���� ��ʽ ȡֵ
                        11 ֧�ֵĽ������� N32 1010 0000 0000 0000 0000 0000 0000 0000
                        12 �̻����� ANS40
                        13 ����� ANS120
                 */
                GET_STR_KEY(pstJson, "bitmap", sTmp);
                snprintf(sBuf, sizeof (sBuf), "11%-32s", sTmp);
                iLen = 2 + 32;
                MEMSET(sTmp);
                GET_STR_KEY(pstJson, "merch_p_name", sTmp);
                snprintf(sBuf + iLen, sizeof (sBuf), "12%-40s", sTmp);
                iLen += 2 + 40;
                MEMSET(sTmp);
                GET_STR_KEY(pstJson, "advert", sTmp);
                snprintf(sBuf + iLen, sizeof (sBuf), "13%-120s", sTmp);
                iLen += 2 + 120;
                sBuf[iLen] = '\0';
                tLog(DEBUG, "029400,len[%d]", iLen);
                return ( tIsoSetBit(&g_stIsoData, iBitNo, sBuf, iLen));
            } else
                return 0;
        case 63: /* �Զ������÷�2: �������ÿ���˾���� */
            GET_STR_KEY(pstJson, "resp_desc", sBuf);
            tStrCpy(sTmp, "   ", 3);
            tStrCpy(sTmp + 3, sBuf, strlen(sBuf));
            return ( tIsoSetBit(&g_stIsoData, iBitNo, sTmp, strlen(sTmp)));
        case 64: /* MAC */
            /* �鱨�ĵ�MAC Buffer */
            GET_STR_KEY(pstJson, "merch_id", sMerchId);
            GET_STR_KEY(pstJson, "term_id", sTermId);
            tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8);
            if ((iLen = tIso2Str(&g_stIsoData, (UCHAR*) caMsgBuf, sizeof (caMsgBuf))) < 0)
                return ( -1);
            if (GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
            {
                tLog(ERROR, "��ȡ�ն�[%s:%s]����Կ����,��������.", sMerchId, sTermId);
                return ( -1);
            }
            //if ((iRet = HSM_GenMac(sTak, (UCHAR*) caMsgBuf, iLen - 8, (UCHAR*) caMac)) < 0) {
            if ((iRet = tHsm_Gen_Pos_Mac(caMac, sTak, caMsgBuf, iLen - 8)) < 0)
            {
                tLog(ERROR, "����MAC����[%d].", iRet);
                return ( -1);
            }
            return ( tIsoSetBit(&g_stIsoData, iBitNo, caMac, 8));

        default:
            tLog(ERROR, "��װ�ı�����[%d]������", iBitNo);
    }
    return 0;
}

int DecpcMsg(char *pcClearTrack, char *pcDecTrack, int iDecTrackLen, char *pcTdkuLmk) {
    int iRet = -1;
    char sBlock[2048] = {0}, sTmp[1024] = {0};
    tLog(DEBUG, "Track Data[%d][%s]", iDecTrackLen, pcDecTrack);
    tLog(DEBUG, "Track Len[%d]", iDecTrackLen);
    //memcpy(sBlock, pcDecTrack, iDecTrackLen);
    /*
    for (i = 0; i < iDecTrackLen; i++) {
        snprintf(sBlock + i * 2, sizeof (sBlock), "%02X", (unsigned char) pcDecTrack[i]);
    }
     * */
    tBcd2Asc((unsigned char *) sBlock, (unsigned char *) pcDecTrack, iDecTrackLen << 1, LEFT_ALIGN);
    //iRet = HSM_Dec_Pcmsg(sTmp, sBlock, iDecTrackLen, pcTdkuLmk);
    iRet = tHsm_Dec_Data(sTmp, pcTdkuLmk, sBlock);
    if (iRet < 0)
        return -1;
    tLog(DEBUG, "Track Clear[%s]", sTmp);
    tAsc2Bcd((unsigned char *) pcClearTrack, (unsigned char *) sTmp, iDecTrackLen << 1, LEFT_ALIGN);

    return 0;
}

/* �������󣬲�� */
int RequestMsg(cJSON *pstReqJson, cJSON *pstDataJson) {
    int i, iRet = -1, iHeadLen = 0;
    char sMsgId[5] = {0}, sProcCode[9] = {0}, sNmiCode[4] = {0}, sMsgType[3] = {0};
    char sHead[12 + 1] = {0};
    char sBindInfo[16 + 1] = {0}, sMsgData[17] = {0}, sSweepType[3 + 1] = {0};
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sBatchNo[BATCH_NO_LEN + 1] = {0}, sTpdu[10 + 1] = {0};
    char sDate[8 + 1] = {0}, sTime[6 + 1] = {0}, sTrace[TRACE_NO_LEN + 1] = {0}, sLogicDate[8 + 1] = {0};
    char sMerchId[15 + 1] = {0}, sTermId[8 + 1] = {0}, sTak[33] = {0}, sTdk[33] = {0}, sBuf[2048] = {0}, sBuf1[2048] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0};
    char sTmpMsg[MSG_MAX_LEN] = {0}, sTmp[128] = {0};
    UCHAR *pcBitMap = NULL;
    UCHAR caLriHead[5] = {0x4c, 0x52, 0x49, 0x00, 0x1c};
    UCHAR caLriData[5]; /* ������LRI����ͷ      */
    UCHAR caLriHead2[3] = {0xAA, 0xAA, 0xAA}; /* ȫ���ļ���      */
    UCHAR caLriData2[3]; /*    ȫ���ļ���   */
    //TransCode stTransCode;

    GET_STR_KEY(pstDataJson, "msg", sTmpMsg);
    int iMsgLen = strlen(sTmpMsg);

    tAsc2Bcd(sMsg, (UCHAR*) sTmpMsg, iMsgLen, LEFT_ALIGN);
    memcpy(caLriData, (sMsg + 5), 5);
    tDumpHex("��", (char *) caLriData, 5);
    memcpy(caLriData2, (sMsg + 8), 3);
    tDumpHex("����", (char *) caLriData2, 3);
    /* tpdu */
    memcpy(sTmp, sMsg, 5);
    tBcd2Asc((UCHAR*) sTpdu, (UCHAR*) sTmp, 10, LEFT_ALIGN);
    SET_STR_KEY(pstReqJson, "tpdu", sTpdu);

    /* == ����POS���Ľṹ(��������)  == */
    /* 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25   **
     **       [-- TPDU 5B--] [--  LRI����ͷ      33B --                              **
     **                      [ LRI�ױ�־5B] [ TEL1 8B ���к���    ] [ TEL2 8B ����   **
     **                      4c 52 49 00 1c                                          **
     **                                                                              **
     ** 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50   **
     **       -- LRI����ͷ         33B --          ]                                 **
     ** TEL2 8B] 'X'[L3 ] 'I'[L4 ] [ NODE 6B       ] [ ISO8583 MSG                   **
     **          58 00 09 49 00 06                                                   **
     **                                                                              */
    if (!memcmp(caLriData, caLriHead, 5))
    {
        iHeadLen = 44; /* 5+6+33 */
        g_pcMsg = (char*) sMsg + iHeadLen;
        g_iMsgLen = iMsgLen / 2 - iHeadLen;

        /* ����ͷ */
        MEMSET(sTmp);
        memcpy(sTmp, sMsg + 5 + 33, 6);
        tBcd2Asc((UCHAR*) sHead, (UCHAR*) sTmp, 12, LEFT_ALIGN);
        SET_STR_KEY(pstReqJson, "head", sHead);

        /* ��ȡ�ն˰���Ϣ */
        memcpy(sTmp, sMsg + 12, 8);
        tBcd2Asc((UCHAR*) sBindInfo, (UCHAR*) sTmp, 16, LEFT_ALIGN);
        SET_STR_KEY(pstReqJson, "bind_info", sBindInfo);
    }/* == ����POS���Ľṹ(����������) == */
    else if (!memcmp(caLriData2, caLriHead2, 3))
    {
        iHeadLen = 5 + 3 + 3 + 15 + 8; /* 5+3(ͷ)+3(����汾��,��AAAAAA)+15+8 */
        g_pcMsg = (char*) sMsg + iHeadLen;
        g_iMsgLen = iMsgLen / 2 - iHeadLen;
        memcpy(sMerchId, sMsg + 5 + 3 + 3, 15);
        memcpy(sTermId, sMsg + 5 + 3 + 3 + 15, 8);
        tLog(INFO, "ȫ���ļ���:merchId[%s]termId[%s].", sMerchId, sTermId);
        if (GetTermKey(sMerchId, sTermId, sTak, sTdk) < 0)
        {
            tLog(ERROR, "��ȡ�ն�[%s:%s]����Կ����.", sMerchId, sTermId);
            return ( -1);
        }
        tLog(DEBUG, "tdk[%s],tak[%s].", sTdk, sTak);
        memset(sBuf, 0, sizeof (sBuf));
        memset(sBuf1, 0, sizeof (sBuf1));
        memcpy(sBuf, g_pcMsg, g_iMsgLen);
        DecpcMsg(sBuf1, sBuf, g_iMsgLen, sTdk);
        tDumpHex("ȫ���Ľ���", sBuf1, g_iMsgLen);
        g_pcMsg = sBuf1;
        /* ����ͷ */
        MEMSET(sTmp);
        memcpy(sTmp, sMsg + 5, 3);
        tBcd2Asc((UCHAR*) sHead, (UCHAR*) sTmp, 6, LEFT_ALIGN);
        strcat(sHead + 6, "000000");
        tLog(DEBUG, "head:%s", sHead);
        SET_STR_KEY(pstReqJson, "head", sHead);
    } else
    {
        iHeadLen = 11; /* 5+6 */
        g_pcMsg = (char*) sMsg + iHeadLen;
        g_iMsgLen = iMsgLen / 2 - iHeadLen;
        /* ����ͷ */
        MEMSET(sTmp);
        memcpy(sTmp, sMsg + 5, 6);
        tBcd2Asc((UCHAR*) sHead, (UCHAR*) sTmp, 12, LEFT_ALIGN);
        SET_STR_KEY(pstReqJson, "head", sHead);
    }
    tClearIso(&g_stIsoData);
    if ((iRet = tStr2Iso(&g_stIsoData, (UCHAR*) g_pcMsg, g_iMsgLen)) < 0)
    {
        tLog(ERROR, "����8583�������ݴ���[%d].", iRet);
        return ( -1);
    }

    tDumpIso(&g_stIsoData, "������");

    tIsoGetBit(&g_stIsoData, 1, sMsgId, sizeof (sMsgId));
    tIsoGetBit(&g_stIsoData, 3, sProcCode, sizeof (sProcCode));
    tTrim(sProcCode);

    /* �����Զ�����60: ��Ϣ�����롢���κš����������Ϣ�� */
    tIsoGetBit(&g_stIsoData, 60, sMsgData, sizeof (sMsgData));
    tStrCpy(sMsgType, sMsgData, 2);
    tStrCpy(sBatchNo, sMsgData + 2, 6);
    SET_STR_KEY(pstReqJson, "batch_no", sBatchNo);
    tStrCpy(sNmiCode, sMsgData + 8, 3);
    SET_STR_KEY(pstReqJson, "60_data", sMsgData)

    if (sMsgData[12] == '2')
    {
        SET_STR_KEY(pstReqJson, "invoice_no", "2");
        tLog(ERROR, "fallback��־, invoice_no=2)!");
    }
    /* ʹ��44���ж϶�ά��ͨ�����ͣ�ֻ�ж�ά�뽻�׵������ײ�ʹ��44���������⴦�� */
    tIsoGetBit(&g_stIsoData, 44, sSweepType, sizeof (sSweepType));
    /* �������ڲ��������� */
    if (GetTranType(pstReqJson, sMsgId, sProcCode, sMsgType, sNmiCode, sSweepType) < 0)
    {
        tLog(ERROR, "�޷������Ľ�������(MsgId=%s,ProcCode=%s,MsgType=%s, NmiCode=%s,SweepType[%s])!"
                , sMsgId, sProcCode, sMsgType, sNmiCode, sSweepType);
        return TRANS_ERR;
    }
    GET_STR_KEY(pstReqJson, "trans_code", sTransCode);
    tLog(ERROR, "��������[%s](MsgId=%s,MsgType=%s,ProcCode=%s,NmiCode=%s,SweepType=%s)!"
            , sTransCode, sMsgId, sProcCode, sNmiCode, sMsgType, sSweepType);

    /* ȡ�ý��׵�Bit Map��׼���� */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++)
    {
        if ((!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6))
                &&(!memcmp(g_staBitMap[i].sMsgType, sMsgType, strlen(sMsgType))))
        {
            pcBitMap = g_staBitMap[i].pcUnpackBitmap;
            break;
        }
    }
    if (g_staBitMap[i].sTransCode[0] == '\0')
    {
        tLog(ERROR, "����[%s][%s]δ������λͼ.", sTransCode, sMsgType);
        return TRANS_ERR;
    }
    {
        /* ��ȡ�������ڣ�ʱ�䣬rrn��ÿһ�����׶��Ǵ�manager��ͳ��� */
        tGetDate(sDate, "", -1);
        tGetTime(sTime, "", -1);
        snprintf(sTmp, sizeof (sTmp), "%s%s", sDate + 5, sTime);
        SET_STR_KEY(pstReqJson, "trans_date", sDate);
        SET_STR_KEY(pstReqJson, "trans_time", sTime);
        sprintf(sTmp, "%s%s", sDate + 4, sTime);
        SET_STR_KEY(pstReqJson, "transmit_time", sTmp);
        if (GetSysTrace(sTrace) < 0)
        {
            tLog(ERROR, "��ȡϵͳ��ˮ��ʧ��,���׷���.");
            return -1;
        }
        sprintf(sTmp, "%s%s", sTime, sTrace);
        SET_STR_KEY(pstReqJson, "rrn", sTmp);
        SET_STR_KEY(pstReqJson, "sys_trace", sTrace);
        if (FindValueByKey(sLogicDate, "LOGIC_DATE") < 0)
        {
            tLog(ERROR, "��ȡ�߼�����ʧ��.");
            return -1;
        }
        SET_STR_KEY(pstReqJson, "logic_date", sLogicDate);
    }
    /* ����Bit Map��׼�������������� */
    for (i = 0; pcBitMap[i] != 255; i++)
    {
        tLog(DEBUG, "GetBit��[%d].", pcBitMap[i]);
        iRet = GetFld(pcBitMap[i], pstReqJson);
        if (iRet < 0)
        {
            tLog(ERROR, "��⽻��[%s][%s]������[%d]����[%d]!"
                    , sTransCode, sMsgType, pcBitMap[i], iRet);
            return ( iRet);
        }
    }
    /* �յ�������ʼ����ֵ */
#if 0
    GET_STR_KEY(pstReqJson, "acq_id", sAcqInstId);
    if (NULL == sAcqInstId || sAcqInstId[0] == '\0')
    {

        SET_STR_KEY(pstReqJson, "acq_id", "00000000");
        tLog(DEBUG, "����δ����32��,��Ĭ��ֵ[%s]", "00000000");
    }

    //DUMP_JSON(pstReqJson);
    /* �����ˮ ���������λ����ĸ��ʹ��INLINE_TRANS_DETAIL�����ֵ�ʹ��POS_TRANS_DETAIL*/
    if (FindTransCode(&stTransCode, sTransCode) < 0)
    {
        tLog(ERROR, "������δ����,���׷�������.");
        return -1;
    }
    if (TRUE_FLAG == stTransCode.sLogFlag[0])
    {
        if (!strcmp(stTransCode.sGroupCode, POS_GROUP))
        {
            if (AddPosTransLs(pstReqJson) < 0)
            {
                tLog(ERROR, "��ˮ��¼ʧ��,���׷�������.");
                return -1;
            }
        } else if (!strcmp(stTransCode.sGroupCode, INLINE_GROUP))
        {
            if (AddInlineTransLs(pstReqJson) < 0)
            {
                tLog(ERROR, "��ˮ��¼ʧ��,���׷�������.");

                return -1;
            }
        }
    }
#endif
    //DUMP_JSON(pstReqJson);
    return 0;
}

//int ModulePack(char *pcMsg, int *piMsgLen, void *pvJson) {

int ResponseMsg(cJSON *pstRepJson, cJSON *pstDataJson) {
    int i, iRet = -1;
    char sMsgId[5] = {0}, sProcCode[9] = {0}, sNmiCode[4] = {0}, sMsgType[3] = {0}, sTmp[2048] = {0};
    UCHAR sMsg[MSG_MAX_LEN] = {0};
    char sHead[12 + 1] = {0};
    UCHAR *pcBitMap = NULL;
    char sTransCode[TRANS_CODE_LEN + 1] = {0}, sReqFlag[1 + 1] = {0}, sBatchNo[BATCH_NO_LEN + 1] = {0}, sTpdu[10 + 1] = {0};
    //TransCode stTransCode;
    char sIstRespCode[2 + 1] = {0}, sRespCode[2 + 1] = {0};

    //DUMP_JSON(pstDataJson);
    GET_STR_KEY(pstDataJson, "istresp_code", sIstRespCode);
    GET_STR_KEY(pstDataJson, "resp_code", sRespCode);
    /* ��������쳣���أ���Ӧ������ܻ��滻resp_code */
    if (sRespCode[0] == '\0')
    {
        tLog(ERROR, "��Ӧ����,���׷�������.");
        return -1;
    }


    GET_STR_KEY(pstDataJson, "trans_code", sTransCode);
#if 0 
    /* �����ˮ ���������λ����ĸ��ʹ��INLINE_TRANS_DETAIL�����ֵ�ʹ��POS_TRANS_DETAIL*/
    if (FindTransCode(&stTransCode, sTransCode) < 0)
    {
        tLog(ERROR, "������δ����,���׷�������.");
        return TRANS_ERR;
    }
    /* ������ˮ ���������λ����ĸ��ʹ��INLINE_TRANS_DETAIL�����ֵ�ʹ��POS_TRANS_DETAIL*/
    if (TRUE_FLAG == stTransCode.sLogFlag[0])
    {
        if (!strcmp(stTransCode.sGroupCode, POS_GROUP))
        {
            if (UpdPosTransLs(pstDataJson) < 0)
            {
                tLog(ERROR, "����POS������ˮ���ʧ��,��������.");
                return -1;
            }
        } else if (!strcmp(stTransCode.sGroupCode, INLINE_GROUP))
        {
            if (UpdInlineTransLs(pstDataJson) < 0)
            {
                tLog(ERROR, "����INLINE������ˮ���ʧ��,��������.");
                return -1;
            }
        }
    }
#endif
    if (memcmp(sIstRespCode, "00", 2) == 0 && 0 != memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "�������سɹ�����ϵͳ����ʧ��,���׷�������.");
        return TRANS_ERR;
    }

    tClearIso(&g_stIsoData);
    if (GetMsgData(sTransCode, NULL, sMsgId, sProcCode, sMsgType, sNmiCode) < 0)
    {
        tLog(ERROR, "�޷���װ�Ľ�������(TransCode=[%s])!", sTransCode);
        return TRANS_ERR;
    }
    tLog(ERROR, "��������[%s](MsgId=%s,MsgType=%s,ProcCode=%s,NmiCode=%s)!"
            , sTransCode, sMsgId, sProcCode, sNmiCode, sMsgType);

    /* Ӧ����Ϣ��� */
    sMsgId[2]++;
    tIsoSetBit(&g_stIsoData, 1, sMsgId, 4);
    if (sProcCode[0])
    {
        tIsoSetBit(&g_stIsoData, 3, sProcCode, 6);
    }

    /* ����Զ�����60: ��Ϣ�����롢���κš����������Ϣ�� */
    GET_STR_KEY(pstDataJson, "batch_no", sBatchNo);
    snprintf(sTmp, sizeof (sTmp), "%2s%6s%3s", sMsgType, sBatchNo, sNmiCode);
    tIsoSetBit(&g_stIsoData, 60, sTmp, 11);

    /* ȡ�ý��׵�Bit Map��׼���� */
    for (i = 0; g_staBitMap[i].sTransCode[0] != '\0'; i++)
    {
        if (!memcmp(g_staBitMap[i].sTransCode, sTransCode, 6)
                &&!memcmp(g_staBitMap[i].sMsgType, sMsgType, strlen(sMsgType)))
        {
            pcBitMap = g_staBitMap[i].pcPackBitmap;
            break;
        }
    }

    if (g_staBitMap[i].sTransCode[0] == '\0')
    {
        tLog(ERROR, "����[%s][%s]δ�������λͼ.", sTransCode, sMsgType);
        return TRANS_ERR;
    }

    /* ����Bit Map��׼�������������� */
    for (i = 0; pcBitMap[i] != 255; i++)
    {
        tLog(DEBUG, "SetBit��[%d].", pcBitMap[i]);
        iRet = SetFld(pcBitMap[i], pstDataJson);
        if (iRet < 0)
        {
            tLog(ERROR, "��װ����[%s][%s]������[%d]����[%d]!"
                    , sTransCode, sMsgType, pcBitMap[i], iRet);
            return ( iRet);
        }
    }
    /* Ԥ��2λ����+5λtpdu+6����ͷ */
    if ((iRet = tIso2Str(&g_stIsoData, (UCHAR*) sTmp, sizeof (sTmp))) < 0)
    {
        tLog(ERROR, "����8583���Ĵ���[%d]", iRet);
        return ( -1);
    }

    tBcd2Asc((UCHAR*) sMsg + 22, (UCHAR*) sTmp, iRet << 1, LEFT_ALIGN);

    /* TPDUԴ��ַĿ�ĵ�ַת�� */
    GET_STR_KEY(pstDataJson, "tpdu", sTpdu);
    tStrCpy(sTmp, sTpdu + 2, 4);
    memcpy(sTpdu + 2, sTpdu + 6, 4);
    memcpy(sTpdu + 6, sTmp, 4);

    tDumpIso(&g_stIsoData, "Ӧ����");

    /* ǩ��Ӧ����д����Ҫ�� */
    GET_STR_KEY(pstDataJson, "req_flag", sReqFlag);
    {
        /* ����Ҫ���Ȳ��� */
    }

    memcpy(sMsg, sTpdu, 10);
    GET_STR_KEY(pstDataJson, "head", sHead);
    memcpy(sMsg + 10, sHead, 12);
    SET_STR_KEY(pstRepJson, "msg", (const char*) sMsg);
    return 0;
}


