/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_log.h"
#include "t_db.h"
#include "t_tools.h"
#include "agent.h"

int FindAgentByMerhId(Agent *pstAgent, char *pcMerchId) {
    char sMerchId[] = {0}, sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    /* 查找商户所属代理商 */
    /*snprintf(sSqlStr, sizeof (sSqlStr), "select a.AGENT_ID,a.AGENT_NAME,a.STATUS,a.ZMK, a.AGENT_LEVEL, a.GRADE_AGENT_LEVEL, m.merch_type "
            " from B_AGENT a join B_MERCH m on m.AGENT_ID=a.AGENT_ID where m.MERCH_ID='%s'", pcMerchId);
    */
    snprintf(sSqlStr, sizeof (sSqlStr), "select a.AGENT_ID,a.AGENT_NAME,a.STATUS,a.ZMK, a.AGENT_LEVEL, a.GRADE_AGENT_LEVEL, a.AGENT_ORGANIZATION "
            " from B_AGENT a join B_MERCH m on m.AGENT_ID=a.AGENT_ID where m.MERCH_ID= :merch_id");
    if (tExecutePre(&pstRes, sSqlStr, 0, 0, 1, pcMerchId, ":merch_id") < 0) {
        tLog(ERROR, "查找商户[%s]所属的代理商资料,失败.", pcMerchId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstAgent->sAgentId);
        STRV(pstRes, 2, pstAgent->sAgentname);
        STRV(pstRes, 3, pstAgent->sStatus);
        STRV(pstRes, 4, pstAgent->sZmk);
        STRV(pstRes, 5, pstAgent->sAgentLevel);
        STRV(pstRes, 6, pstAgent->sGradeAgentLevel);
        STRV(pstRes, 7, pstAgent->sAgentOrganization);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无代理商[%s]资料,请先添加机构.", pcMerchId);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}

int FindAgentById(Agent *pstAgent, char *pcAgentId) {
    char sMerchId[] = {0}, sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;

    /* 查找商户所属代理商 */
    snprintf(sSqlStr, sizeof (sSqlStr), "select AGENT_ID,AGENT_NAME,STATUS,ZMK "
            " from B_AGENT where agent_id='%s'", pcAgentId);

    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "查找代理商[%s]资料,失败.", pcAgentId);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pstAgent->sAgentId);
        STRV(pstRes, 2, pstAgent->sAgentname);
        STRV(pstRes, 3, pstAgent->sStatus);
        STRV(pstRes, 4, pstAgent->sZmk);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "无代理商[%s]资料,请先添加机构.", pcAgentId);
        tReleaseRes(pstRes);
        return -1;
    }
    tReleaseRes(pstRes);
    return 0;
}
