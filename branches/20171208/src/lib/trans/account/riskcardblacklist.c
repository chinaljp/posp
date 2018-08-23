#include "t_cjson.h"
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "merch.h"
#include "t_cjson.h"
#include "resp_code.h"
#include "postransdetail.h"
#include "card.h"

/******************************************************************************/
/*      函数名:     RiskCardBlackList()                                        */
/*      功能说明:   银行卡黑名单                          */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int RiskCardBlackList(cJSON *pstJson, int *piFlag) {
    int iRet = 0;
    char sCardNo[19 + 1] = {0}, sECardNo[19 + 1] = {0}, sSql[128] = {0};
    char sKey[128 + 1] = {0};
    cJSON * pstTransJson;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    if (FindValueByKey(sKey, "kms.encdata.key") < 0) {
        ErrHanding(pstTransJson, "96", "查找秘钥失败");
        tLog(ERROR, "查找key[kms.encdata.key]值,失败.");
        return -1;
    }
    if (tHsm_Enc_Asc_Data(sECardNo, sKey, sCardNo) < 0) {
        ErrHanding(pstTransJson, "96", "加密卡号数据失败.");
        tLog(ERROR, "加密卡号数据失败.");
        return -1;
    }
    tLog(ERROR, "加密卡号数据[%s]", sECardNo);
    /* 银行卡黑名单   */
    sprintf(sSql, "SELECT COUNT(1) FROM B_RISK_BLACK_CARD WHERE STATUS='1' AND CARD_NO_ENCRYPT='%s'", sECardNo);
    iRet = tQueryCount(sSql);
    if (iRet > 0) {
        ErrHanding(pstTransJson, "62", "不受理此卡[%s].", sCardNo);
        return -1;
    }
    tLog(INFO, "卡号[%s]黑名单检查通过.", sCardNo);
    return 0;
}