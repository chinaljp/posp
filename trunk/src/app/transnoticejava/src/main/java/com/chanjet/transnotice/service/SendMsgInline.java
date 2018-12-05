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

import java.util.List;

@Service
public class SendMsgInline {
    private static final Logger logger = LoggerFactory.getLogger(SendMsgInline.class);

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
    public void sendMsgInlineSplitting() throws Exception {
        logger.info("ɨ�轻����ˮ��Ķ�ά�뽻����ˮ["+ transCode +"]");
        List<InlineTransDetail> list = this.inlineTransDtailMapper.findTransDtailInline(transCode);
        logger.debug(" list.size : " + list.size());
        for(InlineTransDetail inlineTransDetail : list){

            String jsonString = JSONObject.toJSONString(inlineTransDetail);
            logger.info("��ȡ���Ľ��׼�¼ ��[" + jsonString +"]");
            logger.info("--------------------------��ʼ��������Ϣ[" + inlineTransDetail.getsRrn() + "]--------------------------");
            SendMsg sendMsg = new SendMsg();
            //logger.info("dAmount = " + posTransDetail.getdAmount() );
            //logger.info("sAmount = " + String.format("%.2f",posTransDetail.getdAmount()));
            //���׽�� ��λ����
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
            if( inlineTransDetail.getsTransCode().substring(3,4).equals("6") ) {
                //sendMsg.setOriginalAuthCode("");//��ά�뽻������Ȩ��ֱ���� ""
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

            sendMsg.setSysRespCode(inlineTransDetail.getsRespCode());
            sendMsg.setSysRespDesc(inlineTransDetail.getsRespDesc());
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
            logger.info("sMsg : ["+ sMsg+"]");
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
            if ( recvMsg.getResponseCode() != null && recvMsg.getResponseCode().equals("00") ) {
                this.noticeStatMapper.UpdNoticeStatByParam("b_inline_tarns_detail",inlineTransDetail.getsRrn(),inlineTransDetail.getsTransDate());
            }

            /*֪ͨʧ�� ����֪ͨ���ʹ���*/
            if ( recvMsg.getResponseCode() == null || !recvMsg.getResponseCode().equals("00") ) {
                this.noticeStatMapper.UpdNoticeCntByParam("b_inline_tarns_detail",inlineTransDetail.getsRrn(),inlineTransDetail.getsTransDate());
            }
        }
    }
}
