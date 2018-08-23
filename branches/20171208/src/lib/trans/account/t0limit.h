/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t0limit.h
 * Author: Administrator
 *
 * Created on 2017年3月9日, 下午3:33
 */

#ifndef T0LIMIT_H
#define T0LIMIT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   char  sUserCode[15 + 1];   /*  商户号   */
   double  dTotallimit;           /*  T0总额度   */
   double  dUsedlimit;            /*  T0已用额度 */
   double  dUsable_limit;         /*  T0可用额度 */
   
}T0Merchlimit;


#ifdef __cplusplus
}
#endif

#endif /* T0LIMIT_H */

