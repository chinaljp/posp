/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include "t_extiso.h"

IsoTable g_staIsoTable[] = {
        {   4, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 01: 消息类型         */
        {  19, LLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },         /* 02: 主账号           */
        {   6, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 03: 交易处理码       */
        {  12, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 04: 交易金额         */
        {   2, 0x00 },                                              /* 05   */
        {   2, 0x00 },                                              /* 06   */
        {   10, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN  },       /* 07   */
        {   2, 0x00 },                                              /* 08   */
        {   2, 0x00 },                                              /* 09   */
        {   2, 0x00 },                                              /* 10   */
        {   6, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 11: 终端流水号       */
        {   6, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 12: 交易时间         */ 
        {   4, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 13: 交易日期         */
        {   4, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 14: 卡有效期         */
        {   4, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 15: 清算日期         */
        {   2, 0x00 },                                              /* 16   */
        {   2, 0x00 },                                              /* 17   */
        {   2, 0x00 },                                              /* 18   */
        {   2, 0x00 },                                              /* 19   */
        {   2, 0x00 },                                              /* 20   */
        {   2, 0x00 },                                              /* 21   */
        {   3, FIXED|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },          /* 22: 服务点输入方式码 */
        {   3, FIXED|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },          /* 23: IC卡序列号       */
        {   2, 0x00 },                                              /* 24   */
        {   2, FIXED|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },          /* 25: 服务点条件码     */
        {   2, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 26: 服务点PIN 获取码 */
        {   2, 0x00 },                                              /* 27   */
        {   2, 0x00 },                                              /* 28   */
        {   2, 0x00 },                                              /* 29   */
        {   2, 0x00 },                                              /* 30   */
        {   2, 0x00 },                                              /* 31   */
        {  11, LLVAR|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },          /* 32: 收单机构标识     */
        {   2, 0x00 },                                              /* 33   */
        {   2, 0x00 },                                              /* 34   */
        {  37, LLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },         /* 35: 2 磁道数据       */
        { 104, LLLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },        /* 36: 3 磁道数据       */
        {  12, FIXED|OTHER|ASCII|UNSYMBOL|ZERO|RIGHT_ALIGN },       /* 37: RRN              */
        {   6, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 38: 授权码           */
        {   2, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 39: 应答码           */
        {   2, 0x00 },                                              /* 40   */
        {   8, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 41: 终端编号         */
        {  15, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 42: 商户编号         */
        {   2, 0x00 },                                              /* 43   */
        {  25, LLVAR|OTHER|ASCII|UNSYMBOL|BLANK|RIGHT_ALIGN },      /* 44: 接收机构, 收单结构 */
        {   2, 0x00 },                                              /* 45   */
        {   2, 0x00 },                                              /* 46   */
        {   2, 0x00 },                                              /* 47   */
        { 322, LLLVAR|OTHER|ASCII|UNSYMBOL|ZERO|LEFT_ALIGN },         /* 48: 附加数据         */
        {   3, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 49: 交易货币代码     */
        {   2, 0x00 },                                              /* 50   */
        {   2, 0x00 },                                              /* 51   */
        {   8, FIXED|BINARY|ASCII|UNSYMBOL|ZERO|RIGHT_ALIGN },      /* 52: PIN              */
        {  16, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 53: 安全控制信息     */
        {  40, LLLVAR|OTHER|ASCII|UNSYMBOL|ZERO|RIGHT_ALIGN },      /* 54: 附加金额         */
        { 255, LLLVAR|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 55: IC卡数据域       */
        {   2, 0x00 },                                              /* 56   */
        {   2, 0x00 },                                              /* 57   */
        { 100, LLLVAR|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },         /* 58: 电子钱包交易信息 */
        { 100, LLLVAR|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },      /* 59: 自定义域         */
        {  16, LLLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },        /* 60: 自定义域         */
        {  29, LLLVAR|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },        /* 61: 原始信息域       */
        { 999, LLLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },      /* 62: 自定义域         */
        { 163, LLLVAR|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },      /* 63: 自定义域         */
        {   8, FIXED|BINARY|ASCII|UNSYMBOL|ZERO|RIGHT_ALIGN }       /* 64: MAC              */
};