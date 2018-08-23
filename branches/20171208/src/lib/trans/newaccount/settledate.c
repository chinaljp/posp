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

/* ��ȡ���п������� */
int GetCardSettleDate(cJSON *pstTransJson, int *piFlag) {
    char sDate[8 + 1] = {0}, sRespCode[2 + 1] = {0};

    FindCardSettleDate(sDate);
    SET_STR_KEY(pstTransJson, "settle_date", sDate);
    tLog(INFO, "��ȡ�����ճɹ�,T+1��[%s]����", sDate);
    return 0;
}

/* ��ȡ��ά������� */
int GetQrSettleDate(cJSON *pstTransJson, int *piFlag) {
    char sDate[8 + 1] = {0}, sRespCode[2 + 1] = {0};
    
    GET_STR_KEY(pstTransJson, "resp_code", sRespCode);
    if (memcmp(sRespCode, "00", 2))
    {
        tLog(ERROR, "����ʧ��[%s],�����������.", sRespCode);
        return 0;
    }
    FindQrSettleDate(sDate);
    SET_STR_KEY(pstTransJson, "settle_date", sDate);
    tLog(INFO, "��ȡ�����ճɹ�,T+1��[%s]����.", sDate);
    return 0;
}