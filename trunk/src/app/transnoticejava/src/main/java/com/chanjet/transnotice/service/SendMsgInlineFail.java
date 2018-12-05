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

        SimpleDateFormat df = new SimpleDateFormat("HHmmss");//设置日期格式
        String sCurTime =  df.format(new Date());
        if ( sCurTime.compareTo("000000") >= 0 && sCurTime.compareTo("001000") <= 0 ) {
            logger.info("当前时间sCurTime:["+ sCurTime +"]，扫描历史表推送前一日的二维码交易失败的通知["+ transCode +"]");
            list = this.inlineTransDtailMapper.findTransDtailInlineFailHis(transCode);
            tableName = "b_inline_tarns_detail_his";
        }
        else {
            logger.info("当前时间sCurTime:["+ sCurTime +"]，扫描当前流水表中的二维码交易失败的通知["+ transCode +"]");
            list = this.inlineTransDtailMapper.findTransDtailInlineFail(transCode);
            tableName = "b_inline_tarns_detail";
        }

        logger.debug(" list.size : " + list.size());
        for(InlineTransDetail inlineTransDetail : list){

            String jsonString = JSONObject.toJSONString(inlineTransDetail);
            logger.info("获取到的交易记录 ：[" + jsonString +"]");
            logger.info("--------------------------开始处理交易信息[" + inlineTransDetail.getsRrn() + "]--------------------------");
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

            /*部分信息需要从原交易中获取*/
            logger.info( "sTransCode 第4位：" + inlineTransDetail.getsTransCode().substring(3,4).toString() );
            /* pos刷卡 撤销交易 从当前流水表中获取部分原交易信息 */
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
            /*二维码失败交易 应答信息*/
            if( "00".equals(inlineTransDetail.getsRespCode()) ) {
                sendMsg.setSysRespCode("R3");
                sendMsg.setSysRespDesc("未支付");
            }
            else {
                sendMsg.setSysRespCode(inlineTransDetail.getsRespCode());
                sendMsg.setSysRespDesc(inlineTransDetail.getsRespDesc().replaceAll("\r|\n", "").trim());
            }

            sendMsg.setSysTraceNo(inlineTransDetail.getsTraceNo());

            //获取终端信息
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
            /*按照key首字母顺序排序*/
            String sMsg = Sort.sort(sendMsg);
            logger.debug("sMsg : ["+ sMsg+"]");
            sendMsg.setSign(Signature.getMD5(key,sMsg));
            logger.debug("getSign : ["+ sendMsg.getSign() +"]");

            logger.info("------------------------交易信息处理完成[" + inlineTransDetail.getsRrn() + "]------------------------");

            logger.info("--------------------------发送通知开始[" + inlineTransDetail.getsRrn() + "]--------------------------");
            String sUrl = inlineTransDetail.getsAgentParam();
            String sendJsonString = JSONObject.toJSONString(sendMsg);
            logger.info("Send Msg : [" + sendJsonString + "] to [" + sUrl + "]");
            RecvMsg recvMsg = HttpClient.post(sendJsonString,sUrl);
            logger.info("recvMsg : [" + JSONObject.toJSONString(recvMsg) + "]");
            logger.info("--------------------------发送通知完成[" + inlineTransDetail.getsRrn() + "]--------------------------");

            /*通知成功需更新 本条交易记录的 通知状态notice_status*/
            if ( recvMsg.getResponseCode() != null && "00".equals(recvMsg.getResponseCode()) ) {
                this.noticeStatMapper.UpdNoticeStatByParam(tableName,inlineTransDetail.getsRrn(),inlineTransDetail.getsTransDate());
            }

            /*通知失败 更新通知推送次数*/
            if ( recvMsg.getResponseCode() == null || !"00".equals(recvMsg.getResponseCode()) ) {
                this.noticeStatMapper.UpdNoticeCntByParam(tableName,inlineTransDetail.getsRrn(),inlineTransDetail.getsTransDate());
            }
        }
    }
}