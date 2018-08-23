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

int FindMerchName(char *pcMerchName, char *pcMerchId) {
    char sSqlStr[2048] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;

    snprintf(sSqlStr, sizeof (sSqlStr), "select  merch_name from B_MERCH where merch_id='%s' ", pcMerchId);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "�����̻�[%s]����ʧ��.", pcMerchId);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcMerchName);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "���̻�[%s]����.", pcMerchId);
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
    //�����ǰ��������1��������ȡ������ģ������Fee_type ���ڷֹ�˾����ļ���
    if (iAgentLevel == 1) {
        snprintf(sSqlStr, sizeof (sSqlStr), "select  af.fee_type from B_AGENT_FEE a  join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE "
                "where a.agent_id = '%s' ", pcAgentId);
        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "����[%s]Fee_Typeʧ��.", pcAgentId);
            return -1;
        }
        if (NULL == pstRes) {
            tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
            return -1;
        }
        while (OCI_FetchNext(pstRes)) {
            STRV(pstRes, 1, pcTempFeeType);
        }

        if (0 == OCI_GetRowCount(pstRes)) {
            tLog(ERROR, "�޴�����[%s]������Ϣ.", pcAgentId);
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
    /* �����̻����������� */
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
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "���Ҵ����̷���[%s]����,ʧ��.", pcAgentId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
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
        tLog(ERROR, "�޴�����[%s]��������,������ӻ���.", pcAgentId);
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
    /* �����̻����������� */
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
        tLog(ERROR, "���Ҵ����̷���[%s]����,ʧ��.", pcAgentId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
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
        tLog(ERROR, "�޴�����[%s]��������,������ӻ���.", pcAgentId);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    return 0;
}

int CalcAgentFee(double *dTotalAmt, double *dTotalFee, char *pcType, char *pcFeeType, double *dAgentFee, double *dAgentCost, AgentFee *pstAgentFee) {

    if (strcmp(pcType, "3") == 0) {
        if (strcmp(pcFeeType, "BB") == 0) {
            //�����̳ɱ�
            *dAgentCost = *dTotalAmt * (pstAgentFee->dBbRate) / 100;
            tLog(DEBUG, "��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dBbRate / 100, *dAgentCost, pstAgentFee->dBbMax);
            if (!DBL_ZERO(pstAgentFee->dBbMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dBbMax) ? pstAgentFee->dBbMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dBbRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dBbRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "WX") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dWxRate) / 100;
            tLog(DEBUG, "��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dWxRate / 100, *dAgentCost, pstAgentFee->dWxMax);
            if (!DBL_ZERO(pstAgentFee->dWxMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dWxMax) ? pstAgentFee->dWxMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dWxRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dWxRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "D0") == 0) {
            //�����̳ɱ�
            *dAgentCost = *dTotalAmt * (pstAgentFee->dD0Rate) / 100;
            tLog(DEBUG, "��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dD0Rate / 100, *dAgentCost, 0);
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dCRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dCRadio / 100, *dAgentFee);
        }
    } else if (strcmp(pcType, "0") == 0) {//��ǿ���
        //��׼��
        if (strcmp(pcFeeType, "B") == 0) {
            //�����̳ɱ�
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDRate / 100, *dAgentCost, pstAgentFee->dDMax);
            if (!DBL_ZERO(pstAgentFee->dDMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMax) ? pstAgentFee->dDMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dDRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dDRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "Y") == 0) { //�Ż���
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDYRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDYRate / 100, *dAgentCost, pstAgentFee->dDYMax);
            if (!DBL_ZERO(pstAgentFee->dDYMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDYMax) ? pstAgentFee->dDYMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dDYRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dDYRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "M") == 0) { //������
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDMRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDMRate / 100, *dAgentCost, pstAgentFee->dDMMax);
            if (!DBL_ZERO(pstAgentFee->dDMMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dDMRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dDMRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "YN") == 0) { //������NFC
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYcNfcDRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYcNfcDRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dYdRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dYdRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "YM") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYc2FeeDRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYc2FeeDRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dYdRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dYdRadio / 100, *dAgentFee);
        }

        //���ǿ���
    } else if (strcmp(pcType, "1") == 0) {
        //��׼��
        if (strcmp(pcFeeType, "B") == 0) {
            //�����̳ɱ�
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCRate / 100, *dAgentCost, pstAgentFee->dCMax);
            if (!DBL_ZERO(pstAgentFee->dCMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCMax) ? pstAgentFee->dCMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dCRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dCRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "Y") == 0) { //�Ż���
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCYRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCYRate / 100, *dAgentCost, pstAgentFee->dCYMax);
            if (!DBL_ZERO(pstAgentFee->dCYMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCYMax) ? pstAgentFee->dCYMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dCYRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dCYRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "M") == 0) { //������
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCMRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCMRate / 100, *dAgentCost, pstAgentFee->dCMMax);
            if (!DBL_ZERO(pstAgentFee->dCMMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCMMax) ? pstAgentFee->dCMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dCMRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dCMRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "YN") == 0) { //������NFC
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYcNfcCRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYcNfcCRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dYcRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dYcRadio / 100, *dAgentFee);
        } else if (strcmp(pcFeeType, "YM") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYc2FeeCRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYc2FeeCRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            *dAgentFee = (*dTotalFee - *dAgentCost) * pstAgentFee->dYcRadio / 100;
            tLog(DEBUG, "�̻�������[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �����̷���[%f]", *dTotalFee, *dAgentCost, pstAgentFee->dYcRadio / 100, *dAgentFee);
        }
    }
    return 0;
}

int CalcParentAgentFee(double *dTotalAmt, double *dAgentCostDown, double *dFee, char *pcType, char *pcFeeType, double *dAgentFee, double *dAgentCost, AgentFee *pstAgentFee, double dRatio) {
    //�����̷���=���̻�������-�����ɱ���X������Ӷ����-���̻�������-�¼��ɱ���X�¼���Ӷ����
    if (strcmp(pcType, "0") == 0) {
        if (strcmp(pcFeeType, "B") == 0) {
            //�����̳ɱ�
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDRate / 100, *dAgentCost, pstAgentFee->dDMax);
            if (!DBL_ZERO(pstAgentFee->dDMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMax) ? pstAgentFee->dDMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dDRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dDRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf],�����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dDRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "Y") == 0) { //�Ż���
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDYRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDYRate / 100, *dAgentCost, pstAgentFee->dDYMax);
            if (!DBL_ZERO(pstAgentFee->dDYMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDYMax) ? pstAgentFee->dDYMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dDYRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dDYRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dDYRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "M") == 0) { //������
            *dAgentCost = *dTotalAmt * (pstAgentFee->dDMRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dDMRate / 100, *dAgentCost, pstAgentFee->dDMMax);
            if (!DBL_ZERO(pstAgentFee->dDMMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dDMRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dDMRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dDMRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "YN") == 0) { //������NFC
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYcNfcDRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYcNfcDRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dYdRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dYdRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dYdRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "YM") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYc2FeeDRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYc2FeeDRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dYdRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dYdRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dYdRadio / 100, dRatio/100, *dFee, *dAgentFee);
        }

    } else if (strcmp(pcType, "1") == 0) {
        //��׼��
        if (strcmp(pcFeeType, "B") == 0) {
            //�����̳ɱ�
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCRate / 100, *dAgentCost, pstAgentFee->dCMax);
            if (!DBL_ZERO(pstAgentFee->dCMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCMax) ? pstAgentFee->dCMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dCRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dCRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dCRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "Y") == 0) { //�Ż���
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCYRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCYRate / 100, *dAgentCost, pstAgentFee->dCYMax);
            if (!DBL_ZERO(pstAgentFee->dCYMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCYMax) ? pstAgentFee->dCYMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dCYRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dCYRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dCYRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "M") == 0) { //������
            *dAgentCost = *dTotalAmt * (pstAgentFee->dCMRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dCMRate / 100, *dAgentCost, pstAgentFee->dCMMax);
            if (!DBL_ZERO(pstAgentFee->dCMMax))
                *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dCMMax) ? pstAgentFee->dCMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dCMRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dCMRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dCMRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "YN") == 0) { //������NFC
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYcNfcCRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYcNfcCRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dYcRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dYcRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dYcRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "YM") == 0) {
            *dAgentCost = *dTotalAmt * (pstAgentFee->dYc2FeeCRate) / 100;
            tLog(DEBUG, "������[%s] ��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f]", pcType, pcFeeType, *dTotalAmt, pstAgentFee->dYc2FeeCRate / 100, *dAgentCost);
            //if (!DBL_ZERO(pstAgentFee->dDMMax))
            //    *dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dDMMax) ? pstAgentFee->dDMMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dYcRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dYcRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dYcRadio / 100, dRatio/100, *dFee, *dAgentFee);
        }
    } else if (strcmp(pcType, "3") == 0) {
        if (strcmp(pcFeeType, "WX") == 0) { //΢��
            *dAgentCost = *dTotalAmt * (pstAgentFee->dWxRate) / 100;
            tLog(DEBUG, "��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dWxRate / 100, *dAgentCost, pstAgentFee->dWxMax);
            //if (!DBL_ZERO(pstAgentFee->dWxMax))
            //*dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dWxMax) ? pstAgentFee->dWxMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dWxRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dWxRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dWxRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "BB") == 0) {//֧����
            *dAgentCost = *dTotalAmt * (pstAgentFee->dBbRate) / 100;
            tLog(DEBUG, "��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dBbRate / 100, *dAgentCost, pstAgentFee->dBbMax);
            //if (!DBL_ZERO(pstAgentFee->dBbMax))
            //*dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dBbMax) ? pstAgentFee->dBbMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dBbRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dBbRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dBbRadio / 100, dRatio/100, *dFee, *dAgentFee);
        } else if (strcmp(pcFeeType, "D0") == 0) {//D0
            *dAgentCost = *dTotalAmt * (pstAgentFee->dD0Rate) / 100;
            tLog(DEBUG, "��������[%s] �ܽ��׽��[%f], �����̳ɱ�����[%lf], �����̳ɱ�[%f], �����̳ɱ����ֵ[%f]", pcFeeType, *dTotalAmt, pstAgentFee->dD0Rate / 100, *dAgentCost, 0);
            //if (!DBL_ZERO(pstAgentFee->dBbMax))
            //*dAgentCost = DBL_CMP(*dAgentCost, pstAgentFee->dBbMax) ? pstAgentFee->dBbMax : *dAgentCost;
            tMake(dAgentCost, 2, 'u');
            //�����̷���
            //*dAgentFee = (*dAgentCostDown - *dAgentCost + *dFee1) * pstAgentFee->dCRadio / 100;
            *dAgentFee = (*dFee - *dAgentCost) * pstAgentFee->dCRadio / 100 - (*dFee - *dAgentCostDown) * dRatio/ 100;
            tLog(DEBUG, "���δ����̳ɱ�[%f], �����̳ɱ�[%f], �����̷�Ӷ����[%lf], �¼������̷�Ӷ����[%f], �̻�������[%lf], �����̷���[%f]", *dAgentCostDown, *dAgentCost, pstAgentFee->dCRadio / 100, dRatio/100, *dFee, *dAgentFee);
        }
    }
    return 0;
}

int FeeAdjust(char *pcType, char *pcTransDate) {
    cJSON *pstJson = NULL, *pstTransJson = NULL, *pstAgentJson = NULL;
    double dAmount = 0, dFee = 0;
    int iAgentLevelMax = 0, i = 0, j;
    char sCardType[2 + 1] = {0}, sMerchId[15 + 1] = {0}, sId[32 + 1] = {0}, sUserCode[15 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[16 + 1] = {0}, sRrn[12 + 1] = {0}, sMerchName[500] = {0};
    char sSqlStr[1024] = {0}, sFeeType[2 + 1] = {0}, sTransCode[6 + 1] = {0}, sMerchOrderNo[50] = {0};
    Agent stAgent;
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;

    pstJson = cJSON_CreateObject();
    if (NULL == pstJson) {
        tLog(ERROR, "����pstJsonʧ��.");
        return -1;
    }

    pstTransJson = cJSON_CreateObject();
    if (NULL == pstTransJson) {
        cJSON_Delete(pstJson);
        tLog(ERROR, "����pstTransJsonʧ��.");
        return -1;
    }

    pstAgentJson = cJSON_CreateObject();
    if (NULL == pstAgentJson) {
        cJSON_Delete(pstJson);
        cJSON_Delete(pstTransJson);
        tLog(ERROR, "����pstAgentJsonʧ��.");
        return -1;
    }

    //��ȡ������Ϣ
    if (strncmp(pcType, "pos", 3) == 0) {
        snprintf(sSqlStr, sizeof (sSqlStr), "select id,amount,merch_id,fee,card_type,fee_type,trans_code,user_code,trans_date,trans_time,rrn,merch_order_no from "
                //"(select * from b_pos_trans_detail_his sample(1)) where fee_flag = 'M' and trans_date='%s' and rownum=1", pcTransDate);
                //"(select * from b_pos_trans_detail order by dbms_random.value) where fee_flag = 'M' and rownum=1");
                " b_pos_trans_detail where fee_flag = 'M' and rownum=1 ");
    } else if (strncmp(pcType, "qrcode", 3) == 0) {
        snprintf(sSqlStr, sizeof (sSqlStr), "select id,amount,merch_id,fee,decode(card_type,null,'3',card_type),decode(substr(trans_code,3,1),'W','WX','B','BB','Y',fee_type),trans_code,user_code,trans_date,trans_time,rrn,merch_order_no from "
                "b_inline_tarns_detail  where fee_flag = 'M' and rownum=1");
    } else {
        tLog(ERROR, "����1�������.");
        return -2;
    }
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "��ȡһ�����ּ�¼ʧ��.");
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sId);
        DOUV(pstRes, 2, dAmount);
        STRV(pstRes, 3, sMerchId);
        DOUV(pstRes, 4, dFee);
        STRV(pstRes, 5, sCardType);
        STRV(pstRes, 6, sFeeType);
        STRV(pstRes, 7, sTransCode);
        STRV(pstRes, 8, sUserCode);
        STRV(pstRes, 9, sTransDate);
        STRV(pstRes, 10, sTransTime);
        STRV(pstRes, 11, sRrn);
        STRV(pstRes, 12, sMerchOrderNo);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "����Ҫ����%s�Ľ�����ˮ.", pcType);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        sleep(60);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);

    //��ȡ�̻���Ӧ��������Ϣ
    if (FindAgentByMerhId(&stAgent, sMerchId) < 0) {
        tLog(DEBUG, "δ�ҵ��̻�[%s]�����Ĵ�����.", sMerchId);
    }

    if (FindMerchName(sMerchName, sMerchId) < 0) {
        tLog(DEBUG, "δ�ҵ��̻�[%s]������.", sMerchId);
    }
    
    tLog(DEBUG, "agent_id[%s],agent_name[%s],agent_status[%s],zmk_key[%s],agent_level[%s],grade_agent_level[%s] ", stAgent.sAgentId, stAgent.sAgentname, stAgent.sStatus, stAgent.sZmk, stAgent.sAgentLevel, stAgent.sGradeAgentLevel);
    SET_STR_KEY(pstAgentJson, "agent_id", stAgent.sAgentId);
    SET_STR_KEY(pstAgentJson, "agent_name", stAgent.sAgentname);
    SET_STR_KEY(pstAgentJson, "agent_status", stAgent.sStatus);
    SET_STR_KEY(pstAgentJson, "zmk_key", stAgent.sZmk);
    SET_STR_KEY(pstAgentJson, "agent_level", stAgent.sAgentLevel);
    SET_STR_KEY(pstAgentJson, "grade_agent_level", stAgent.sGradeAgentLevel);
    SET_JSON_KEY(pstJson, "agent", pstAgentJson);
    tLog(DEBUG, "id[%s],merch_id[%s],amount[%f],merch_order_no[%s] ", sId, sMerchId, dAmount,sMerchOrderNo);
    SET_STR_KEY(pstTransJson, "id", sId);
    SET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    SET_DOU_KEY(pstTransJson, "amount", dAmount);
    SET_DOU_KEY(pstTransJson, "fee", dFee);
    SET_STR_KEY(pstTransJson, "card_type", sCardType);
    SET_STR_KEY(pstTransJson, "fee_type", sFeeType);
    SET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    SET_STR_KEY(pstTransJson, "trans_type", pcType);
    SET_STR_KEY(pstTransJson, "user_code", sUserCode);
    SET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    SET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    SET_STR_KEY(pstTransJson, "rrn", sRrn);
    SET_STR_KEY(pstTransJson, "merch_name", sMerchName);
    SET_STR_KEY(pstTransJson, "merch_orderno", sMerchOrderNo);
    SET_JSON_KEY(pstJson, "data", pstTransJson);

    //��ȡ�����̼���
    //GET_STR_KEY(pstAgentJson, "agent_level", sAgentLevel);
    tLog(DEBUG, "�������̼���%s", stAgent.sAgentLevel);
    for (i = atoi(stAgent.sAgentLevel), j = 0; i >= 0; i--, j++) {
        AgentProcForeach(i, j, pstJson);
    }
    cJSON_Delete(pstJson);
}

int AgentProcForeach(int iAgentLevel, int j, cJSON *pstJson) {
    char sSqlStr[2048] = {0}, sTransType[2] = {0};
    char sAgentId[8 + 1] = {0}, sTempFeeType[1 + 1] = {0}, sId[32 + 1] = {0};
    char sCardType[1 + 1] = {0}, sFeeType[1 + 1] = {0}, sAgentlevel[2 + 1] = {0}, sRrn[12 + 1] = {0}, sGradeAgentLevel[3 + 1] = {0};
    char sAgentName[255 + 1] = {0}, sTransCode[6 + 1] = {0},  sUserCode[15 + 1] = {0}, sMerchId[15 + 1] = {0};
    char sTransDate[8 + 1] = {0}, sTransTime[16 + 1] = {0}, sMerchName[500] = {0}, sMerchOrderNo[50] = {0};
    double dTotalAmt = 0.0, dTotalFee = 0.0, dAgentFee = 0.0, dAgentCost = 0.0, dRatio = 0.0;
    double dAmt = 0.0, dFee = 0.0, dAddAmt = 0.0, dAgentCostDown = 0.0;
    int iCnt = 0, iNo = 0;
    cJSON *pstTransJson = NULL, *pstAgentJson = NULL;
    AgentFee stAgentFee;
    OCI_Resultset *pstRes = NULL, *pstInsRes = NULL, *pstUpdRes = NULL;
    OCI_Statement* pstState = NULL, *pstStateS = NULL, *pstState1 = NULL;


    pstTransJson = GET_JSON_KEY(pstJson, "data");
    pstAgentJson = GET_JSON_KEY(pstJson, "agent");
    //��ȡ������Ϣ
    GET_DOU_KEY(pstTransJson, "amount", dAmt);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "fee_type", sFeeType);
    GET_STR_KEY(pstTransJson, "id", sId);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "user_code", sUserCode);
    GET_STR_KEY(pstTransJson, "trans_date", sTransDate);
    GET_STR_KEY(pstTransJson, "trans_time", sTransTime);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    GET_STR_KEY(pstTransJson, "merch_name", sMerchName);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "merch_orderno", sMerchOrderNo);


    //��ȡ��������Ϣ      
    GET_STR_KEY(pstAgentJson, "agent_id", sAgentId);
    GET_STR_KEY(pstAgentJson, "agent_level", sAgentlevel);
    GET_STR_KEY(pstAgentJson, "grade_agent_level", sGradeAgentLevel);
    GET_STR_KEY(pstAgentJson, "agent_name", sAgentName);

    //�����ǰ��������1��������ȡ������ģ������Fee_type ���ڷֹ�˾����ļ���
    if (iAgentLevel == 1) {
        if (FindAgentTempFeeType(iAgentLevel, sAgentId, sTempFeeType) == 0) {
            DEL_KEY(pstAgentJson, "temp_fee_type");
            SET_STR_KEY(pstAgentJson, "temp_fee_type", sTempFeeType);
            tLog(DEBUG, "������[%s]temp_fee_type[%s].", sAgentId, sTempFeeType);
        } else {
            tLog(ERROR, "���Ҵ�����[%s]temp_fee_typeʧ��.", sAgentId);
            return -1;
        }
    }
    if (sAgentlevel[0] > '0') {
        if (FindAgentFeeById(&stAgentFee, sAgentId) < 0) {
            tLog(ERROR, "��ȡ������[%s]����ʧ��.", sAgentId);
        }
    } else {
        GET_STR_KEY(pstAgentJson, "temp_fee_type", sTempFeeType);
        if (FindSubCompFeeById(&stAgentFee, sAgentId, sTempFeeType) < 0) {
            tLog(ERROR, "��ȡ������[%s]����ʧ��.", sAgentId);
        }
    }
    //�����̻����������̷����Լ��ɱ�
    if (j == 0) {
        GET_DOU_KEY(pstTransJson, "fee", dFee);
        GET_STR_KEY(pstTransJson, "trans_type", sTransType);
        if (CalcAgentFee(&dAmt, &dFee, sCardType, sFeeType, &dAgentFee, &dAgentCost, &stAgentFee) < 0) {
            tLog(ERROR, "������[%s]���ʼ���ʧ��.", sAgentId);
        }
        dAgentCostDown = dAgentCost;
        //dFee1 = dFee - dAgentCost - dAgentFee;

        //������ˮ���ӱ�
        snprintf(sSqlStr, sizeof (sSqlStr), "insert into b_append_trans_detail(ref_id,group_code,merch_id,user_code,merch_name,card_type,trans_date,trans_time,rrn,amount,merch_fee,merch_order_no) "
                "values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%lf','%lf','%s') ", sId, sTransType,sMerchId,sUserCode,sMerchName,sCardType,sTransDate,sTransTime,sRrn,dAmt,dFee,sMerchOrderNo);
        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "������ˮ���ӱ�.");
            return -1;
        }
        tLog(DEBUG, "Ӱ���¼��[%d]", OCI_GetAffectedRows(pstState));
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);

    } else { //�����̻����������̷����Լ��ɱ�
        GET_DOU_KEY(pstAgentJson, "agent_cost_down", dAgentCostDown);
        GET_DOU_KEY(pstTransJson, "fee", dFee);
        GET_DOU_KEY(pstAgentJson, "dratio", dRatio);
        if (CalcParentAgentFee(&dAmt, &dAgentCostDown, &dFee, sCardType, sFeeType, &dAgentFee, &dAgentCost, &stAgentFee, dRatio) < 0) {
            tLog(ERROR, "��������[%s]���ʼ���ʧ��.", sAgentId);
        }
        //dFee1 = dAgentCostDown - dAgentCost + dFee1 - dAgentFee;
        dAgentCostDown = dAgentCost;
        //DEL_KEY(pstAgentJson, "fee1");
        DEL_KEY(pstAgentJson, "agent_cost_down");
        DEL_KEY(pstAgentJson, "dratio");

    }
    //���·���
    snprintf(sSqlStr, sizeof (sSqlStr), "update b_append_trans_detail set agent_id%s = '%s', agent_name%s = '%s', fee%s = %f, cost_amount%s = %f "
            ",ratio%s = %f, grade_agent_level%s = '%s' where ref_id ='%s' ", sAgentlevel, sAgentId, sAgentlevel, sAgentName, sAgentlevel, dAgentFee, sAgentlevel, dAgentCost, sAgentlevel, stAgentFee.dCRadio, sAgentlevel, sGradeAgentLevel, sId);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "����[%s]����,ʧ��.", sAgentId);
        return -1;
    }
    tLog(DEBUG, "Ӱ���¼��[%d]", OCI_GetAffectedRows(pstState));
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);

    //�����ϼ���������Ϣ�����´δ���
    if (iAgentLevel > 0) {
        snprintf(sSqlStr, sizeof (sSqlStr), "select agent_id,agent_name,agent_level,grade_agent_level from b_agent where agent_id = (select p_agent_id from b_agent "
                "where agent_id = '%s') ", sAgentId);
        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "����[%s]����,ʧ��.", sAgentId);
            return -1;
        }
        if (NULL == pstRes) {
            tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
            return -1;
        }
        while (OCI_FetchNext(pstRes)) {
            STRV(pstRes, 1, sAgentId);
            STRV(pstRes, 2, sAgentName);
            STRV(pstRes, 3, sAgentlevel);
            STRV(pstRes, 4, sGradeAgentLevel);
        }

        if (0 == OCI_GetRowCount(pstRes)) {
            tLog(ERROR, "�޴�����[%s]�ϼ�������Ϣ.", sAgentId);
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
        //����FEE_FLAG��ʶ Y:�Ѽ������ N:δ�������
        if (sTransCode[2] == 'W' || sTransCode[2] == 'B' || sTransCode[2] == 'Y') {
            snprintf(sSqlStr, sizeof (sSqlStr), "update b_inline_tarns_detail set FEE_FLAG='Y' where id ='%s' ", sId);
        } else {
            snprintf(sSqlStr, sizeof (sSqlStr), "update b_pos_trans_detail set FEE_FLAG='Y' where id ='%s' ", sId);
        }

        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "����[%s]��������ʶʧ��.", sRrn);
            return -1;
        }
        tLog(DEBUG, "Ӱ���¼��[%d]", OCI_GetAffectedRows(pstState));
        tCommit();
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
    }


    tLog(ERROR, "��ȡ������[%s]����[%f]�ɱ�[%f].", sAgentId, dAgentFee, dAgentCost);
    tLog(DEBUG, "������[%s:%s]%s.", sAgentId, sCardType, sAgentName);

    return 0;
}
