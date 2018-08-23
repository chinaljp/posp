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
/*      函数名:     JudgeCardTypeName()                                       */
/*      功能说明:   解析卡类型和商户结算人姓名                                */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

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

    if (!memcmp(sTransCode, "02A000", 6)) {/* pos机鉴权交易关闭*/
        ErrHanding(pstTransJson, "12", "POS机通道鉴权关闭");
        return -1;
    }

    if (FindJqCardBin(&stCardBin, sCardNo) != 0) {
        sprintf(sErr, "解析支付卡[%s]卡Bin失败.", sCardNo);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "ZB", sErr);
        return -1;
    }
    if (memcmp(sTransCode, "0AA002", 6)) {/* 卡鉴权可以支持借记卡*/
        if (stCardBin.cCardType == '0') {
            ErrHanding(pstTransJson, "W1", "鉴权不受理借记卡,卡Bin[%s].", stCardBin.sCardBin);
            return -1;
        }
        tLog(INFO, "卡为信用卡可以鉴权,卡Bin[%s]", stCardBin.sCardBin);
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

    /* 交易鉴权，卡鉴权不做到账人限制*/
    if (!memcmp(sTransCode, "0AA000", 6) || !memcmp(sTransCode, "02A000", 6)) {
        iRet = FindCardName(sMerchId, sName, sIdCard);
        if (iRet == 1) {
            sprintf(sErr, "鉴权人与到账人姓名不符");
            tLog(ERROR, sErr);
            ErrHanding(pstTransJson, "E5", sErr);
            return -1;
        }
        if (iRet == 2) {
            sprintf(sErr, "鉴权人与到账人身份证号不符");
            tLog(ERROR, sErr);
            ErrHanding(pstTransJson, "EA", sErr);
            return -1;
        }

        if (FindCardNum(pstTransJson) != 0) {
            sprintf(sErr, "同一张卡不能重复鉴权,商户号[%s]", sMerchId);
            ErrHanding(pstTransJson, "E3", sErr);
            return -1;
        }

    }

    if (!memcmp(sTransCode, "0AA002", 6)) {
        iRet = FindSamCardNum(pstTransJson);
        if (iRet == 1) {
            sprintf(sErr, "同一张卡不能重复鉴权,商户号[%s]", sMerchId);
            ErrHanding(pstTransJson, "E3", sErr);
            return -1;
        } else if (iRet < 0) {
            ErrHanding(pstTransJson, "96", "系统错误");
            return -1;
        }
    }

    /*
            if ( DBJudgeArea(pstNetTran) != 0 ){
		
                    sprintf( sErr, "鉴权人为限制鉴权地区");
                    tLog( ERR, sErr );
                    SetRespCode( pstNetTran, "W7", pstTranData->stSysParam.sInstId );
                    return -1;
            }
     */

    return 0;

}
#if 0
/******************************************************************************/
/*		函数名: 	Upauthflow()				      */
/*		功能说明:	更新鉴权流水				      */
/*		输入参数:	TranData *pstTranData			      */
/*		输出参数:						      */
/*		返回说明:	0 成功	非 0 不成功 			      */

/******************************************************************************/
int Upauthflow(cJSON *pstTranData, int *piFlag) {
    char sMerchId[16] = {0};
    char sErr[50] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    if (UpautoFlag(pstTransJson) < 0) {
        sprintf(sErr, "更新鉴权流水失败,商户号[%s]", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;
}
#endif

/******************************************************************************/
/*		函数名: 	FindcontUplimit()	                  */
/*		功能说明:	查询鉴权成功个数，更新额度		  */
/*		输入参数:	TranData *pstTranData			  */
/*		输出参数:						  */
/*		返回说明:	0 成功	非 0 不成功			  */

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
        sprintf(sErr, "获取鉴权卡个数失败,商户号[%s]", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    tLog(INFO, "UpLimit");
    if (UpLimit(iRet, pstTransJson) < 0) {
        sprintf(sErr, "更新额度失败,商户号[%s]", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;
}


/******************************************************************************/
/*		函数名: 	Findlimit()				  */
/*		功能说明:	获取额度  			          */
/*		输入参数:	TranData *pstTranData			  */
/*		输出参数:						  */
/*		返回说明:	0 成功	非 0 不成功 		          */

/******************************************************************************/
int Findlimit(cJSON *pstTranData, int *piFlag) {
    int iRet = 0;
    cJSON *pstTransJson;
    char sErr[512] = {0};
    char sMerchId[16] = {0};
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    if (FindLimit(pstTransJson) < 0) {
        sprintf(sErr, "查询额度失败,商户号[%s]", sMerchId);
        tLog(ERROR, sErr);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;
}

int Empflow(cJSON *pstTranData, int *piFlag) {
    /* 此函数为了提交上一个交易流的事务 */
    return 0;
}

/******************************************************************************/
/*		函数名: 	Insertauthflow()												  */
/*		功能说明:	记录鉴权流水									  */
/*		输入参数:	TranData *pstTranData									  */
/*		输出参数:															  */
/*		返回说明:	0 成功	非 0 不成功 									  */

/******************************************************************************/

int Insertauthflow(cJSON *pstTranData, int *piFlag) {

    int iNum = 0;
    char sErr[50] = {0}, sMerchId[16] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    iNum = InstTranFlow(pstTransJson);
    if (iNum < 0) {
        sprintf(sErr, "鉴权流水插入失败,商户号[%s]", sMerchId);
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
        tLog(INFO, "鉴权失败时直接返回");
        return 0;
    }
    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    iNum = JudgewhiteFlow(pstTransJson);
    if (iNum < 0) {
        sprintf(sErr, "鉴权白名单插入失败,卡号[%s]", sCardNo);
        ErrHanding(pstTransJson, "96", sErr);
        return -1;
    }
    return 0;
}
/******************************************************************************/
/*		函数名: 	AddLimitWallet()												  */
/*		功能说明:	添加商户额度变更流水									  */
/*		输入参数:	TranData *pstTranData									  */
/*		输出参数:															  */
/*		返回说明:	0 成功	非 0 不成功 									  */

/******************************************************************************/
int AddLimitWallet(cJSON *pstTranData, int *piFlag) {
    int iNum = 0;
    char sErr[50] = {0}, sMerchId[16] = {0}, sRespCode[2 + 1] = {0};
    cJSON *pstTransJson;
    pstTransJson = GET_JSON_KEY(pstTranData, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(INFO, "鉴权失败不记录额度变更流水");
        return 0;
    }

    iNum = InstLimitFlow(pstTransJson);
    if (iNum < 0) {
        sprintf(sErr, "商户额度变更流水插入失败,商户号[%s]", sMerchId);
    }
    return 0;
}
