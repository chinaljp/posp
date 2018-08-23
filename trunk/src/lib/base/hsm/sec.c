/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <ctype.h>
#include "t_app_conf.h"
#include "sec.h"
#include "t_tools.h"
#include "t_log.h"

#define MSGHEAD  "00000000"
#define MSGHEADLEN  0
#define KEYLEN   32
/*#define CHARFORMAT		"EBCD" */
#define CHARFORMAT  "ASC"
#define DUMP
#define LINENUM   50
#define COMM_FLAG       1   /* 加密机TCP模式, 0:短链接, 1:长连接 */

int g_iSockFd = -1;

#define     GetFullLen( iLen, iUnitLen ) \
                ( ( ( iLen - 1 ) / iUnitLen + 1 ) * iUnitLen )

#define read_lock(fd, offset, whence, len)  lock_reg(fd, F_SETLK, F_RDLCK, offset, whence, len)
#define readw_lock(fd, offset, whence, len) lock_reg(fd, F_SETLKW, F_RDLCK, offset, whence, len)
#define write_lock(fd, offset, whence, len) lock_reg(fd, F_SETLK, F_WRLCK, offset, whence, len)
#define writew_lock(fd, offset, whence, len) lock_reg(fd, F_SETLKW, F_WRLCK, offset, whence, len)
#define un_lock(fd, offset, whence, len)  lock_reg(fd, F_SETLK, F_UNLCK, offset, whence, len)
static unsigned long SEC_MAIN = 0;
static unsigned long SEC_BACK = 0;
static unsigned short SEC_PORT = 0;
static unsigned short SEC_TIMEOUT = 0;
unsigned char secbuf_in[SECBUF_MAX_SIZE];
unsigned char secbuf_out[SECBUF_MAX_SIZE];
unsigned char ascii_table[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};
/**********************************
 *      码制转换程序模块           *
 *      名  称:  transcode.c       *
 *      日  期: 1993年12月22日     *
 ***********************************/
/*      函  数:  ASCtoEBC(instr,outstr,lenth}   ASCll  code  to  EBCDIC  code  
                 instr:   input  string  
                 outstr:  output string      
                 lenth:  input string lenth */
/*      函  数:  EBCtoASC(instr,outstr,lenth}   EBCDIC  code  to  ASCll  code  
                 instr:   input  string  
                 outstr:  output string      
                 lenth:  input string lenth */
extern ASCtoEBC();
extern EBCtoASC();
extern AtoE();
extern EtoA();
static int highbit, lowbit;

EBCtoASC(instr, outstr, lenth)
char *instr; /*   input string   */
char *outstr; /*   output string   */
int lenth; /*   input string  lenth  */
{
    int invar, outvar; /*   in/out  point   */
    int intval, intval1; /*   get  ascll value    */
    /*
    char s[3];
    char t[3];
     */
    outvar = 0;
    /*   turn  a string   */
    for (invar = 0; invar <= lenth; invar++) {
        intval1 = instr[invar];
        intval = turnint(intval1);
        if (intval == 0x0e) /*     chinese  code   */ {
            invar++;
            intval1 = instr[invar];
            intval = turnint(intval1);
            while ((invar <= lenth) && (intval != 0x0f)) {
                highbit = intval;
                /*s[0]=instr[invar];*/
                invar++;
                intval1 = instr[invar];
                intval = turnint(intval1);
                lowbit = intval;
                /*s[1]=instr[invar];
                s[2]='\0';
                printf("string=%s,highbit=%x,lowbit=%x\n",s,highbit,lowbit);*/
                if (highbit >= 0x48)
                    /*  exchange chinese from as/400 to unix */
                    code_ebctoasc();
                else
                    /*  exchange chinese char from as/400 to unix */
                    db_ebctoasc();
                outstr[outvar] = highbit;
                /*t[0]=outstr[outvar];*/
                outvar++;
                outstr[outvar] = lowbit;
                /*t[1]=outstr[outvar];
                t[2]='\0';
                printf("string=%s,highbit=%x,lowbit=%x\n",t,highbit,lowbit);
                getchar();*/
                outvar++;
                invar++;
                intval1 = instr[invar];
                intval = turnint(intval1);
            }
            /*   insert  two  space    */
            outstr[outvar] = 0x20;
            outvar++;
            outstr[outvar] = 0x20;
            outvar++;
        } else {
            /*   ascll  code   */
            highbit = intval;
            switch (highbit) {
                    /*  char   '[' ']' '|' '!' '^'   */
                case 0x5a:
                    outstr[outvar] = 0x21;
                    break;
                case 0xa0:
                    outstr[outvar] = 0x5b;
                    break;
                case 0xb0:
                    outstr[outvar] = 0x5d;
                    break;
                case 0x4f:
                    outstr[outvar] = 0x7c;
                    break;
                case 0x6a:
                    outstr[outvar] = 0x20;
                    break;
                default:
                    /*  exchange ebcdic code to ascll code */
                    outstr[outvar] = ebcdic_asc(highbit);
                    break;
            }
            outvar++;
        }
    }
    outvar--;
} /*    end  */

ASCtoEBC(instr, outstr, lenth)
char *instr; /*   input string   */
char *outstr; /*    output  string  */
int lenth; /*   input string  lenth  */
{
    int num; /*   space  number   */
    int invar, outvar; /*   in/out  point   */
    int intval, intval1; /*   get ascll value  */
    /*char s[3];*/
    /*   turn  a string   */
    num = 0;
    outvar = 0;
    for (invar = 0; invar <= lenth; invar++) {
        intval1 = instr[invar];
        intval = turnint(intval1);
        if (intval >= 0x80) /*   chinese  code  */ {
            outstr[outvar] = 0x0e; /*    add  0e    */
            outvar++;
            while ((invar <= lenth) && (intval >= 0x80)) {
                highbit = intval;
                /*
                s[0]=instr[invar];
                 */
                invar++;
                intval1 = instr[invar];
                intval = turnint(intval1);
                lowbit = intval;
                /*
                s[1]=instr[invar];
                s[2]='\0';
                printf("string=%s,highbit=%x,lowbit=%x\n",s,highbit,lowbit);
                getchar();
                 */
                if (highbit >= 0xb0)
                    /* exchange  chinese from unix to as/400 */
                    code_asctoebc();
                else
                    /*  exchange  chinese  char from unix to as/400 */
                    db_asctoebc();
                outstr[outvar] = highbit;
                outvar++;
                outstr[outvar] = lowbit;
                outvar++;
                invar++;
                intval1 = instr[invar];
                intval = turnint(intval1);
            }
            outstr[outvar] = 0x0f;
            outvar++;
            invar--;
            num = num + 2; /*   two  space   */
        } else {
            highbit = intval; /*    ascll  code   */
            if (highbit == 0x20) /*    space    */
                if (num != 0) {
                    num--;
                    continue;
                }
            /*   char  '[' ']' '|' '!' '^'     */
            switch (highbit) {
                case 0x21:
                    outstr[outvar] = 0x5a;
                    break;
                case 0x5b:
                    outstr[outvar] = 0xa0;
                    break;
                case 0x5d:
                    outstr[outvar] = 0xb0;
                    break;
                case 0x7c:
                    /*   outstr[outvar]=0x4f;*/
                    outstr[outvar] = 0x6a;
                    break;
                default:
                    /*   exchange  ascll code  from  unix  to  as/400  */
                    outstr[outvar] = asc_ebcdic(highbit);
                    break;
            } /*  switch  */
            outvar++;
        }
    }
    outvar--;
} /*  main  */

AtoE(char *s, int len) {
    char buf[2048];
    ASCtoEBC(s, buf, len);
    memcpy(s, buf, len);
}

EtoA(char *s, int len) {
    char buf[2048];
    EBCtoASC(s, buf, len);
    memcpy(s, buf, len);
}

/*   get  ascll code  value  for a bit  of  input  string  */
turnint(intval1)
int intval1;
{
    int turnvar, turnvar1;
    int intval;
    turnvar = intval1;
    turnvar1 = abs(0xff + turnvar + 1);
    if ((intval1 >= 0) && (intval1 < 0x80))
        intval = turnvar;
    else
        intval = turnvar1;
    return (intval); /*    output:intval   */
}

/* EXCHANGE  ASCll CODE TO EBCDIC */
asc_ebcdic(in_asc)
int in_asc; /*  input is ascll code  */
{
    static int asctoebc[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*   ten   bit   */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 64, 79, 127, 123, 91, 108, 80, 125, 77,
        93, 92, 78, 107, 96, 75, 97, 240, 241, 242,
        243, 244, 245, 246, 247, 248, 249, 122, 94, 76,
        126, 110, 111, 124, 193, 194, 195, 196, 197, 198,
        199, 200, 201, 209, 210, 211, 212, 213, 214, 215,
        216, 217, 226, 227, 228, 229, 230, 231, 232, 233,
        74, 224, 90, 95, 109, 121, 129, 130, 131, 132,
        133, 134, 135, 136, 137, 145, 146, 147, 148, 149,
        150, 151, 152, 153, 162, 163, 164, 165, 166, 167,
        168, 169, 192, 106, 208, 161, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0};
    int out_ebc;
    out_ebc = asctoebc[in_asc];
    return (out_ebc);
}

/*   exchange  EBCDIC  code  into  ASCll    code   */
ebcdic_asc(in_ebc) /*    input  ebcdic  code    */
int in_ebc;
{
    static int ebctoasc[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 32, 0, 0, 0, 0, 0, /*    ten  bit  */
        0, 0, 0, 0, 91, 46, 60, 40, 43, 33,
        38, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        93, 36, 42, 41, 59, 94, 45, 47, 0, 0,
        0, 0, 0, 0, 0, 0, 124, 44, 37, 95,
        62, 63, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 96, 58, 35, 64, 39, 61, 34, 0, 97,
        98, 99, 100, 101, 102, 103, 104, 105, 0, 0,
        0, 0, 0, 0, 0, 106, 107, 108, 109, 110,
        111, 112, 113, 114, 0, 0, 0, 0, 0, 0,
        0, 126, 115, 116, 117, 118, 119, 120, 121, 122,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 123, 65, 66, 67, 68, 69, 70, 71,
        72, 73, 0, 0, 0, 0, 0, 0, 125, 74,
        75, 76, 77, 78, 79, 80, 81, 82, 0, 0,
        0, 0, 0, 0, 92, 0, 83, 84, 85, 86,
        87, 88, 89, 90, 0, 0, 0, 0, 0, 0,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
        0, 0, 0, 0, 0, 0};
    int out_asc;
    out_asc = ebctoasc[in_ebc];
    return (out_asc);
}
/* EXCHANGE  CHINESE  CHARACTER  ASCll_EBCDIC  */
/*   unix  chinese char is changed as/400 chinese char   */
int asctoebc [260] [2] = {0x40, 0x40, 0x43, 0x44, 0x43, 0x41, 0x43, 0x45, 0x45,
    0x45, 0x45, 0x46, 0x44, 0x60, 0x44, 0x5b, 0x44, 0x5d,
    0x44, 0x4a, 0x43, 0xa1, 0x44, 0x7c, 0x44, 0x7f, 0x44,
    0x61, 0x44, 0x71, 0x44, 0x62, 0x44, 0x72, 0x44, 0x63,
    0x44, 0x73, 0x44, 0x64, 0x44, 0x74, 0x44, 0x65, 0x44,
    0x75, 0x43, 0x42, 0x43, 0x43, 0x44, 0x42, 0x44, 0x43,
    0x45, 0x5b, 0x45, 0x5c, 0x44, 0x66, 0x44, 0x76, 0x44,
    0x4b, 0x44, 0x7a, 0x44, 0x7b, 0x45, 0x62, 0x45, 0x63,
    0x45, 0x64, 0x45, 0x65, 0x45, 0x66, 0x45, 0x67, 0x45,
    0x68, 0x45, 0x69, 0x45, 0x6a, 0x45, 0x6b, 0x45, 0x6c,
    0x45, 0x6d, 0x45, 0x6e, 0x45, 0x6f, 0x45, 0x70, 0x45,
    0x71, 0x45, 0x72, 0x45, 0x73, 0x45, 0x74, 0x45, 0x75,
    0x45, 0x76, 0x45, 0x77, 0x44, 0x4c, 0x45, 0x79, 0x45,
    0x7a, 0x44, 0x67, 0x44, 0x77, 0x44, 0x4d, 0x44, 0x78,
    0x44, 0x68, 0x44, 0x69, 0x44, 0x79, 0x44, 0xed, 0x44,
    0xee, 0x44, 0xef, 0x44, 0x4e, 0x42, 0xe0, 0x45, 0x88,
    0x43, 0x4a, 0x42, 0x4a, 0x45, 0x8b, 0x44, 0x6a, 0x44,
    0x6e, 0x44, 0xe5, 0x44, 0xe6, 0x44, 0xe0, 0x44, 0xe1,
    0x44, 0xe4, 0x44, 0xe7, 0x44, 0xe8, 0x44, 0xe9, 0x44,
    0xea, 0x44, 0xe2, 0x44, 0xe3, 0x44, 0x6b, 0x44, 0xf0,
    0x44, 0xf1, 0x44, 0xf2, 0x44, 0xf3, 0x44, 0x7d, 0x45,
    0xb1, 0x45, 0xb2, 0x45, 0xb3, 0x45, 0xb4, 0x45, 0xb5,
    0x45, 0xb6, 0x45, 0xb7, 0x45, 0xb8, 0x45, 0xb9, 0x45,
    0xba, 0x45, 0xbb, 0x45, 0xbc, 0x45, 0xbd, 0x45, 0xbe,
    0x45, 0xbf, 0x45, 0xc0, 0x45, 0xc1, 0x45, 0xc2, 0x45,
    0xc3, 0x45, 0xc4, 0x45, 0xc5, 0x45, 0xc6, 0x45, 0xc7,
    0x45, 0xc8, 0x45, 0xc9, 0x45, 0xca, 0x45, 0xcb, 0x45,
    0xcc, 0x45, 0xcd, 0x45, 0xce, 0x45, 0xcf, 0x45, 0xd0,
    0x45, 0xd1, 0x45, 0xd2, 0x45, 0xd3, 0x45, 0xd4, 0x45,
    0xd5, 0x45, 0xd6, 0x45, 0xd7, 0x45, 0xd8, 0x45, 0xe1,
    0x45, 0xe2, 0x45, 0xe3, 0x45, 0xe4, 0x45, 0xe5, 0x45,
    0xe6, 0x45, 0xe7, 0x45, 0xe8, 0x45, 0xe9, 0x45, 0xea,
    0x45, 0xf1, 0x45, 0xf2, 0x45, 0xf3, 0x45, 0xf4, 0x45,
    0xf5, 0x45, 0xf6, 0x45, 0xf7, 0x45, 0xf8, 0x45, 0xf9,
    0x45, 0xfa, 0x41, 0xf1, 0x41, 0xf2, 0x41, 0xf3, 0x41,
    0xf4, 0x41, 0xf5, 0x41, 0xf6, 0x41, 0xf7, 0x41, 0xf8,
    0x41, 0xf9, 0x41, 0xfa, 0x41, 0xfb, 0x41, 0xfc, 0x42,
    0x5a, 0x42, 0x7f, 0x42, 0x7b, 0x42, 0x5b, 0x42, 0x6c,
    0x42, 0x50, 0x44, 0x50, 0x42, 0x4d, 0x42, 0x5d, 0x42,
    0x5c, 0x42, 0x4e, 0x42, 0x6b, 0x42, 0x60, 0x42, 0x4b,
    0x42, 0x61, 0x42, 0xf0, 0x42, 0xf1, 0x42, 0xf2, 0x42,
    0xf3, 0x42, 0xf4, 0x42, 0xf5, 0x42, 0xf6, 0x42, 0xf7,
    0x42, 0xf8, 0x42, 0xf9, 0x42, 0x7a, 0x42, 0x5e, 0x42,
    0x4c, 0x42, 0x7e, 0x42, 0x6e, 0x42, 0x6f, 0x42, 0x7c,
    0x42, 0xc1, 0x42, 0xc2, 0x42, 0xc3, 0x42, 0xc4, 0x42,
    0xc5, 0x42, 0xc6, 0x42, 0xc7, 0x42, 0xc8, 0x42, 0xc9,
    0x42, 0xd1, 0x42, 0xd2, 0x42, 0xd3, 0x42, 0xd4, 0x42,
    0xd5, 0x42, 0xd6, 0x42, 0xd7, 0x42, 0xd8, 0x42, 0xd9,
    0x42, 0xe2, 0x42, 0xe3, 0x42, 0xe4, 0x42, 0xe5, 0x42,
    0xe6, 0x42, 0xe7, 0x42, 0xe8, 0x42, 0xe9, 0x44, 0x44,
    0x43, 0xe0, 0x44, 0x45, 0x44, 0x70, 0x42, 0x6d, 0x42,
    0x79, 0x42, 0x81, 0x42, 0x82, 0x42, 0x83, 0x42, 0x84,
    0x42, 0x85, 0x42, 0x86, 0x42, 0x87, 0x42, 0x88, 0x42,
    0x89, 0x42, 0x91, 0x42, 0x92, 0x42, 0x93, 0x42, 0x94,
    0x42, 0x95, 0x42, 0x96, 0x42, 0x97, 0x42, 0x98, 0x42,
    0x99, 0x42, 0xa2, 0x42, 0xa3, 0x42, 0xa4, 0x42, 0xa5,
    0x42, 0xa6, 0x42, 0xa7, 0x42, 0xa8, 0x42, 0xa9, 0x42,
    0xc0, 0x42, 0x4f, 0x42, 0xd0, 0x42, 0xa1};
int ebctoasc [260] [2] = {0xa1, 0xa1, 0xa1, 0xa2, 0xa1, 0xa3, 0xa1, 0xa4, 0xa1,
    0xa5, 0xa1, 0xa6, 0xa1, 0xa7, 0xa1, 0xa8, 0xa1, 0xa9,
    0xa1, 0xaa, 0xa1, 0xab, 0xa1, 0xac, 0xa1, 0xad, 0xa1,
    0xae, 0xa1, 0xaf, 0xa1, 0xb0, 0xa1, 0xb1, 0xa1, 0xb2,
    0xa1, 0xb3, 0xa1, 0xb4, 0xa1, 0xb5, 0xa1, 0xb6, 0xa1,
    0xb7, 0xa1, 0xb8, 0xa1, 0xb9, 0xa1, 0xba, 0xa1, 0xbb,
    0xa1, 0xbc, 0xa1, 0xbd, 0xa1, 0xbe, 0xa1, 0xbf, 0xa1,
    0xc0, 0xa1, 0xc1, 0xa1, 0xc2, 0xa1, 0xc3, 0xa1, 0xc4,
    0xa1, 0xc5, 0xa1, 0xc6, 0xa1, 0xc7, 0xa1, 0xc8, 0xa1,
    0xc9, 0xa1, 0xca, 0xa1, 0xcb, 0xa1, 0xcc, 0xa1, 0xcd,
    0xa1, 0xce, 0xa1, 0xcf, 0xa1, 0xd0, 0xa1, 0xd1, 0xa1,
    0xd2, 0xa1, 0xd3, 0xa1, 0xd4, 0xa1, 0xd5, 0xa1, 0xd6,
    0xa1, 0xd7, 0xa1, 0xd8, 0xa1, 0xd9, 0xa1, 0xda, 0xa1,
    0xdb, 0xa1, 0xdc, 0xa1, 0xdd, 0xa1, 0xde, 0xa1, 0xdf,
    0xa1, 0xe0, 0xa1, 0xe1, 0xa1, 0xe2, 0xa1, 0xe3, 0xa1,
    0xe4, 0xa1, 0xe5, 0xa1, 0xe6, 0xa1, 0xe7, 0xa1, 0xe8,
    0xa1, 0xe9, 0xa1, 0xea, 0xa1, 0xeb, 0xa1, 0xec, 0xa1,
    0xed, 0xa1, 0xee, 0xa1, 0xef, 0xa1, 0xf0, 0xa1, 0xf1,
    0xa1, 0xf2, 0xa1, 0xf3, 0xa1, 0xf4, 0xa1, 0xf5, 0xa1,
    0xf6, 0xa1, 0xf7, 0xa1, 0xf8, 0xa1, 0xf9, 0xa1, 0xfa,
    0xa1, 0xfb, 0xa1, 0xfc, 0xa1, 0xfd, 0xa1, 0xfe, 0xa2,
    0xb1, 0xa2, 0xb2, 0xa2, 0xb3, 0xa2, 0xb4, 0xa2, 0xb5,
    0xa2, 0xb6, 0xa2, 0xb7, 0xa2, 0xb8, 0xa2, 0xb9, 0xa2,
    0xba, 0xa2, 0xbb, 0xa2, 0xbc, 0xa2, 0xbd, 0xa2, 0xbe,
    0xa2, 0xbf, 0xa2, 0xc0, 0xa2, 0xc1, 0xa2, 0xc2, 0xa2,
    0xc3, 0xa2, 0xc4, 0xa2, 0xc5, 0xa2, 0xc6, 0xa2, 0xc7,
    0xa2, 0xc8, 0xa2, 0xc9, 0xa2, 0xca, 0xa2, 0xcb, 0xa2,
    0xcc, 0xa2, 0xcd, 0xa2, 0xce, 0xa2, 0xcf, 0xa2, 0xd0,
    0xa2, 0xd1, 0xa2, 0xd2, 0xa2, 0xd3, 0xa2, 0xd4, 0xa2,
    0xd5, 0xa2, 0xd6, 0xa2, 0xd7, 0xa2, 0xd8, 0xa2, 0xd9,
    0xa2, 0xda, 0xa2, 0xdb, 0xa2, 0xdc, 0xa2, 0xdd, 0xa2,
    0xde, 0xa2, 0xdf, 0xa2, 0xe0, 0xa2, 0xe1, 0xa2, 0xe2,
    0xa2, 0xe5, 0xa2, 0xe6, 0xa2, 0xe7, 0xa2, 0xe8, 0xa2,
    0xe9, 0xa2, 0xea, 0xa2, 0xeb, 0xa2, 0xec, 0xa2, 0xed,
    0xa2, 0xee, 0xa2, 0xf1, 0xa2, 0xf2, 0xa2, 0xf3, 0xa2,
    0xf4, 0xa2, 0xf5, 0xa2, 0xf6, 0xa2, 0xf7, 0xa2, 0xf8,
    0xa2, 0xf9, 0xa2, 0xfa, 0xa2, 0xfb, 0xa2, 0xfc, 0xa3,
    0xa1, 0xa3, 0xa2, 0xa3, 0xa3, 0xa3, 0xa4, 0xa3, 0xa5,
    0xa3, 0xa6, 0xa3, 0xa7, 0xa3, 0xa8, 0xa3, 0xa9, 0xa3,
    0xaa, 0xa3, 0xab, 0xa3, 0xac, 0xa3, 0xad, 0xa3, 0xae,
    0xa3, 0xaf, 0xa3, 0xb0, 0xa3, 0xb1, 0xa3, 0xb2, 0xa3,
    0xb3, 0xa3, 0xb4, 0xa3, 0xb5, 0xa3, 0xb6, 0xa3, 0xb7,
    0xa3, 0xb8, 0xa3, 0xb9, 0xa3, 0xba, 0xa3, 0xbb, 0xa3,
    0xbc, 0xa3, 0xbd, 0xa3, 0xbe, 0xa3, 0xbf, 0xa3, 0xc0,
    0xa3, 0xc1, 0xa3, 0xc2, 0xa3, 0xc3, 0xa3, 0xc4, 0xa3,
    0xc5, 0xa3, 0xc6, 0xa3, 0xc7, 0xa3, 0xc8, 0xa3, 0xc9,
    0xa3, 0xca, 0xa3, 0xcb, 0xa3, 0xcc, 0xa3, 0xcd, 0xa3,
    0xce, 0xa3, 0xcf, 0xa3, 0xd0, 0xa3, 0xd1, 0xa3, 0xd2,
    0xa3, 0xd3, 0xa3, 0xd4, 0xa3, 0xd5, 0xa3, 0xd6, 0xa3,
    0xd7, 0xa3, 0xd8, 0xa3, 0xd9, 0xa3, 0xda, 0xa3, 0xdb,
    0xa3, 0xdc, 0xa3, 0xdd, 0xa3, 0xde, 0xa3, 0xdf, 0xa3,
    0xe0,
    0xa3, 0xe1, 0xa3, 0xe2, 0xa3, 0xe3, 0xa3, 0xe4, 0xa3,
    0xe5, 0xa3, 0xe6, 0xa3, 0xe7, 0xa3, 0xe8, 0xa3, 0xe9,
    0xa3, 0xea, 0xa3, 0xeb, 0xa3, 0xec, 0xa3, 0xed, 0xa3,
    0xee, 0xa3, 0xef, 0xa3, 0xf0, 0xa3, 0xf1, 0xa3, 0xf2,
    0xa3, 0xf3, 0xa3, 0xf4, 0xa3, 0xf5, 0xa3, 0xf6, 0xa3,
    0xf7, 0xa3, 0xf8, 0xa3, 0xf9, 0xa3, 0xfa, 0xa3, 0xfb,
    0xa3, 0xfc, 0xa3, 0xfd, 0xa3, 0xfe};

/*   unix  chinese char is changed as/400 chinese char   */
db_asctoebc() {
    int arr, turnvar, turnvar1;
    if (highbit == 0xa9)
        /* trans MAKE TABLE SIGN (09 AREA) */
        highbit = highbit - 0x63;
    else {
        for (arr = 0; arr <= 259; ++arr) {
            turnvar = ebctoasc[arr][0];
            turnvar1 = ebctoasc[arr][1];
            if (turnvar == highbit && turnvar1 == lowbit)
                break;
        }
        highbit = asctoebc[arr] [0];
        lowbit = asctoebc[arr] [1];
    }
    return;
}

/*   as/400  chinese char is changed unix chinese char   */
db_ebctoasc() {
    int arr, turnvar, turnvar1;
    if (highbit == 0x46)
        /* trans MAKE TABLE SIGN (09 AREA) */
        highbit = highbit + 0x63;
    else {
        for (arr = 0; arr <= 259; ++arr) {
            turnvar = asctoebc[arr][0];
            turnvar1 = asctoebc[arr][1];
            if (turnvar == highbit && turnvar1 == lowbit)
                break;
        }
        highbit = ebctoasc[arr] [0];
        lowbit = ebctoasc[arr] [1];
    }
    return;
}
/*    TURN CHINESE CODE FROM UNIX TO AS/400  */

/* extern int highbit,lowbit;    chinese  high and lower  bit  */
code_asctoebc() {
    int highbit1, lowbit1, outhigh, outlow;
    highbit1 = highbit - 0xb0;
    /*  turn   high  bit   */
    if (highbit1 % 2 != 0)
        highbit1++;
    highbit1 /= 2;
    outhigh = highbit1 + 0x48;
    /*  turn  lower  bit   */
    if (highbit % 2 != 0) {
        if (lowbit < 0xe0)
            outlow = lowbit - 0x60;
        else
            outlow = lowbit - 0x5f;
    } else
        outlow = lowbit - 1;
    /*  send  output  */
    highbit = outhigh;
    lowbit = outlow;
    return;
}
/* TURN CHINESE CODE FROM AS/400  TO  UNIX  */

/*  extern int highbit,lowbit;*/
code_ebctoasc() {
    int highbit1, lowbit1, outhigh, outlow;
    if (highbit != 0x48) {
        highbit1 = highbit - 0x48;
        highbit1 *= 2;
        if (lowbit <= 0x9f)
            highbit1--;
        outhigh = highbit1 + 0xb0;
        if (lowbit <= 0x9f) {
            if (lowbit < 0x81)
                outlow = lowbit + 0x60;
            else
                outlow = lowbit + 0x5f;
        } else
            outlow = lowbit + 1;
    } else {
        outhigh = 0xb0;
        outlow = lowbit + 1;
    };
    /*  send  output  */
    highbit = outhigh;
    lowbit = outlow;
    return;
}

void fl_asc_to_bcd(unsigned char *bcd_buf, unsigned char *ascii_buf, int conv_len, unsigned char type) {
    int cnt;
    char ch, ch1;
    if (conv_len & 0x01 && type) ch1 = 0;
    else ch1 = 0x55;
    for (cnt = 0; cnt < conv_len; ascii_buf++, cnt++) {
        if (*ascii_buf >= 'a') ch = *ascii_buf - 'a' + 10;
        else if (*ascii_buf >= 'A') ch = *ascii_buf - 'A' + 10;
        else if (*ascii_buf >= '0') ch = *ascii_buf - '0';
        else ch = 0;
        if (ch1 == 0x55) ch1 = ch;
        else {
            *bcd_buf++ = ch1 << 4 | ch;
            ch1 = 0x55;
        }
    }
    if (ch1 != 0x55) *bcd_buf = ch1 << 4;
}

void fl_bcd_to_asc(unsigned char *ascii_buf, unsigned char *bcd_buf, int conv_len, unsigned char type) {
    int cnt;

    if (conv_len & 0x01 && type) {
        cnt = 1;
        conv_len++;
    } else cnt = 0;
    for (; cnt < conv_len; cnt++, ascii_buf++) {
        *ascii_buf = ((cnt & 0x01) ? (*bcd_buf++&0x0f) : (*bcd_buf >> 4));
        *ascii_buf += ((*ascii_buf > 9) ? ('A' - 10) : '0');
    }
}

void Dec2Hex(unsigned char *s, int n) {
    int i, j, len;
    char a[40], tmp[17];
    unsigned char ss[17];
    j = 3;
    memset(a, 0, 40);
    do {
        a[j] = n % 16;
        n = n / 16;
        j--;
    } while (n > 0);
    for (i = 0; i <= 3; i++)
        sprintf(tmp + i, "%x", a[i]);
    fl_asc_to_bcd(ss, (unsigned char *) tmp, 16, 0);
    memcpy(s, ss, 2);
}

void hex2asc(unsigned char dat, unsigned char **new) {
    **new = ascii_table[dat >> 4];
    *new += 1;
    **new = ascii_table[dat & 0x0f];
    *new += 1;
}

int asc2hex(unsigned char dat, unsigned char *new) {
    if ((dat >= '0') && (dat <= '9'))
        *new = dat - '0';
    else if ((dat >= 'A') && (dat <= 'F'))
        *new = dat - 'A' + 10;
    else if ((dat >= 'a') && (dat <= 'f'))
        *new = dat - 'a' + 10;
    else
        return -1;
    return 0;
}

void hex2str(unsigned char *read, unsigned char **write, int len) {
    while (len--)
        hex2asc(*read++, write);
}

int str2hex(unsigned char *read, unsigned char *write, int len) {
    unsigned char dat;
    while (len--) {
        if (asc2hex(*read++, &dat))
            return -1;
        *write = *write << 4 | dat;
        if (!(len & 1)) {
            ++write;
        }
    }
    return 0;
}

int GetLog() {
    struct tm *pNow;
    char sLogFile[1000];
    long lTime;


    memset(sLogFile, 0, sizeof ( sLogFile));
    time(&lTime);
    pNow = localtime(&lTime);

    //sprintf( sLogFile ,"%s.%02d%02d",HSM_LOG,pNow->tm_mon+1,pNow->tm_mday );	
    sprintf(sLogFile, "/home/acqsys/log/hsm/hsm.log");
    pLog = fopen(sLogFile, "a");
    if (pLog == (FILE *) NULL) {
        tLog(DEBUG, "get hsm log err");
        return -1;
    }
    tLog(DEBUG, "get hsm log ok [%s]", sLogFile);
    fprintf(pLog, "%04d-%02d-%02d  %02d:%02d:%02d\n", pNow->tm_year + 1900, pNow->tm_mon + 1, pNow->tm_mday, pNow->tm_hour, pNow->tm_min, pNow->tm_sec);
    return 0;

}

void SecLog(char *prompt, char *data, int len) {
    int i;
    FILE *fp;
    char sLogFile[1000], sTime[10] = {0};
    struct tm *pNow;
    long lTime;
    struct timeval tv;


    time(&lTime);
    pNow = localtime(&lTime);
    memset(sLogFile, 0, sizeof ( sLogFile));
    sprintf(sLogFile, "%s/log/%s.%02d%02d", getenv("HOME"), HSM_LOG, pNow->tm_mon + 1, pNow->tm_mday);
    fp = fopen(sLogFile, "a");
    if (fp == (FILE *) NULL) {
        tLog(DEBUG, "get hsm log err");
        return;
    }

    sprintf(sTime, "%02d%02d%02d",
            pNow->tm_hour, pNow->tm_min, pNow->tm_sec);
    gettimeofday(&tv, NULL);
    fprintf(fp, "\n\n*********************************Time[%s:%03d]\n", sTime, (int) (tv.tv_usec / 1000));
    fprintf(fp, "[%s] [length = %d]\n", prompt, len);
    fprintf(fp, "str [%s]\n", data);

    for (i = 0; i < len; i++)
        fprintf(fp, "%02x ", data[i] & 0xff);
    fprintf(fp, "\n");
    fclose(fp);
}

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len) {
    struct flock lock;
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
    return (fcntl(fd, cmd, &lock));
}

/*
int modify_file() {
    int fd, len, i;
    char buf[2000];
    fd = open(HSM_CONF, O_RDWR);
    if (fd < 0)
        return -1;
    if (writew_lock(fd, 0, SEEK_SET, 0) < 0) {
        return -1;
    }
    len = read(fd, buf, 1024);
    for (i = 0; i < len; i++) {
        if (buf[i] == 'M') {
            if (buf[i + 1] == 'A' && buf[i + 2] == 'I' && buf[i + 3] == 'N') {
                memcpy(&buf[i], "BACK", 4);
                i += 4;
            }
        } else if (buf[i] == 'B') {
            if (buf[i + 1] == 'A' && buf[i + 2] == 'C' && buf[i + 3] == 'K') {
                memcpy(&buf[i], "MAIN", 4);
                i += 4;
            }
        }
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        return -1;
    }
    if (write(fd, buf, len) != len) {
        return -1;
    }
    fsync(fd);
    if (un_lock(fd, 0, SEEK_SET, 0) < 0) {
        return -1;
    }
    close(fd);
    return 0;
}
 */
int RtcsInitTcp(int TcpFd) {
    int optlen;
    int on = 1;
    struct linger LingerVar;
    LingerVar.l_onoff = 1;
    LingerVar.l_linger = 0;
    optlen = sizeof (LingerVar);
    if (setsockopt(TcpFd, SOL_SOCKET, SO_LINGER, (char *) &LingerVar, optlen) != 0) {
        return (-1);
    }
    if (setsockopt(TcpFd, SOL_SOCKET, SO_OOBINLINE, (char *) &on, sizeof (on)) != 0) {
        return (-1);
    }
    return (0);
}

int tcprecv(int sockfd, unsigned char * buffer) {
    int trylen, trynum, rlen;
    trylen = trynum = rlen = 0;
    while (1) {
        rlen = recv(sockfd, buffer + trylen, 1460, 0);
        if ((rlen <= 0) && (trynum == 0)) {
            return -1;
        }
        if (rlen == 1460) {
            trylen += rlen;
            rlen = 0;
            trynum = 1;
            sleep(1);
        } else {
            rlen += trylen;
            return rlen;
        }
    }
}
extern HsmConf g_stHsmConf;

int sec_init(void) {
    FILE *fd;
    char *str;
    char buf[256];
    char str_main[] = "MAIN";
    char str_back[] = "BACK";
    char str_port[] = "PORT";
    char str_timeout[] = "TIMEOUT";


    if (tLoadHsmConf() < 0) {
        return -1;
    }

    SEC_MAIN = inet_addr(g_stHsmConf.sHsmIP1);
    SEC_BACK = inet_addr(g_stHsmConf.sHsmIP2);
    SEC_PORT = atoi(g_stHsmConf.sHsmPort1);
    SEC_TIMEOUT = 14;
    /*
    fd = fopen(HSM_CONF, "r");
    if (!fd) {
        return -1;
    }
    while (fgets(&buf[0], 256, fd)) {
        if (buf[0] == '#')
            continue;
        if (!strncmp(buf, str_main, strlen((char *) str_main))) {
            str = strchr(buf, '=');
            if (str) {
                ++str;
                SEC_MAIN = inet_addr(str);
            }
        } else if (!strncmp(buf, str_back, strlen((char *) str_back))) {
            str = strchr(buf, '=');
            if (str) {
                ++str;
                SEC_BACK = inet_addr(str);
            }
        } else if (!strncmp(buf, str_port, strlen((char *) str_port))) {
            str = strchr(buf, '=');
            if (str) {
                ++str;
                SEC_PORT = atoi(str);
            }
        } else if (!strncmp(buf, str_timeout, strlen((char *) str_timeout))) {
            str = strchr(buf, '=');
            if (str) {
                ++str;
                SEC_TIMEOUT = atoi(str);
            }
        }
    }
     
    fclose(fd);*/
    if (SEC_PORT == 0 || SEC_MAIN == 0)
        return -1;
    return 0;
}

void sec_disc(int sockfd) {
    close(sockfd);
    sockfd = -1;
}

int sec_conn(int addr) {
    struct sockaddr_in dest_addr;
    int sockfd = -1;
    int retval;
    fd_set allset;
    struct timeval tv;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }
    memset((char *) &dest_addr, 0, sizeof (struct sockaddr_in));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SEC_PORT);
    dest_addr.sin_addr.s_addr = addr;
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
        return -1;
    retval = connect(sockfd, (struct sockaddr *) &dest_addr, sizeof (struct sockaddr));
    if (retval < 0) {
        FD_ZERO(&allset);
        FD_SET(sockfd, &allset);
        tv.tv_sec = SEC_TIMEOUT;
        tv.tv_usec = 0;
        if (select(sockfd + 1, NULL, &allset, NULL, &tv) <= 0) {
            return -1;
        }
        RtcsInitTcp(sockfd);
        return sockfd;
    }
    RtcsInitTcp(sockfd);
    return sockfd;
}

int sec_connect_back() {
    int sockfd;
    if (SEC_BACK == 0 || SEC_PORT == 0)
        if (sec_init() != 0) {
            return -1;
        }
    if (SEC_BACK == 0)
        return -1;
    if ((sockfd = sec_conn(SEC_BACK)) < 0)
        return -1;
    return sockfd;
}

int sec_connect_main() {
    int sockfd;
    if (SEC_MAIN == 0 || SEC_PORT == 0)
        if (sec_init() != 0) {
            return -1;
        }
    if (SEC_MAIN == 0)
        return -1;
    if ((sockfd = sec_conn(SEC_MAIN)) < 0)
        return -1;
    return sockfd;
}

int sec_connect(void) {
    int sockfd;
    if (SEC_PORT == 0)
        if (sec_init() != 0) {
            return -1;
        }
    if ((sockfd = sec_conn(SEC_MAIN)) < 0) {
        if ((sockfd = sec_conn(SEC_BACK)) < 0) {
            return -1;
        }
        SEC_PORT = 0;
        //modify_file();
    }
    return sockfd;
}

int sec_send(int sockfd, unsigned char *buf, int len) {
    int count;
    fd_set allset;
    struct timeval tv;
    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);
    tv.tv_sec = SEC_TIMEOUT;
    tv.tv_usec = 0;
    if (select(sockfd + 1, NULL, &allset, NULL, &tv) <= 0) {
        return -1;
    }
    if ((count = send(sockfd, buf, len, 0)) <= 0) {
        perror("sec_send: send in sec_send()");
        return -1;
    }
    return count;
}

int sec_recv(int sockfd, unsigned char *buf, int len) {
    int rlen;
    fd_set allset;
    struct timeval tv;
    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);
    tv.tv_sec = SEC_TIMEOUT;
    tv.tv_usec = 0;
    if (select(sockfd + 1, &allset, NULL, NULL, &tv) <= 0) {
        return -1;
    }
    rlen = tcprecv(sockfd, buf);
    if (rlen <= 0) {
        perror("sec_recv: recv in sec_recv()");
        return -1;
    }
    return rlen;
}

/* only for B0 extend command */
int HSM_LINK(unsigned char *in, int wlen, unsigned char *out) {
    int retval = -1;
    char sBuf[1024] = {0};
    int i;

    /* 长连接模式只建连一次 */
    if ((0 == COMM_FLAG) || (g_iSockFd < 0)) {
        g_iSockFd = sec_connect();
        if (g_iSockFd < 0)
            return -ESEC_CONNECT;
    }

#ifdef DUMP
    SecLog("send data to hsm ascii", in + 2, wlen - 2);
#endif
    if (strncmp(CHARFORMAT, "EBCD", 4) == 0) {
        memset(sBuf, 0, sizeof ( sBuf));
        memcpy(sBuf, in + 2, wlen - 2);
        AtoE(sBuf, wlen - 2);

        for (i = 0; i < wlen - 2; i++)
            sBuf[i] = sBuf[i] & 0xff;

        memcpy(in + 2, sBuf, wlen - 2);
#ifdef DUMP
        SecLog("send data to hsm ebcd", in + 2, wlen - 2);
#endif
    }
    retval = sec_send(g_iSockFd, in, wlen);
    if (retval < 0) {
        sec_disc(g_iSockFd);
        return -ESEC_SEND;
    }
    retval = sec_recv(g_iSockFd, out, SECBUF_MAX_SIZE);
    if (retval < 0) {
        sec_disc(g_iSockFd);
        return -ESEC_RECV;
    }
    if (strncmp(CHARFORMAT, "EBCD", 4) == 0) {
        memset(sBuf, 0, sizeof ( sBuf));
        memcpy(sBuf, out + 2, retval - 2);
        EtoA(sBuf, retval - 2);
        memcpy(out + 2, sBuf, retval - 2);
#ifdef DUMP
        SecLog("recieve data from hsm ebcd", out + 2, retval - 2);
#endif
    }
#ifdef DUMP
    SecLog("recieve data from hsm", out + 2, retval - 2);
#endif

    /* 长连接不拆连 */
    /* dis-connect */
    if (0 == COMM_FLAG) {
        sec_disc(g_iSockFd);
    }

    /* error message */
    if (memcmp(out + MSGHEADLEN + 2 + 2, "00", 2)) {
        return -1;
    }
    return 0;
}

/* only for B0 extend command */
void HSM_UNLINK(void) {
    if (g_iSockFd > 0) {
        sec_disc(g_iSockFd);
    }
}

int HSM_GetHsmStat(char *sStat) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    strncpy(sSendBuf + MSGHEADLEN, "NO00", 4);
    iLen = MSGHEADLEN + 4;
    sSendBuf[ iLen - 1 ] = 0x19;
    iLen = iLen + 1;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sStat, secbuf_out + 2 + MSGHEADLEN + 2, 2);
    return 0;
}

/*********************************** APP *****************************/

int GetMac8(unsigned char *sMacData, int iLen, char *sMac8) {
    int iOff;
    int i, j, iPos;
    int iDataLen;
    char sMacTmp[100];
    char sMacAsc[100];

    memset(sMacTmp, 0, sizeof ( sMacTmp));
    memset(sMacAsc, 0, sizeof ( sMacAsc));

    j = 8 - iLen % 8;

    memcpy(sMacData + iLen, sMacAsc, j);
    iDataLen = iLen + j;

    for (iPos = 0; iPos < iDataLen; iPos += iOff) {

        iOff = ((iDataLen - iPos) >= 8) ? 8 : iDataLen - iPos;

        for (i = 0; i < iOff; i++)
            sMacTmp[i] ^= sMacData[ i + iPos ];


    }

    memcpy(sMac8, sMacTmp, 8);
    //	memset( sMacAsc,0,sizeof( sMacAsc ));

    //fl_bcd_to_asc( sMacAsc,sMacTmp,16,0);

    //memcpy( sMac8,sMacAsc,16 );

    return 0;

}

/************************
 *  生成ZPK              *
 *  入参1: LMK加密下的zmk*
 *  出参1: ZMK加密下的zpk*
 *  出参2: LMK加密下的zpk*
 *  出参3: zpk校验值16位 *
 ************************/
int HSM_GenZpk(char *sZmkuLmk, char *sZpkuZmk, char *sZpkuLmk, char *sCheckValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];

    if (strlen(sZmkuLmk) > 16) {
        return (HSM_GenZpk2(sZmkuLmk, sZpkuZmk, sZpkuLmk, sCheckValue));
    }

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "IA", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sZmkuLmk, 16);
    iLen = iLen + 16;

    memcpy(sSendBuf + iLen, ";ZZ0", 4);
    iLen = iLen + 4;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;

    memcpy(sZpkuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    memcpy(sZpkuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16, 16);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16 * 2, 16);
    return 0;
}
/*
int	HSM_GenZpk( char *sZmkuLmk, char *sZpkuZmk, char *sZpkuLmk ,char *sCheckValue )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "A01001Z", 7);
iLen = iLen + 7;
memcpy( sSendBuf + iLen, sZmkuLmk, 16 );
iLen = iLen + 16;
memcpy( sSendBuf + iLen ,"Z", 1 );
iLen = iLen + 1;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sZpkuLmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 ,16 );
memcpy( sZpkuZmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16 ,16 );
memcpy( sCheckValue,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16 * 2, 16 );
return 0;
}
 */
/*
int	HSM_GenZpk( char *sZmkuLmk,char *sZpkuZmk, char *sZpkuLmk,char *sCheckValue )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "IA", 2);
iLen = iLen + 2;
memcpy( sSendBuf + iLen, sZmkuLmk, KEYLEN );
iLen = iLen + KEYLEN;
memcpy( sSendBuf + iLen, ";UU0",4 );
iLen = iLen + 4;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sZpkuZmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2, KEYLEN );
memcpy( sZpkuLmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN ,KEYLEN );
memcpy( sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN * 2 ,16 );
return 0;
}
 */

/************************
 *  生成双倍长的ZPK      *
 *  入参1: LMK加密下的zmk*
 *  出参1: ZMK加密下的zpk*
 *  出参2: LMK加密下的zpk*
 *  出参3: zpk校验值16位 *
 ************************/
int HSM_GenZpk2(char *sZmkuLmk, char *sZpkuZmk, char *sZpkuLmk, char *sCheckValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "IAX", 3);
    iLen = iLen + 3;
    memcpy(sSendBuf + iLen, sZmkuLmk, 32);
    iLen = iLen + 32;

    memcpy(sSendBuf + iLen, ";XX0", 4);
    iLen = iLen + 4;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;

    memcpy(sZpkuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, 32);
    memcpy(sZpkuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + 1 + 32, 32);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 32 * 2 + 2, 16);
    return 0;
}
/*
int	HSM_GenZpk2( char *sZmkuLmk, char *sZpkuZmk, char *sZpkuLmk ,char *sCheckValue )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "A01001X", 7);
iLen = iLen + 7;
memcpy( sSendBuf + iLen, sZmkuLmk, 32 );
iLen = iLen + 32;
memcpy( sSendBuf + iLen ,"X", 1 );
iLen = iLen + 1;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sZpkuLmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2+1 ,32 );
memcpy( sZpkuZmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 +1+1+ 32 ,32 );
memcpy( sCheckValue,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 32 * 2+2, 16 );
return 0;
}
 */

/************************************************
 * 生成ZMK主密钥                                      *
 * 入参1: 明文成份串(每个成份均为32位字符串)    *
 * 入参2: 成份个数                              *
 * 出参2: 明文LMK成份加密下的ZMK                *
 * **********************************************/
int HSM_Gen_Zmk(char* sComp, int iCompNum, char *sZmk, char *sCheckValue) {
    int iLen, iCompLen = 0;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "A5", 2);
    iLen = iLen + 2;
    sprintf(sSendBuf + iLen, "%d", iCompNum);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, "000X", 4);
    iLen = iLen + 4;
    iCompLen = strlen(sComp);
    memcpy(sSendBuf + iLen, sComp, iCompLen);
    iLen = iLen + iCompLen;
    /*memcpy( sSendBuf + iLen ,"X'19", 4 );
    iLen = iLen + 4;*/
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;

    memcpy(sZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, 32);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + 32, 16);

    return 0;
}

/************************
 *  生成双倍长ZAK         *
 *  入参1: LMK加密下的zmk*
 *  出参1: ZMK加密下的zak*
 *  出参2: LMK加密下的zak*
 *  出参3: zak校验值6位 *
 ************************/
int HSM_GenZak2(char *sZmkuLmk, char *sZakuZmk, char *sZakuLmk, char *sCheckValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "FI1X", 4);
    iLen = iLen + 4;
    memcpy(sSendBuf + iLen, sZmkuLmk, 32);
    iLen = iLen + 32;

    memcpy(sSendBuf + iLen, ";XX0", 4);
    iLen = iLen + 4;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;

    memcpy(sZakuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, 32);
    memcpy(sZakuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + 1 + 32, 32);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 32 * 2 + 2, 16);
    return 0;
}
/*
int	HSM_GenZak2( char *sZmkuLmk, char *sZakuZmk, char *sZakuLmk ,char *sCheckValue )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "A01008X", 7);
iLen = iLen + 7;
memcpy( sSendBuf + iLen, sZmkuLmk, 32 );
iLen = iLen + 32;
memcpy( sSendBuf + iLen ,"X", 1 );
iLen = iLen + 1;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sZakuLmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2+1 ,32 );
memcpy( sZakuZmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 +1+1+ 32 ,32 );
memcpy( sCheckValue,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 32 * 2+2, 16 );
return 0;
}
 */

/*****************************************
 *     转换ZMK加密下的TMK到LMK加密下的TMK *
 *    入掺1:  LMK加密下的zmk              *
 *    入参2:  ZMK加密下的tmk              *
 *    出参1:  LMK加密下的tmk              *
 ******************************************/
int HSM_TranTmk_Zmk2Lmk(char *sZmkuLmk, char *sTmkuZmk, char *sTmkuLmk, char *pcChkValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sChkValue[33] = {0};

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;

    /* 命令代码 */
    strncpy(sSendBuf + iLen, "A6", 2);
    iLen = iLen + 2;

    strncpy(sSendBuf + iLen, "000", 1);
    iLen = iLen + 3;

    /* LMK对（04-05）下加密的ZMK 1A+32H */
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZmkuLmk, 32);
    iLen = iLen + 32;
    /* ZMK下加密的TMK 1A+32H */
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sTmkuZmk, 32);
    iLen = iLen + 32;
    /*
    1.密钥方案(LMK) 1A
     */
    memcpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;

    memcpy(sChkValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 33, 16);
    if ((memcmp(pcChkValue, "00000000", 8)) && (memcmp(sChkValue, pcChkValue, 8)))
        //if (memcmp(sChkValue, pcChkValue, 8))
        return -1;
    memcpy(sTmkuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, KEYLEN);
    return 0;
}

/*****************************************
 *     转换ZMK加密下的ZPK到LMK加密下的ZPK *
 *    入掺1:  LMK加密下的zmk              *
 *    入参2:  ZMK加密下的zpk              *
 *    出参1:  LMK加密下的zpk              *
 ******************************************/
int HSM_TranZpk_Tmk2Lmk(char *sZmkuLmk, char *sZpkuTmk, char *sZpkuLmk, char *pcChkValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sChkValue[33] = {0};

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;

    /* 命令代码 */
    strncpy(sSendBuf + iLen, "A6", 2);
    iLen = iLen + 2;

    strncpy(sSendBuf + iLen, "001", 3);
    iLen = iLen + 3;

    /* LMK对（04-05）下加密的ZMK 1A+32H */
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZmkuLmk, 32);
    iLen = iLen + 32;
    /* ZMK下加密的TMK 1A+32H */
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZpkuTmk, 32);
    iLen = iLen + 32;
    /*
     *         1.密钥方案(LMK) 1A
     *             */
    memcpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sChkValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 33, 16);
    //LiuZe 2013-08-19 
    //
    //if (memcmp(sChkValue, pcChkValue, 8))
    //    return -1;
    if ((memcmp(pcChkValue, "00000000", 8)) && (memcmp(sChkValue, pcChkValue, 8)))
        return -1;
    memcpy(sZpkuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, KEYLEN);
    return 0;
}

/*****************************************
 *     转换ZMK加密下的ZPK到LMK加密下的ZPK *
 *    入掺1:  LMK加密下的zmk              *
 *    入参2:  ZMK加密下的zpk              *
 *    出参1:  LMK加密下的zpk              *
 ******************************************/
int HSM_TranTmk_Lmk2Zmk(char *sZmkuLmk, char *sTmkuLmk, char *sKeyType, char *sTmkuZmk, char *pcChkValue) {
    int iLen;
    int iKeyLen = 16;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sChkValue[33] = {0};

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;

    /* 命令代码 */
    strncpy(sSendBuf + iLen, "A8", 2);
    iLen = iLen + 2;

    strncpy(sSendBuf + iLen, sKeyType, 3);
    iLen = iLen + 3;

    /* LMK对（04-05）下加密的ZMK 1A+32H */
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZmkuLmk, strlen(sZmkuLmk));
    iLen = iLen + strlen(sZmkuLmk);
    /* ZMK下加密的TMK 1A+32H */
    iKeyLen = strlen(sTmkuLmk);
    if (iKeyLen > 16) {
        strncpy(sSendBuf + iLen, "X", 1);
        iLen = iLen + 1;
    }
    memcpy(sSendBuf + iLen, sTmkuLmk, iKeyLen);
    iLen = iLen + iKeyLen;
    /*
     *         1.密钥方案(LMK) 1A
     *             */
    if (iKeyLen > 16)
        memcpy(sSendBuf + iLen, "X", 1);
    else
        memcpy(sSendBuf + iLen, "Z", 1);
    iLen = iLen + 1;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    //memcpy(sChkValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 33, 16);
    //LiuZe 2013-08-19 
    //
    //if (memcmp(sChkValue, pcChkValue, 8))
    //    return -1;
    if (iKeyLen > 16) {
        memcpy(sTmkuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, KEYLEN);
        sTmkuZmk[KEYLEN] = '\0';
        memcpy(pcChkValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + KEYLEN, 16);
        pcChkValue[16] = '\0';
    } else {
        memcpy(sTmkuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
        sTmkuZmk[KEYLEN] = '\0';
        memcpy(pcChkValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16, 16);
        pcChkValue[16] = '\0';
    }
    return 0;
}

/*****************************************
 *     转换LMK加密下的ZPK到ZMK加密下的ZPK *
 *    入掺1:  LMK加密下的zmk              *
 *    入参2:  LMK加密下的zpk              *
 *    出参1:  ZMK加密下的zpk              *
 ******************************************/
int HSM_TranZpk_Lmk2Zmk(char *sZmkuLmk, char *sZpkuLmk, char *sZpkuZmk) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "GC", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sZmkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, sZpkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, ";UU0", 4);
    iLen = iLen + 4;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sZpkuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, KEYLEN);
    return 0;
}

/************************
 *  生成ZAK              *
 *  入参1: LMK加密下的zmk*
 *  出参1: ZMK加密下的zak*
 *  出参2: LMK加密下的zak*
 *  出参3: zak校验值6位 *
 ************************/
int HSM_GenZak(char *sZmkuLmk, char *sZakuZmk, char *sZakuLmk, char *sCheckValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];

    if (strlen(sZmkuLmk) > 16) {
        return ( HSM_GenZak2(sZmkuLmk, sZakuZmk, sZakuLmk, sCheckValue));

    }

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "FI1", 3);
    iLen = iLen + 3;
    memcpy(sSendBuf + iLen, sZmkuLmk, 16);
    iLen = iLen + 16;

    memcpy(sSendBuf + iLen, ";ZZ0", 4);
    iLen = iLen + 4;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;

    memcpy(sZakuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    memcpy(sZakuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16, 16);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16 * 2, 16);
    return 0;
}
/*
int	HSM_GenZak( char *sZmkuLmk, char *sZakuZmk, char *sZakuLmk ,char *sCheckValue )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "A01008Z", 7);
iLen = iLen + 7;
memcpy( sSendBuf + iLen, sZmkuLmk, 16 );
iLen = iLen + 16;
memcpy( sSendBuf + iLen ,"Z", 1 );
iLen = iLen + 1;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sZakuLmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 ,16 );
memcpy( sZakuZmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16 ,16 );
memcpy( sCheckValue,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16 * 2, 16 );
return 0;
}
 */
/*
int	HSM_GenZak( char *sZmkuLmk,char *sZakuZmk, char *sZakuLmk,char *sCheckValue )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "FI1", 3);
iLen = iLen + 3;
memcpy( sSendBuf + iLen, sZmkuLmk, KEYLEN );
iLen = iLen + KEYLEN;
memcpy( sSendBuf + iLen, ";UU0",4 );
iLen = iLen + 4;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sZakuZmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2, KEYLEN );
memcpy( sZakuLmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN ,KEYLEN );
memcpy( sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN * 2 ,16 );
return 0;
}
 */

/*****************************************
 *     转换ZMK加密下的ZAK到LMK加密下的ZAK *
 *    入掺1:  LMK加密下的zmk              *
 *    入参2:  ZMK加密下的zak              *
 *    出参1:  LMK加密下的zak              *
 ******************************************/
int HSM_TranZak_Tmk2Lmk(char *sTmkuLmk, char *sZakuTmk, char *sZakuLmk, char *pcChkValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sChkValue[33] = {0};
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "A6", 2);
    iLen = iLen + 2;

    strncpy(sSendBuf + iLen, "008", 3);
    iLen = iLen + 3;
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sTmkuLmk, 32);
    iLen = iLen + 32;

    memcpy(sSendBuf + iLen, sZakuTmk, 16);
    iLen = iLen + 16;

    strncpy(sSendBuf + iLen, "Z", 1);
    iLen = iLen + 1;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sChkValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16, 16);
    //LiuZe 2013-08-19
    //
    //if (memcmp(sChkValue, pcChkValue, 8))
    //    return -1;
    if ((memcmp(pcChkValue, "00000000", 8)) && (memcmp(sChkValue, pcChkValue, 8)))
        return -1;
    memcpy(sZakuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    return 0;
}

/*****************************************
 *     转换ZMK加密下的ZAK到LMK加密下的ZAKcups 使用 *
 *    入掺1:  LMK加密下的zmk              *
 *    入参2:  ZMK加密下的zak              *
 *    出参1:  LMK加密下的zak              *
 ******************************************/
int HSM_TranZak_CUPSLmk(char *sTmkuLmk, char *sZakuTmk, char *sZakuLmk, char *pcChkValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sChkValue[33] = {0};
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "A6", 2);
    iLen = iLen + 2;

    strncpy(sSendBuf + iLen, "008", 3);
    iLen = iLen + 3;
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sTmkuLmk, 32);
    iLen = iLen + 32;

    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZakuTmk, 32);
    iLen = iLen + 32;

    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sChkValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 16, 16);
    //LiuZe 2013-08-19
    //
    //if (memcmp(sChkValue, pcChkValue, 8))
    //    return -1;
    if ((memcmp(pcChkValue, "00000000", 8)) && (memcmp(sChkValue, pcChkValue, 8)))
        return -1;
    memcpy(sZakuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, 32);
    return 0;
}

/*****************************************
 *     转换LMK加密下的ZAK到ZMK加密下的ZAK *
 *    入掺1:  LMK加密下的zmk              *
 *    入参2:  LMK加密下的zak              *
 *    出参1:  ZMK加密下的zak              *
 ******************************************/
int HSM_TranZak_Lmk2Zmk(char *sZmkuLmk, char *sZakuLmk, char *sZakuZmk) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "FM1", 3);
    iLen = iLen + 3;
    memcpy(sSendBuf + iLen, sZmkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, sZakuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, ";UU0", 4);
    iLen = iLen + 4;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sZakuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, KEYLEN);
    return 0;
}

/**************************************
 *  产生终端用的TMK                    *
 *  用于第一次产生TMK                  *
 *  入参1: LMK加密下的ZMK              *
 *  出参1: LMK加密下的ZMK              *
 *  出掺2: ZMK加密下的TMK              *
 **************************************/
int HSM_FirstGenTmk(char *sZmkuLmk, char *sTmkuZmk, char *sTmkuLmk, char *sCheckValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "A01000X", 7);
    iLen = iLen + 7;
    memcpy(sSendBuf + iLen, sZmkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sTmkuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, KEYLEN);
    memcpy(sTmkuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + KEYLEN + 1, KEYLEN);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN * 2 + 2, 16);
    return 0;
}
/***************************************************************************/

/**************************
 *  生成TPK                *
 *  入参1: LMK加密下的tmk  *
 *  出参1: LMK加密下的原tpk*
 *  出参2: LMK加密下的tpk  *
 *  出参3: tpk校验值16位   *
 ************************/
int HSM_GenTpk(char *sTmkuLmk, char *sTpkuZmk, char *sTpkuLmk, char *sCheckValue) {
    int iLen;
    int iRet;
    char sLenBuf[100];
    char sSendBuf[2048];

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "A01001X", 7);
    iLen = iLen + 7;
    memcpy(sSendBuf + iLen, sTmkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;

    memcpy(sTpkuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, KEYLEN);
    memcpy(sTpkuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + KEYLEN + 1, KEYLEN);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN * 2 + 2, 16);
    return 0;
}
/*
int	HSM_GenTpk( char *sSrcTpkuLmk, char *sTpkuTmk,char *sTpkuLmk,char *sCheckValue )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "HC", 2);
iLen = iLen + 2;
memcpy( sSendBuf + iLen, sSrcTpkuLmk, KEYLEN );
iLen = iLen + KEYLEN;
memcpy( sSendBuf + iLen, ";UU0",4 );
iLen = iLen + 4;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sTpkuTmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 , KEYLEN );
memcpy( sTpkuLmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN, KEYLEN );
memcpy( sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN * 2 ,16 );
return 0;
}
 */

/**************************
 *  生成TAK                *
 *  入参1: LMK加密下的tmk  *
 *  出参1: LMK加密下的原tak*
 *  出参2: LMK加密下的tak  *
 *  出参3: tpk校验值6位   *
 ************************/
int HSM_GenTak(char *sTmkuLmk, char *sTakuZmk, char *sTakuLmk, char *sCheckValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    //strncpy( sSendBuf+ iLen, "A01008X", 7);
    strncpy(sSendBuf + iLen, "A01003X", 7);
    iLen = iLen + 7;
    memcpy(sSendBuf + iLen, sTmkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sTakuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, KEYLEN);
    memcpy(sTakuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + KEYLEN + 1, KEYLEN);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN * 2 + 2, 16);
    return 0;
}

int HSM_GenTdk(char *sTmkuLmk, char *sTdkuZmk, char *sTdkuLmk, char *sCheckValue) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "A0100AX", 7);
    iLen = iLen + 7;
    memcpy(sSendBuf + iLen, sTmkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sTdkuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1, KEYLEN);
    memcpy(sTdkuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + KEYLEN + 1, KEYLEN);
    memcpy(sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN * 2 + 2, 16);
    return 0;
}

/*
int	HSM_GenTak( char *sTmkuLmk, char *sTakuTmk,char *sTakuLmk,char *sCheckValue )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "HA", 2);
iLen = iLen + 2;
memcpy( sSendBuf + iLen, sTmkuLmk, KEYLEN );
iLen = iLen + KEYLEN;
memcpy( sSendBuf + iLen, ";UU0",4 );
iLen = iLen + 4;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sTakuTmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 , KEYLEN );
memcpy( sTakuLmk,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN, KEYLEN );
memcpy( sCheckValue, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + KEYLEN * 2 ,16 );
return 0;
}
 */

/*****************************************
 *     转换LMK加密下的TAK到ZMK加密下的TAK *
 *    入掺1:  LMK加密下的zmk              *
 *    入参2:  LMK加密下的tak              *
 *    出参1:  ZMK加密下的zak              *
 ******************************************/
int HSM_TranTak_Lmk2Zmk(char *sZmkuLmk, char *sTakuLmk, char *sTakuZmk) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "MG", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sZmkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, sTakuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, ";UU0", 4);
    iLen = iLen + 4;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sTakuZmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, KEYLEN);
    return 0;
}

/***************************************
 * 生成MAC                              *
 * 入掺:LMK加密下的TAK                  *
 * 入掺:掺于计算的MAC数据               *
 * 入掺:掺于计算的MAC数据长度           *
 * 入掺:MAC 16位16进制数                *
 ****************************************/
int HSM_GenMac(char *sTakuLmk, unsigned char *sMacData, int iDataLen, unsigned char *sMac) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sHexBuf[1024];
    char sHexLen[100];
    char sMac8[100];
    char sSrcMac[1024];
    char sMacOk[10];

    memset(sMac8, 0, sizeof ( sMac8));
    memset(sMacOk, 0, sizeof ( sMacOk));
    memset(sSrcMac, 0, sizeof ( sSrcMac));

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    memcpy(sSrcMac, sMacData, iDataLen);
    GetMac8(sSrcMac, iDataLen, sMac8);

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "MS001", 5);
    //strncpy( sSendBuf+ iLen, "MS011", 5);
    iLen = iLen + 5;
    //memcpy( sSendBuf + iLen,"1X", 2 );
    memcpy(sSendBuf + iLen, "0X", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sTakuLmk, 32);
    iLen = iLen + 32;

    memset(sHexLen, 0, sizeof ( sHexLen));
    Dec2Hex(sHexLen, 8);
    memset(sHexBuf, 0, sizeof ( sHexBuf));
    fl_bcd_to_asc(sHexBuf, sHexLen, 4, 0);
    memcpy(sSendBuf + iLen, sHexBuf, 4);
    iLen = iLen + 4;


    memset(sHexBuf, 0, sizeof ( sHexBuf));
    //fl_bcd_to_asc( sHexBuf,sMac8,16,0 );
    //memcpy( sSendBuf + iLen , sHexBuf,16  );
    //iLen = iLen + 16 ;

    memcpy(sSendBuf + iLen, sMac8, 8);
    iLen = iLen + 8;


    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sMacOk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    fl_asc_to_bcd(sMac, sMacOk, 16, 0);


    return 0;
}
/* 

int	HSM_GenMac( char *sTakuLmk,unsigned char *sMacData,int iDataLen,unsigned char *sMac )
{
int	iLen;
int	iRet;
char	sLenBuf[10];
char	sSendBuf[1024];
char	sHexBuf[1024];
char	sHexLen[100];
memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "MS011", 5);
iLen = iLen + 5;
memcpy( sSendBuf + iLen,"1X", 2 );
iLen = iLen + 2;
memcpy( sSendBuf + iLen, sTakuLmk, 32 );
iLen = iLen + 32;
memset( sHexLen, 0,sizeof( sHexLen ));
Dec2Hex(sHexLen, iDataLen  );
memset( sHexBuf, 0,sizeof( sHexBuf ));
fl_bcd_to_asc( sHexBuf,sHexLen,4,0);
memcpy( sSendBuf+iLen, sHexBuf,4 );
iLen = iLen + 4;
memset( sHexBuf, 0,sizeof( sHexBuf ));
fl_bcd_to_asc( sHexBuf,sMacData,iDataLen*2,0 );
memcpy( sSendBuf + iLen , sHexBuf,iDataLen*2  );
iLen = iLen + iDataLen*2 ;

Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sMac,secbuf_out + 2 + MSGHEADLEN + 2 + 2 ,16 );
return 0;
}
 */

/********************************
 * 转换PIN从TPK到ZPK             *
 * 入掺:卡号                     *
 * 入掺:原PIN 16位               *
 * 入掺:LMK加密下的TPK           *
 * 入掺:LMK加密下的ZPK           *
 * 出掺:ZPK加密下的PIN 16位      *
 *********************************/
int HSM_TranPin_Tpk2Zpk(char *sCardNo, char *sOldPin, char *sTpkuLmk, char *sZpkuLmk, char *sNewPin) {
    int iLen;
    int iRet;
    int iCardLen;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sCardBuf[20];

    if (strlen(sZpkuLmk) > 16) {

        return ( HSM_TranPin_Tpk2Zpk2(sCardNo, sOldPin, sTpkuLmk, sZpkuLmk, sNewPin));
    }

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "CCX", 3);
    iLen = iLen + 3;
    memcpy(sSendBuf + iLen, sTpkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, sZpkuLmk, 16);

    iLen = iLen + 16;
    memcpy(sSendBuf + iLen, "12", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sOldPin, 16);
    iLen = iLen + 16;

    memcpy(sSendBuf + iLen, "0101", 4);
    iLen = iLen + 4;
    memset(sCardBuf, 0, sizeof ( sCardBuf));
    iCardLen = strlen(sCardNo);
    strncpy(sCardBuf, sCardNo + (iCardLen - 13), 12);

    memcpy(sSendBuf + iLen, sCardBuf, 12);
    iLen = iLen + 12;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sNewPin, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 2, 16);
    return 0;
}
/*
int	HSM_TranPin_Tpk2Zpk( char *sCardNo, char *sOldPin,char *sTpkuLmk, char *sZpkuLmk,char *sNewPin)
{
int	iLen;
int	iRet;
int	iCardLen;
char	sLenBuf[10];
char	sSendBuf[1024];
char	sCardBuf[20];

if ( strlen( sZpkuLmk ) > 16 )
{

return ( HSM_TranPin_Tpk2Zpk2( sCardNo,sOldPin,sTpkuLmk,sZpkuLmk,sNewPin ));
}

memset( secbuf_in,0,sizeof( secbuf_in ));
memset( secbuf_out,0,sizeof( secbuf_out ));
memset( sSendBuf,0,sizeof( sSendBuf ));
memset( sLenBuf,0,sizeof( sLenBuf ));

strcat( sSendBuf, MSGHEAD );
iLen = MSGHEADLEN;
strncpy( sSendBuf+ iLen, "CCX", 3);
iLen = iLen + 3;
memcpy( sSendBuf + iLen, sTpkuLmk, KEYLEN );
iLen = iLen + KEYLEN;
memcpy( sSendBuf + iLen, sZpkuLmk, 16 );
iLen = iLen + 16;
memcpy( sSendBuf + iLen, "12",2 );
iLen = iLen + 2;
memcpy( sSendBuf + iLen , sOldPin, 16 );
iLen = iLen + 16;

memcpy( sSendBuf + iLen,"0101",4 );
iLen = iLen + 4;
memset( sCardBuf,0,sizeof( sCardBuf ));
iCardLen = strlen( sCardNo );
strncpy( sCardBuf, sCardNo + (iCardLen -13 ), 12 );

memcpy( sSendBuf+iLen ,sCardBuf, 12 );
iLen = iLen + 12;
Dec2Hex( sLenBuf,iLen );
memcpy( secbuf_in,sLenBuf,2 );
memcpy( secbuf_in+2, sSendBuf,iLen );
iRet = HSM_LINK( secbuf_in,iLen+2, secbuf_out );
if ( iRet < 0 )
return iRet;
memcpy( sNewPin,secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 2 ,16 );
return 0;
}
 */

/********************************
 * 转换PIN从TPK到LMK             *
 * 入掺:卡号                     *
 * 入掺:璓IN 16位               *
 * 入掺:LMK加密下的TPK           *
 * 出掺:LMK加密下的PIN 16位      *
 *********************************/
int HSM_TranPin_Tpk2Lmk(char *sCardNo, char *sOldPin, char *sTpkuLmk, char *sNewPin) {
    int iLen;
    int iRet;
    int iCardLen;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sCardBuf[20];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "JC", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sTpkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    memcpy(sSendBuf + iLen, sOldPin, 16);
    iLen = iLen + 16;

    memcpy(sSendBuf + iLen, "0101", 4);
    iLen = iLen + 4;
    memset(sCardBuf, 0, sizeof ( sCardBuf));
    iCardLen = strlen(sCardNo);
    strncpy(sCardBuf, sCardNo + (iCardLen - 13), 12);

    memcpy(sSendBuf, sCardBuf, 12);
    iLen = iLen + 12;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sNewPin, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    return 0;
}

/********************************
 * 转换PIN从LMK到ZPK             *
 * 入掺:卡号                     *
 * 入掺:原PIN 16位               *
 * 入掺:LMK加密下的ZPK           *
 * 出掺:ZPK加密下的PIN 16位      *
 *********************************/
int HSM_TranPin_Lmk2Zpk(char *sCardNo, char *sOldPin, char *sZpkuLmk, char *sNewPin) {
    int iLen;
    int iRet;
    int iCardLen;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sCardBuf[20];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "JG", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sZpkuLmk, 16);
    iLen = iLen + 16;
    memcpy(sSendBuf + iLen, "01", 4);
    iLen = iLen + 2;
    memset(sCardBuf, 0, sizeof ( sCardBuf));
    iCardLen = strlen(sCardNo);
    strncpy(sCardBuf, sCardNo + (iCardLen - 13), 12);

    memcpy(sSendBuf, sCardBuf, 12);
    iLen = iLen + 12;
    memcpy(sSendBuf, sOldPin, 16);
    iLen = iLen + 16;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sNewPin, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    return 0;
}

void Xor(char *pcXorResult, char *pcData, int iLen) {
    char sTmp[8] = {0};
    int i, iPos = 0, iOff = 0;

    memcpy(sTmp, pcData, 8);
    for (iPos = 8; iPos < iLen; iPos += iOff) {
        iOff = ((iLen - iPos) >= 8) ? 8 : iLen - iPos;
        for (i = 0; i < iOff; i++)
            sTmp[i] ^= pcData[ i + iPos ];
    }
    memcpy(pcXorResult, sTmp, 8);
}

/****************************************
 * 银联CBC标准MAC算法                    *
 ****************************************/
int HSM_GenStdMac(char *sZakuLmk, unsigned char *sMacData, int iDataLen, unsigned char *sMac) {
    int iLen, iDataLenEx;
    int iRet;
    char sLenBuf[100], sDataLen[5];
    char sSendBuf[1024], sMacDataEx[1024] = {0};
    char sMacOk[100], sTmp[8] = {0};

    memset(sMacOk, 0, sizeof ( sMacOk));
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "MS010", 5);
    iLen = iLen + 5;
    /* BCD */
    memcpy(sSendBuf + iLen, "0", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZakuLmk, 16);
    iLen = iLen + 16;

    /* 长度需为8的倍数 */
    iDataLenEx = GetFullLen(iDataLen, 8);
    sprintf(sDataLen, "%04x", iDataLenEx);
    int i;
    for (i = 0; i < 4; i++) {
        if (sDataLen[i] >= 'a' && sDataLen[i] <= 'f') {
            sDataLen[i] = sDataLen[i] - 32;
            break;
        }
    }
    //printf( "sDataLen = [%s]\n", sDataLen );
    memcpy(sSendBuf + iLen, sDataLen, 4);
    iLen = iLen + 4;

    memcpy(sMacDataEx, sMacData, iDataLen);
    memcpy(sSendBuf + iLen, sMacDataEx, iDataLenEx);
    iLen = iLen + iDataLenEx;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sMac, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 8);
    sMac[8] = '\0';
    /*
    fl_asc_to_bcd( sMac,sMacOk,16,0 );
     */
    return 0;
}

/****************************************
 * 银联CBC标准MAC算法 3DES                   *
 ****************************************/
int HSM_GenStdMac3Des(char *sZakuLmk, unsigned char *sMacData, int iDataLen, unsigned char *sMac) {
    int iLen, iDataLenEx;
    int iRet;
    char sLenBuf[100], sDataLen[5];
    char sSendBuf[1024], sMacDataEx[1024] = {0};
    char sMacOk[100], sTmp[8] = {0};

    memset(sMacOk, 0, sizeof ( sMacOk));
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "MU011", 5);
    iLen = iLen + 5;
    /* BCD */
    memcpy(sSendBuf + iLen, "0", 1);
    iLen = iLen + 1;
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZakuLmk, 32);
    iLen = iLen + 32;

    /* 长度需为8的倍数 */
    iDataLenEx = GetFullLen(iDataLen, 8);
    sprintf(sDataLen, "%04x", iDataLenEx);
    //printf( "sDataLen = [%s]\n", sDataLen );
    memcpy(sSendBuf + iLen, sDataLen, 4);
    iLen = iLen + 4;

    memcpy(sMacDataEx, sMacData, iDataLen);
    memcpy(sSendBuf + iLen, sMacDataEx, iDataLenEx);
    iLen = iLen + iDataLenEx;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    //memcpy(sMacOk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    //fl_asc_to_bcd(sMac, sMacOk, 16, 0);

    memcpy(sMac, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 8);
    sMac[8] = '\0';

    return 0;
}

/****************************************
 * T0CBC标准MAC算法                    *
 ****************************************/
int HSM_GenStdT0Mac(char *sZakuLmk, unsigned char *sMacData, int iDataLen, unsigned char *sMac) {
    int iLen, iDataLenEx;
    int iRet;
    char sLenBuf[100], sDataLen[5];
    char sSendBuf[1024], sMacDataEx[1024] = {0}, str[1024] = {0};
    char sMacOk[100], sTmp[8] = {0};

    memset(sMacOk, 0, sizeof ( sMacOk));
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "MS010", 5);
    iLen = iLen + 5;
    /* BCD */
    memcpy(sSendBuf + iLen, "0", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZakuLmk, 16);
    iLen = iLen + 16;
    //memcpy( sSendBuf + iLen, "0000000000000000", 16 );
    //iLen = iLen + 16;

    /* 长度需为8的倍数 */
    iDataLenEx = GetFullLen(iDataLen, 8);
    //iDataLenEx = iDataLenEx/2;
    sprintf(sDataLen, "%04x", iDataLenEx);
    int i;
    for (i = 0; i < 4; i++) {
        if (sDataLen[i] >= 'a' && sDataLen[i] <= 'f') {
            sDataLen[i] = sDataLen[i] - 32;
            break;
        }
    }
    //printf( "sDataLen = [%s]\n", sDataLen );
    memcpy(sSendBuf + iLen, sDataLen, 4);
    iLen = iLen + 4;

    memcpy(sMacDataEx, sMacData, iDataLen);
    memcpy(sSendBuf + iLen, sMacDataEx, iDataLenEx);
    iLen = iLen + iDataLenEx;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sMac, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    sMac[16] = '\0';
    /*
    fl_asc_to_bcd( sMac,sMacOk,16,0 );
     */
    return 0;
}

/***************************************
 * 生成 发卡机构MAC                     *
 * 入掺:LMK加密下的ZAK                  *
 * 入掺:掺于计算的MAC数据 16字节hex     *
 * 入掺:掺于计算的MAC数据长度 banery长度          *
 * 入掺:MAC 16位16进制数                *
 ****************************************/
int HSM_GenHostMac(char *sZakuLmk, unsigned char *sMacData, int iDataLen, unsigned char *sMac) {
    int iLen;
    int iRet;
    char sLenBuf[100];
    char sSendBuf[1024];
    char sHexBuf[1024];
    char sHexLen[100];
    char sMacOk[100], sTmp[8] = {0};

    memset(sMacOk, 0, sizeof ( sMacOk));
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "MS010", 5);
    iLen = iLen + 5;
    /* bcd */
    memcpy(sSendBuf + iLen, "0", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZakuLmk, 16);
    iLen = iLen + 16;

    memset(sHexLen, 0, sizeof ( sHexLen));
    memcpy(sSendBuf + iLen, "0010", 4);
    iLen = iLen + 4;
    Xor(sTmp, sMacData, iDataLen);
    fl_bcd_to_asc(sHexBuf, sTmp, 16, 0);
    memcpy(sSendBuf + iLen, sHexBuf, 16);
    iLen = iLen + 16;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sMac, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 8);
    sMac[8] = '\0';
    /*
    fl_asc_to_bcd( sMac,sMacOk,16,0 );
     */
    return 0;
}

/***************************************
 * 生成发卡机构端双倍长mackey的MAC      *
 * 入掺:LMK加密下的ZAK                  *
 * 入掺:掺于计算的MAC数据               *
 * 入掺:掺于计算的MAC数据长度           *
 * 入掺:MAC 16位16进制数                *
 ****************************************/
int HSM_GenHostMac2(char *sZakuLmk, unsigned char *sMacData, int iDataLen, unsigned char *sMac) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sHexBuf[1024];
    char sHexLen[100];
    char sMacOk[100];

    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));
    memset(sMacOk, 0, sizeof ( sMacOk));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "MS010", 5);
    iLen = iLen + 5;
    memcpy(sSendBuf + iLen, "1", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZakuLmk, 16);
    iLen = iLen + 16;
    memset(sHexLen, 0, sizeof ( sHexLen));
    Dec2Hex(sHexLen, iDataLen);
    memset(sHexBuf, 0, sizeof ( sHexBuf));
    fl_bcd_to_asc(sHexBuf, sHexLen, 4, 0);
    memcpy(sSendBuf + iLen, sHexBuf, 4);
    iLen = iLen + 4;
    memset(sHexBuf, 0, sizeof ( sHexBuf));
    fl_bcd_to_asc(sHexBuf, sMacData, iDataLen * 2, 0);
    memcpy(sSendBuf + iLen, sHexBuf, iDataLen * 2);
    iLen = iLen + iDataLen * 2;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sMacOk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    //printf("Mac[%s]\n", sMacOk);
    fl_asc_to_bcd(sMac, sMacOk, 16, 0);
    return 0;
}

/*****************************
 *  加密一个明文密码          *
 *  入参1:sClearPwd,明文密码  *
 *  入参2:sCardNo   卡号      *
 *  出参: sPin  LMK加密的pin  *
 *****************************/
int HSM_EncPin(char *sClearPwd, char *sCardNo, char *sPin) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sHexBuf[1024];
    char sHexLen[100];
    char sPinB[100];
    char sCard[100];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "BA", 2);
    iLen = iLen + 2;
    memset(sPinB, 'F', sizeof ( sPinB));
    sPinB[8] = 0;
    memcpy(sPinB, sClearPwd, 6);
    memcpy(sSendBuf + iLen, sPinB, 8);
    iLen = iLen + 8;
    memset(sCard, 0, sizeof (sCard));
    sCard[13] = 0;
    memcpy(sCard, sCardNo + (strlen(sCardNo) - 13), 12);
    memcpy(sSendBuf + iLen, sCard, 12);
    iLen = iLen + 12;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sPin, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 8);
    return 0;
}

/*****************************
 *  解密一个LMK下的密码       *
 *  入参1: sPin  LMK加密的pin *
 *  入参2:sCardNo   卡号      *
 *  入参1:sClearPwd,明文密码  *
 *****************************/
int HSM_DecPin(char *sPin, char *sCardNo, char *sClearPwd) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sHexBuf[1024];
    char sHexLen[100];
    char sPinB[100];
    char sCard[100];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "NG", 2);
    iLen = iLen + 2;
    memset(sCard, 0, sizeof (sCard));
    sCard[13] = 0;
    memcpy(sCard, sCardNo + (strlen(sCardNo) - 13), 12);
    memcpy(sSendBuf + iLen, sCard, 12);
    iLen = iLen + 12;

    memcpy(sSendBuf + iLen, sPin, 7);
    iLen = iLen + 7;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sClearPwd, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 6);
    return 0;
}

/*****************************
 *  转换LMK下的密码到ZPK下加密*
 *  入参1:LMK下的ZPK          *
 *  入参2 卡号                *
 *  入参3:LMK加密下的密码     *
 *  出参:ZPK加密下的PIN       *
 *****************************/
int HSM_TranPin_LMK2ZPK(char *sZpkuLmk, char *sCardNo, char *sPinuLmk, char *sPinuZpk) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sHexBuf[1024];
    char sHexLen[100];
    char sPinB[100];
    char sCard[100];

    /*
    if ( strlen( sZpkuLmk ) > 16 )
    {
    return  ( HSM_TranPin_LMK2ZPK2( sZpkuLmk, sCardNo,sPinuLmk,sPinuZpk ) );
    }
     */


    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "JG", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sZpkuLmk, 16);
    iLen = iLen + 16;
    memcpy(sSendBuf + iLen, "01", 2);
    iLen = iLen + 2;
    memset(sCard, 0, sizeof (sCard));
    sCard[13] = 0;
    memcpy(sCard, sCardNo + (strlen(sCardNo) - 13), 12);
    memcpy(sSendBuf + iLen, sCard, 12);
    iLen = iLen + 12;

    memcpy(sSendBuf + iLen, sPinuLmk, 7);
    iLen = iLen + 7;

    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sPinuZpk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 16);
    return 0;
}

/*****************************
 *  转换ZPK下的密码到LMK下加密*
 *  入参1:LMK下的ZPK          *
 *  入参2 卡号                *
 *  入参3:ZPK加密下的密码     *
 *  出参: LMK加密下的PIN       *
 *****************************/
int HSM_TranPIn_ZPK2LMK(char *sZpkuLmk, char *sCardNo, char *sPinuZpk, char *sPinuLmk) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sHexBuf[1024];
    char sHexLen[100];
    char sPinB[100];
    char sCard[100];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "JE", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sZpkuLmk, 16);
    iLen = iLen + 16;
    memcpy(sSendBuf + iLen, sPinuZpk, 16);
    iLen = iLen + 16;
    memcpy(sSendBuf + iLen, "01", 2);
    iLen = iLen + 2;
    memset(sCard, 0, sizeof (sCard));
    sCard[13] = 0;
    memcpy(sCard, sCardNo + (strlen(sCardNo) - 13), 12);
    memcpy(sSendBuf + iLen, sCard, 12);
    iLen = iLen + 12;


    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sPinuLmk, secbuf_out + 2 + MSGHEADLEN + 2 + 2, 8);
    return 0;
}

/********************************
 * 转换PIN从双倍长TPK到双倍长ZPK *
 * 入掺:卡号                     *
 * 入掺:原PIN 16位               *
 * 入掺:LMK加密下的TPK           *
 * 入掺:LMK加密下的ZPK           *
 * 出掺:ZPK加密下的PIN 16位      *
 *********************************/
int HSM_TranPin_Tpk2Zpk2(char *sCardNo, char *sOldPin, char *sTpkuLmk, char *sZpkuLmk, char *sNewPin) {
    int iLen;
    int iRet;
    int iCardLen;
    char sLenBuf[10];
    char sSendBuf[1024];
    char sCardBuf[20];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    strncpy(sSendBuf + iLen, "CCX", 3);
    iLen = iLen + 3;
    memcpy(sSendBuf + iLen, sTpkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    strncpy(sSendBuf + iLen, "X", 1);
    iLen = iLen + 1;
    memcpy(sSendBuf + iLen, sZpkuLmk, 32);
    iLen = iLen + 32;
    memcpy(sSendBuf + iLen, "12", 2);
    iLen = iLen + 2;
    memcpy(sSendBuf + iLen, sOldPin, 16);
    iLen = iLen + 16;

    memcpy(sSendBuf + iLen, "0101", 4);
    iLen = iLen + 4;
    memset(sCardBuf, 0, sizeof ( sCardBuf));
    iCardLen = strlen(sCardNo);
    strncpy(sCardBuf, sCardNo + (iCardLen - 13), 12);

    memcpy(sSendBuf + iLen, sCardBuf, 12);
    iLen = iLen + 12;
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(sNewPin, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 2, 16);
    return 0;
}

int HSM_Enc_Track(char *pcDecData, char *pcClearData, int iDataLen, char *pcTdkuLmk) {
    int iLen;
    int iRet;
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    /* Command code 2A */
    strncpy(sSendBuf + iLen, "E00010X", 7);
    iLen = iLen + 7;
    memcpy(sSendBuf + iLen, pcTdkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    /* Asc */
    memcpy(sSendBuf + iLen, "11000000", 8);
    iLen = iLen + 8;

    sprintf(sLenBuf, "%03d", (iDataLen + 1) / 2);
    memcpy(sSendBuf + iLen, sLenBuf, 3);
    iLen = iLen + 3;
    memcpy(sSendBuf + iLen, pcClearData, iDataLen);
    iLen = iLen + iDataLen;

    memset(sLenBuf, 0, sizeof (sLenBuf));
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(pcDecData, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + 3, 16);
    pcDecData[16] = '\0';
    return 0;
}

int HSM_Dec_Track(char *pcClearData, char *pcDecData, int iDataLen, char *pcTdkuLmk) {
    int iLen;
    int iRet;
    char sTmp[64] = {0};
    char sLenBuf[10];
    char sSendBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    /* Command code 2A */
    strncpy(sSendBuf + iLen, "E00110X", 7);
    iLen = iLen + 7;
    memcpy(sSendBuf + iLen, pcTdkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    /* asc */
    memcpy(sSendBuf + iLen, "11000000", 8);
    iLen = iLen + 8;
    sprintf(sLenBuf, "%03d", (iDataLen + 1) / 2);
    memcpy(sSendBuf + iLen, sLenBuf, 3);
    iLen = iLen + 3;
    memcpy(sSendBuf + iLen, pcDecData, iDataLen);
    iLen = iLen + iDataLen;

    memset(sLenBuf, 0, sizeof (sLenBuf));
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(pcClearData, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + 3, 16);
    pcClearData[16] = '\0';
    return 0;
}

int HSM_Dec_Pcmsg(char *pcClearData, char *pcDecData, int iDataLen, char *pcTdkuLmk) {
    int iLen;
    int ilen;
    int iRet;
    char sTmp[64] = {0};
    char sLenBuf[10];
    char sLenBuf1[10];
    char sSendBuf[1024];
    char sHexLen[100];
    char sHexBuf[1024];
    memset(secbuf_in, 0, sizeof ( secbuf_in));
    memset(secbuf_out, 0, sizeof ( secbuf_out));
    memset(sSendBuf, 0, sizeof ( sSendBuf));
    memset(sLenBuf, 0, sizeof ( sLenBuf));

    strcat(sSendBuf, MSGHEAD);
    iLen = MSGHEADLEN;
    /* Command code 2A */
    strncpy(sSendBuf + iLen, "E00110X", 7);
    iLen = iLen + 7;
    memcpy(sSendBuf + iLen, pcTdkuLmk, KEYLEN);
    iLen = iLen + KEYLEN;
    /* asc */
    memcpy(sSendBuf + iLen, "00000000", 8);
    iLen = iLen + 8;
    //sprintf(sLenBuf, "%03d", (iDataLen + 1) / 2);
    //ilen = (iDataLen + 1 ) / 2 ;

    memset(sHexLen, 0, sizeof ( sHexLen));
    Dec2Hex(sHexLen, iDataLen);
    memset(sHexBuf, 0, sizeof ( sHexBuf));
    fl_bcd_to_asc(sHexBuf, sHexLen, 4, 0);
    memcpy(sSendBuf + iLen, sHexBuf + 1, 3);
    /*
        memset(sLenBuf, 0, sizeof (sLenBuf));
        Dec2Hex(sLenBuf, ilen );
        sprintf(sLenBuf1, "%03s", sLenBuf);
        memcpy(sSendBuf + iLen, sLenBuf1, 3);
     */
    iLen = iLen + 3;
    memcpy(sSendBuf + iLen, pcDecData, iDataLen);
    iLen = iLen + iDataLen;

    memset(sLenBuf, 0, sizeof (sLenBuf));
    Dec2Hex(sLenBuf, iLen);
    memcpy(secbuf_in, sLenBuf, 2);
    memcpy(secbuf_in + 2, sSendBuf, iLen);
    iRet = HSM_LINK(secbuf_in, iLen + 2, secbuf_out);
    if (iRet < 0)
        return iRet;
    memcpy(pcClearData, secbuf_out + 2 + MSGHEADLEN + 2 + 2 + 1 + 3, iDataLen);
    pcClearData[iDataLen] = '\0';
    return 0;
}


