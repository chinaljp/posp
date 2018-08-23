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



int UpFeeMsg(char *pcRrn,char *pcFeeType,double dFee,char *pcFeeDesc) {
    char sSqlStr[1024]      = {0};
    OCI_Resultset *pstRes   = NULL;
    
    snprintf(sSqlStr,sizeof(sSqlStr),"update b_inline_tarns_detail set fee_type = '%s',fee = %.02f,fee_desc = '%s',fee_flag = 'M' where rrn = '%s'",
            pcFeeType,dFee,pcFeeDesc,pcRrn);

    if ( tExecute(&pstRes, sSqlStr) < 0 || tGetAffectedRows() <= 0 ) {
        tLog(DEBUG, "sql[%s]", sSqlStr);
        tReleaseRes(pstRes);
        return ( -1 );
    }
    tLog(DEBUG,"������[%d]��",tGetAffectedRows());
    tReleaseRes(pstRes);
    return ( 0 );
}

/*���㲢���������� */
int MerchCardFee(cJSON *pstJson, int *piFlag) {
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sFeeDesc[255 + 1] = {0}, sCardType[1 + 1] = {0}, sRrn[12 + 1] = {0};
    MerchFee stMerchFee;
    double dAmount = 0.0, dFee = 0.0;
    cJSON * pstTransJson = NULL;

    pstTransJson = GET_JSON_KEY(pstJson, "data");
    GET_STR_KEY(pstTransJson, "merch_id", sMerchId);
    GET_DOU_KEY(pstTransJson, "amount", dAmount);
    GET_STR_KEY(pstTransJson, "card_type", sCardType);
    GET_STR_KEY(pstTransJson, "rrn", sRrn);
    
    if (FindMerchFee(&stMerchFee, sMerchId) < 0) {
        ErrHanding(pstTransJson, "96", "δ�ҵ��̻�[%s]�ķ�������.", sMerchId);
        return -1;
    }
    tLog( DEBUG, "�̻�[%s]�������[%s]",sCardType,sCardType[0] == '1'?"���ÿ�":"��ǿ�" );
    if(sCardType[0] == '1') {
        dFee = (dAmount / 100) * stMerchFee.dQCRate / 100;
        sprintf(sFeeDesc, "%f", stMerchFee.dQCRate / 100);
    }
    else {
        dFee = (dAmount / 100) * stMerchFee.dQDRate / 100;
        sprintf(sFeeDesc, "%f", stMerchFee.dQDRate / 100);
    }
    
    tMake(&dFee, 2, 'u');
    tLog(INFO, "�̻�[%s]�������������[%f]", sMerchId, dFee);
    if ( UpFeeMsg( sRrn,"B",dFee,sFeeDesc ) < 0 ) {
        ErrHanding(pstTransJson, "96", "�������[%s],�����̻�[%s]�������ѳ���.", sRrn,sMerchId);
        return ( -1 );
    }
    
    return 0;
}