#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"



/******************************************************************************/
/*      ������:     GetUrl()                                           */
/*      ����˵��:   ����Ԥ��������                                              */
/*      �������:   TranData *pstTranData                                     */
/*      �������:                                                             */
/*      ����˵��:   0 �ɹ�  �� 0 ���ɹ�                                       */

/******************************************************************************/
int GetUrl(cJSON *pstJson, int *piFlag) {

    char sErr[128] = {0};
	char sSql[2048];
    int iRet;
    cJSON * pstTransJson = NULL;
    
pstTransJson = GET_JSON_KEY(pstJson, "data");
    if ( GenUrl( pstTransJson ) != 0 )
    {
        ErrHanding(pstTransJson, "96","ȡ��RRN��Ӧ��URL��ַʧ��");
        return -1;
    }

    return 0;
}
