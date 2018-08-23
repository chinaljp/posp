/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   sec.h
 * Author: feng.gaoo
 *
 * Created on 2017年2月28日, 下午9:33
 */

#ifndef SEC_H
#define SEC_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef __SEC_API_H__
#define __SEC_API_H__

#ifndef TRUE
#define  TRUE  0
#endif

#ifndef FALSE
#define  FALSE -1
#endif

#define ESEC_CONNECT        0x91
#define ESEC_SEND           0x92
#define ESEC_RECV           0x93
#define ESEC_SEND_TIMEOUT   0x94
#define ESEC_RECV_TIMEOUT   0x95
#define ESEC_PARA				 0x96
#define SECBUF_MAX_SIZE  2000
#define MAX_LEN			255
#define DATALEN			16

FILE * pLog=NULL;


#define		HSM_LOG		"hsm"

#endif	



#ifdef __cplusplus
}
#endif

#endif /* SEC_H */

