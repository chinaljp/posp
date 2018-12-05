package com.chanjet.transnotice.mapper;

import com.chanjet.transnotice.entity.OtransInfo;
import com.chanjet.transnotice.entity.PosTransDetail;
import org.apache.ibatis.annotations.*;

import java.util.List;

/**
 * Created by Gjq on 2018/8/8.
 */
@Mapper
public interface PosTransDetailMapper {
    @Select("select a.trans_code,a.agent_id,a.trans_date,a.trans_time,a.card_no,a.trace_no,a.sys_trace,a.rrn,a.auth_code," +
            " a.batch_no,a.merch_order_no,a.input_mode,a.card_type,a.merch_id,a.term_id,b.merch_name," +
            " b.mobile,a.amount,a.fee,a.fee_type,a.settle_date,a.resp_code,a.resp_desc,a.o_trans_date,a.o_rrn,c.agent_param" +
            " from b_pos_trans_detail a join b_merch b on a.merch_id = b.merch_id" +
            " join b_agent_function c on b.agent_id = c.agent_id" +
            " where a.notice_count < 5 and c.notice_flag = '1' and a.merch_id = b.merch_id" +
            " and a.trans_code in ('${transCode}')" +
            " and a.notice_status = '0' and  resp_code != 'XX'")
    @Results({
            @Result(column="trans_code", property="sTransCode"),
            @Result(column="agent_id", property="sAcqId"),
            @Result(column="trans_date", property="sTransDate"),
            @Result(column="trans_time", property="sTransTime"),
            @Result(column="card_no", property="sCardNo"),
            @Result(column="trace_no", property="sTraceNo"),
            @Result(column="sys_trace", property="sSysTrace"),
            @Result(column="rrn", property="sRrn"),
            @Result(column="auth_code", property="sAuthCode"),
            @Result(column="batch_no", property="sBatchNo"),
            @Result(column="merch_order_no", property="sMerchOrderNo"),
            @Result(column="input_mode", property="sInputMode"),
            @Result(column="card_type", property="sCardType"),
            @Result(column="merch_id", property="sMerchId"),
            @Result(column="term_id", property="sTermId"),
            @Result(column="merch_name", property="sMerchName"),
            @Result(column="mobile", property="sMobileNo"),
            @Result(column="amount", property="dAmount"),
            @Result(column="fee", property="dFee"),
            @Result(column="fee_type", property="sFeeType"),
            @Result(column="settle_date", property="sSettleDate"),
            @Result(column="resp_code", property="sRespCode"),
            @Result(column="resp_desc", property="sRespDesc"),
            @Result(column="o_trans_date", property="sOTransDate"),
            @Result(column="o_rrn", property="sORrn"),
            @Result(column="agent_param", property="sAgentParam")
    })
    List<PosTransDetail> findTransDetailPos(@Param("transCode") String transCode);

    @Select("select trace_no,batch_no,auth_code from ${tabName}" +
            " where rrn = #{rrn,jdbcType=VARCHAR} and trans_date = #{transDate,jdbcType=VARCHAR}")
    @Results({
            @Result(column="trace_no", property="sOtraceNo"),
            @Result(column="batch_no", property="sObatchNo"),
            @Result(column="auth_code", property="sOauthCode")
    })
    OtransInfo findOTransInfoPosByParam(@Param("tabName") String tabName, @Param("rrn") String rrn, @Param("transDate") String transDate);
}
