/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   param.h
 * Author: Administrator
 *
 * Created on 2017年3月24日, 下午4:15
 */

#ifndef PARAM_H
#define PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

    int InitParam(int iArgc, char* pcArgv[]);
    char *GetLogName();
    char *GetSvrName();
    char *GetLibName();
    int GetAppNum();


#ifdef __cplusplus
}
#endif

#endif /* PARAM_H */

