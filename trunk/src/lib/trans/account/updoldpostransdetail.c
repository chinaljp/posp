#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* �������ѳ���ԭ���� */
int UpdPosTransDetail(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "����ʧ��[%s],���ָ�ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "1") < 0) {
        ErrHanding(pstTransJson, "96", "����ԭ���ѽ���[%s:%s]��Ч��־[1]ʧ��.", sOTransDate, sORrn);
        return -1;
    }
    tLog(DEBUG, "����ԭ���ѽ���[%s:%s]��Ч��־[1]�ɹ�.", sOTransDate, sORrn);
    return 0;
}

/* �������ѳ���ԭ���� */
int UpdAuthTransDetail(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "����ʧ��[%s],���ָ�ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "1") != 0) {
        if (UpdTransDetail("B_POS_TRANS_DETAIL_HIS", sOTransDate, sORrn, "1") != 0) {
            ErrHanding(pstTransJson, "96", "����ԭ���ѽ���[%s:%s]��Ч��־[1]ʧ��.", sOTransDate, sORrn);
            return -1;
        }
    }
    tLog(DEBUG, "����ԭ���ѽ���[%s:%s]��Ч��־[1]�ɹ�.", sOTransDate, sORrn);
    return 0;
}

/* �������ѳ���,Ԥ��Ȩ����ԭ���� */
int Upd0003PosTransDetail(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "����ʧ��[%s],���ָ�ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "2") < 0) {
        ErrHanding(pstTransJson, "96", "����ԭ���ѽ���[%s:%s]��Ч��־[2]ʧ��.", sOTransDate, sORrn);
        return -1;
    }
    tLog(DEBUG, "����ԭ���ѽ���[%s:%s]��Ч��־[2]�ɹ�.", sOTransDate, sORrn);
    return 0;
}

/* �������ѳ�������ԭ���� */
int Upd0023PosTransDetail(cJSON *pstJson, int *piFlag) {
    cJSON * pstTransJson = NULL;
    char sRespCode[2 + 1] = {0};
    char sORrn[12 + 1] = {0}, sOTransDate[8 + 1] = {0};
    char sOORrn[12 + 1] = {0}, sOOTransDate[8 + 1] = {0};
    int iRet = -1;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);

    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "����ʧ��[%s],���ָ�ԭ���ѽ��׵���Ч��־.", sRespCode);
        return 0;
    }

    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOTransDate, sORrn, "2") < 0) {
        ErrHanding(pstTransJson, "96", "����ԭ��������[%s:%s]��Ч��־[2]ʧ��.", sOTransDate, sORrn);
        return -1;
    }
    tLog(INFO, "����ԭ��������[%s:%s]��Ч��־[2]�ɹ�.", sOTransDate, sORrn);
    GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
    GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);

    if (UpdTransDetail("B_POS_TRANS_DETAIL", sOOTransDate, sOORrn, "0") < 0) {
        ErrHanding(pstTransJson, "96", "����ԭ���ѽ���[%s:%s]��Ч��־[0]ʧ��.", sOOTransDate, sOORrn);
        return -1;
    }
    tLog(INFO, "����ԭ���ѽ���[%s:%s]��Ч��־[0]�ɹ�.", sOOTransDate, sOORrn);
    return 0;
}

/******************************************************************************/
/*      ������:     UpdJournalResv()                                          */
/*      ����˵��:   �����ཻ�׸���ԭ������ˮ                                  */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UpdJournalResv(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sOOTransDate[8 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0}, sOORrn[12 + 1] = {0};
    char sOOOTransDate[8 + 1] = {0}, sOOORrn[12 + 1] = {0};

    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);
    GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
    /* ��ȡԭ������Ϣ */

    if (NULL != strstr("020003,020023,T20003,024003,024103,024023,024123,M20003,M20023", sTransCode)) {
        tStrCpy(sTrace, sTraceNo, 6);
    } else {
        tStrCpy(sTrace, sOTraceNo, 6);
    }
    tLog(ERROR, "trace no[%s]", sTrace);
#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* ����ԭ������ˮ����Ч��־valid_flag */
    if (UpdOrgJon(sMerchId, sTermId, sTrace, '2') != 0) {
        ErrHanding(pstTransJson, "96", "ԭ���׸��±�־ʧ�ܣ���ԭ������ˮ��ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sTrace, sMerchId, sTermId);
        return -1;
    }
    //���ѳ�������,Ԥ��Ȩ��ɳ��������ָ�ԭ������Ч��־
    if (NULL != strstr("020023,024123,M20023", sTransCode)) {
        if (UpdOrgJonRecover(sOORrn) != 0) {
            ErrHanding(pstTransJson, "96", "ԭ���ѽ��׻ָ���Ч��־ʧ�ܣ�rrn[%s].", \
                sOORrn);
            return -1;
        }
    }

    //Ԥ��Ȩ��ɳ��������ָ�ԭԤ��Ȩ����autho_flagΪ1
    if (NULL != strstr("024123", sTransCode)) {

        if (GetOrgRrnAndTransDate(sOORrn, sOOTransDate, sOOORrn, sOOOTransDate) != 0) {
            ErrHanding(pstTransJson, "96", "����ԭԭRRN[%s],TRANS_DATE[%s]ʧ��.", \
                sOORrn, sOOTransDate);
            return -1;
        }
        if (UpdOrgJonAuthRecover(sOOORrn, sOOOTransDate, '1') != 0) {
            ErrHanding(pstTransJson, "96", "ԭ���ѽ��׻ָ���Ч��־ʧ�ܣ�rrn[%s].", \
                sOORrn);
            return -1;
        }
    }

    //Ԥ��Ȩ���������ָ�ԭԤ��Ȩ��Ч��־
    if (0 == memcmp(sTransCode, "024023", 6)) {
        if (UpdOrgJonAuthUndo(sOORrn, sOOTransDate, '0') != 0) {
            ErrHanding(pstTransJson, "96", "ԭ���ѽ��׻ָ���Ч��־ʧ�ܣ�rrn[%s].", \
                sOORrn);
            return -1;
        }
    }

    /*Ԥ��Ȩ��ɳ����ָ�ԭԤ��Ȩ״̬��ʶ*/
    if (0 == memcmp(sTransCode, "024103", 6)) {
        GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
        GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);
        if (UpdOrgJonAuthRecover(sOORrn, sOOTransDate, '0') != 0) {
            ErrHanding(pstTransJson, "96", "ԭ���ѽ��׻ָ���Ч��־ʧ�ܣ�rrn[%s] date[%s].", \
                sOORrn, sOOTransDate);
            return -1;
        }
    }

    tLog(INFO, "ԭ���׸��±�־�ɹ���ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sTrace, sMerchId, sTermId);

    return 0;
}


/******************************************************************************/
/*      ������:     UpdJournalUndo()                                          */
/*      ����˵��:   �����ཻ�׸���ԭ������ˮ                                  */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UpdJournalUndo(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sOORrn[12 + 1] = {0}, sOOTransDate[8 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};

    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    /* ��ȡԭ������Ϣ */

    tStrCpy(sTrace, sOTraceNo, 6);
    tLog(ERROR, "trace no[%s]", sTrace);


#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* ����ԭ������ˮ����Ч��־valid_flag */
    if (UpdOrgJon(sMerchId, sTermId, sTrace, '1') != 0) {
        ErrHanding(pstTransJson, "96", "ԭ���׸��±�־ʧ�ܣ���ԭ������ˮ��ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sTrace, sMerchId, sTermId);
        return -1;
    }
    /*Ԥ��Ȩ��ɳ����ָ�ԭԤ��Ȩ״̬��ʶ*/
    if (0 == memcmp(sTransCode, "024102", 6)) {
        GET_STR_KEY(pstTransJson, "oo_trans_date", sOOTransDate);
        GET_STR_KEY(pstTransJson, "oo_rrn", sOORrn);
        if (UpdOrgJonAuthRecover(sOORrn, sOOTransDate, '0') != 0) {
            ErrHanding(pstTransJson, "96", "ԭ���ѽ��׻ָ���Ч��־ʧ�ܣ�rrn[%s] date[%s].", \
                sOORrn, sOOTransDate);
            return -1;
        }
    }

    tLog(INFO, "ԭ���׸��±�־�ɹ���ԭ��ˮ��[%s] ԭ�̻���[%s] ԭ�ն˺�[%s].", \
                sTrace, sMerchId, sTermId);
    return 0;
}


/******************************************************************************/
/*      ������:     UpdJournalAuth()                                          */
/*      ����˵��:   Ԥ��Ȩ�ཻ�׸���ԭ������ˮ                                  */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UpdJournalAuth(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sHisTransDate[8 + 1] = {0}, sHisRrn[12 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};

    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "his_trans_date", sHisTransDate);
    GET_STR_KEY(pstTransJson, "his_rrn", sHisRrn);
    /* ��ȡԭ������Ϣ */

    tLog(ERROR, "sHisRrn[%s] sHisTransDate[%s]", sHisRrn, sHisTransDate);


#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* ����ԭ������ˮ����Ч��־valid_flag */
    if (UpdOrgJonAuth(sHisRrn, sHisTransDate, '1') != 0) {
        ErrHanding(pstTransJson, "96", "ԭ���׸��±�־ʧ�ܣ���ԭ������ˮ��ԭrrn[%s] ԭ��������[%s].", \
                sHisRrn, sHisTransDate);
        return -1;
    }

    tLog(INFO, "ԭ���׸��±�־�ɹ���ԭrrn[%s] ԭ��������[%s].", \
                sHisRrn, sHisTransDate);
    return 0;
}


/******************************************************************************/
/*      ������:     UpdJournalAuthUndo()                                          */
/*      ����˵��:   Ԥ��Ȩ�ཻ�׸���ԭ������ˮ                                  */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UpdJournalAuthUndo(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sHisTransDate[8 + 1] = {0}, sHisRrn[12 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};

    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "his_trans_date", sHisTransDate);
    GET_STR_KEY(pstTransJson, "his_rrn", sHisRrn);
    /* ��ȡԭ������Ϣ */

    tLog(ERROR, "sHisRrn[%s] sHisTransDate[%s]", sHisRrn, sHisTransDate);


#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* ����ԭ������ˮ����Ч��־valid_flag */
    if (UpdOrgJonAuthUndo(sHisRrn, sHisTransDate, '1') != 0) {
        ErrHanding(pstTransJson, "96", "ԭ���׸��±�־ʧ�ܣ���ԭ������ˮ��ԭrrn[%s] ԭ��������[%s].", \
                sHisRrn, sHisTransDate);
        return -1;
    }

    tLog(INFO, "ԭ���׸��±�־�ɹ���ԭrrn[%s] ԭ��������[%s].", \
                sHisRrn, sHisTransDate);
    return 0;
}


/******************************************************************************/
/*      ������:     UpdJournalHis()                                          */
/*      ����˵��:   �˻��ཻ�׸���ԭ������ˮ                                  */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int UpdJournalHis(cJSON *pstJson, int *piFlag) {
    char sTrace[7] = {0}, sTraceNo[7] = {0}, sOTraceNo[7] = {0};
    char sTransCode[9] = {0}, sOTransDate[8 + 1] = {0};
    char sMerchId[16] = {0}, sTermId[9] = {0};
    char sORrn[12 + 1] = {0};
    double dUnrefundAmt = 0L;
    cJSON * pstTransJson = NULL;
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    GET_STR_KEY(pstTransJson, "trace_no", sTraceNo);
    GET_STR_KEY(pstTransJson, "o_trace_no", sOTraceNo);
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    GET_STR_KEY(pstTransJson, "o_trans_date", sOTransDate);
    GET_STR_KEY(pstTransJson, "o_rrn", sORrn);
    GET_DOU_KEY(pstTransJson, "refund_amt", dUnrefundAmt);

    /* ��ȡԭ������Ϣ */


#ifdef _DEBUG
    tErrLog(DEBUG, "update journal: TransCode[%s], RRN[%s]"
            , pstNetTran->sTransCode, pstHisAcctJon->sRrn);
#endif /* _DEBUG */

    /* ����ԭ������ˮ����Ч��־valid_flag */
    if (UpdOrgJonHis(sOTransDate, sORrn, '3', dUnrefundAmt) != 0) {
        ErrHanding(pstTransJson, "96", "ԭ���׸��±�־ʧ�ܣ���ԭ���ײο���[%s]��ԭ��������[%s] ���˿���[%lf].", \
                sORrn, sOTransDate, dUnrefundAmt);
        return -1;
    }

    tLog(INFO, "ԭ���׸��±�־�ɹ���ԭ���ײο���[%s]��ԭ��������[%s] ���˿���[%lf].", \
                sORrn, sOTransDate, dUnrefundAmt);

    return 0;
}
