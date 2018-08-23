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
#include "merch.h"
#include "t_macro.h"
#include "status.h"
#include "term.h"

int ChkUser(cJSON *pstTransJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sTermId[TERM_ID_LEN + 1] = {0};
    char sAccountUuid[64 + 1] = {0};
    Agent stAgent;
    Merch stMerch;
    Terminal stTerminal;

    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_STR_KEY(pstTransJson, "term_id", sTermId);

    /* ��ȡ��������Ϣ*/
    MEMSET_ST(stAgent);
    if (FindAgentByMerhId(&stAgent, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, "C2", "δ�ҵ��̻�[%s]�����Ĵ�����.", sMerchId);
        return -1;
    }
    /* ���ô�������Ϣ */
    SET_STR_KEY(pstTransJson, "agent_id", stAgent.sAgentId);
    SET_STR_KEY(pstTransJson, "agent_name", stAgent.sAgentname);
    SET_STR_KEY(pstTransJson, "agent_status", stAgent.sStatus);
    SET_STR_KEY(pstTransJson, "zmk_key", stAgent.sZmk);
    SET_STR_KEY(pstTransJson, "agent_organization", stAgent.sAgentOrn);
    /* ��ȡ�̻���Ϣ */
    MEMSET_ST(stMerch);
    if (FindMerchInfoById(&stMerch, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, "C2", "δ�ҵ��̻�[%s]����Ϣ.", sMerchId);
        return -1;
    }
    SET_STR_KEY(pstTransJson, "merch_name", stMerch.sCName);
    SET_STR_KEY(pstTransJson, "merch_p_name", stMerch.sPName); /* ƾ����ӡ���� */
    SET_STR_KEY(pstTransJson, "user_code", stMerch.sUserCode);
    SET_STR_KEY(pstTransJson, "merch_type", stMerch.sMerchType);
    SET_STR_KEY(pstTransJson, "mcc", stMerch.sMcc);

    /* ��ȡ�ն���Ϣ */
    MEMSET_ST(stTerminal);
    if (FindTermInfoById(&stTerminal, sMerchId, sTermId) < 0)
    {
        ErrHanding(pstTransJson, "C1", "δ�ҵ��̻�[%s]�ն�[%s]����Ϣ.", sMerchId, sTermId);
        return -1;
    }
    SET_STR_KEY(pstTransJson, "tmk_key", stTerminal.sTmk);
    SET_STR_KEY(pstTransJson, "term_sn", stTerminal.sTermSn);

    /* ����̻�account_uuid�Ƿ���� */
    if (FindMerchAcctUuid(sAccountUuid, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, "C2", "δ�ҵ��̻�[%s]��account_uuid��Ϣ.", sMerchId);
        return -1;
    }
    //����̻�account_uuid�Ƿ����
    if (strlen(sAccountUuid) == 0)
    {
        ErrHanding(pstTransJson, "C2", "�̻�[%s]account_uuid[%s]������.", sMerchId, sAccountUuid);
        return -1;
    }
    /* ���ֱ��������״̬ */
    if (STAT_OPEN != stAgent.sStatus[0])
    {
        ErrHanding(pstTransJson, "C2", "�̻�[%s]������[%s:%s]״̬[%s]������.", sMerchId, stAgent.sAgentId, stAgent.sAgentname, stAgent.sStatus);
        return -1;
    }
    /* ����̻�״̬ */
    if (STAT_OPEN != stMerch.sStatus[0])
    {
        ErrHanding(pstTransJson, "C2", "�̻�[%s]״̬[%s]������.", sMerchId, stMerch.sStatus);
        return -1;
    }
    /* ����ն�״̬ */
    if (STAT_OPEN != stTerminal.sStatus[0])
    {
        ErrHanding(pstTransJson, "C1", "�̻�[%s]�ն�[%s]״̬[%s]������.", sMerchId, sTermId, stTerminal.sStatus);
        return -1;
    }
    tLog(INFO, "�̻�[%s]������[%s:%s]״̬[%s]����.", sMerchId, stAgent.sAgentId, stAgent.sAgentname, stAgent.sStatus);
    tLog(INFO, "�̻�[%s:%s]״̬[%s]����.", sMerchId, stMerch.sCName, stMerch.sStatus);
    tLog(INFO, "�̻�[%s]�ն�[%s]״̬[%s]����.", sMerchId, sTermId, stTerminal.sStatus);
    return 0;
}


