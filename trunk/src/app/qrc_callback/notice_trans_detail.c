/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_redis.h"
#include "param.h"
#include "t_signal.h"
#include "t_tools.h"
#include "t_log.h"
#include "t_cjson.h"
#include "t_db.h"
#include <curl/curl.h>
#include <iconv.h>

//extern char* GetNoticeCnt();
#define QUERY_CNT  6

long lWeight[] = {1, 10, 20, 100, 200, 300};

/*ȥ��ȫ����\r,\n,\t,''*/
char *tTrimAll(char *pcStr) {
    char *pcTmp = NULL, *pcTmp1 = NULL;

    for (pcTmp = pcStr; *pcTmp != '\0'; pcTmp++) {
        if (*pcTmp != ' ' && *pcTmp != '\t' &&
                *pcTmp != '\n' && *pcTmp != '\r')
            ;
        else {
            memmove(pcTmp, pcTmp + 1, strlen(pcTmp + 1) + 1);
            pcTmp--;
        }
    }
    return ( pcStr);
}

int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen,
        char *outbuf, size_t outlen) {
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}

size_t HttpResponseHandler(char *ptr, size_t size, size_t nmemb, void *userdata) {
    //tTrimAll(ptr);
    memcpy(userdata, ptr, size * nmemb);
    return (size * nmemb);
}

int HttpRequest(char *pcPostUrl, char *pcJsonData, char *pcHttpReponse) {
    CURL *curl;
    CURLcode res;
    //    char sHttpResponse[2048] = {0};
    struct curl_slist *plist = NULL;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, pcPostUrl);
    // ����http���͵���������ΪJSON  
    plist = curl_slist_append(plist, "Content-Type:application/json;charset=UTF-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    // ����ҪPOST��JSON����  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pcJsonData);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpResponseHandler);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pcHttpReponse);
    // Perform the request, res will get the return code   
    res = curl_easy_perform(curl);
    // Check for errors  
    if (res != CURLE_OK) {
        tLog(ERROR, "curl_easy_perform() failed:%s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }
    if (strstr(pcHttpReponse, "html") != NULL) {
        tLog(ERROR, "http���ش���");
        curl_easy_cleanup(curl);
        return -1;
    }
    tLog(ERROR, "HttpReponse[%s]", pcHttpReponse);
    // always cleanup  
    curl_easy_cleanup(curl);
    return 0;
}

int FindValueByKey(char *pcValue, char *pcKey) {
    char sSqlStr[512] = {0};
    OCI_Resultset *pstRes = NULL;
    OCI_Statement* pstState = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "select key_value from s_param where key='%s'", pcKey);

    pstState = tExecuteEx(&pstRes, sSqlStr);
    if (pstState == NULL) {
        tLog(ERROR, "����Key[%s]��ֵʧ��.", pcKey);
        return -1;
    }
    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]���ΪNULL.", sSqlStr);
        return -1;
    }
    while (OCI_FetchNext(pstRes)) {
        STRV(pstRes, 1, pcValue);
        tTrim(pcValue);
    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "��Key[%s]��ֵ.", pcKey);
        tReleaseResEx(pstState);
        OCI_StatementFree(pstState);
        return -1;
    }
    tReleaseResEx(pstState);
    OCI_StatementFree(pstState);
    return 0;
}

int JudgeTime(char *pcTransmitTime, int iQueryCount) {
    char sTransTime[8 + 1] = {0};
    time_t tCTime, tCntTime;
    time_t tTransTime;
    struct tm stm;
    int i;
    //��õ�ǰʱ��
    time(&tCTime);
    strptime(pcTransmitTime, "%Y%m%d%H%M%S", &stm);
    tTransTime = mktime(&stm);
    tCntTime = tCTime - tTransTime;
    tLog(ERROR, "�µ�ʱ��[%s]", pcTransmitTime);
    tLog(ERROR, "��ǰ��ѯ����[%d],��ǰʱ���[%ld],�µ�ʱ���[%ld],���ʱ��[%ld]", iQueryCount, tCTime, tTransTime, tCntTime);
    for (i = 0; i < 6; i++) {
        tLog(ERROR, "�趨��ѯ���ʱ��[%ld]", lWeight[iQueryCount]);
        if (abs(tCntTime - lWeight[iQueryCount]) <= 5)
            return 0;
        else
            return -1;
    }
    return -1;
}

int UpdQRNoticeFlag(cJSON *pstJson) {
    char sSqlStr[512] = {0};
    char sDate[9] = {0}, sTime[7] = {0};
    char sRrn[12 + 1] = {0};
    int iCnt = 0, iResCnt = 0;
    char sTranTime[14 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDate[8 + 1] = {0};
    char sMerchOrderNo[26 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstJson, "orderNo", sMerchOrderNo);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET "
            " NOTICE_FLAG='1' WHERE MERCH_ORDER_NO='%s' and amount>0 "
            , sMerchOrderNo);
    tLog(INFO, "sql[%s]", sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tCommit();
    tLog(DEBUG, "Ӱ���¼��[%d]", iResCnt);
    return 0;
}

int UpdEPOSNoticeFlag(cJSON *pstJson) {
    char sSqlStr[512] = {0};
    char sDate[9] = {0}, sTime[7] = {0};
    char sRrn[12 + 1] = {0};
    int iCnt = 0, iResCnt = 0;
    char sTranTime[14 + 1] = {0}, sTransTime[6 + 1] = {0}, sTransDate[8 + 1] = {0};
    char sMerchOrderNo[26 + 1] = {0};
    OCI_Resultset *pstRes = NULL;

    GET_STR_KEY(pstJson, "orderNo", sMerchOrderNo);
    snprintf(sSqlStr, sizeof (sSqlStr), "UPDATE B_INLINE_TARNS_DETAIL SET "
            " NOTICE_FLAG='1' WHERE rrn='%s' and amount>0 "
            , sMerchOrderNo);
    tLog(INFO, "sql[%s]", sSqlStr);
    iResCnt = tExecuteUpd(sSqlStr);
    if (iResCnt < 0) {
        return -1;
    }
    tCommit();
    tLog(DEBUG, "Ӱ���¼��[%d]", iResCnt);
    return 0;
}

int ScanQrTrans(struct timeval *tvS) {

    char sSqlStr[2048];
    char sRrn[RRN_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0};
    char sTransDate[TRANS_DATE_LEN] = {0}, sTransTime[TRANS_TIME_LEN + 1] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sHttpResponse[2048 + 1] = {0};
    char sTraceNo[TRACE_NO_LEN + 1] = {0}, sReqSvrId[64] = {0};
    char sAgentId[INST_ID_LEN + 1] = {0}, sAmount[14 + 1] = {0};
    double dAmount = 0;
    char sRespCode[2 + 1] = {0}, sOrderNo[30 + 1] = {0}, sROrderNo[30 + 1] = {0};
    char sUrl[100 + 1] = {0}, sTermId[8 + 1] = {0};
    char *pcMsg = NULL;
    int iQrCount, iSecs = 0;
    time_t tmEnd;
    struct timeval tvE;

    cJSON *pstJson = NULL, *pstRecvJson = NULL;
    char sKey[64] = {0};


    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
    TRANS_CODE,\
    AMOUNT,\
    TRANS_DATE, \
    TRANS_TIME, \
    MERCH_ID, \
    TERM_ID, \
    TRACE_NO, \
    RRN, \
    AGENT_ID, \
    MERCH_ORDER_NO, \
    CALLBACK_URL, \
    QUERY_COUNT \
    FROM B_INLINE_TARNS_DETAIL \
    WHERE substr(trans_code,2,3) in('AW2','AB2','AW1','AB1','AY1','AY2') and valid_flag='0' \
    and resp_code='00'and notice_flag ='0' and sysdate-create_time<=10/60/24 ");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        pstJson = cJSON_CreateObject();
        if (NULL == pstJson) {
            tLog(ERROR, "��������Jsonʧ��.");
            return 0;
        }

        STRV(pstRes, 1, sTransCode);
        DOUV(pstRes, 2, dAmount);
        STRV(pstRes, 3, sTransDate);
        STRV(pstRes, 4, sTransTime);
        STRV(pstRes, 5, sMerchId);
        STRV(pstRes, 6, sTermId);
        STRV(pstRes, 7, sTraceNo);
        STRV(pstRes, 8, sRrn);
        STRV(pstRes, 9, sAgentId);
        STRV(pstRes, 10, sOrderNo);
        STRV(pstRes, 11, sUrl);
        INTV(pstRes, 12, iQrCount);
        //STRV(pstRes, 12, sPlatcode);
        if (sTransCode[2] == 'W') {
            SET_STR_KEY(pstJson, "payMethod", "WXPAY");
        } else if (sTransCode[2] == 'B') {
            SET_STR_KEY(pstJson, "payMethod", "ALIPAY");
        }

        //tLog(INFO, "test1[%s].", sOrderNo);
        SET_STR_KEY(pstJson, "merchantNo", sMerchId);
        //        SET_STR_KEY(pstJson, "merchantName", sMerchName);
        //SET_STR_KEY(pstJson, "traceNo", sTraceNo);
        //SET_STR_KEY(pstJson, "rrn", sRrn);
        //SET_STR_KEY(pstJson, "trans_date", sTransDate);
        //SET_STR_KEY(pstJson, "trans_time", sTransTime);
        //SET_STR_KEY(pstJson, "term_id", sTermId);
        //strcpy(sTransDateTime, sTransDate);
        //strcat(sTransDateTime, sTransTime);
        //SET_STR_KEY(pstJson, "tranTime", sTransDateTime);
        SET_STR_KEY(pstJson, "orderNo", sOrderNo);
        sprintf(sAmount, "%.2f", dAmount);
        //SET_DOU_KEY(pstJson, "amount", dAmount);
        SET_STR_KEY(pstJson, "amount", sAmount);
        SET_STR_KEY(pstJson, "respCode", "00");
        //SET_STR_KEY(pstJson, "rrn", sRrn);
        //SET_STR_KEY(pstJson, "platCode", sPlatcode);
        //SET_STR_KEY(pstJson, "url", sUrl);
        //SET_INT_KEY(pstJson, "query_count", iQrCount);

        tLog(INFO, "��������ά����ˮ��Ϣ.");
        pcMsg = cJSON_PrintUnformatted(pstJson);
        //tSendMsg("QR_CALLBACK_Q", pcMsg, strlen(pcMsg));
        if (strlen(sUrl) == 0) {
            tLog(ERROR, "�ص�UrlΪ���޷���������");
        } else {
            tLog(ERROR, "��[%s]����http����,json����Ϊ[%s]", sUrl, pcMsg);
            if (HttpRequest(sUrl, pcMsg, sHttpResponse) < 0) {
                tLog(ERROR, "http����ʧ��");
            } else {
                if (strstr(sHttpResponse, "null") != NULL) {
                    tLog(ERROR, "http���������ﺬnull");
                } else {
                    pstRecvJson = cJSON_Parse(sHttpResponse);
                    GET_STR_KEY(pstRecvJson, "respCode", sRespCode);
                    GET_STR_KEY(pstRecvJson, "orderNo", sROrderNo);
                    if (memcmp(sRespCode, "00", 2) == 0) {
                        /*���¶�ά��ص���־*/
                        if (UpdQRNoticeFlag(pstRecvJson) < 0) {
                            tLog(ERROR, "��ά����»ص���־ʧ�ܣ�");
                        } else {
                            tLog(ERROR, "��ά��ص�[%s]�����ͳɹ���", sROrderNo);
                        }
                    } else {
                        tLog(ERROR, "��ά�붩��[%s]�ص�ʧ��,������[%s]", sROrderNo, sRespCode);
                    }
                    cJSON_Delete(pstRecvJson);
                }

            }

        }
        cJSON_Delete(pstJson);

    }
    if (0 == OCI_GetRowCount(pstRes)) {
        tLog(ERROR, "δ�����������͵Ķ�ά����ˮ.....");
        tReleaseRes(pstRes);
        return -2;
    }
    tReleaseRes(pstRes);
    return 0;
}
//�ɹ�ʧ����ˮ����Ҫ����

int ScanEposTrans(struct timeval *tvS) {

    char sSqlStr[2048];
    char sRrn[RRN_LEN + 1] = {0}, sTransCode[TRANS_CODE_LEN + 1] = {0};
    char sTransDate[TRANS_DATE_LEN] = {0}, sTransTime[TRANS_TIME_LEN + 1] = {0};
    char sMerchId[MERCH_ID_LEN + 1] = {0}, sHttpResponse[2048 + 1] = {0};
    char sTraceNo[TRACE_NO_LEN + 1] = {0}, sRespDesc[255] = {0}, sRespDescUTF8[500] = {0};
    char sAgentId[INST_ID_LEN + 1] = {0}, sAmount[14 + 1] = {0};
    double dAmount = 0;
    char sRespCode[2 + 1] = {0}, sValidFlag[1 + 1] = {0}, sOrderNo[30 + 1] = {0}, sROrderNo[30 + 1] = {0};
    char sUrl[100 + 1] = {0}, sTermId[8 + 1] = {0};
    char *pcMsg = NULL;
    int iQrCount, iSecs = 0;
    time_t tmEnd;
    struct timeval tvE;

    cJSON *pstJson = NULL, *pstRecvJson = NULL;
    char sKey[64] = {0};


    OCI_Resultset *pstRes = NULL;
    snprintf(sSqlStr, sizeof (sSqlStr), "SELECT  \
    TRANS_CODE,\
    AMOUNT,\
    TRANS_DATE, \
    TRANS_TIME, \
    MERCH_ID, \
    TERM_ID, \
    TRACE_NO, \
    RRN, \
    AGENT_ID, \
    MERCH_ORDER_NO, \
    CALLBACK_URL, \
    RESP_DESC, \
    RESP_CODE, \
    VALID_FLAG, \
    QUERY_COUNT \
    FROM B_INLINE_TARNS_DETAIL \
    WHERE substr(trans_code,2,3) in('AQ0') and istresp_code is not null and rrn is not null\
    and notice_flag ='2' and sysdate-create_time<=10/60/24 ");
    if (tExecute(&pstRes, sSqlStr) < 0) {
        tLog(ERROR, "sql[%s] err!", sSqlStr);
        return -1;
    }

    if (NULL == pstRes) {
        tLog(ERROR, "sql[%s]�����ΪNULL.", sSqlStr);
        return -1;
    }

    while (OCI_FetchNext(pstRes)) {

        pstJson = cJSON_CreateObject();
        if (NULL == pstJson) {
            tLog(ERROR, "��������Jsonʧ��.");
            return 0;
        }

        STRV(pstRes, 1, sTransCode);
        DOUV(pstRes, 2, dAmount);
        STRV(pstRes, 3, sTransDate);
        STRV(pstRes, 4, sTransTime);
        STRV(pstRes, 5, sMerchId);
        STRV(pstRes, 6, sTermId);
        STRV(pstRes, 7, sTraceNo);
        STRV(pstRes, 8, sRrn);
        STRV(pstRes, 9, sAgentId);
        STRV(pstRes, 10, sOrderNo);
        STRV(pstRes, 11, sUrl);
        STRV(pstRes, 12, sRespDesc);
        STRV(pstRes, 13, sRespCode);
        STRV(pstRes, 14, sValidFlag);
        INTV(pstRes, 15, iQrCount);
        //STRV(pstRes, 12, sPlatcode);
        SET_STR_KEY(pstJson, "payMethod", "EPOS");
        //tLog(INFO, "test1[%s].", sOrderNo);
        SET_STR_KEY(pstJson, "merchantNo", sMerchId);
        //        SET_STR_KEY(pstJson, "merchantName", sMerchName);
        //SET_STR_KEY(pstJson, "traceNo", sTraceNo);
        //SET_STR_KEY(pstJson, "rrn", sRrn);
        //SET_STR_KEY(pstJson, "trans_date", sTransDate);
        //SET_STR_KEY(pstJson, "trans_time", sTransTime);
        //SET_STR_KEY(pstJson, "term_id", sTermId);
        //strcpy(sTransDateTime, sTransDate);
        //strcat(sTransDateTime, sTransTime);
        //SET_STR_KEY(pstJson, "tranTime", sTransDateTime);
        SET_STR_KEY(pstJson, "orderNo", sRrn);
        sprintf(sAmount, "%.2f", dAmount);
        //SET_DOU_KEY(pstJson, "amount", dAmount);
        SET_STR_KEY(pstJson, "amount", sAmount);
        if (sValidFlag[0] == '4' && !memcmp(sRespCode, "00", 2)) {
            SET_STR_KEY(pstJson, "respCode", "96");
            //            SET_STR_KEY(pstJson, "respDesc", "δ�յ�����֪ͨ");
            code_convert("gb2312", "utf-8", "δ�յ�����֪ͨ", strlen("δ�յ�����֪ͨ"), sRespDescUTF8, sizeof (sRespDescUTF8));
            tLog(INFO, "gb2312[%s].utf-8[%s]", sRespDesc, sRespDescUTF8);
            SET_STR_KEY(pstJson, "respDesc", sRespDescUTF8);
        } else {
            SET_STR_KEY(pstJson, "respCode", sRespCode);
            code_convert("gb2312", "utf-8", sRespDesc, strlen(sRespDesc), sRespDescUTF8, sizeof (sRespDescUTF8));
            tLog(INFO, "gb2312[%s].utf-8[%s]", sRespDesc, sRespDescUTF8);
            SET_STR_KEY(pstJson, "respDesc", sRespDescUTF8);
        }




        //SET_STR_KEY(pstJson, "rrn", sRrn);
        //SET_STR_KEY(pstJson, "platCode", sPlatcode);
        //SET_STR_KEY(pstJson, "url", sUrl);
        //SET_INT_KEY(pstJson, "query_count", iQrCount);

        tLog(INFO, "������Epos��ˮ��Ϣ.");
        pcMsg = cJSON_PrintUnformatted(pstJson);
        //tSendMsg("QR_CALLBACK_Q", pcMsg, strlen(pcMsg));
        //����url
        if (FindValueByKey(sUrl, "epos.callback.url") < 0) {
            tLog(ERROR, "����key[epos.callback.url]�ص���ַ,ʧ��.");
            return -1;
        }
        tLog(ERROR, "��[%s]����http����,json����Ϊ[%s]", sUrl, pcMsg);
        if (HttpRequest(sUrl, pcMsg, sHttpResponse) < 0) {
            tLog(ERROR, "http����ʧ��");
        } else {
            if (strstr(sHttpResponse, "null") != NULL) {
                tLog(ERROR, "http���������ﺬnull");
            } else {
                pstRecvJson = cJSON_Parse(sHttpResponse);
                GET_STR_KEY(pstRecvJson, "respCode", sRespCode);
                GET_STR_KEY(pstRecvJson, "orderNo", sROrderNo);
                if (memcmp(sRespCode, "00", 2) == 0) {
                    /*����Epos�ص���־*/
                    if (UpdEPOSNoticeFlag(pstRecvJson) < 0) {
                        tLog(ERROR, "epos���»ص���־ʧ�ܣ�");
                    } else {
                        tLog(ERROR, "epos�ص�[%s]�����ͳɹ���", sROrderNo);
                    }
                } else {
                    tLog(ERROR, "epos����[%s]�ص�ʧ��,������[%s]", sROrderNo, sRespCode);
                }
                cJSON_Delete(pstRecvJson);
            }

        }

        cJSON_Delete(pstJson);
    }


    if (0 == OCI_GetRowCount(pstRes)) {
        //����ʱ��
        time(&tmEnd);
        gettimeofday(&tvE, NULL);
        iSecs = tvE.tv_sec - (*tvS).tv_sec;
        //5���ӡһ����־��ֹ������־��̫��
        if (iSecs > 5) {
            *tvS = tvE;
            tLog(ERROR, "δ�����������͵�Epos��ˮ.....");
        }
        tReleaseRes(pstRes);
        return -2;
    }
    tReleaseRes(pstRes);
    return 0;
}