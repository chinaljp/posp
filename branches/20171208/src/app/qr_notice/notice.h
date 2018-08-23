/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   saf_trans_detail.h
 * Author: feng.gaoo
 *
 * Created on 2017年3月7日, 下午1:54
 */

#ifndef SAF_TRANS_DETAIL_H
#define SAF_TRANS_DETAIL_H
#ifdef __cplusplus
extern "C" {
#endif
#include "t_cjson.h"

    int UpdSafCnt(cJSON *pstJson);
    int UpdSafLs(cJSON *pstJson);
    int AddSafLs(cJSON *pstJson);


#ifdef __cplusplus
}
#endif

#endif /* SAF_TRANS_DETAIL_H */

