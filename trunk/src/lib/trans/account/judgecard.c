#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"
#include "card.h"



/******************************************************************************/
/*      ������:     JudgeCard()                                           */
/*      ����˵��:   ����Ԥ��������                                              */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int JudgeCard(cJSON *pstJson, int *piFlag) {

    CardBin stCardBin;
    char sErr[128] = {0};
    char sCardNo[20] = {0}, sTrack2[37 + 1] = {0}, sTrack3[104 + 1] = {0};

    int iRet;

    memset(&stCardBin, 0, sizeof (CardBin));
    cJSON * pstTransJson;

    /*
    if (pstNetTran->cSafFlag == '1') {
        tErrLog(INFO, "SAF�޽���Ԥ����.");
        return 0;
    }
     */

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "card_no", sCardNo);
    GET_STR_KEY(pstTransJson, "track2", sTrack2);
    GET_STR_KEY(pstTransJson, "track3", sTrack3);
    if (FindCardBinEx(&stCardBin, sCardNo, sTrack2, sTrack3) != 0) {
        ErrHanding(pstTransJson, "ZB", "����֧����[%s]��Binʧ��.", sCardNo);
        return -1;
    }
    tLog(DEBUG, "stCardBin.sCardId,%s", stCardBin.sCardId);
    tLog(DEBUG, "stCardBin.sCardType,%s", stCardBin.sCardType);

    SET_STR_KEY(pstTransJson, "card_bin", stCardBin.sCardId);
    SET_STR_KEY(pstTransJson, "card_type", stCardBin.sCardType);
    SET_STR_KEY(pstTransJson, "iss_id", stCardBin.sIssBankId);
    /*
        tStrCpy( pstNetTran->sExpireDate, sTrack + stCardBin.lExpPos - 1, 4 );
     */
    SET_STR_KEY(pstTransJson, "card_name", stCardBin.sCardName);
#if 0
#if PRELOAD_CARDBIN
    if (DBFindCardBin(&stCardBin, pstNetTran, g_stSvcInstInfo.pstPreLoadInfo) != 0) {
        sprintf(sErr, "������Binʧ��.", pstNetTran->sCard1);
        tErrLog(ERR, sErr);
        pSendMsg('2', sErr);
        SetRespCode(pstNetTran, "ZB", pstTranData->stSysParam.sInstId);
        //return -1;
    }
#else /* PRELOAD_CARDBIN */
    if (DBLoadCardBin() != 0) {
        strcpy(sErr, "���ؿ�BIN��Ϣʧ��.");
        tErrLog(ERR, sErr);
        pSendMsg('2', sErr);
        SetRespCode(&pstTranData->stNetTran, "96", pstTranData->stSysParam.sInstId);
        return -1;
    }
    if (DBFindCardBin(&stCardBin, pstNetTran) != 0) {
        sprintf(sErr, "������Binʧ��.");
        tErrLog(ERR, sErr);
        pSendMsg('2', sErr);
        SetRespCode(pstNetTran, "ZB", pstTranData->stSysParam.sInstId);
        //return -1;
    }
#endif /* PRELOAD_CARDBIN */
#endif
#if 0
    /* ������  */
    if (stCardBin.cAcqFlag == '0') {
        tErrLog(ERR, "��������[%s]������˿�Bin[%s].", pstNetTran->sIssInstId, stCardBin.sCardBin);
        SetRespCode(pstNetTran, "C9", pstTranData->stSysParam.sInstId);
        return -1;
    }
    sprintf(sSql, "select count(1) from cjzf_agent.black_card_no where card_no=%s", pstNetTran->sCard1);
    iRet = tQueryCount(sSql);
    if (iRet > 0) {
        tErrLog(ERR, "��������[%s]������˿�[%s].", pstNetTran->sIssInstId, pstNetTran->sCard1);
        SetRespCode(pstNetTran, "C9", pstTranData->stSysParam.sInstId);
        return -1;
    }
#endif
    tLog(INFO, "������������[%s]��Bin[%s]�ɹ�[%s].", stCardBin.sIssBankId, stCardBin.sCardBin, stCardBin.sCardType[0] == '1' ? "���ÿ�" : "��ǿ�");
    return 0;
}
