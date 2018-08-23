/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t_log.h
 * Author: feng.gaoo
 *
 * Created on 2016年12月11日, 下午9:26
 */

#ifndef T_LOG_H
#define T_LOG_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdarg.h> /* for va_list */
#include <stdio.h> /* for size_t */
#include <zlog.h>
    extern zlog_category_t *g_stLogCxt;

    typedef enum {
        DEBUG = 20,
        INFO = 40,
        NOTICE = 60,
        WARN = 80,
        ERROR = 100,
        FATAL = 120
    } LogLevel;

#define LOG_NAME    "log.conf"
    /* 打印日志 
     *      DEBUG = 20,
            INFO = 40,
            NOTICE = 60,
            WARN = 80,
            ERROR = 100,
            FATAL = 120
     */
#define tLog(LogLevel,format, args...)  \
    zlog(g_stLogCxt, __FILE__, sizeof (__FILE__) - 1, __func__, sizeof (__func__) - 1, __LINE__, \
        LogLevel, format,##args)

    int tInitLog(char *pcLogName);
    void tSetLogName(char *pcLogName);
    int tReloadLog();
    void tDoneLog();
    void tDumpHex(char *pcTitle, char *pcData, int iLen);


#ifdef __cplusplus
}
#endif

#endif /* T_LOG_H */

