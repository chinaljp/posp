/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   resp_code.h
 * Author: feng.gaoo
 *
 * Created on 2017��3��6��, ����10:33
 */

#ifndef RESP_CODE_H
#define RESP_CODE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "t_cjson.h"
#define MAX_RESP_CODE   256
#define DAF_RESP_DESC   "�ܾ���ϵͳ�������Ժ�����"

    /* ������Ϣ���ֵ */
#define ERR_MAX_LEN 512

    typedef struct {
        char sRespCode[4 + 1];
        char sRespDesc[255 + 1];
    } RespCode;

    void ErrHanding(cJSON *pstJson, char *pcRespCode, ...);
#ifdef __cplusplus
}
#endif

#endif /* RESP_CODE_H */

