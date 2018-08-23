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
/*      ������:     JudgeCard()                                           */
/*      ����˵��:   ����Ԥ��������                                              */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

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
    
    /*���׽��Ϊ0�Ľ�ֹ����*/
    GET_STR_KEY(pstTransJson,"trans_code",sTransCode);
    GET_DOU_KEY(pstTransJson,"amount",dAmount); 
    dAmount /= 100;
    tLog(INFO,"����[%s],���׽��Ϊ[%.02f]",sTransCode,dAmount);
    if( DBL_ZERO(dAmount) ) {
        tLog(ERROR,"����[%s],���׽��Ϊ[%.02f],��ֹ����",sTransCode,dAmount);
        ErrHanding(pstTransJson, "13", "���׽���Ϊ��.");
        return ( -1 );
    }
    
    GET_STR_KEY(pstTransJson, "id", sId);
    
    tTrim(sId);
    tLog(DEBUG,"sId=[%s]",sId);
    if( FindCardMsg( &stCardMsg, sId ) < 0 ) {
        tLog(DEBUG, "��ȡ����IDΪ[%s]���׿���Ϣʧ��.",sId);
        ErrHanding(pstTransJson, "96", "��ȡ���׿���Ϣʧ��.");
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
    tHsm_Enc_Asc_Data(sEncData2, stCardMsg.sKey, "������");
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
    /*���ݽ���*/
    if (tHsm_Dec_Asc_Data(sCardNo, stCardMsg.sKey, stCardMsg.sCardNoE) < 0) {
        ErrHanding(pstTransJson, "96", "���ܿ���ʧ��.");
        tLog(ERROR, "���ܿ���ʧ��.");
        return -1;
    }
    if( stCardMsg.sCardHolderNameE[0] != '\0'  ) {/*����Ϊ�գ����н��� */
        if (tHsm_Dec_Asc_Data(sCardHolderName, stCardMsg.sKey, stCardMsg.sCardHolderNameE) < 0) {
            ErrHanding(pstTransJson, "96", "���ֿܳ�������ʧ��.");
            tLog(ERROR, "���ֿܳ�������ʧ��.");
            return -1;
        }
    }
    if( stCardMsg.sMobileNoE[0] != '\0'  ) {/*����Ϊ�գ����н��� */
        if (tHsm_Dec_Asc_Data(sMobileNo, stCardMsg.sKey, stCardMsg.sMobileNoE) < 0) {
            ErrHanding(pstTransJson, "96", "���ֿܳ����ֻ���ʧ��.");
            tLog(ERROR, "���ֿܳ����ֻ���ʧ��.");
            return -1;
        }
    }
    if( stCardMsg.sCardCvnNoE[0] != '\0'  ) {/*����Ϊ�գ����н��� */
        if (tHsm_Dec_Asc_Data(sCardCvnNo, stCardMsg.sKey, stCardMsg.sCardCvnNoE) < 0) {
            ErrHanding(pstTransJson, "96", "�������п���ȫ��ʧ��.");
            tLog(ERROR, "�������п���ȫ��ʧ��.");
            return -1;
        }
    }
    if ( stCardMsg.sCardExpDateE[0] != '\0' ) {/*����Ϊ�գ����н��� */
        if (tHsm_Dec_Asc_Data(sCardExpDate, stCardMsg.sKey, stCardMsg.sCardExpDateE) < 0) {
            ErrHanding(pstTransJson, "96", "�������п���Ч��ʧ��.");
            tLog(ERROR, "�������п���Ч��ʧ��.");
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
        tLog(DEBUG,"����֧����[%s]��Binʧ��.",sCardNo);
        ErrHanding(pstTransJson, "ZB", "����֧����,��Binʧ��.");
        return -1;
    }
    
    tLog(DEBUG, "stCardBin.sCardId,%s", stCardBin.sCardId);
    tLog(DEBUG, "stCardBin.sCardType,%s", stCardBin.sCardType);
    
    SET_STR_KEY(pstTransJson, "card_no", sCardNo);
    SET_STR_KEY(pstTransJson, "card_bin", stCardBin.sCardId);
    SET_STR_KEY(pstTransJson, "card_type", stCardBin.sCardType);
    SET_STR_KEY(pstTransJson, "iss_id", stCardBin.sIssBankId);
    SET_STR_KEY(pstTransJson, "card_name", stCardBin.sCardName);

    tLog(INFO, "������������[%s]��Bin[%s]�ɹ�[%s].", stCardBin.sIssBankId, stCardBin.sCardBin, stCardBin.sCardType[0] == '1' ? "���ÿ�" : "��ǿ�");

    return 0;
}
