/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   signal.h
 * Author: feng.gaoo
 *
 * Created on 2016年12月11日, 下午9:03
 */

#ifndef SIGNAL_H
#define SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif
    extern int g_iQuitLoop;

#define APP_EXIT    0
#define APP_RUN     1

    int tIsQuit();
    void tSigQuit(int iSigNo);
    void tSigMsgQuit(int iSigNo);
    void tSigProc();



#ifdef __cplusplus
}
#endif

#endif /* SIGNAL_H */

