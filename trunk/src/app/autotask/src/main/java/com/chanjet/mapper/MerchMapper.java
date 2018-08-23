package com.chanjet.mapper;

import com.chanjet.entity.AgentInfo;
import com.chanjet.entity.MerchInfo;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;
@Mapper
public interface MerchMapper {
    @Select("select a.AGENT_ID,a.AGENT_NAME,a.STATUS,a.ZMK, a.AGENT_LEVEL, a.GRADE_AGENT_LEVEL from B_AGENT a join B_MERCH m on m.AGENT_ID=a.AGENT_ID where m.MERCH_ID=#{merch_id}")
    AgentInfo findMerchAgent(@Param("merch_id") String merch_id);
    @Select("select a.merch_id,a.merch_name,a.user_code,b.d0_type from b_merch a join b_merch_fee b on a.merch_id= b.merch_Id where a.merch_id = #{merch_id}")
    MerchInfo findMerchInfo(@Param("merch_id") String merch_id);
}
