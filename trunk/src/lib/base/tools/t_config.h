/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t_config.h
 * Author: feng.gaoo
 *
 * Created on 2016��12��5��, ����7:30
 */

#ifndef T_CONFIG_H
#define T_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

int     tGetConfig( char *pcFileName, char *pcStructDes, char *pcStruName, void *pvStructAddr, char *pcGroup );
void    tFreeConfig( );


#ifdef __cplusplus
}
#endif

#endif /* T_CONFIG_H */

