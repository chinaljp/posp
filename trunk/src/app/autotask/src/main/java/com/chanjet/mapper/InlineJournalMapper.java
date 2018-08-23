package com.chanjet.mapper;

import com.chanjet.entity.EposQueryDetailInfo;
import com.chanjet.entity.InlineDetailInfo;
import com.chanjet.entity.QrcodeQueryDetailInfo;
import org.apache.ibatis.annotations.*;

import java.util.List;

@Mapper
public interface InlineJournalMapper {
    @Select("select id,amount,merch_id,fee,card_type,fee_type,trans_code,user_code,trans_date,trans_time,rrn from b_inline_tarns_detail  where fee_flag = 'M' and substr(trans_code,3,1) ='Q' and resp_code='00' and rownum<=#{num}")
    List<InlineDetailInfo> findJournalEpos(@Param("num") Integer state);

    @Select("select id,amount,merch_id,fee,card_type,fee_type,trans_code,user_code,trans_date,trans_time,rrn from b_inline_tarns_detail_his  where fee_flag = 'M' and substr(trans_code,3,1) ='Q' and resp_code='00' and rownum<=#{num}")
    List<InlineDetailInfo> findJournalEposHis(@Param("num") Integer state);


    @Select("select id,amount,merch_id,fee,decode(card_type,null,'3',card_type) as card_type,decode(substr(trans_code,3,1),'W','WX','B','BB','Y',fee_type) as fee_type,trans_code,user_code,trans_date,trans_time,rrn,merch_order_no from " +
            "b_inline_tarns_detail  where fee_flag = 'M' and substr(trans_code,3,1) in('W','B','Y') and rownum<=#{num}")
    List<InlineDetailInfo> findJournalQrcode(@Param("num") Integer state);

    @Select("select id,amount,merch_id,fee,decode(card_type,null,'3',card_type) as card_type,decode(substr(trans_code,3,1),'W','WX','B','BB','Y',fee_type) as fee_type,trans_code,user_code,trans_date,trans_time,rrn,merch_order_no from " +
            "b_inline_tarns_detail_his  where fee_flag = 'M' and substr(trans_code,3,1) in('W','B','Y') and rownum<=#{num}")
    List<InlineDetailInfo> findJournalQrcodeHis(@Param("num") Integer state);

    @Update("update b_inline_tarns_detail set FEE_FLAG='Y' where id = #{id}")
    public int updFeeFlag(@Param("id") String id);

    @Update("UPDATE B_INLINE_TARNS_DETAIL SET QUERY_COUNT=QUERY_COUNT+1 WHERE MERCH_ORDER_NO=#{orderNo} and amount>0")
    public int updQRCnt(@Param("orderNo") String orderNo);

    @Update("UPDATE B_INLINE_TARNS_DETAIL SET QUERY_COUNT=QUERY_COUNT+1 WHERE rrn=#{rrn} and amount>0")
    public int updEposCnt(@Param("rrn") String orderNo);

    @Update("update b_inline_tarns_detail_his set FEE_FLAG='Y' where id = #{id}")
    public int updFeeFlagHis(@Param("id") String id);

    @Select("select trans_code,amount,trans_date,trans_time,merch_id,term_id,trace_no,rrn,agent_id,merch_order_no,order_url,query_count,create_time,sysdate as cur_time from " +
            "b_inline_tarns_detail  where query_count < 7 and substr(trans_code,3,2) in('W2','B2','W1','B1','Y2','Y1') and valid_flag='4' and resp_code='00' and sysdate-create_time<=15/60/24")
    List<QrcodeQueryDetailInfo> scanQrTrans();

    @Select("select trans_code,amount,trans_date,trans_time,merch_id,term_id,trace_no,rrn,agent_id,merch_order_no,order_url,query_count,create_time,sysdate as cur_time from " +
            "b_inline_tarns_detail  where query_count < 7 and substr(trans_code,3,2) in('Q0') and valid_flag='4' and resp_code in('00','FD') and sysdate-create_time<=15/60/24")
    List<EposQueryDetailInfo> scanEposTrans();
}
