package com.chanjet.mapper;

import com.chanjet.entity.AppendTransDetail;
import org.apache.ibatis.annotations.Insert;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
@Mapper
public interface AppendTransDetailMapper {

    @Insert("insert into b_append_trans_detail(ref_id,group_code,merch_id,user_code,merch_name,card_type,trans_date,trans_time,rrn,amount,merch_fee,merch_order_no,settle_date,check_flag" +
            ",agent_id0,agent_name0,fee0,cost_amount0,ratio0,grade_agent_level0" +
            ",agent_id1,agent_name1,fee1,cost_amount1,ratio1,grade_agent_level1" +
            ",agent_id2,agent_name2,fee2,cost_amount2,ratio2,grade_agent_level2" +
            ",agent_id3,agent_name3,fee3,cost_amount3,ratio3,grade_agent_level3" +
            ",agent_id4,agent_name4,fee4,cost_amount4,ratio4,grade_agent_level4" +
            ",agent_id5,agent_name5,fee5,cost_amount5,ratio5,grade_agent_level5" +
            ",agent_id6,agent_name6,fee6,cost_amount6,ratio6,grade_agent_level6" +
            ",agent_id7,agent_name7,fee7,cost_amount7,ratio7,grade_agent_level7" +
            ",agent_id8,agent_name8,fee8,cost_amount8,ratio8,grade_agent_level8" +
            ")" +
            " values(#{appendTransDetail.ref_id,jdbcType=VARCHAR},#{appendTransDetail.group_code,jdbcType=VARCHAR},#{appendTransDetail.merch_id,jdbcType=VARCHAR},#{appendTransDetail.user_code,jdbcType=VARCHAR},#{appendTransDetail.merch_name,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.card_type,jdbcType=VARCHAR},#{appendTransDetail.trans_date,jdbcType=VARCHAR},#{appendTransDetail.trans_time,jdbcType=VARCHAR},#{appendTransDetail.rrn,jdbcType=VARCHAR},#{appendTransDetail.amount,jdbcType=NUMERIC}" +
            ",#{appendTransDetail.merch_fee,jdbcType=NUMERIC},#{appendTransDetail.merch_order_no,jdbcType=VARCHAR},#{appendTransDetail.settle_date,jdbcType=VARCHAR},#{appendTransDetail.check_flag,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id0,jdbcType=VARCHAR},#{appendTransDetail.agent_name0,jdbcType=VARCHAR},#{appendTransDetail.fee0,jdbcType=NUMERIC},#{appendTransDetail.cost_amount0,jdbcType=NUMERIC},#{appendTransDetail.ratio0,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level0,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id1,jdbcType=VARCHAR},#{appendTransDetail.agent_name1,jdbcType=VARCHAR},#{appendTransDetail.fee1,jdbcType=NUMERIC},#{appendTransDetail.cost_amount1,jdbcType=NUMERIC},#{appendTransDetail.ratio1,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level1,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id2,jdbcType=VARCHAR},#{appendTransDetail.agent_name2,jdbcType=VARCHAR},#{appendTransDetail.fee2,jdbcType=NUMERIC},#{appendTransDetail.cost_amount2,jdbcType=NUMERIC},#{appendTransDetail.ratio2,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level2,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id3,jdbcType=VARCHAR},#{appendTransDetail.agent_name3,jdbcType=VARCHAR},#{appendTransDetail.fee3,jdbcType=NUMERIC},#{appendTransDetail.cost_amount3,jdbcType=NUMERIC},#{appendTransDetail.ratio3,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level3,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id4,jdbcType=VARCHAR},#{appendTransDetail.agent_name4,jdbcType=VARCHAR},#{appendTransDetail.fee4,jdbcType=NUMERIC},#{appendTransDetail.cost_amount4,jdbcType=NUMERIC},#{appendTransDetail.ratio4,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level4,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id5,jdbcType=VARCHAR},#{appendTransDetail.agent_name5,jdbcType=VARCHAR},#{appendTransDetail.fee5,jdbcType=NUMERIC},#{appendTransDetail.cost_amount5,jdbcType=NUMERIC},#{appendTransDetail.ratio5,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level5,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id6,jdbcType=VARCHAR},#{appendTransDetail.agent_name6,jdbcType=VARCHAR},#{appendTransDetail.fee6,jdbcType=NUMERIC},#{appendTransDetail.cost_amount6,jdbcType=NUMERIC},#{appendTransDetail.ratio6,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level6,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id7,jdbcType=VARCHAR},#{appendTransDetail.agent_name7,jdbcType=VARCHAR},#{appendTransDetail.fee7,jdbcType=NUMERIC},#{appendTransDetail.cost_amount7,jdbcType=NUMERIC},#{appendTransDetail.ratio7,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level7,jdbcType=VARCHAR}" +
            ",#{appendTransDetail.agent_id8,jdbcType=VARCHAR},#{appendTransDetail.agent_name8,jdbcType=VARCHAR},#{appendTransDetail.fee8,jdbcType=NUMERIC},#{appendTransDetail.cost_amount8,jdbcType=NUMERIC},#{appendTransDetail.ratio8,jdbcType=NUMERIC},#{appendTransDetail.grade_agent_level8,jdbcType=VARCHAR}" +
            ")")
    public  int insertAppendTransDetail(@Param("appendTransDetail") AppendTransDetail appendTransDetail);

}
