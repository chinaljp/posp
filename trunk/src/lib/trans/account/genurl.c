#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_tools.h"
#include "t_log.h"
#include "t_db.h"
#include "t_cjson.h"



/******************************************************************************/
/*      函数名:     GetUrl()                                           */
/*      功能说明:   解析预付卡类型                                              */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int GetUrl(cJSON *pstJson, int *piFlag) {

    char sErr[128] = {0};
	char sSql[2048];
    int iRet;
    cJSON * pstTransJson = NULL;
    
pstTransJson = GET_JSON_KEY(pstJson, "data");
    if ( GenUrl( pstTransJson ) != 0 )
    {
        ErrHanding(pstTransJson, "96","取得RRN对应的URL地址失败");
        return -1;
    }

    return 0;
}
