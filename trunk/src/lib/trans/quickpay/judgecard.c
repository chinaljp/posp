#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "card.h"
#include "quickpay.h"


/******************************************************************************/
/*      函数名:     JudgeCard()                                           */
/*      功能说明:   解析预付卡类型                                              */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int JudgeCard(cJSON *pstJson, int *piFlag) {

    CardBin stCardBin;
    CardMsg stCardMsg;
    char sTransCode[6+1] = {0};
    double dAmount = 0.00;
    char sCardNo[21+1] = {0}, sId[32+1] = {0};
    char sCardHolderName[128+1] = {0},sMobileNo[11+1] = {0},sCardCvnNo[8+1] = {0},sCardExpDate[4+1] = {0};
    
    memset(&stCardBin, 0, sizeof (CardBin));
    cJSON * pstTransJson;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    
    /*交易金额为0的禁止交易*/
    GET_STR_KEY(pstTransJson,"trans_code",sTransCode);
    GET_DOU_KEY(pstTransJson,"amount",dAmount); 
    dAmount /= 100;
    tLog(INFO,"交易[%s],交易金额为[%.02f]",sTransCode,dAmount);
    if( DBL_ZERO(dAmount) ) {
        tLog(ERROR,"交易[%s],交易金额为[%.02f],禁止交易",sTransCode,dAmount);
        ErrHanding(pstTransJson, "13", "交易金额不能为零.");
        return ( -1 );
    }
    
    GET_STR_KEY(pstTransJson, "id", sId);
    
    tTrim(sId);
    tLog(DEBUG,"sId=[%s]",sId);
    if( FindCardMsg( &stCardMsg, sId ) < 0 ) {
        tLog(DEBUG, "获取卡包ID为[%s]交易卡信息失败.",sId);
        ErrHanding(pstTransJson, "96", "获取交易卡信息失败.");
        return ( -1 );
    }
    
    /*******/
#if 0   
    char sEncData1[255] = {0};
    char sEncData2[255] = {0};
    char sEncData3[255] = {0};
    char sEncData4[255] = {0};
    char sEncData5[255] = {0};
    tHsm_Enc_Asc_Data(sEncData1, stCardMsg.sKey, "6222620720000410263");
    tHsm_Enc_Asc_Data(sEncData2, stCardMsg.sKey, "刘胡兰");
    tHsm_Enc_Asc_Data(sEncData3, stCardMsg.sKey, "15046522536");
    tHsm_Enc_Asc_Data(sEncData4, stCardMsg.sKey, "625");
    tHsm_Enc_Asc_Data(sEncData5, stCardMsg.sKey, "29\07");
    tLog(ERROR, "sEncData1=[%s]",sEncData1);
    tLog(ERROR, "sEncData2=[%s]",sEncData2);
    tLog(ERROR, "sEncData3=[%s]",sEncData3);
    tLog(ERROR, "sEncData4=[%s]",sEncData4);
    tLog(ERROR, "sEncData5=[%s]",sEncData5);
    /*******/
#endif    
    /*数据解密*/
    if (tHsm_Dec_Asc_Data(sCardNo, stCardMsg.sKey, stCardMsg.sCardNoE) < 0) {
        ErrHanding(pstTransJson, "96", "解密卡号失败.");
        tLog(ERROR, "解密卡号失败.");
        return -1;
    }
    if( stCardMsg.sCardHolderNameE[0] != '\0'  ) {/*若不为空，进行解密 */
        if (tHsm_Dec_Asc_Data(sCardHolderName, stCardMsg.sKey, stCardMsg.sCardHolderNameE) < 0) {
            ErrHanding(pstTransJson, "96", "解密持卡人姓名失败.");
            tLog(ERROR, "解密持卡人姓名失败.");
            return -1;
        }
    }
    if( stCardMsg.sMobileNoE[0] != '\0'  ) {/*若不为空，进行解密 */
        if (tHsm_Dec_Asc_Data(sMobileNo, stCardMsg.sKey, stCardMsg.sMobileNoE) < 0) {
            ErrHanding(pstTransJson, "96", "解密持卡人手机号失败.");
            tLog(ERROR, "解密持卡人手机号失败.");
            return -1;
        }
    }
    if( stCardMsg.sCardCvnNoE[0] != '\0'  ) {/*若不为空，进行解密 */
        if (tHsm_Dec_Asc_Data(sCardCvnNo, stCardMsg.sKey, stCardMsg.sCardCvnNoE) < 0) {
            ErrHanding(pstTransJson, "96", "解密银行卡安全码失败.");
            tLog(ERROR, "解密银行卡安全码失败.");
            return -1;
        }
    }
    if ( stCardMsg.sCardExpDateE[0] != '\0' ) {/*若不为空，进行解密 */
        if (tHsm_Dec_Asc_Data(sCardExpDate, stCardMsg.sKey, stCardMsg.sCardExpDateE) < 0) {
            ErrHanding(pstTransJson, "96", "解密银行卡有效期失败.");
            tLog(ERROR, "解密银行卡有效期失败.");
            return -1;
        }
    }
    
    tTrim(sCardNo);
    tTrim(sCardHolderName);
    tTrim(sMobileNo);
    tTrim(sCardCvnNo);
    tTrim(sCardExpDate);
    
    SET_STR_KEY(pstTransJson, "key_name",stCardMsg.sKey);
    SET_STR_KEY(pstTransJson, "card_no_enc", stCardMsg.sCardNoE);
    SET_STR_KEY(pstTransJson, "card_no", sCardNo);
    SET_STR_KEY(pstTransJson, "cardNm", sCardHolderName);
    SET_STR_KEY(pstTransJson, "mobNo", sMobileNo);
    SET_STR_KEY(pstTransJson, "cvnNo", sCardCvnNo);
    SET_STR_KEY(pstTransJson, "expDt", sCardExpDate);
    
    if (FindCardBinEx(&stCardBin, sCardNo) != 0) {
        tLog(DEBUG,"解析支付卡[%s]卡Bin失败.",sCardNo);
        ErrHanding(pstTransJson, "ZB", "解析支付卡,卡Bin失败.");
        return -1;
    }
    
    tLog(DEBUG, "stCardBin.sCardId,%s", stCardBin.sCardId);
    tLog(DEBUG, "stCardBin.sCardType,%s", stCardBin.sCardType);
    
    SET_STR_KEY(pstTransJson, "card_no", sCardNo);
    SET_STR_KEY(pstTransJson, "card_bin", stCardBin.sCardId);
    SET_STR_KEY(pstTransJson, "card_type", stCardBin.sCardType);
    SET_STR_KEY(pstTransJson, "iss_id", stCardBin.sIssBankId);
    SET_STR_KEY(pstTransJson, "card_name", stCardBin.sCardName);

    tLog(INFO, "解析发卡机构[%s]卡Bin[%s]成功[%s].", stCardBin.sIssBankId, stCardBin.sCardBin, stCardBin.sCardType[0] == '1' ? "信用卡" : "借记卡");

    return 0;
}
