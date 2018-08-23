/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/******************************************************************************/
/*      函数名:     TransferJson()                                                */
/*      功能说明:   交易路由检查，发送和接受                                  */
/*      输入参数:   TranData *pstTranData                                     */
/*      输出参数:                                                             */
/*      返回说明:   0 成功  非 0 不成功                                       */

/******************************************************************************/
int TransferJson(cJSON *pstTranData, int *piFlag) {
#if 0
    NetTran *pstNetTran;
	Qrcode stQrcode, stOut;
    int iOutLen = 0, iRet = -1,iConvRet = -1;
    char sSvcName[30 + 1] = {0}, sErr[128] = {0};
    char sPTTransCode[7] = {0};
    char sOldPTTransCode[7] = {0};
    char sOoldPTTransCode[7] = {0};
    memset(&stOut, 0x00, sizeof(Qrcode));
	memset(&stQrcode, 0x00, sizeof(Qrcode));

	pstNetTran = &pstTranData->stNetTran;
	iRet = NetTran2Qrcode( &stQrcode, pstNetTran );
	if(iRet < 0){
		SetRespCode(pstNetTran, "96", pstTranData->stSysParam.sInstId);
		return -1;
	}else if(iRet == 2){
		SetRespCode(pstNetTran, "64", pstTranData->stSysParam.sInstId);
		return -1;
	}else if(iRet == 1){
		SetRespCode(pstNetTran, "25", pstTranData->stSysParam.sInstId);
		return -1;

	}
    strcpy(sSvcName,"ISSINT8");
    iRet = mSvcCall(sSvcName, (void *) &stQrcode, sizeof(Qrcode), (void *) &stOut, &iOutLen, 57, 0);

    if (iRet < 0) {
        if (iRet == QUEUE_RCVMSG_TIMEOUT) {
            tErrLog(ERR, "交易[%s]流水号[%s]调用服务[%s]返回超时.", \
                    pstNetTran->sTransCode, pstNetTran->sLocTraceNo, sSvcName);
            FILE *f =fopen("/tmp/timeout","w+");
            if(f != NULL)
            {
                char sdebug[128];
                memset(sdebug,0x00,128);
                sprintf(sdebug,"[%s][%s][%ld][%s]", pstNetTran->sMerchId, pstNetTran->sTermId,time(NULL),pstNetTran->sChannelId);
                fprintf(f,"%s",sdebug);
                fclose(f);
            }
            SetRespCode(pstNetTran, "68", pstTranData->stSysParam.sInstId);
            return -1;
        }
        sprintf(sErr, "交易[%s]调用中间件服务[%s],发卡机构[%s]失败[%d].", \
            pstNetTran->sTransCode, sSvcName, pstNetTran->sIssInstId, iRet);
        tErrLog(ERR, sErr);
        SetRespCode(pstNetTran, "96", pstTranData->stSysParam.sInstId);
        return -1;
    }

	Qrcode2NetTran( &stOut, pstNetTran );
  //  PrtNetTran( pstNetTran );
    /* 查找匹配应答 */
    if (strcmp(pstNetTran->sRrn, stOut.sRrn)) 
	{
            tErrLog(ERR, "发卡机构[%s]服务[%s]发送rrn[%s]返回rrn[%s] 无匹配应答.", \
                    pstNetTran->sIssInstId, sSvcName, pstNetTran->sRrn, stOut.sRrn);
            SetRespCode(pstNetTran, "96", pstTranData->stSysParam.sInstId);
            return -1;
    }

    tErrLog(INFO, "发卡机构[%s]服务[%s]收单流水号[%s]返回应答码[%s] 交易路由成功.", \
                    pstNetTran->sIssInstId, sSvcName, pstNetTran->sLocTraceNo, pstNetTran->sRespCode);
#endif
    return 0;
}
