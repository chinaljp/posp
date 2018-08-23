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

    /* 获取代理商信息*/
    MEMSET_ST(stAgent);
    if (FindAgentByMerhId(&stAgent, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, "C2", "未找到商户[%s]所属的代理商.", sMerchId);
        return -1;
    }
    /* 设置代理商信息 */
    SET_STR_KEY(pstTransJson, "agent_id", stAgent.sAgentId);
    SET_STR_KEY(pstTransJson, "agent_name", stAgent.sAgentname);
    SET_STR_KEY(pstTransJson, "agent_status", stAgent.sStatus);
    SET_STR_KEY(pstTransJson, "zmk_key", stAgent.sZmk);
    SET_STR_KEY(pstTransJson, "agent_organization", stAgent.sAgentOrn);
    /* 获取商户信息 */
    MEMSET_ST(stMerch);
    if (FindMerchInfoById(&stMerch, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, "C2", "未找到商户[%s]的信息.", sMerchId);
        return -1;
    }
    SET_STR_KEY(pstTransJson, "merch_name", stMerch.sCName);
    SET_STR_KEY(pstTransJson, "merch_p_name", stMerch.sPName); /* 凭条打印名称 */
    SET_STR_KEY(pstTransJson, "user_code", stMerch.sUserCode);
    SET_STR_KEY(pstTransJson, "merch_type", stMerch.sMerchType);
    SET_STR_KEY(pstTransJson, "mcc", stMerch.sMcc);

    /* 获取终端信息 */
    MEMSET_ST(stTerminal);
    if (FindTermInfoById(&stTerminal, sMerchId, sTermId) < 0)
    {
        ErrHanding(pstTransJson, "C1", "未找到商户[%s]终端[%s]的信息.", sMerchId, sTermId);
        return -1;
    }
    SET_STR_KEY(pstTransJson, "tmk_key", stTerminal.sTmk);
    SET_STR_KEY(pstTransJson, "term_sn", stTerminal.sTermSn);

    /* 检查商户account_uuid是否存在 */
    if (FindMerchAcctUuid(sAccountUuid, sMerchId) < 0)
    {
        ErrHanding(pstTransJson, "C2", "未找到商户[%s]的account_uuid信息.", sMerchId);
        return -1;
    }
    //检查商户account_uuid是否存在
    if (strlen(sAccountUuid) == 0)
    {
        ErrHanding(pstTransJson, "C2", "商户[%s]account_uuid[%s]不存在.", sMerchId, sAccountUuid);
        return -1;
    }
    /* 检查直属代理商状态 */
    if (STAT_OPEN != stAgent.sStatus[0])
    {
        ErrHanding(pstTransJson, "C2", "商户[%s]代理商[%s:%s]状态[%s]非正常.", sMerchId, stAgent.sAgentId, stAgent.sAgentname, stAgent.sStatus);
        return -1;
    }
    /* 检查商户状态 */
    if (STAT_OPEN != stMerch.sStatus[0])
    {
        ErrHanding(pstTransJson, "C2", "商户[%s]状态[%s]非正常.", sMerchId, stMerch.sStatus);
        return -1;
    }
    /* 检查终端状态 */
    if (STAT_OPEN != stTerminal.sStatus[0])
    {
        ErrHanding(pstTransJson, "C1", "商户[%s]终端[%s]状态[%s]非正常.", sMerchId, sTermId, stTerminal.sStatus);
        return -1;
    }
    tLog(INFO, "商户[%s]代理商[%s:%s]状态[%s]正常.", sMerchId, stAgent.sAgentId, stAgent.sAgentname, stAgent.sStatus);
    tLog(INFO, "商户[%s:%s]状态[%s]正常.", sMerchId, stMerch.sCName, stMerch.sStatus);
    tLog(INFO, "商户[%s]终端[%s]状态[%s]正常.", sMerchId, sTermId, stTerminal.sStatus);
    return 0;
}


