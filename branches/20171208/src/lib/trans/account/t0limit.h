/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   t0limit.h
 * Author: Administrator
 *
 * Created on 2017��3��9��, ����3:33
 */

#ifndef T0LIMIT_H
#define T0LIMIT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   char  sUserCode[15 + 1];   /*  �̻���   */
   double  dTotallimit;           /*  T0�ܶ��   */
   double  dUsedlimit;            /*  T0���ö�� */
   double  dUsable_limit;         /*  T0���ö�� */
   
}T0Merchlimit;


#ifdef __cplusplus
}
#endif

#endif /* T0LIMIT_H */

