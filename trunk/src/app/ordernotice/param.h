/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   config.h
 * Author: feng.gaoo
 *
 * Created on 2016��12��8��, ����10:31
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

    int InitParam(int iArgc, char* pcArgv[]);
    char *GetLogName();
    char *GetSvrName();
    char *GetLibName();
    int GetAppNum();
    int GetNoticeCnt();


#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */

