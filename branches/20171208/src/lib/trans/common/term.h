/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   term.h
 * Author: Pay
 *
 * Created on 2017��3��8��, ����10:09
 */

#ifndef TERM_H
#define TERM_H

#ifdef __cplusplus
extern "C" {
#endif

    /* �ն���Ϣ */
    typedef struct {
        char sMerchId[15 + 1]; /* �����̻���                               */
        char sTermId[8 + 1]; /* �ն˱��                                 */
        char sStatus[1 + 1]; /* �ն�״̬,0:����,1:�ر�                   */
        char sTermType[2 + 1]; /* �ն�����(��ΣPOS��3X��ʶ)                **
                                ** 00:����, 01:����, 02:����,  03����,      **
                                ** 04:����, 05:����, 06:��ý��,07����       */
        char sPinKey[32 + 1]; /* PIN��Կ                                  */
        char sMacKey[32 + 1]; /* MAC��Կ                                  */
        char sTmk[32 + 1]; /* TMK��Կ                                  */
        char sTdk[32 + 1]; /* TDK��Կ                                  */
        char sDownFlag[1 + 1]; /* �������ر�־,0:δ����; 1:������          */
        char sTermSn[50+1]; /* �ն����к� */
    } Terminal;

    int FindTermInfoById(Terminal *pcTerminal, char *pcMerchId, char *pcTermId);
    int FindTermIdBySn(char *pcMerchId, char *pcTermId, char *pcTermVender, char *pcTermSN);
    int UpdTermKey(char *psTmkuLmk, char * psTpkuTmk, char *psTakuTmk, char *psTdkuTmk, char *pstMerchId, char *pstTermId);
    int FindTermParam(char *pcMerchName, char *pcAdvertise, char *pcMerchId, char *pcTermId);
#ifdef __cplusplus
}
#endif

#endif /* TERM_H */

