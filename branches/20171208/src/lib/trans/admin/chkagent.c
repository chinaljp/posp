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
#include "agent.h"
#include "t_macro.h"

int ChkAgent(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0};
    Agent stAgent;
    cJSON *pstAgentJson = NULL;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);

    if (FindAgentByMerhId(&stAgent, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]�����Ĵ�����.", sMerchId);
        return -1;
    }

    pstAgentJson = cJSON_CreateObject();
    if (NULL == pstAgentJson) {
        ErrHanding(pstTransJson, "96", "�����̻�[%s]�Ĵ���json�ṹʧ��.", sMerchId);
        return -1;
    }

    SET_STR_KEY(pstAgentJson, "agent_id", stAgent.sAgentId);
    SET_STR_KEY(pstAgentJson, "agent_name", stAgent.sAgentname);
    SET_STR_KEY(pstAgentJson, "agent_status", stAgent.sStatus);
    SET_STR_KEY(pstAgentJson, "zmk_key", stAgent.sZmk);
    SET_JSON_KEY(pstTransJson, "agent", pstAgentJson);
    SET_STR_KEY(pstTransJson, "agent_organization", stAgent.sAgentOrn);
    tLog(INFO, "�̻�[%s]������[%s][%s]���ɹ�.", sMerchId, stAgent.sAgentId, stAgent.sAgentname);
    return 0;
}