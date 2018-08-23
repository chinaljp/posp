/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include "t_extiso.h"

IsoTable g_staIsoTable[] = {
        {   4, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 01: ��Ϣ����         */
        {  19, LLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },         /* 02: ���˺�           */
        {   6, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 03: ���״�����       */
        {  12, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 04: ���׽��         */
        {   2, 0x00 },                                              /* 05   */
        {   2, 0x00 },                                              /* 06   */
        {   10, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN  },       /* 07   */
        {   2, 0x00 },                                              /* 08   */
        {   2, 0x00 },                                              /* 09   */
        {   2, 0x00 },                                              /* 10   */
        {   6, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 11: �ն���ˮ��       */
        {   6, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 12: ����ʱ��         */ 
        {   4, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 13: ��������         */
        {   4, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 14: ����Ч��         */
        {   4, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 15: ��������         */
        {   2, 0x00 },                                              /* 16   */
        {   2, 0x00 },                                              /* 17   */
        {   2, 0x00 },                                              /* 18   */
        {   2, 0x00 },                                              /* 19   */
        {   2, 0x00 },                                              /* 20   */
        {   2, 0x00 },                                              /* 21   */
        {   3, FIXED|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },          /* 22: ��������뷽ʽ�� */
        {   3, FIXED|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },          /* 23: IC�����к�       */
        {   2, 0x00 },                                              /* 24   */
        {   2, FIXED|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },          /* 25: �����������     */
        {   2, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 26: �����PIN ��ȡ�� */
        {   2, 0x00 },                                              /* 27   */
        {   2, 0x00 },                                              /* 28   */
        {   2, 0x00 },                                              /* 29   */
        {   2, 0x00 },                                              /* 30   */
        {   2, 0x00 },                                              /* 31   */
        {  11, LLVAR|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },          /* 32: �յ�������ʶ     */
        {   2, 0x00 },                                              /* 33   */
        {   2, 0x00 },                                              /* 34   */
        {  37, LLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },         /* 35: 2 �ŵ�����       */
        { 104, LLLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },        /* 36: 3 �ŵ�����       */
        {  12, FIXED|OTHER|ASCII|UNSYMBOL|ZERO|RIGHT_ALIGN },       /* 37: RRN              */
        {   6, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 38: ��Ȩ��           */
        {   2, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 39: Ӧ����           */
        {   2, 0x00 },                                              /* 40   */
        {   8, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 41: �ն˱��         */
        {  15, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 42: �̻����         */
        {   2, 0x00 },                                              /* 43   */
        {  25, LLVAR|OTHER|ASCII|UNSYMBOL|BLANK|RIGHT_ALIGN },      /* 44: ���ջ���, �յ��ṹ */
        {   2, 0x00 },                                              /* 45   */
        {   2, 0x00 },                                              /* 46   */
        {   2, 0x00 },                                              /* 47   */
        { 322, LLLVAR|OTHER|ASCII|UNSYMBOL|ZERO|LEFT_ALIGN },         /* 48: ��������         */
        {   3, FIXED|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 49: ���׻��Ҵ���     */
        {   2, 0x00 },                                              /* 50   */
        {   2, 0x00 },                                              /* 51   */
        {   8, FIXED|BINARY|ASCII|UNSYMBOL|ZERO|RIGHT_ALIGN },      /* 52: PIN              */
        {  16, FIXED|OTHER|BCD|UNSYMBOL|ZERO|RIGHT_ALIGN },         /* 53: ��ȫ������Ϣ     */
        {  40, LLLVAR|OTHER|ASCII|UNSYMBOL|ZERO|RIGHT_ALIGN },      /* 54: ���ӽ��         */
        { 255, LLLVAR|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },       /* 55: IC��������       */
        {   2, 0x00 },                                              /* 56   */
        {   2, 0x00 },                                              /* 57   */
        { 100, LLLVAR|OTHER|BCD|UNSYMBOL|ZERO|LEFT_ALIGN },         /* 58: ����Ǯ��������Ϣ */
        { 100, LLLVAR|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },      /* 59: �Զ�����         */
        {  16, LLLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },        /* 60: �Զ�����         */
        {  29, LLLVAR|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },        /* 61: ԭʼ��Ϣ��       */
        { 999, LLLVAR|OTHER|BCD|UNSYMBOL|BLANK|LEFT_ALIGN },      /* 62: �Զ�����         */
        { 163, LLLVAR|OTHER|ASCII|UNSYMBOL|BLANK|LEFT_ALIGN },      /* 63: �Զ�����         */
        {   8, FIXED|BINARY|ASCII|UNSYMBOL|ZERO|RIGHT_ALIGN }       /* 64: MAC              */
};