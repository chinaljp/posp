#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "quickpay.h"

/*EPOS 无卡快捷支付结果查询 从交易流水表中获取原交易信息 (已撤销的交易允许查询)*/
int ChkOrTransMsg(cJSON *pstJson, int *piFlag) {

    cJSON *pstTransJson = NULL;
    QuickPayDetail stQuickPayDetail;
    char sRrn[12 + 1];
    
    MEMSET_ST(stQuickPayDetail);
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "o_rrn", sRrn);
    if (FindOriTransMsg( &stQuickPayDetail,sRrn ) < 0) {
        ErrHanding(pstTransJson, "25", "原交易检查失败，无原交易流水，原交易rrn[%s].", sRrn);
        return ( -1);
    }
    tLog(DEBUG, "sValidFlag = [%s]", stQuickPayDetail.sValidFlag);
    
    SET_DOU_KEY(pstTransJson,"amount",stQuickPayDetail.dAmount);
    SET_STR_KEY(pstTransJson,"qr_order_no",stQuickPayDetail.sMerchOrderNo);
    //SET_STR_KEY(pstTransJson,"o_trans_time",stQuickPayDetail.sTransTime);
    SET_STR_KEY(pstTransJson,"o_trans_date",stQuickPayDetail.sTransDate);

    return ( 0 );
}