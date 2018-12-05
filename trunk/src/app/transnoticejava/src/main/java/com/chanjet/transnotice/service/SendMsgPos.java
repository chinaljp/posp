package com.chanjet.transnotice.service;

import com.alibaba.fastjson.JSONObject;
import com.chanjet.transnotice.entity.*;
import com.chanjet.transnotice.mapper.NoticeStatMapper;
import com.chanjet.transnotice.mapper.PosTransDetailMapper;
import com.chanjet.transnotice.mapper.TermInfoMapper;
import com.chanjet.transnotice.sdk.HttpClient;
import com.chanjet.transnotice.sdk.Signature;
import com.chanjet.transnotice.sdk.Sort;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;

@Service
public class SendMsgPos {
    private static final Logger logger = LoggerFactory.getLogger(SendMsgPos.class);

    @Autowired
    private PosTransDetailMapper posTransDetailMapper;
    @Autowired
    private TermInfoMapper termInfoMapper;
    @Autowired
    private NoticeStatMapper noticeStatMapper;

    @Value("${key}")
    private String key;

    @Value("${postranscode}")
    private String transCode;

    /**
     * @Title: sendMsgPosSplitting
     * @Description: 
     * @Author: GuoJiaqing
     * @Date: Create at 2018/8/30 13:09
     * @Param: []
     * @return: void
     */
    @Transactional(rollbackFor = Exception.class)
    public void sendMsgPosSplitting() throws Exception {
        logger.info("ɨ�轻����ˮ���pos������ˮ["+ transCode +"]");
        //System.out.println("ɨ�轻����ˮ���pos������ˮ["+ transCode +"]");
        List<PosTransDetail> list = this.posTransDetailMapper.findTransDetailPos(transCode);
        logger.debug(" list.size : " + list.size());
        for(PosTransDetail posTransDetail : list){

            String jsonString = JSONObject.toJSONString(posTransDetail);
            logger.info("��ȡ���Ľ��׼�¼ ��[" + jsonString +"]");
            logger.info("--------------------------��ʼ��������Ϣ[" + posTransDetail.getsRrn() + "]--------------------------");
            SendMsg sendMsg = new SendMsg();
            //logger.info("dAmount = " + posTransDetail.getdAmount() );
            //logger.info("sAmount = " + String.format("%.2f",posTransDetail.getdAmount()));
            //��ȡ��λС��λ�ַ���
            //sendMsg.setAmount(String.format("%.2f",posTransDetail.getdAmount()));
            sendMsg.setAmount(String.format("%.0f",posTransDetail.getdAmount()*100)); //����ֶη�����˵������λΪ��

            sendMsg.setAuthCode(posTransDetail.getsAuthCode());
            sendMsg.setBatchNo(posTransDetail.getsBatchNo());
            sendMsg.setCardNo(posTransDetail.getsCardNo());
            sendMsg.setCardType(posTransDetail.getsCardType());
            sendMsg.setFeeType(posTransDetail.getsFeeType());
            sendMsg.setInputMode(posTransDetail.getsInputMode());
            sendMsg.setMerchantId(posTransDetail.getsMerchId());
            sendMsg.setMerchantName(posTransDetail.getsMerchName());
            sendMsg.setMobileNo(posTransDetail.getsMobileNo());
            sendMsg.setOrderId(posTransDetail.getsMerchOrderNo());

            /*������Ϣ��Ҫ��ԭ�����л�ȡ*/
            logger.info( "sTransCode ��6λ��" + posTransDetail.getsTransCode().substring(5,6).toString() );
            /* posˢ�� �������� �ӵ�ǰ��ˮ���л�ȡ����ԭ������Ϣ */
            if( "2".equals(posTransDetail.getsTransCode().substring(5,6)) && !"25".equals(posTransDetail.getsRespCode()) ) {

                sendMsg.setOriginalRrn(posTransDetail.getsORrn());
                sendMsg.setOriginalTranDate(posTransDetail.getsOTransDate());
                //OtransInfo oposTransInfo = new OtransInfo();
                OtransInfo oposTransInfo = this.posTransDetailMapper.findOTransInfoPosByParam("b_pos_trans_detail",posTransDetail.getsORrn(),posTransDetail.getsOTransDate());

                logger.info("OposTransInfo : ["+ JSONObject.toJSONString(oposTransInfo) +"]");
                sendMsg.setOriginalAuthCode(oposTransInfo.getsOauthCode());
                sendMsg.setOriginalbatchNo(oposTransInfo.getsObatchNo());
                sendMsg.setOriginaltraceNo(oposTransInfo.getsOtraceNo());
            }
            /*  posˢ�� �˻����� ����ʷ��ˮ���л�ȡ����ԭ������Ϣ */
            else if( "1".equals(posTransDetail.getsTransCode().substring(5,6)) && !"25".equals(posTransDetail.getsRespCode()) ) {
                sendMsg.setOriginalRrn(posTransDetail.getsORrn());
                sendMsg.setOriginalTranDate(posTransDetail.getsOTransDate());
                //OtransInfo oposTransInfo = new OtransInfo();
                OtransInfo oposTransInfo = this.posTransDetailMapper.findOTransInfoPosByParam("b_pos_trans_detail_his",posTransDetail.getsORrn(),posTransDetail.getsOTransDate());

                logger.info("OposTransInfo : ["+ JSONObject.toJSONString(oposTransInfo) +"]");
                sendMsg.setOriginalAuthCode(oposTransInfo.getsOauthCode());
                sendMsg.setOriginalbatchNo(oposTransInfo.getsObatchNo());
                sendMsg.setOriginaltraceNo(oposTransInfo.getsOtraceNo());
            }

            sendMsg.setPlatCode(posTransDetail.getsAcqId());
            sendMsg.setRrn(posTransDetail.getsRrn());

            sendMsg.setSettleAmount(String.format("%.0f",(posTransDetail.getdAmount()-posTransDetail.getdFee())*100));

            sendMsg.setSettleDate(posTransDetail.getsSettleDate());

            sendMsg.setSysRespCode(posTransDetail.getsRespCode());
            sendMsg.setSysRespDesc(posTransDetail.getsRespDesc().replaceAll("\r|\n", "").trim());
            sendMsg.setSysTraceNo(posTransDetail.getsTraceNo());
            sendMsg.setTermId(posTransDetail.getsTermId());

            //��ȡ�ն���Ϣ
            TermInfo termInfo = this.termInfoMapper.findTermInfoByParam(posTransDetail.getsTermId());
            sendMsg.setTermModel(termInfo.getsTermModel());
            sendMsg.setTermSn(termInfo.getsTermSn());

            sendMsg.setTraceNo(posTransDetail.getsTraceNo());
            sendMsg.setTranCode(posTransDetail.getsTransCode());
            sendMsg.setTranTime(posTransDetail.getsTransDate()+posTransDetail.getsTransTime());

            logger.debug("key : ["+ key+"]");
            /*����key����ĸ˳������*/
            String sMsg = Sort.sort(sendMsg);
            logger.debug("sMsg : ["+ sMsg+"]");
            sendMsg.setSign(Signature.getMD5(key,sMsg));
            //logger.debug("getSign : ["+ sendMsg.getSign() +"]");
            logger.info("------------------------������Ϣ�������[" + posTransDetail.getsRrn() + "]------------------------");

            logger.info("--------------------------����֪ͨ��ʼ[" + posTransDetail.getsRrn() + "]--------------------------");
            String sUrl = posTransDetail.getsAgentParam();
            String sendJsonString = JSONObject.toJSONString(sendMsg);
            logger.info("Send Msg : [" + sendJsonString + "] to [" + sUrl + "]");
            RecvMsg recvMsg = HttpClient.post(sendJsonString,sUrl);
            logger.info("recvMsg : [" + JSONObject.toJSONString(recvMsg) + "]");
            logger.info("--------------------------����֪ͨ���[" + posTransDetail.getsRrn() + "]--------------------------");

            //�������ݲ���Ҫ��֤ǩ��
            //Signature.verificationSign(key,recvMsg)
            /*֪ͨ�ɹ������ �������׼�¼�� ֪ͨ״̬notice_status*/
            if (recvMsg.getResponseCode() != null && recvMsg.getResponseCode().equals("00")) {
                this.noticeStatMapper.UpdNoticeStatByParam("b_pos_trans_detail", posTransDetail.getsRrn(), posTransDetail.getsTransDate());
            }

            /*֪ͨʧ�� ����֪ͨ���ʹ���*/
            if (recvMsg.getResponseCode() == null || !recvMsg.getResponseCode().equals("00")) {
                this.noticeStatMapper.UpdNoticeCntByParam("b_pos_trans_detail", posTransDetail.getsRrn(), posTransDetail.getsTransDate());
            }
        }

    }
}
