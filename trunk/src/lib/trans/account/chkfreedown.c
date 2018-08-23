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
#include "t_macro.h"
#include "status.h"
#include "trans_code.h"

int DBChkMerchFreeFlag(char * psMerchId, char * psTermId) {
    char sMerchId[15 + 1] = {0};
    char sTermId[8 + 1] = {0};
    char sSqlStr[512] = {0}, sMerchFreeFlag[2] = {0}, sTermFreeFlag[2] = {0};
    OCI_Resultset *pstRes = NULL;

    strcpy(sMerchId, psMerchId);
    strcpy(sTermId, psTermId);

    snprintf(sSqlStr, sizeof (sSqlStr), "select a.free_flag,b.free_flag from b_merch a join b_term b on b.user_code=a.user_code \
          WHERE a.merch_id = '%s' and a.status='1' and b.term_id='%s' and b.status='1'", sMerchId, sTermId);
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, sMerchFreeFlag);
        STRV(pstRes, 2, sTermFreeFlag);
    }

    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�ҵ���¼.");
        tReleaseRes(pstRes);
        return -1;
    }
    /*
       b_merch:  0:δ��ͨ˫�� 1:�ѿ�ͨ˫��, 2:��ֹ��ͨ˫��, 3:��ͨ����
     * b_term:  0�������� 1���Ѹ���
     */
    if (sMerchFreeFlag[0] == '0' || sMerchFreeFlag[0] == '2' || sMerchFreeFlag[0] == '3') {
        tLog(INFO, "�̻�δ��ͨ˫�⣬���ֹ��ͨ����ͨ�����������طǽӲ���");
        tReleaseRes(pstRes);
        return 0;
    }
    if (sMerchFreeFlag[0] == '1' && sTermFreeFlag[0] == '1') {
        tLog(INFO, "�̻��ѱ���˫�⣬�ն��Ѹ��·ǽӲ����������޸ı���ͷ");
        tReleaseRes(pstRes);
        return 0;
    }

    if (sMerchFreeFlag[0] == '1' && sTermFreeFlag[0] == '2') {
        tLog(INFO, "�̻��ѱ���˫�⣬�ն˲����·ǽӲ����������޸ı���ͷ");
        tReleaseRes(pstRes);
        return 0;
    }

    if (sMerchFreeFlag[0] == '1' && sTermFreeFlag[0] == '0') {
        tLog(INFO, "�̻��ѱ���˫�⣬�ն�û���·ǽӲ������޸ı���ͷ����Ҫ��Ϊ��9��");
        tReleaseRes(pstRes);
        return 1;
    }

    return 0;

}

/*
 * ����̻��Ƿ�ͨ˫�⣬�ն��Ƿ����طǽӲ���
 * 
 * ==========================================================*
 Ӧ�������	����汾��	�ն�״̬	    ����Ҫ��	����ʹ��
   N2	           N2	           N1	      N1	          N6
 * ==========================================================*
 */

int ChkFreeDown(cJSON *pstJson, int *piFlag) {

    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    char sHeard[12 + 1] = {0};
    int iRet = 0;
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);
    iRet = DBChkMerchFreeFlag(sMerchId, sTermId);
    if (iRet < 0) {
        ErrHanding(pstTransJson, "96", "�����̻�[%s]˫���־��Ϣʧ��.", sMerchId);
        return -1;
    }
    if (iRet == 1) {
        GET_STR_KEY(pstTransJson, "head", sHeard);
        /* ���ñ���ͷ����Ҫ��Ϊ��9���ǽ�ҵ���������*/
        sHeard[5] = '9';
        SET_STR_KEY(pstTransJson, "head", sHeard);
    }
    return 0;
}
