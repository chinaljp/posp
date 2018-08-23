/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   daemon.h
 * Author: feng.gaoo
 *
 * Created on 2016年12月11日, 下午9:20
 */

#ifndef DAEMON_H
#define DAEMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define tDaemon()   \
    pid_t tPid; \
    tPid = fork();  \
    if (tPid < 0) { \
        fprintf(stderr, "Init Daemon Failed! %d:%s", errno, strerror(errno));   \
        return (EXIT_FAILURE);  \
        } else if (tPid > 0)    \
        return (EXIT_SUCCESS);  \
    setsid();   \
    setpgrp();  


#ifdef __cplusplus
}
#endif

#endif /* DAEMON_H */

