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

    tLog(DEBUG, "����[%s],SN[%s].", sTermfatory, sTermSn);

    tTrim(sTermfatory);
    tTrim(sTermSn);

    /* ͨ���ն˳��Һ�SN���кţ������̻��ź��ն˺� */
    if (FindTermIdBySn(sMerchId, sTermId, sTermfatory, sTermSn) < 0) {
        ErrHanding(pstTransJson, "97", "��ȡ�ն˳���[%s]SN���к�[%s]���̻���,�ն˺���Ϣʧ��.", sTermfatory, sTermSn);
        return -1;
    }

    if (sMerchId[0] == '\0') {
        ErrHanding(pstTransJson, "97", "��ȡ�ն˳���[%s]SN���к�[%s]δ���̻�.", sTermfatory, sTermSn);
        return -1;
    }
    SET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    SET_STR_KEY(pstTransJson, "term_id", sTermId);

    /* �������ر�־ */
    {
        /* ��û���� */
    }

    tLog(INFO, "��ȡ�ն˳���[%s]SN���к�[%s]���̻���[%s]�ն˺�[%s]�ɹ�.", \
        sTermfatory, sTermSn, sMerchId, sTermId);

    return 0;
}


/******************************************************************************/
/*      ������:     DownParam()                  	                          */
/*      ����˵��:   �·���棬�̻�����   					  */
/*      �������:   cJSON *pstTranData                                        */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int DownParam(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson;
    char sMerchId[16] = {0}, sTermId[9] = {0};
    char sBitmap[33] = {0}, sAdvertise[121] = {0};
    Merch stMerch;

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    tLog(INFO, "��ȡ�̻���:�̻���[%s],�ն˺�[%s]", sMerchId, sTermId);

    /* ��ȡ�̻����� */
    MEMSET_ST(stMerch);
    if (FindMerchInfoById(&stMerch, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "��ȡ�̻���[%s]��Ϣʧ��.", sMerchId);
        return -1;
    }

    /* ƾ����ӡ���� */
    SET_STR_KEY(pstTransJson, "merch_p_name", stMerch.sPName);
    /*�������Ƕ�û��ʹ��,�·�ͳһ�ģ���ϵͳ������������,��ʹû���ã�Ҳ��Ӱ��ʹ�ã������ж�ʧ����� */
    FindValueByKey(sAdvertise, "ADVERT");
    SET_STR_KEY(pstTransJson, "advert", sAdvertise);

    /* ����λͼ���̻��������������ڻ�û�У���ʹ�����еĽ���λͼ�������� */
    strcpy(sBitmap, "11111111111111111111111111111111");
    SET_STR_KEY(pstTransJson, "bitmap", sBitmap);

    tLog(INFO, "�̻���[%s]�ն˺�[%s]�·��̻�����[%s],���[%s]�ɹ�,λͼ[%s]����λͼ.", sMerchId, sTermId, stMerch.sPName, sAdvertise, sBitmap);

    return 0;
}