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
#include "auth.h"


/******************************************************************************/
/*      ������:     JudgeCardTypeName()                                       */
/*      ����˵��:   ���������ͺ��̻�����������                                */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int JudgeCardTypeName(cJSON *pstTranData, int *piFlag) {
    CardBin stCardBin;
    char sErr[128] = {0}, sCardType[2] = {0}, sTransCode[6 + 1] = {0};
    cJSON *pstTransJson;
    char sCardNo[20] = {0}, sName[20] = {0}, sMerchId[16] = {0}, sIdCard[20] = {0};
    int iRet = 0;
    memset(&stCardBin, 0, sizeof (CardBin));
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "cer_no", sIdCard);

    if (!memcmp(sTransCode, "02A000", 6)) {/* pos����Ȩ���׹ر�*/
        ErrHanding(pstTransJson, "12", "POS��ͨ����Ȩ�ر�");
        return -1;
    }

    if (FindJqCardBin(&stCardBin, sCardNo) != 0) {
        sprintf(sErr, "����֧����[%s]��Binʧ��.", sCardNo);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "ZB", sErr);
        return -1;
    }
    if (memcmp(sTransCode, "0AA002", 6)) {/* ����Ȩ����֧�ֽ�ǿ�*/
        if (stCardBin.cCardType == '0') {
            ErrHanding(pstTransJson, "W1", "��Ȩ�������ǿ�,��Bin[%s].", stCardBin.sCardBin);
            return -1;
        }
        tLog(INFO, "��Ϊ���ÿ����Լ�Ȩ,��Bin[%s]", stCardBin.sCardBin);
    }

    tTrim(stCardBin.sCardId);
    SET_STR_KEY(pstTransJson, "card_id", stCardBin.sCardBin);
    sCardType[0] = stCardBin.cCardType;
    SET_STR_KEY(pstTransJson, "card_type", sCardType);

    tTrim(stCardBin.sIssBankId);
    SET_STR_KEY(pstTransJson, "iss_bank_id", stCardBin.sIssBankId);

    tTrim(stCardBin.sCardName);
    SET_STR_KEY(pstTransJson, "card_name", stCardBin.sCardName);

    GET_STR_KEY(pstTransJson, "account_name", sName);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    /* ���׼�Ȩ������Ȩ��������������*/
    if (!memcmp(sTransCode, "0AA000", 6) || !memcmp(sTransCode, "02A000", 6)) {
        iRet = FindCardName(sMerchId, sName, sIdCard);
        if (iRet == 1) {
            sprintf(sErr, "��Ȩ���뵽������������");
            tLog(ERROR, sErr);
            ErrHanding(pstTransJson, "E5", sErr);
            return -1;
        }
        if (iRet == 2) {
            sprintf(sErr, "��Ȩ���뵽�������֤�Ų���");
            tLog(ERROR, sErr);
            ErrHanding(pstTransJson, "EA", sErr);
            return -1;
        }

        if (FindCardNum(pstTransJson) != 0) {
            sprintf(sErr, "ͬһ�ſ������ظ���Ȩ,�̻���[%s]", sMerchId);
            ErrHanding(pstTransJson, "E3", sErr);
            return -1;
        }

    }

    if (!memcmp(sTransCode, "0AA002", 6)) {
        iRet = FindSamCardNum(pstTransJson);
        if (iRet == 1) {
            sprintf(sErr, "ͬһ�ſ������ظ���Ȩ,�̻���[%s]", sMerchId);
            ErrHanding(pstTransJson, "E3", sErr);
            return -1;
        } else if (iRet < 0) {
            ErrHanding(pstTransJson, "96", "ϵͳ����");
            return -1;
        }
    }

    /*
            if ( DBJudgeArea(pstNetTran) != 0 ){
		
                    sprintf( sErr, "��Ȩ��Ϊ���Ƽ�Ȩ����");
                    tLog( ERR, sErr );
                    SetRespCode( pstNetTran, "W7", pstTranData->stSysParam.sInstId );
                    return -1;
            }
     */

    return 0;

}
#if 0
/******************************************************************************/
/*		������: 	Upauthflow()				      */
/*		����˵��:	���¼�Ȩ��ˮ				      */
/*		�������:	TranData *pstTranData			      */
/*		�������:						      */
/*		����˵��:	0 �ɹ�	�� 0 ���ɹ� 			      */

/******************************************************************************/
int Upauthflow(cJSON *pstTranData, int *piFlag) {
    char sMerchId[16] = {0};
    char sErr[50] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    if (UpautoFlag(pstTransJson) < 0) {
        sprintf(sErr, "���¼�Ȩ��ˮʧ��,�̻���[%s]", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;
}
#endif

/******************************************************************************/
/*		������: 	FindcontUplimit()	                  */
/*		����˵��:	��ѯ��Ȩ�ɹ����������¶��		  */
/*		�������:	TranData *pstTranData			  */
/*		�������:						  */
/*		����˵��:	0 �ɹ�	�� 0 ���ɹ�			  */

/******************************************************************************/
int FindcontUplimit(cJSON *pstTranData, int *piFlag) {
    int iRet = 0;
    cJSON *pstTransJson;
    char sErr[512] = {0};
    char sMerchId[16] = {0};
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    iRet = FindCont(pstTransJson);
    if (iRet < 0) {
        sprintf(sErr, "��ȡ��Ȩ������ʧ��,�̻���[%s]", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    tLog(INFO, "UpLimit");
    if (UpLimit(iRet, pstTransJson) < 0) {
        sprintf(sErr, "���¶��ʧ��,�̻���[%s]", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;
}


/******************************************************************************/
/*		������: 	Findlimit()				  */
/*		����˵��:	��ȡ���  			          */
/*		�������:	TranData *pstTranData			  */
/*		�������:						  */
/*		����˵��:	0 �ɹ�	�� 0 ���ɹ� 		          */

/******************************************************************************/
int Findlimit(cJSON *pstTranData, int *piFlag) {
    int iRet = 0;
    cJSON *pstTransJson;
    char sErr[512] = {0};
    char sMerchId[16] = {0};
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    if (FindLimit(pstTransJson) < 0) {
        sprintf(sErr, "��ѯ���ʧ��,�̻���[%s]", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;
}

int Empflow(cJSON *pstTranData, int *piFlag) {
    /* �˺���Ϊ���ύ��һ�������������� */
    return 0;
}

/******************************************************************************/
/*		������: 	Insertauthflow()												  */
/*		����˵��:	��¼��Ȩ��ˮ									  */
/*		�������:	TranData *pstTranData									  */
/*		�������:															  */
/*		����˵��:	0 �ɹ�	�� 0 ���ɹ� 									  */

/******************************************************************************/

int Insertauthflow(cJSON *pstTranData, int *piFlag) {

    int iNum = 0;
    char sErr[50] = {0}, sMerchId[16] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    iNum = InstTranFlow(pstTransJson);
    if (iNum < 0) {
        sprintf(sErr, "��Ȩ��ˮ����ʧ��,�̻���[%s]", sMerchId);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;

}

int Upwhitelist(cJSON *pstTranData, int *piFlag) {
    int iNum = 0;
    char sErr[50 + 1] = {0}, sCardNo[20 + 1] = {0}, sRespCode[2 + 1] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(INFO, "��Ȩʧ��ʱֱ�ӷ���");
        return 0;
    }
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    iNum = JudgewhiteFlow(pstTransJson);
    if (iNum < 0) {
        sprintf(sErr, "��Ȩ����������ʧ��,����[%s]", sCardNo);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;
}
/******************************************************************************/
/*		������: 	AddLimitWallet()												  */
/*		����˵��:	����̻���ȱ����ˮ									  */
/*		�������:	TranData *pstTranData									  */
/*		�������:															  */
/*		����˵��:	0 �ɹ�	�� 0 ���ɹ� 									  */

/******************************************************************************/
int AddLimitWallet(cJSON *pstTranData, int *piFlag) {
    int iNum = 0;
    char sErr[50] = {0}, sMerchId[16] = {0}, sRespCode[2 + 1] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(INFO, "��Ȩʧ�ܲ���¼��ȱ����ˮ");
        return 0;
    }

    iNum = InstLimitFlow(pstTransJson);
    if (iNum < 0) {
        sprintf(sErr, "�̻���ȱ����ˮ����ʧ��,�̻���[%s]", sMerchId);
    }
    return 0;
}
