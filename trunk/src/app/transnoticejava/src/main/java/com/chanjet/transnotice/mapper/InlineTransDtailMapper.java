package com.chanjet.transnotice.mapper;

import com.chanjet.transnotice.entity.InlineTransDetail;
import com.chanjet.transnotice.entity.OtransInfo;
import org.apache.ibatis.annotations.*;

import java.util.List;

/**
 * Created by Gjq on 2018/8/8.
 */
@Mapper
public interface InlineTransDtailMapper {
    @Select("select a.trans_code,a.agent_id,a.trans_date,a.trans_time,a.card_no,a.trace_no,a.sys_trace,a.rrn,a.batch_no," +
            " a.merch_order_no,a.card_type,a.merch_id,a.term_id,b.merch_name,b.mobile,a.amount," +
            " a.fee,a.fee_type,a.settle_date,a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param" +
            " from b_inline_tarns_detail a join b_merch b on a.merch_id = b.merch_id" +
            " join b_agent_function c on b.agent_id = c.agent_id" +
            " left join b_dev_term d on a.term_id = d.term_id" +
            " where c.notice_flag = '1' and a.merch_id = b.merch_id" +
            " and a.trans_code in ('${transCode}')" +
            " and a.notice_status ='0' and a.notice_count < 5 and a.valid_flag in('0','1') and resp_code != 'XX'" )
    @Results({
            @Result(column = "trans_code", property = "sTransCode"),
            @Result(column = "agent_id", property = "sAcqId"),
            @Result(column = "trans_date", property = "sTransDate"),
            @Result(column = "trans_time", property = "sTransTime"),
            @Result(column = "card_no", property = "sCardNo"),
            @Result(column = "trace_no", property = "sTraceNo"),
            @Result(column = "sys_trace", property = "sSysTrace"),
            @Result(column = "rrn", property = "sRrn"),
            @Result(column = "batch_no", property = "sBatchNo"),
            @Result(column = "merch_order_no", property = "sMerchOrderNo"),
            @Result(column = "card_type", property = "sCardType"),
            @Result(column = "merch_id", property = "sMerchId"),
            @Result(column = "term_id", property = "sTermId"),
            @Result(column = "merch_name", property = "sMerchName"),
            @Result(column = "mobile", property = "sMobileNo"),
            @Result(column = "amount", property = "dAmount"),
            @Result(column = "fee", property = "dFee"),
            @Result(column = "fee_type", property = "sFeeType"),
            @Result(column = "settle_date", property = "sSettleDate"),
            @Result(column = "resp_code", property = "sRespCode"),
            @Result(column = "resp_desc", property = "sRespDesc"),
            @Result(column = "o_trans_date", property = "sOTransDate"),
            @Result(column = "o_rrn", property = "sORrn"),
            @Result(column = "agent_param", property = "sAgentParam")
    })
    List<InlineTransDetail> findTransDtailInline(@Param("transCode") String transCode);

    /*获取二维码失败交易流水 30分钟之前的 valid_flag 为 ‘4’ 的交易流水 默认为失败交易*/
    @Select("select a.trans_code,a.agent_id,a.trans_date,a.trans_time,a.card_no,a.trace_no,a.sys_trace,a.rrn,a.batch_no," +
            " a.merch_order_no,a.card_type,a.merch_id,a.term_id,b.merch_name,b.mobile,a.amount," +
            " a.fee,a.fee_type,a.settle_date,a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param" +
            " from b_inline_tarns_detail a join b_merch b on a.merch_id = b.merch_id" +
            " join b_agent_function c on b.agent_id = c.agent_id" +
            " where c.notice_flag = '1' and a.merch_id = b.merch_id" +
            " and a.trans_code in ('${transCode}')" +
            " and a.notice_status ='0' and a.notice_count < 5 and a.valid_flag = '4' and resp_code != 'XX' and sysdate-a.create_time>=30/60/24" )
    @Results({
            @Result(column = "trans_code", property = "sTransCode"),
            @Result(column = "agent_id", property = "sAcqId"),
            @Result(column = "trans_date", property = "sTransDate"),
            @Result(column = "trans_time", property = "sTransTime"),
            @Result(column = "card_no", property = "sCardNo"),
            @Result(column = "trace_no", property = "sTraceNo"),
            @Result(column = "sys_trace", property = "sSysTrace"),
            @Result(column = "rrn", property = "sRrn"),
            @Result(column = "batch_no", property = "sBatchNo"),
            @Result(column = "merch_order_no", property = "sMerchOrderNo"),
            @Result(column = "card_type", property = "sCardType"),
            @Result(column = "merch_id", property = "sMerchId"),
            @Result(column = "term_id", property = "sTermId"),
            @Result(column = "merch_name", property = "sMerchName"),
            @Result(column = "mobile", property = "sMobileNo"),
            @Result(column = "amount", property = "dAmount"),
            @Result(column = "fee", property = "dFee"),
            @Result(column = "fee_type", property = "sFeeType"),
            @Result(column = "settle_date", property = "sSettleDate"),
            @Result(column = "resp_code", property = "sRespCode"),
            @Result(column = "resp_desc", property = "sRespDesc"),
            @Result(column = "o_trans_date", property = "sOTransDate"),
            @Result(column = "o_rrn", property = "sORrn"),
            @Result(column = "agent_param", property = "sAgentParam")
    })
    List<InlineTransDetail> findTransDtailInlineFail(@Param("transCode") String transCode);

    /*日切时间段 扫描二维码历史流水表中前一日的 valid_flag 为 ‘4’的交易流水 为失败交易*/
    @Select("select a.trans_code,a.agent_id,a.trans_date,a.trans_time,a.card_no,a.trace_no,a.sys_trace,a.rrn,a.batch_no," +
            " a.merch_order_no,a.card_type,a.merch_id,a.term_id,b.merch_name,b.mobile,a.amount," +
            " a.fee,a.fee_type,a.settle_date,a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param" +
            " from b_inline_tarns_detail_his a join b_merch b on a.merch_id = b.merch_id" +
            " join b_agent_function c on b.agent_id = c.agent_id" +
            " where c.notice_flag = '1' and a.merch_id = b.merch_id" +
            " and a.trans_code in ('${transCode}')" +
            " and a.notice_status ='0' and a.notice_count < 5 and a.valid_flag = '4' and resp_code != 'XX' and a.trans_date = TO_CHAR( SYSDATE-1,'YYYYMMDD')" )
    @Results({
            @Result(column = "trans_code", property = "sTransCode"),
            @Result(column = "agent_id", property = "sAcqId"),
            @Result(column = "trans_date", property = "sTransDate"),
            @Result(column = "trans_time", property = "sTransTime"),
            @Result(column = "card_no", property = "sCardNo"),
            @Result(column = "trace_no", property = "sTraceNo"),
            @Result(column = "sys_trace", property = "sSysTrace"),
            @Result(column = "rrn", property = "sRrn"),
            @Result(column = "batch_no", property = "sBatchNo"),
            @Result(column = "merch_order_no", property = "sMerchOrderNo"),
            @Result(column = "card_type", property = "sCardType"),
            @Result(column = "merch_id", property = "sMerchId"),
            @Result(column = "term_id", property = "sTermId"),
            @Result(column = "merch_name", property = "sMerchName"),
            @Result(column = "mobile", property = "sMobileNo"),
            @Result(column = "amount", property = "dAmount"),
            @Result(column = "fee", property = "dFee"),
            @Result(column = "fee_type", property = "sFeeType"),
            @Result(column = "settle_date", property = "sSettleDate"),
            @Result(column = "resp_code", property = "sRespCode"),
            @Result(column = "resp_desc", property = "sRespDesc"),
            @Result(column = "o_trans_date", property = "sOTransDate"),
            @Result(column = "o_rrn", property = "sORrn"),
            @Result(column = "agent_param", property = "sAgentParam")
    })
    List<InlineTransDetail> findTransDtailInlineFailHis(@Param("transCode") String transCode);

    @Select("select trace_no,batch_no from b_inline_tarns_detail" +
            " where rrn = #{rrn,jdbcType=VARCHAR} and trans_date = #{transDate,jdbcType=VARCHAR}")
    @Results({
            @Result(property="sOtraceNo",column="trace_no"),
            @Result(property="sObatchNo",column="batch_no")
    })
    OtransInfo findOTransInfoInlineByParam(@Param("rrn") String rrn, @Param("transDate") String transDate);
}
