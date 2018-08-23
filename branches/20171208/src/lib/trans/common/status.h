/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   status.h
 * Author: feng.gaoo
 *
 * Created on 2017年3月16日, 下午5:44
 */

#ifndef STATUS_H
#define STATUS_H

#ifdef __cplusplus
extern "C" {
#endif
/* 所有的业务状态都在这个头文件中定义 */
#define STAT_OPEN   '1' /* 正常/有效 */
#define STAT_CLOSE  '0' /* 关闭/无效 */
#define STAT_CANCEL 'X' /* 注销 */


#ifdef __cplusplus
}
#endif

#endif /* STATUS_H */

