/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   s_param.h
 * Author: feng.gaoo
 *
 * Created on 2017年3月16日, 下午6:12
 */

#ifndef S_PARAM_H
#define S_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif
    /* 系统参数表，通过key查找value */
    int FindValueByKey(char *pcValue, char *pcKey);


#ifdef __cplusplus
}
#endif

#endif /* S_PARAM_H */

