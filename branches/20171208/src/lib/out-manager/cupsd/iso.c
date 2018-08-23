/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include "t_extiso.h"

IsoTable g_staIsoTable[] = {
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 001: 消息类型码      */
    { 19, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 002: 主账号          */
    { 6, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 003: 交易处理码      */
    { 12, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 004: 交易金额        */
    /*****
     * 修改第5，6域，明确数据类型
     * { 12, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 12, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 005: 清算金额        */
    { 12, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 006: 扣帐金额        */
    { 10, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 007: 交易传输时间    */
    { 2, 0x00}, /* 008  */
    /*****
     * 修改第9，10域，明确数据类型
     * { 12, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 8, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 009: 清算汇率        */
    { 8, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 010: 扣帐汇率        */
    { 6, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 011: 系统跟踪号      */
    { 6, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 012: 交易时间        */
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 013: 交易日期        */
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 014: 卡有效期        */
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 015: 清算日期        */
    /*****
     * 修改第16域，明确数据类型
     * { 12, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 016: 兑换日期        */
    { 2, 0x00}, /* 017  */
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 018: 商户类型(MCC)   */
    /*****
     * 修改第16域，明确数据类型
     * { 2, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 019: 受理机构国家代码*/
    { 2, 0x00}, /* 020  */
    { 2, 0x00}, /* 021  */
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 022: 服务点输入方式码*/
     /*****
     * 修改第23域，明确数据类型
     * { 2, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 023: 卡序列号        */
    { 2, 0x00}, /* 024  */
    { 2, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 025: 服务点条件码    */
    { 2, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 026: 服务点PIN获取码 */
    { 2, 0x00}, /* 027  */
     /*****
     * 修改第28域，明确数据类型
     * { 2, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 9, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 028: 手续费          */
    { 2, 0x00}, /* 029  */
    { 2, 0x00}, /* 030  */
    { 2, 0x00}, /* 031  */
    { 11, LLVAR | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 032: 受理机构标识    */
    { 11, LLVAR | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 033: 发送机构标识    */
    { 2, 0x00}, /* 034  */
    { 37, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 035: 2磁道数据       */
    { 104, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 036: 3磁道数据       */
    { 12, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 037: RRN             */
    { 6, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 038: 授权码          */
    { 2, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 039: 应答码          */
    { 2, 0x00}, /* 040  */
    { 8, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 041: 终端编号        */
    { 15, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 042: 商户编号        */
    { 40, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 043: 商户名称和地址  */
    { 25, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | RIGHT_ALIGN}, /* 044: 附加响应数据    */
     /*****
     * 修改第45域，明确数据类型
     * { 2, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 79, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | RIGHT_ALIGN}, /* 045  */
    { 2, 0x00}, /* 046  */
    { 2, 0x00}, /* 047  */
    { 512, LLLVAR | BINARY | ASCII | UNSYMBOL | ZERO | LEFT_ALIGN}, /* 048: 附加数据        */
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 049: 货币代码        */
    { 3, 0x00}, /* 050  */
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 051: 持卡人账货币代码        */
    { 8, FIXED | BINARY | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 052: PIN             */
    { 16, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 053: 安全控制信息    */
    { 40, LLLVAR | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 054: 账户余额        */
    { 255, LLLVAR | BINARY | ASCII | UNSYMBOL | ZERO | LEFT_ALIGN}, /* 055: ic卡数据域      */
    { 2, 0x00}, /* 056  */
    { 10, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 057:附加域        */
    { 2, 0x00}, /* 058  */
    { 2, 0x00}, /* 059  */
    { 30, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 060: 自定义域        */
    { 200, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 061: 持卡人身份认证信息  */
    { 2, 0x00}, /* 062  */
    { 2, 0x00}, /* 063  */
    { 2, 0x00}, /* 064  */
    { 2, 0x00}, /* 065  */
    { 2, 0x00}, /* 066  */
    { 2, 0x00}, /* 067  */
    { 2, 0x00}, /* 068  */
    { 2, 0x00}, /* 069  */
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 070: 网络管理信息码  */
    { 2, 0x00}, /* 071  */
    { 2, 0x00}, /* 072  */
    { 2, 0x00}, /* 073  */
    { 2, 0x00}, /* 074  */
    { 2, 0x00}, /* 075  */
    { 2, 0x00}, /* 076  */
    { 2, 0x00}, /* 077  */
    { 2, 0x00}, /* 078  */
    { 2, 0x00}, /* 079  */
    { 2, 0x00}, /* 080  */
    { 2, 0x00}, /* 081  */
    { 2, 0x00}, /* 082  */
    { 2, 0x00}, /* 083  */
    { 2, 0x00}, /* 084  */
    { 2, 0x00}, /* 085  */
    { 2, 0x00}, /* 086  */
    { 2, 0x00}, /* 087  */
    { 2, 0x00}, /* 088  */
    { 2, 0x00}, /* 089  */
    { 42, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 090: 原始信息        */
    { 2, 0x00}, /* 091  */
    { 2, 0x00}, /* 092  */
    { 2, 0x00}, /* 093  */
    { 2, 0x00}, /* 094  */
    { 2, 0x00}, /* 095  */
    { 8, FIXED | BINARY | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 096: 报文安全码      */
    { 2, 0x00}, /* 097  */
    { 2, 0x00}, /* 098  */
    { 2, 0x00}, /* 099  */
    { 11, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 100: 接收机构标识    */
    { 2, 0x00}, /* 101  */
    { 30, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 102: 接收机构标识    */
    { 30, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 103: 接收机构标识    */
    { 512, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 104: 终端上送硬件序列号及密文数据  */
    { 2, 0x00}, /* 105  */
    { 2, 0x00}, /* 106  */
    { 2, 0x00}, /* 107  */
    { 2, 0x00}, /* 108  */
    { 2, 0x00}, /* 109  */
    { 2, 0x00}, /* 110  */
    { 2, 0x00}, /* 111  */
    { 2, 0x00}, /* 112  */
    { 2, 0x00}, /* 113  */
    { 2, 0x00}, /* 114  */
    { 2, 0x00}, /* 115  */
    { 2, 0x00}, /* 116  */
    { 2, 0x00}, /* 117  */
    { 2, 0x00}, /* 118  */
    { 2, 0x00}, /* 119  */
    { 2, 0x00}, /* 120  */
    { 100, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 121: CUPS保留信息    */
    { 100, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 122: 受理方保留信息  */
    { 100, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 123: 发卡方保留信息  */
    { 2, 0x00}, /* 124  */
    { 2, 0x00}, /* 125  */
    { 2, 0x00}, /* 126  */
    { 2, 0x00}, /* 127  */
    { 8, FIXED | BINARY | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 128: MAC             */
};
