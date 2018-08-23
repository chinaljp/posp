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
#include "t_cjson.h"
#include "merch.h"
#include "term.h"
#include"s_param.h"

int TermInit(cJSON *pstJson, int *piFlag) {
    char sErr[128] = {0};
    char sDate[16] = {0};
    char sTermfatory[20] = {0}, sTermSn[20] = {0};
    char cBindFlag, cInitFlag, sTermId[9] = {0}, sMerchId[16] = {0};
    cJSON *pstTransJson = NULL;
    cJSON *pstDate;

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "term_fatory", sTermfatory);
    GET_STR_KEY(pstTransJson, "term_sn", sTermSn);

    tLog(DEBUG, "厂家[%s],SN[%s].", sTermfatory, sTermSn);

    tTrim(sTermfatory);
    tTrim(sTermSn);

    /* 通过终端厂家和SN序列号，查找商户号和终端号 */
    if (FindTermIdBySn(sMerchId, sTermId, sTermfatory, sTermSn) < 0) {
        ErrHanding(pstTransJson, "97", "获取终端厂家[%s]SN序列号[%s]的商户号,终端号信息失败.", sTermfatory, sTermSn);
        return -1;
    }

    if (sMerchId[0] == '\0') {
        ErrHanding(pstTransJson, "97", "获取终端厂家[%s]SN序列号[%s]未绑定商户.", sTermfatory, sTermSn);
        return -1;
    }
    SET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    SET_STR_KEY(pstTransJson, "term_id", sTermId);

    /* 更新下载标志 */
    {
        /* 还没处理 */
    }

    tLog(INFO, "获取终端厂家[%s]SN序列号[%s]的商户号[%s]终端号[%s]成功.", \
        sTermfatory, sTermSn, sMerchId, sTermId);

    return 0;
}


/******************************************************************************/
/*      函数名:     DownParam()                  	                          */
/*      功能说明:   下发广告，商户名称   					  */
/*      输入参数:   cJSON *pstTranData                                        */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int DownParam(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson;
    char sMerchId[16] = {0}, sTermId[9] = {0};
    char sBitmap[33] = {0}, sAdvertise[121] = {0};
    Merch stMerch;

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    tLog(INFO, "获取商户名:商户号[%s],终端号[%s]", sMerchId, sTermId);

    /* 获取商户名称 */
    MEMSET_ST(stMerch);
    if (FindMerchInfoById(&stMerch, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "获取商户号[%s]信息失败.", sMerchId);
        return -1;
    }

    /* 凭条打印名称 */
    SET_STR_KEY(pstTransJson, "merch_p_name", stMerch.sPName);
    /*广告语，但是都没有使用,下发统一的，在系统参数表里配置,即使没配置，也不影响使用，即不判断失败情况 */
    FindValueByKey(sAdvertise, "ADVERT");
    SET_STR_KEY(pstTransJson, "advert", sAdvertise);

    /* 交易位图在商户规则表里，但是现在还没有，先使用所有的交易位图，做测试 */
    strcpy(sBitmap, "11111111111111111111111111111111");
    SET_STR_KEY(pstTransJson, "bitmap", sBitmap);

    tLog(INFO, "商户号[%s]终端号[%s]下发商户名称[%s],广告[%s]成功,位图[%s]测试位图.", sMerchId, sTermId, stMerch.sPName, sAdvertise, sBitmap);

    return 0;
}