/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_config.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"
#include "t_macro.h"
#include "agent.h"

int FindMerchNameACode(char *pcMerchName, char *pcMerchId, char *pcUserCode) {
    char sSqlStr[2048] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select  merch_name,user_code from B_MERCH where merch_id='%s' ", pcMerchId);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "查找商户[%s]名称失败.", pcMerchId);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcMerchName);
        STRV(pstRes, 2, pcUserCode);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无商户[%s]姓名.", pcMerchId);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    return 0;
}

int FindAgentTempFeeType(int iAgentLevel, char *pcAgentId, char *pcTempFeeType) {
    char sSqlStr[2048] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;
    //如果当前代理商是1级代理商取出费率模板类型Fee_type 用于分公司分润的计算
    if (iAgentLevel == 1) {
        snprintf(sSqlStr, sizeof (sSqlStr), "select  af.fee_type from B_AGENT_FEE a  join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE "
                "where a.agent_id = '%s' ", pcAgentId);
        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "查找[%s]Fee_Type失败.", pcAgentId);
            return -1;
        }
        if (NULL == pstRes) {
            tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
            return -1;
        }
        while (OCI_FetchNext(pstRes)) {
            STRV(pstRes, 1, pcTempFeeType);
        }

        if (0 == OCI_GetRowCount(pstRes)) {
            tLog(ERROR, "无代理商[%s]机构信息.", pcAgentId);
            tReleaseResEx(pstState);
            OCI_StatementFree(pstState);
            return -1;
        }
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return 0;
        //DEL_KEY(pstAgentJson, "temp_fee_type");
        //SET_STR_KEY(pstAgentJson, "temp_fee_type", sTempFeeType);
    }
    return -1;
}

int FindSubCompFeeById(AgentFee *pstAgentFee, char *pcAgentId, char *pcTempFeeType) {
    char sMerchId[] = {0}, sSqlStr[2048] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;
    /* 查找商户所属代理商 */
    if (pcTempFeeType[0] == '0') {
        snprintf(sSqlStr, sizeof (sSqlStr), "select a.agent_id,C_FEE_RATE,C_FEE_MAX,C_FEE_RATIO"
                ",CY_FEE_RATE,CY_FEE_MAX,CY_FEE_RATIO"
                ",CM_FEE_RATE,CM_FEE_MAX,CM_FEE_RATIO"
                ",D_FEE_RATE,D_FEE_MAX,D_FEE_RATIO"
                ",DY_FEE_RATE,DY_FEE_MAX,DY_FEE_RATIO"
                ",DM_FEE_RATE,DM_FEE_MAX,DM_FEE_RATIO"
                ",WX_FEE_RATE,WX_FEE_MAX,WX_FEE_RATIO"
                ",BB_FEE_RATE,BB_FEE_MAX,BB_FEE_RATIO"
                ",YC_FREE_FEE_RATE,YD_FREE_FEE_RATE,YC_NFC_FEE_RATE"
                ",YD_NFC_FEE_RATE,YC_QRCODE_FEE_RATE,YD_QRCODE_FEE_RATE"
                ",YC_FEE_RATIO,YD_FEE_RATIO,D0_FEE_RATE "
                " from B_AGENT_FEE a "
                " join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE"
                " where a.agent_id='%s'", pcAgentId);
    } else if (pcTempFeeType[0] == '1') {
        snprintf(sSqlStr, sizeof (sSqlStr), "select a.agent_id,S_C_FEE_RATE,S_C_FEE_MAX,S_C_FEE_RATIO"
                ",S_CY_FEE_RATE,S_CY_FEE_MAX,S_CY_FEE_RATIO"
                ",S_CM_FEE_RATE,S_CM_FEE_MAX,S_CM_FEE_RATIO"
                ",S_D_FEE_RATE,S_D_FEE_MAX,S_D_FEE_RATIO"
                ",S_DY_FEE_RATE,S_DY_FEE_MAX,S_DY_FEE_RATIO"
                ",S_DM_FEE_RATE,S_DM_FEE_MAX,S_DM_FEE_RATIO"
                ",S_WX_FEE_RATE,S_WX_FEE_MAX,S_WX_FEE_RATIO"
                ",S_BB_FEE_RATE,S_BB_FEE_MAX,S_BB_FEE_RATIO"
                ",S_YC_FREE_FEE_RATE,S_YD_FREE_FEE_RATE,S_YC_NFC_FEE_RATE"
                ",S_YD_NFC_FEE_RATE,S_YC_QRCODE_FEE_RATE,S_YD_QRCODE_FEE_RATE"
                ",S_YC_FEE_RATIO,S_YD_FEE_RATIO,S_D0_FEE_RATE"
                " from B_AGENT_FEE a "
                " join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE"
                " where a.agent_id='%s'", pcAgentId);
    }
    tLog(ERROR, "sql[%s].", sSqlStr);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "查找代理商费率[%s]资料,失败.", pcAgentId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstAgentFee->sAgentId);
        DOUV(pstRes, 2, pstAgentFee->dCRate);
        DOUV(pstRes, 3, pstAgentFee->dCMax);
        DOUV(pstRes, 4, pstAgentFee->dCRadio);
        DOUV(pstRes, 5, pstAgentFee->dCYRate);
        DOUV(pstRes, 6, pstAgentFee->dCYMax);
        DOUV(pstRes, 7, pstAgentFee->dCYRadio);
        DOUV(pstRes, 8, pstAgentFee->dCMRate);
        DOUV(pstRes, 9, pstAgentFee->dCMMax);
        DOUV(pstRes, 10, pstAgentFee->dCMRadio);
        DOUV(pstRes, 11, pstAgentFee->dDRate);
        DOUV(pstRes, 12, pstAgentFee->dDMax);
        DOUV(pstRes, 13, pstAgentFee->dDRadio);
        DOUV(pstRes, 14, pstAgentFee->dDYRate);
        DOUV(pstRes, 15, pstAgentFee->dDYMax);
        DOUV(pstRes, 16, pstAgentFee->dDYRadio);
        DOUV(pstRes, 17, pstAgentFee->dDMRate);
        DOUV(pstRes, 18, pstAgentFee->dDMMax);
        DOUV(pstRes, 19, pstAgentFee->dDMRadio);
        DOUV(pstRes, 20, pstAgentFee->dWxRate);
        DOUV(pstRes, 21, pstAgentFee->dWxMax);
        DOUV(pstRes, 22, pstAgentFee->dWxRadio);
        DOUV(pstRes, 23, pstAgentFee->dBbRate);
        DOUV(pstRes, 24, pstAgentFee->dBbMax);
        DOUV(pstRes, 25, pstAgentFee->dBbRadio);
        DOUV(pstRes, 26, pstAgentFee->dYc2FeeCRate);
        DOUV(pstRes, 27, pstAgentFee->dYc2FeeDRate);
        DOUV(pstRes, 28, pstAgentFee->dYcNfcCRate);
        DOUV(pstRes, 29, pstAgentFee->dYcNfcDRate);
        DOUV(pstRes, 30, pstAgentFee->dYcQrCRate);
        DOUV(pstRes, 31, pstAgentFee->dYcQrDRate);
        DOUV(pstRes, 32, pstAgentFee->dYcRadio);
        DOUV(pstRes, 33, pstAgentFee->dYdRadio);
        DOUV(pstRes, 34, pstAgentFee->dD0Rate);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无代理商[%s]费率资料,请先添加机构.", pcAgentId);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    return 0;
}

int FindAgentFeeById(AgentFee *pstAgentFee, char *pcAgentId) {
    char sMerchId[] = {0}, sSqlStr[2048] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;
    /* 查找商户所属代理商 */
    snprintf(sSqlStr, sizeof (sSqlStr), "select a.agent_id,C_FEE_RATE,C_FEE_MAX,C_FEE_RATIO"
            ",CY_FEE_RATE,CY_FEE_MAX,CY_FEE_RATIO"
            ",CM_FEE_RATE,CM_FEE_MAX,CM_FEE_RATIO"
            ",D_FEE_RATE,D_FEE_MAX,D_FEE_RATIO"
            ",DY_FEE_RATE,DY_FEE_MAX,DY_FEE_RATIO"
            ",DM_FEE_RATE,DM_FEE_MAX,DM_FEE_RATIO"
            ",WX_FEE_RATE,WX_FEE_MAX,WX_FEE_RATIO"
            ",BB_FEE_RATE,BB_FEE_MAX,BB_FEE_RATIO"
            ",YC_FREE_FEE_RATE,YD_FREE_FEE_RATE,YC_NFC_FEE_RATE"
            ",YD_NFC_FEE_RATE,YC_QRCODE_FEE_RATE,YD_QRCODE_FEE_RATE"
            ",YC_FEE_RATIO,YD_FEE_RATIO,D0_FEE_RATE "
            " from B_AGENT_FEE a "
            " join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE"
            " where a.agent_id='%s'", pcAgentId);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "查找代理商费率[%s]资料,失败.", pcAgentId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstAgentFee->sAgentId);
        DOUV(pstRes, 2, pstAgentFee->dCRate);
        DOUV(pstRes, 3, pstAgentFee->dCMax);
        DOUV(pstRes, 4, pstAgentFee->dCRadio);
        DOUV(pstRes, 5, pstAgentFee->dCYRate);
        DOUV(pstRes, 6, pstAgentFee->dCYMax);
        DOUV(pstRes, 7, pstAgentFee->dCYRadio);
        DOUV(pstRes, 8, pstAgentFee->dCMRate);
        DOUV(pstRes, 9, pstAgentFee->dCMMax);
        DOUV(pstRes, 10, pstAgentFee->dCMRadio);
        DOUV(pstRes, 11, pstAgentFee->dDRate);
        DOUV(pstRes, 12, pstAgentFee->dDMax);
        DOUV(pstRes, 13, pstAgentFee->dDRadio);
        DOUV(pstRes, 14, pstAgentFee->dDYRate);
        DOUV(pstRes, 15, pstAgentFee->dDYMax);
        DOUV(pstRes, 16, pstAgentFee->dDYRadio);
        DOUV(pstRes, 17, pstAgentFee->dDMRate);
        DOUV(pstRes, 18, pstAgentFee->dDMMax);
        DOUV(pstRes, 19, pstAgentFee->dDMRadio);
        DOUV(pstRes, 20, pstAgentFee->dWxRate);
        DOUV(pstRes, 21, pstAgentFee->dWxMax);
        DOUV(pstRes, 22, pstAgentFee->dWxRadio);
        DOUV(pstRes, 23, pstAgentFee->dBbRate);
        DOUV(pstRes, 24, pstAgentFee->dBbMax);
        DOUV(pstRes, 25, pstAgentFee->dBbRadio);
        DOUV(pstRes, 26, pstAgentFee->dYc2FeeCRate);
        DOUV(pstRes, 27, pstAgentFee->dYc2FeeDRate);
        DOUV(pstRes, 28, pstAgentFee->dYcNfcCRate);
        DOUV(pstRes, 29, pstAgentFee->dYcNfcDRate);
        DOUV(pstRes, 30, pstAgentFee->dYcQrCRate);
        DOUV(pstRes, 31, pstAgentFee->dYcQrDRate);
        DOUV(pstRes, 32, pstAgentFee->dYcRadio);
        DOUV(pstRes, 33, pstAgentFee->dYdRadio);
        DOUV(pstRes, 34, pstAgentFee->dD0Rate);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无代理商[%s]费率资料,请先添加机构.", pcAgentId);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    return 0;
}

int CalcAgentFee(double *dTotalAmt, double *dTotalFee, char *pcType, char *pcFeeType, double *dAgentFee, double *dAgentCost, AgentFee *pstAgentFee) {

    if (pcType == NULL) {
        if (strcmp(pcFeeType, "BB") == 0) {
            //代理商成本
            *dAgentCost = *dTotalAmt * (pstAgentFee->dBbRate) / 100;
            tLog(DEBUG, "费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dBbRate / 100, *dAgentCost, pstAgentFee->dBbMax);
            if (!DBL_ZERO(pstAgentFee->dBbMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dBbMax) ? pstAgentFee->dBbMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dBbRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dBbRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "WX") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dWxRate) / 100;
            tLog(DEBUG, "费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dWxRate / 100, *dAgentCost, pstAgentFee->dWxMax);
            if (!DBL_ZERO(pstAgentFee->dWxMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dWxMax) ? pstAgentFee->dWxMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dWxRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dWxRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "D0") == 0) {
            //代理商成本
            *dAgentCost = *dTotalAmt * (pstAgentFee->dD0Rate) / 100;
            tLog(DEBUG, "费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dD0Rate / 100, *dAgentCost, 0);
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dCRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dCRadio / 100, *dAgentFee);
        }
    } else if (strcmp(pcType, "0") == 0) {//借记卡类
        //标准类
        if (strcmp(pcFeeType, "B") == 0) {
            //代理商成本
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDRate / 100, *dAgentCost, pstAgentFee->dDMax);
            if (!DBL_ZERO(pstAgentFee->dDMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMax) ? pstAgentFee->dDMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dDRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dDRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "Y") == 0) { //优惠类
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDYRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDYRate / 100, *dAgentCost, pstAgentFee->dDYMax);
            if (!DBL_ZERO(pstAgentFee->dDYMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDYMax) ? pstAgentFee->dDYMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dDYRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dDYRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "M") == 0) { //减免类
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDMRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDMRate / 100, *dAgentCost, pstAgentFee->dDMMax);
            if (!DBL_ZERO(pstAgentFee->dDMMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dDMRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dDMRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "YN") == 0) { //云闪付NFC
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYcNfcDRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYcNfcDRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dYdRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dYdRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "YM") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYc2FeeDRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYc2FeeDRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dYdRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dYdRadio / 100, *dAgentFee);
        }

        //贷记卡类
    } else if (strcmp(pcType, "1") == 0) {
        //标准类
        if (strcmp(pcFeeType, "B") == 0) {
            //代理商成本
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCRate / 100, *dAgentCost, pstAgentFee->dCMax);
            if (!DBL_ZERO(pstAgentFee->dCMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCMax) ? pstAgentFee->dCMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dCRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dCRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "Y") == 0) { //优惠类
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCYRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCYRate / 100, *dAgentCost, pstAgentFee->dCYMax);
            if (!DBL_ZERO(pstAgentFee->dCYMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCYMax) ? pstAgentFee->dCYMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dCYRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dCYRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "M") == 0) { //减免类
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCMRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCMRate / 100, *dAgentCost, pstAgentFee->dCMMax);
            if (!DBL_ZERO(pstAgentFee->dCMMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCMMax) ? pstAgentFee->dCMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dCMRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dCMRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "YN") == 0) { //云闪付NFC
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYcNfcCRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYcNfcCRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dYcRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dYcRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "YM") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYc2FeeCRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYc2FeeCRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dYcRadio / 100;
            tLog(DEBUG, "商户手续费[%f], 代理商成本[%f], 代理商返佣比例[%lf], 代理商分润[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dYcRadio / 100, *dAgentFee);
        }
    }
    return 0;
}

int CalcParentAgentFee(double *dTotalAmt, double *dAgentCostDown, double *dFee, char *pcType, char *pcFeeType, double *dAgentFee, double *dAgentCost, AgentFee *pstAgentFee, double dRatio) {
    //代理商分润=（商户手续费-本级成本）X本级返佣比例-（商户手续费-下级成本）X下级返佣比例
    if (strcmp(pcType, "0") == 0) {
        if (strcmp(pcFeeType, "B") == 0) {
            //代理商成本
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDRate / 100, *dAgentCost, pstAgentFee->dDMax);
            if (!DBL_ZERO(pstAgentFee->dDMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMax) ? pstAgentFee->dDMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dDRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dDRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf],代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dDRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "Y") == 0) { //优惠类
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDYRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDYRate / 100, *dAgentCost, pstAgentFee->dDYMax);
            if (!DBL_ZERO(pstAgentFee->dDYMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDYMax) ? pstAgentFee->dDYMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dDYRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dDYRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dDYRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "M") == 0) { //减免类
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDMRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDMRate / 100, *dAgentCost, pstAgentFee->dDMMax);
            if (!DBL_ZERO(pstAgentFee->dDMMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dDMRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dDMRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dDMRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "YN") == 0) { //云闪付NFC
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYcNfcDRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYcNfcDRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dYdRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dYdRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dYdRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "YM") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYc2FeeDRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYc2FeeDRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dYdRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dYdRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dYdRadio / 100, dRatio/100, *dFee, *dAgentFee);
        }

    } else if (strcmp(pcType, "1") == 0) {
        //标准类
        if (strcmp(pcFeeType, "B") == 0) {
            //代理商成本
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCRate / 100, *dAgentCost, pstAgentFee->dCMax);
            if (!DBL_ZERO(pstAgentFee->dCMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCMax) ? pstAgentFee->dCMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dCRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dCRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dCRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "Y") == 0) { //优惠类
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCYRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCYRate / 100, *dAgentCost, pstAgentFee->dCYMax);
            if (!DBL_ZERO(pstAgentFee->dCYMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCYMax) ? pstAgentFee->dCYMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dCYRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dCYRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dCYRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "M") == 0) { //减免类
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCMRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCMRate / 100, *dAgentCost, pstAgentFee->dCMMax);
            if (!DBL_ZERO(pstAgentFee->dCMMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCMMax) ? pstAgentFee->dCMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dCMRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dCMRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dCMRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "YN") == 0) { //云闪付NFC
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYcNfcCRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYcNfcCRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dYcRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dYcRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dYcRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "YM") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYc2FeeCRate) / 100;
            tLog(DEBUG, "卡类型[%s] 费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYc2FeeCRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dYcRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dYcRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dYcRadio / 100, dRatio/100, *dFee, *dAgentFee);
        }
    } else if (strcmp(pcType, "3") == 0) {
        if (strcmp(pcFeeType, "WX") == 0) { //微信
            *dAgentCost = *dTotalAmt * (pstAgentFee->dWxRate) / 100;
            tLog(DEBUG, "费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dWxRate / 100, *dAgentCost, pstAgentFee->dWxMax);
            //if (!DBL_ZERO(pstAgentFee->dWxMax))
            //*dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dWxMax) ? pstAgentFee->dWxMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dWxRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dWxRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dWxRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "BB") == 0) {//支付宝
            *dAgentCost = *dTotalAmt * (pstAgentFee->dBbRate) / 100;
            tLog(DEBUG, "费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dBbRate / 100, *dAgentCost, pstAgentFee->dBbMax);
            //if (!DBL_ZERO(pstAgentFee->dBbMax))
            //*dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dBbMax) ? pstAgentFee->dBbMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dBbRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dBbRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dBbRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "D0") == 0) {//D0
            *dAgentCost = *dTotalAmt * (pstAgentFee->dD0Rate) / 100;
            tLog(DEBUG, "费率类型[%s] 总交易金额[%f], 代理商成本费率[%lf], 代理商成本[%f], 代理商成本最大值[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dD0Rate / 100, *dAgentCost, 0);
            //if (!DBL_ZERO(pstAgentFee->dBbMax))
            //*dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dBbMax) ? pstAgentFee->dBbMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //代理商分润
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dCRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dCRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "下游代理商成本[%f], 代理商成本[%f], 代理商返佣比例[%lf], 下级代理商返佣比例[%f], 商户手续费[%lf], 代理商分润[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dCRadio / 100, dRatio/100, *dFee, *dAgentFee);
        }
    }
    return 0;
}

int D0FeePro(char *pcTransDate) {
    cJSON *pstJson = NULL, *pstTransJson = NULL, *pstAgentJson = NULL;
    double dAmount = 0, dFee = 0;
    int iAgentLevelMax = 0, i = 0, j, iCnt;
    char sRrn[12 + 1] = {0}, sMerchId[15 + 1] = {0}, sId[32 + 1] = {0}, sMerchName[500] = {0};
    char sSqlStr[1024] = {0}, sTransDate[8 + 1] = {0}, sUserCode[15 + 1] = {0};
    Agent stAgent;
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }

    pstTransJson = cJSON_CreateObject();
    if (NULL == pstTransJson) {
        cJSON_Delete(pstJson);
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }

    pstAgentJson = cJSON_CreateObject();
    if (NULL == pstAgentJson) {
        cJSON_Delete(pstJson);
        cJSON_Delete(pstTransJson);
        tLog(ERROR, "创建发送Json失败.");
        return -1;
    }

    //获取交易信息
    snprintf(sSqlStr, sizeof (sSqlStr), "select id,amount,merch_id,p_fee,trans_date,cnt from "
            "(select * from b_d0_withdraw_trans_detail order by dbms_random.value) where cal_flag = 'N' and rownum=1");
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "提取一条提现记录失败.");
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sId);
        DOUV(pstRes, 2, dAmount);
        STRV(pstRes, 3, sMerchId);
        DOUV(pstRes, 4, dFee);
        STRV(pstRes, 5, sTransDate);
        INTV(pstRes, 6, iCnt);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无需要计算的D0交易流水.");
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        sleep(60);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);

    //获取商户对应代理商信息
    if (FindAgentByMerhId(&stAgent, sMerchId) < 0) {
        tLog(DEBUG, "未找到商户[%s]所属的代理商.", sMerchId);
    }

    if (FindMerchNameACode(sMerchName, sMerchId, sUserCode) < 0) {
        tLog(DEBUG, "未找到商户[%s]所属的代理商.", sMerchId);
    }

    tLog(DEBUG, "agent_id[%s],agent_name[%s],agent_status[%s],zmk_key[%s],agent_level[%s],grade_agent_level[%s] ", stAgent.sAgentId, stAgent.sAgentname, stAgent.sStatus, stAgent.sZmk, stAgent.sAgentLevel, stAgent.sGradeAgentLevel);
    SET_STR_KEY(pstAgentJson, "agent_id", stAgent.sAgentId);
    SET_STR_KEY(pstAgentJson, "agent_name", stAgent.sAgentname);
    SET_STR_KEY(pstAgentJson, "agent_status", stAgent.sStatus);
    SET_STR_KEY(pstAgentJson, "zmk_key", stAgent.sZmk);
    SET_STR_KEY(pstAgentJson, "agent_level", stAgent.sAgentLevel);
    SET_STR_KEY(pstAgentJson, "grade_agent_level", stAgent.sGradeAgentLevel);
    SET_JSON_KEY(pstJson, "agent", pstAgentJson);
    tLog(DEBUG, "id[%s],merch_id[%s],amount[%f] ", sId, sMerchId, dAmount);
    SET_STR_KEY(pstTransJson, "id", sId);
    SET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    SET_DOU_KEY(pstTransJson, "amount", dAmount);
    SET_DOU_KEY(pstTransJson, "fee", dFee);
    SET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    SET_STR_KEY(pstTransJson, "merch_name", sMerchName);
    SET_STR_KEY(pstTransJson, "user_code", sUserCode);
    SET_INT_KEY(pstTransJson, "count", iCnt);
    SET_JSON_KEY(pstJson, "data", pstTransJson);

    //获取代理商级别
    tLog(DEBUG, "最大代理商级别%d", atoi(stAgent.sAgentLevel));
    for (i = atoi(stAgent.sAgentLevel), j = 0; i >= 0; i--, j++) {
        AgentProcForeach(i, j, pstJson);
    }
    cJSON_Delete(pstJson);
}

int AgentProcForeach(int iAgentLevel, int j, cJSON *pstJson) {
    cJSON *pstTransJson = NULL, *pstAgentJson = NULL;
    char sSqlStr[2048] = {0}, sSql[2048] = {0};
    char sAgentId[8 + 1] = {0}, sId[32 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sType[1 + 1] = {0}, sFeeType[1 + 1] = {0}, sAgentlevel[2 + 1] = {0}, sRrn[12 + 1] = {0}, sGradeAgentLevel[3 + 1] = {0};
    char sAgentName[255 + 1] = {0}, sTempFeeType[1 + 1] = {0}, sMerchName[500] = {0}, sUserCode[15 + 1] = {0}, sTransDate[8 + 1] = {0};
    double dTotalAmt = 0.0, dTotalFee = 0.0, dAgentFee = 0.0, dAgentCost = 0.0, dRatio = 0.0;
    double dAmt = 0.0, dFee = 0.0, dAgentCostDown = 0.0, dFee1 = 0.0;
    int iCnt = 0, iNo = 0;
    AgentFee stAgentFee;
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;

    tLog(DEBUG, "iAgentLevel[%d], j[%d]", iAgentLevel, j);
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    pstAgentJson = GET_JSON_KEY(pstJson, "agent");

    //获取交易信息
    GET_DOU_KEY(pstTransJson, "amount", dAmt);
    //GET_STR_KEY(pstTransJson, "card_type", sCardType);
    //GET_STR_KEY(pstTransJson, "fee_type", sFeeType);
    GET_STR_KEY(pstTransJson, "id", sId);
    //获取代理商信息      
    GET_STR_KEY(pstAgentJson, "agent_id", sAgentId);
    GET_STR_KEY(pstAgentJson, "agent_level", sAgentlevel);
    GET_STR_KEY(pstAgentJson, "grade_agent_level", sGradeAgentLevel);
    GET_STR_KEY(pstAgentJson, "agent_name", sAgentName);
    //如果当前代理商是1级代理商取出费率模板类型Fee_type 用于分公司分润的计算
    if (iAgentLevel == 1) {
        if (FindAgentTempFeeType(iAgentLevel, sAgentId, sTempFeeType) == 0) {
            DEL_KEY(pstAgentJson, "temp_fee_type");
            SET_STR_KEY(pstAgentJson, "temp_fee_type", sTempFeeType);
            tLog(DEBUG, "代理商[%s]temp_fee_type[%s].", sAgentId, sTempFeeType);
        } else {
            tLog(ERROR, "查找代理商[%s]temp_fee_type失败.", sAgentId);
            return -1;
        }
    }
    if (sAgentlevel[0] > '0') {
        if (FindAgentFeeById(&stAgentFee, sAgentId) < 0) {
            tLog(ERROR, "获取代理商[%s]费率失败.", sAgentId);
        }
    } else {
        GET_STR_KEY(pstAgentJson, "temp_fee_type", sTempFeeType);
        if (FindSubCompFeeById(&stAgentFee, sAgentId, sTempFeeType) < 0) {
            tLog(ERROR, "获取代理商[%s]费率失败.", sAgentId);
        }
    }
    //计算商户所属代理商分润以及成本
    if (j == 0) {
        GET_DOU_KEY(pstTransJson, "fee", dFee);
        GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
        GET_STR_KEY(pstTransJson, "merch_name", sMerchName);
        GET_STR_KEY(pstTransJson, "user_code", sUserCode);
        GET_INT_KEY(pstTransJson, "count", iCnt);
        GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
        if (CalcAgentFee(&dAmt, &dFee, NULL, "D0", &dAgentFee, &dAgentCost, &stAgentFee) < 0) {
            tLog(ERROR, "代理商[%s]费率计算失败.", sAgentId);
        }
        dAgentCostDown = dAgentCost;
        //dFee1 = dFee - dAgentCost - dAgentFee;

        //插入流水附加表
        snprintf(sSqlStr, sizeof (sSqlStr), "insert into b_append_trans_detail(ref_id,group_code,trans_date,settle_date,merch_name,user_code,amount,merch_fee,check_flag,cnt,merch_id) "
                "values ('%s','D0','%s','%s','%s','%s','%lf','%lf','Y','%d','%s') ", sId,sTransDate,sTransDate,sMerchName,sUserCode,dAmt,dFee,iCnt,sMerchId);
        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "插入流水附加表.");
            return -1;
        }
        tLog(DEBUG, "影响记录数[%d]", OCI_GetAffectedRows(pstState));
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);

    } else { //计算商户所属代理商分润以及成本
        GET_DOU_KEY(pstAgentJson, "agent_cost_down", dAgentCostDown);
        GET_DOU_KEY(pstTransJson, "fee", dFee);
		GET_DOU_KEY(pstAgentJson, "dratio", dRatio);
        if (CalcParentAgentFee(&dAmt, &dAgentCostDown, &dFee, "3", "D0", &dAgentFee, &dAgentCost, &stAgentFee, dRatio) < 0) {
            tLog(ERROR, "父代理商[%s]费率计算失败.", sAgentId);
        }
        //dFee1 = dAgentCostDown - dAgentCost + dFee1 - dAgentFee;
        dAgentCostDown = dAgentCost;
        //DEL_KEY(pstAgentJson, "fee1");
        DEL_KEY(pstAgentJson, "agent_cost_down");
        DEL_KEY(pstAgentJson, "dratio");

    }
    //更新分润
    snprintf(sSqlStr, sizeof (sSqlStr), "update b_append_trans_detail set agent_id%s = '%s', agent_name%s = '%s', fee%s = %f, cost_amount%s = %f "
            ",ratio%s = %f, grade_agent_level%s = '%s' where ref_id ='%s' ", sAgentlevel, sAgentId, sAgentlevel, sAgentName, sAgentlevel, dAgentFee, sAgentlevel, dAgentCost, sAgentlevel, stAgentFee.dCRadio, sAgentlevel, sGradeAgentLevel, sId);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "更新[%s]分润,失败.", sAgentId);
        return -1;
    }
    tLog(DEBUG, "影响记录数[%d]", OCI_GetAffectedRows(pstState));
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);

    //查找上级代理商信息用于下次处理
    if (iAgentLevel > 0) {
        snprintf(sSqlStr, sizeof (sSqlStr), "select agent_id,agent_name,agent_level,grade_agent_level from b_agent where agent_id = (select p_agent_id from b_agent "
                "where agent_id = '%s') ", sAgentId);
        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "更新[%s]分润,失败.", sAgentId);
            return -1;
        }
        if (NULL == pstRes) {
            tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
            tReleaseResEx(pstState);
            OCI_StatementFree(pstState);
            return -1;
        }
        while (OCI_FetchNext(pstRes)) {
            STRV(pstRes, 1, sAgentId);
            STRV(pstRes, 2, sAgentName);
            STRV(pstRes, 3, sAgentlevel);
            STRV(pstRes, 4, sGradeAgentLevel);
        }

        if (0 == OCI_GetRowCount(pstRes)) {
            tLog(ERROR, "无代理商[%s]上级机构信息.", sAgentId);
            tReleaseResEx(pstState);
            OCI_StatementFree(pstState);
            return -1;
        }
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        DEL_KEY(pstAgentJson, "agent_id");
        DEL_KEY(pstAgentJson, "agent_level");
        DEL_KEY(pstAgentJson, "grade_agent_level");
        DEL_KEY(pstAgentJson, "agent_name");
        DEL_KEY(pstAgentJson, "agent_cost_down");

        SET_STR_KEY(pstAgentJson, "agent_id", sAgentId);
        SET_STR_KEY(pstAgentJson, "agent_level", sAgentlevel);
        SET_STR_KEY(pstAgentJson, "grade_agent_level", sGradeAgentLevel);
        SET_STR_KEY(pstAgentJson, "agent_name", sAgentName);
        SET_DOU_KEY(pstAgentJson, "agent_cost_down", dAgentCostDown);
        //SET_DOU_KEY(pstAgentJson, "fee1", dFee1);
        SET_DOU_KEY(pstAgentJson, "dratio", stAgentFee.dCRadio);
    } else {
        //更新CAL_FLAG标识 Y:已计算分润 N:未计算分润
        snprintf(sSqlStr, sizeof (sSqlStr), "update b_d0_withdraw_trans_detail set CAL_FLAG='Y' where id ='%s' ", sId);
        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "更新[%s]分润计算标识失败.", sId);
            return -1;
        }
        tLog(DEBUG, "影响记录数[%d]", OCI_GetAffectedRows(pstState));
        tCommit();
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
    }


    tLog(ERROR, "获取代理商[%s]分润[%f]成本[%f].", sAgentId, dAgentFee, dAgentCost);
    tLog(DEBUG, "代理商[%s:%s]%s.", sAgentId, sType, sAgentName);

    return 0;
}
