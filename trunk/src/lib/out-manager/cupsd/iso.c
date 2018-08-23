/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include "t_extiso.h"

IsoTable g_staIsoTable[] = {
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 001: ��Ϣ������      */
    { 19, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 002: ���˺�          */
    { 6, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 003: ���״�����      */
    { 12, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 004: ���׽��        */
    /*****
     * �޸ĵ�5��6����ȷ��������
     * { 12, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 12, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 005: ������        */
    { 12, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 006: ���ʽ��        */
    { 10, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 007: ���״���ʱ��    */
    { 2, 0x00}, /* 008  */
    /*****
     * �޸ĵ�9��10����ȷ��������
     * { 12, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 8, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 009: �������        */
    { 8, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 010: ���ʻ���        */
    { 6, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 011: ϵͳ���ٺ�      */
    { 6, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 012: ����ʱ��        */
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 013: ��������        */
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 014: ����Ч��        */
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 015: ��������        */
    /*****
     * �޸ĵ�16����ȷ��������
     * { 12, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 016: �һ�����        */
    { 2, 0x00}, /* 017  */
    { 4, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 018: �̻�����(MCC)   */
    /*****
     * �޸ĵ�16����ȷ��������
     * { 2, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 019: ����������Ҵ���*/
    { 2, 0x00}, /* 020  */
    { 2, 0x00}, /* 021  */
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 022: ��������뷽ʽ��*/
     /*****
     * �޸ĵ�23����ȷ��������
     * { 2, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 023: �����к�        */
    { 2, 0x00}, /* 024  */
    { 2, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 025: �����������    */
    { 2, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 026: �����PIN��ȡ�� */
    { 2, 0x00}, /* 027  */
     /*****
     * �޸ĵ�28����ȷ��������
     * { 2, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 9, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 028: ������          */
    { 2, 0x00}, /* 029  */
    { 2, 0x00}, /* 030  */
    { 2, 0x00}, /* 031  */
    { 11, LLVAR | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 032: ���������ʶ    */
    { 11, LLVAR | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 033: ���ͻ�����ʶ    */
    { 2, 0x00}, /* 034  */
    { 37, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 035: 2�ŵ�����       */
    { 104, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 036: 3�ŵ�����       */
    { 12, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 037: RRN             */
    { 6, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 038: ��Ȩ��          */
    { 2, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 039: Ӧ����          */
    { 2, 0x00}, /* 040  */
    { 8, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 041: �ն˱��        */
    { 15, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 042: �̻����        */
    { 40, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 043: �̻����ƺ͵�ַ  */
    { 25, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | RIGHT_ALIGN}, /* 044: ������Ӧ����    */
     /*****
     * �޸ĵ�45����ȷ��������
     * { 2, 0x00}
     *Changed By LiuZe 2013-08-01 15:06
     ****/
    { 79, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | RIGHT_ALIGN}, /* 045  */
    { 2, 0x00}, /* 046  */
    { 2, 0x00}, /* 047  */
    { 512, LLLVAR | BINARY | ASCII | UNSYMBOL | ZERO | LEFT_ALIGN}, /* 048: ��������        */
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 049: ���Ҵ���        */
    { 3, 0x00}, /* 050  */
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 051: �ֿ����˻��Ҵ���        */
    { 8, FIXED | BINARY | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 052: PIN             */
    { 16, FIXED | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 053: ��ȫ������Ϣ    */
    { 40, LLLVAR | OTHER | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 054: �˻����        */
    { 255, LLLVAR | BINARY | ASCII | UNSYMBOL | ZERO | LEFT_ALIGN}, /* 055: ic��������      */
    { 2, 0x00}, /* 056  */
    { 10, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 057:������        */
    { 2, 0x00}, /* 058  */
    { 2, 0x00}, /* 059  */
    { 30, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 060: �Զ�����        */
    { 200, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 061: �ֿ��������֤��Ϣ  */
    { 2, 0x00}, /* 062  */
    { 2, 0x00}, /* 063  */
    { 2, 0x00}, /* 064  */
    { 2, 0x00}, /* 065  */
    { 2, 0x00}, /* 066  */
    { 2, 0x00}, /* 067  */
    { 2, 0x00}, /* 068  */
    { 2, 0x00}, /* 069  */
    { 3, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 070: ���������Ϣ��  */
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
    { 42, FIXED | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 090: ԭʼ��Ϣ        */
    { 2, 0x00}, /* 091  */
    { 2, 0x00}, /* 092  */
    { 2, 0x00}, /* 093  */
    { 2, 0x00}, /* 094  */
    { 2, 0x00}, /* 095  */
    { 8, FIXED | BINARY | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 096: ���İ�ȫ��      */
    { 2, 0x00}, /* 097  */
    { 2, 0x00}, /* 098  */
    { 2, 0x00}, /* 099  */
    { 11, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 100: ���ջ�����ʶ    */
    { 2, 0x00}, /* 101  */
    { 30, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 102: ���ջ�����ʶ    */
    { 30, LLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 103: ���ջ�����ʶ    */
    { 512, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 104: �ն�����Ӳ�����кż���������  */
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
    { 100, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 121: CUPS������Ϣ    */
    { 100, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 122: ����������Ϣ  */
    { 100, LLLVAR | OTHER | ASCII | UNSYMBOL | BLANK | LEFT_ALIGN}, /* 123: ������������Ϣ  */
    { 2, 0x00}, /* 124  */
    { 2, 0x00}, /* 125  */
    { 2, 0x00}, /* 126  */
    { 2, 0x00}, /* 127  */
    { 8, FIXED | BINARY | ASCII | UNSYMBOL | ZERO | RIGHT_ALIGN}, /* 128: MAC             */
};
