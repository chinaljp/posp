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

int FindLocalCityLifeMerch( char *pcChanneMerchId, char *pcChannelMerchName, char *pcUserCode ) {
    
    char sSqlStr[1024 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    snprintf(sSqlStr, sizeof (sSqlStr), " select l.merch_id,l.merch_name"
                                        " from b_merch b join b_life_merch l on b.city_code = l.area_code"
                                        " where  b.user_code = '%s' and l.status = '1' and rownum = 1", pcUserCode);
    
    tLog(DEBUG, "sql[%s] !", sSqlStr);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while ( OCI_FetchNext(pstRes) ) { 
        STRV(pstRes, 1, pcChanneMerchId);
        STRV(pstRes, 2, pcChannelMerchName);
        
        tTrim(pcChanneMerchId);
        tTrim(pcChannelMerchName);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR,"按照地区编码没有找到本地区的生活圈商户!");
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        MEMSET(sSqlStr);
        snprintf(sSqlStr, sizeof (sSqlStr), "select l.merch_id,l.merch_name from"
                                                " (select m.city_code as city_code from s_city m where m.city_level = '3'"
                                                " start with m.city_code = ( select city_code  from b_merch where user_code = '%s')"
                                                " connect by m.CITY_CODE =prior m.P_CITY_CODE ) b"
                                                " join b_life_merch l on b.city_code = l.city_code where  l.status = '1'  and rownum = 1", pcUserCode);
        tLog(DEBUG, "sql[%s] !", sSqlStr);
        pstState = tExecuteEx(&pstRes, sSqlStr);
        if (pstState == NULL) {
            tLog(ERROR, "sql[%s] err!", sSqlStr);
            return -1;
        }

        if (NULL == pstRes) {
            tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
            return -1;
        }

        while ( OCI_FetchNext(pstRes) ) { 
            STRV(pstRes, 1, pcChanneMerchId);
            STRV(pstRes, 2, pcChannelMerchName);

            tTrim(pcChanneMerchId);
            tTrim(pcChannelMerchName);
        }
        if (0 == OCI_GetRowCount(pstRes)) {
            tLog(ERROR,"按照 地区编码上级的市级编码 没有找到本市的生活圈商户!");
            tReleaseResEx(pstState);
            OCI_StatementFree(pstState);
            return ( -1 );
        }
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    
    return ( 0 );
}

int FindCapitalLifeMerch( char *pcChanneMerchId, char *pcChannelMerchName, char *pcUserCode ) {
    char sSqlStr[1024 + 1] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement *pstState = NULL;
    
    snprintf(sSqlStr, sizeof (sSqlStr), " SELECT c.merch_id,c.merch_name FROM" 
                            " ( select m.city_code as city_code from s_city m where m.city_level = '2'"
                                " start with m.city_code = (select city_code  from b_merch where user_code = '%s')"
                                " connect by m.CITY_CODE =prior m.P_CITY_CODE ) a"
                            " join  s_prov_capital b on a.city_code = b.province_code"
                            " join  b_life_merch c on c.city_code = b.capital_code"
                            " WHERE   c.status = '1' and rownum = 1", pcUserCode);
    
    tLog(DEBUG, "sql[%s] !", sSqlStr);
    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]结果集为NULL.", sSqlStr);
        return -1;
    }

    while ( OCI_FetchNext(pstRes) ) { 
        STRV(pstRes, 1, pcChanneMerchId);
        STRV(pstRes, 2, pcChannelMerchName);
        
        tTrim(pcChanneMerchId);
        tTrim(pcChannelMerchName);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR,"按照省会城市编码没有找到省会城市下的生活圈商户！");
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return ( -1 );
    }    
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    
    return ( 0 );
}