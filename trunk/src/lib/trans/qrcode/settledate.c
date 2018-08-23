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
#include "settle_date.h"
#include "t_macro.h"
#include "status.h"

/* ��ȡ��ά������� */
int GetQrSettleDate(cJSON *pstJson, int *piFlag) {
    char sDate[8 + 1] = {0}, sTransCode[6 + 1] = {0};
    char sCurTime[6+1] = {0},sTmpTime[13+1] = {0};  //add by Gjq at 20180404
    char sBeginTime[6+1] = {0},sEndTime[6+1] = {0}; //add by Gjq at 20180404
    cJSON * pstTransJson = NULL;
    
    pstTransJson = GET_JSON_KEY(pstJson, "data");
    
    /*BEGIN Ϊ��ֹ����ǰ�������˳ɹ��ʵͣ���ά�뽻����ͣʮ���� add by Gjq at 20180403*/
    //��ȡ����ǰ��ͣ���׵���ֹʱ���(235000-000001)
    if( FindValueByKey(sTmpTime,"before.dayend.time") < 0){
        tLog(ERROR, "����key[before.dayend.time] ʧ��.");
        return ( -1 );
    }
    tGetTime(sCurTime, "", -1);
    memcpy(sBeginTime,sTmpTime,6);
    memcpy(sEndTime,sTmpTime+7,6); 
    tLog(DEBUG,"��ǰʱ��Ϊ[%s],����ǰ��ͣ���׵���ֹʱ��[%s],sBeginTime[%s]-sEndTime[%s]",
            sCurTime,sTmpTime,sBeginTime,sEndTime);
    if ( memcmp(sCurTime,sBeginTime,6) > 0 || memcmp(sCurTime,sEndTime,6) <= 0 ) {
        //tLog(ERROR,"��ǰʱ��[%s]Ϊ����ʱ��Σ���������",sCurTime);
        ErrHanding(pstTransJson, "90", "��ǰʱ���[%s]��������",sCurTime);
        return ( -1 );
    }
    /*END Ϊ��ֹ����ǰ�������˳ɹ��ʵͣ���ά�뽻����ͣʮ���� add by Gjq at 20180403*/
    
#if 0
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2)) {
        tLog(ERROR, "����ʧ��[%s],�����������.", sRespCode);
        return 0;
    }
#endif
    
    GET_STR_KEY(pstTransJson, "trans_code", sTransCode);
    if(sTransCode[2] == 'Y') {
        /*��ȡ������ά�뽻�׽�������*/
        tLog(DEBUG,"��ȡ������ά�뽻��[%s]�Ľ������ڡ�",sTransCode);
        FindCardSettleDate(sDate);
    }
    else if( sTransCode[2] == 'W' || sTransCode[2] == 'B' ){
        /*��ȡ ΢�š�֧���� ��ά�뽻�׽�������*/
        tLog(DEBUG,"��ȡ%s��ά�뽻��[%s]�Ľ������ڡ�",sTransCode[2] == 'W'?"΢��":"֧����",sTransCode);
        FindQrSettleDate(sDate);
    } 
    else {
        ErrHanding(pstTransJson, "96", "�Ƿ��ĵĽ�����[%s]",sTransCode);
        return ( -1 );
    }
    SET_STR_KEY(pstTransJson, "settle_date", sDate);
    tLog(ERROR, "��ȡ�����ճɹ�,ʹ��T+1��[%s]����.", sDate);
    return 0;
}