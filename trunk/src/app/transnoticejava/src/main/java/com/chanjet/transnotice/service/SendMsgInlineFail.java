package com.chanjet.transnotice.service;

import com.alibaba.fastjson.JSONObject;
import com.chanjet.transnotice.entity.*;
import com.chanjet.transnotice.mapper.InlineTransDtailMapper;
import com.chanjet.transnotice.mapper.NoticeStatMapper;
import com.chanjet.transnotice.mapper.TermInfoMapper;
import com.chanjet.transnotice.sdk.Signature;
import com.chanjet.transnotice.sdk.HttpClient;
import com.chanjet.transnotice.sdk.Sort;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

@Service
public class SendMsgInlineFail {
    private static final Logger logger = LoggerFactory.getLogger(SendMsgInlineFail.class);

    @Autowired
    private InlineTransDtailMapper inlineTransDtailMapper;
    @Autowired
    private TermInfoMapper termInfoMapper;
    @Autowired
    private NoticeStatMapper noticeStatMapper;

    @Value("${key}")
    private String key;

    @Value("${inlinetranscode}")
    private String transCode;

    @Transactional(rollbackFor = Exception.class)
    public void sendMsgInlineFailSplitting() throws Exception {
        List<InlineTransDetail> list;
        String tableName;

        SimpleDateFormat df = new SimpleDateFormat("HHmmss");//�������ڸ�ʽ
        String sCurTime =  df.format(new Date());
        if ( sCurTime.compareTo("000000") >= 0 && sCurTime.compareTo("001000") <= 0 ) {
            logger.info("��ǰʱ��sCurTime:["+ sCurTime +"]��ɨ����ʷ������ǰһ�յĶ�ά�뽻��ʧ�ܵ�֪ͨ["+ transCode +"]");
            list = this.inlineTransDtailMapper.findTransDtailInlineFailHis(transCode);
            tableName = "b_inline_tarns_detail_his";
        }
        else {
            logger.info("��ǰʱ��sCurTime:["+ sCurTime +"]��ɨ�赱ǰ��ˮ���еĶ�ά�뽻��ʧ�ܵ�֪ͨ["+ transCode +"]");
            list = this.inlineTransDtailMapper.findTransDtailInlineFail(transCode);
            tableName = "b_inline_tarns_detail";
        }

        logger.debug(" list.size : " + list.size());
        for(InlineTransDetail inlineTransDetail : list){

            String jsonString = JSONObject.toJSONString(inlineTransDetail);
            logger.info("��ȡ���Ľ��׼�¼ ��[" + jsonString +"]");
            logger.info("--------------------------��ʼ��������Ϣ[" + inlineTransDetail.getsRrn() + "]--------------------------");
            SendMsg sendMsg = new SendMsg();
            sendMsg.setAmount(String.format("%.0f",inlineTransDetail.getdAmount()*100));

            sendMsg.setBatchNo(inlineTransDetail.getsBatchNo());
            sendMsg.setCardNo(inlineTransDetail.getsCardNo());
            sendMsg.setCardType(inlineTransDetail.getsCardType());

            sendMsg.setFeeType(inlineTransDetail.getsFeeType());
            sendMsg.setMerchantId(inlineTransDetail.getsMerchId());
            sendMsg.setMerchantName(inlineTransDetail.getsMerchName());
            sendMsg.setMobileNo(inlineTransDetail.getsMobileNo());

            sendMsg.setOrderId(inlineTransDetail.getsMerchOrderNo());

            /*������Ϣ��Ҫ��ԭ�����л�ȡ*/
            logger.info( "sTransCode ��4λ��" + inlineTransDetail.getsTransCode().substring(3,4).toString() );
            /* posˢ�� �������� �ӵ�ǰ��ˮ���л�ȡ����ԭ������Ϣ */
            if( "6".equals(inlineTransDetail.getsTransCode().substring(3,4)) ) {
                sendMsg.setOriginalRrn(inlineTransDetail.getsORrn());
                sendMsg.setOriginalTranDate(inlineTransDetail.getsTransDate());

                OtransInfo oposTransInfo = this.inlineTransDtailMapper.findOTransInfoInlineByParam(inlineTransDetail.getsORrn(),inlineTransDetail.getsTransDate());

                logger.info("OposTransInfo : ["+ JSONObject.toJSONString(oposTransInfo) +"]");
                sendMsg.setOriginalbatchNo(oposTransInfo.getsObatchNo());
                sendMsg.setOriginaltraceNo(oposTransInfo.getsOtraceNo());
            }

            sendMsg.setPlatCode(inlineTransDetail.getsAcqId());
            sendMsg.setRrn(inlineTransDetail.getsRrn());
            sendMsg.setSettleAmount(String.format("%.0f",(inlineTransDetail.getdAmount()-inlineTransDetail.getdFee())*100));

            sendMsg.setSettleDate(inlineTransDetail.getsSettleDate());
            /*��ά��ʧ�ܽ��� Ӧ����Ϣ*/
            if( "00".equals(inlineTransDetail.getsRespCode()) ) {
                sendMsg.setSysRespCode("R3");
                sendMsg.setSysRespDesc("δ֧��");
            }
            else {
                sendMsg.setSysRespCode(inlineTransDetail.getsRespCode());
                sendMsg.setSysRespDesc(inlineTransDetail.getsRespDesc().replaceAll("\r|\n", "").trim());
            }

            sendMsg.setSysTraceNo(inlineTransDetail.getsTraceNo());

            //��ȡ�ն���Ϣ
            if( inlineTransDetail.getsTermId() != null) {
                sendMsg.setTermId(inlineTransDetail.getsTermId());
                TermInfo termInfo = this.termInfoMapper.findTermInfoByParam(inlineTransDetail.getsTermId());
                sendMsg.setTermModel(termInfo.getsTermModel());
                sendMsg.setTermSn(termInfo.getsTermSn());
            }

            sendMsg.setTraceNo(inlineTransDetail.getsTraceNo());
            sendMsg.setTranCode(inlineTransDetail.getsTransCode());
            sendMsg.setTranTime(inlineTransDetail.getsTransDate()+inlineTransDetail.getsTransTime());

            logger.debug("key : ["+ key+"]");
            /*����key����ĸ˳������*/
            String sMsg = Sort.sort(sendMsg);
            logger.debug("sMsg : ["+ sMsg+"]");
            sendMsg.setSign(Signature.getMD5(key,sMsg));
            logger.debug("getSign : ["+ sendMsg.getSign() +"]");

            logger.info("------------------------������Ϣ�������[" + inlineTransDetail.getsRrn() + "]------------------------");

            logger.info("--------------------------����֪ͨ��ʼ[" + inlineTransDetail.getsRrn() + "]--------------------------");
            String sUrl = inlineTransDetail.getsAgentParam();
            String sendJsonString = JSONObject.toJSONString(sendMsg);
            logger.info("Send Msg : [" + sendJsonString + "] to [" + sUrl + "]");
            RecvMsg recvMsg = HttpClient.post(sendJsonString,sUrl);
            logger.info("recvMsg : [" + JSONObject.toJSONString(recvMsg) + "]");
            logger.info("--------------------------����֪ͨ���[" + inlineTransDetail.getsRrn() + "]--------------------------");

            /*֪ͨ�ɹ������ �������׼�¼�� ֪ͨ״̬notice_status*/
            if ( recvMsg.getResponseCode() != null && "00".equals(recvMsg.getResponseCode()) ) {
                this.noticeStatMapper.UpdNoticeStatByParam(tableName,inlineTransDetail.getsRrn(),inlineTransDetail.getsTransDate());
            }

            /*֪ͨʧ�� ����֪ͨ���ʹ���*/
            if ( recvMsg.getResponseCode() == null || !"00".equals(recvMsg.getResponseCode()) ) {
                this.noticeStatMapper.UpdNoticeCntByParam(tableName,inlineTransDetail.getsRrn(),inlineTransDetail.getsTransDate());
            }
        }
    }
}