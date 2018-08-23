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
#include "t_macro.h"
#include "status.h"

int ChkMerch(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0};
    Merch stMerch;
    cJSON *pstAgentJson = NULL;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    if (FindMerchInfoById(&stMerch, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]����Ϣ.", sMerchId);
        return -1;
    }

    /* ���״̬ */
    if (STAT_OPEN != stMerch.sStatus[0]) {
        ErrHanding(pstTransJson, "C2", "�̻�[%s]״̬[%s]������.", sMerchId, stMerch.sStatus);
        return -1;
    }

    SET_STR_KEY(pstTransJson, "merch_name", stMerch.sCName);
    SET_STR_KEY(pstTransJson, "merch_p_name", stMerch.sPName); /* ƾ����ӡ���� */
    tLog(INFO, "�̻�[%s][%s]���ɹ�.", sMerchId, stMerch.sCName);
    return 0;
}