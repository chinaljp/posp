package com.chanjet.mapper;

import com.chanjet.entity.AgentFeeRate;
import com.chanjet.entity.AgentInfo;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;
@Mapper
public interface AgentMapper {

    @Select("select agent_id,agent_name,agent_level,grade_agent_level from b_agent where agent_id = #{agent_id}")
    AgentInfo findAgentInfo(@Param("agent_id") String agent_id);
    @Select("select agent_organization from B_AGENT where agent_id=#{agent_id}")
    String findAllAgentId(@Param("agent_id") String agent_id);
//    @Select("select af.fee_type from B_AGENT_FEE a  join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE where a.agent_id = #{}")
//    String findAgentFeeType(@Param("agent_id") String agent_id);
    @Select("select a.agent_id,C_FEE_RATE,C_FEE_MAX,CY_FEE_RATE,CY_FEE_MAX,CM_FEE_RATE," +
            "CM_FEE_MAX,D_FEE_RATE,D_FEE_MAX,DY_FEE_RATE,DY_FEE_MAX,DM_FEE_RATE,DM_FEE_MAX," +
            "WX_FEE_RATE,WX_FEE_MAX,BB_FEE_RATE,BB_FEE_MAX,YC_FREE_FEE_RATE,YD_FREE_FEE_RATE," +
            "YC_NFC_FEE_RATE,YD_NFC_FEE_RATE,YC_QRCODE_FEE_RATE,YD_QRCODE_FEE_RATE,D0_FEE_RATE," +
            "C_FEE_RATIO,DQ_FEE_RATE,CQ_FEE_RATE,fee_type from B_AGENT_FEE a join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE " +
            "where a.agent_id=#{agent_id}")
    AgentFeeRate findAgentFee(@Param("agent_id") String agent_id);
    @Select("select a.agent_id,S_C_FEE_RATE,S_C_FEE_MAX,S_CY_FEE_RATE,S_CY_FEE_MAX,S_CM_FEE_RATE," +
            "S_CM_FEE_MAX,S_D_FEE_RATE,S_D_FEE_MAX,S_DY_FEE_RATE,S_DY_FEE_MAX,S_DM_FEE_RATE,S_DM_FEE_MAX," +
            "S_WX_FEE_RATE,S_WX_FEE_MAX,S_BB_FEE_RATE,S_BB_FEE_MAX,S_YC_FREE_FEE_RATE,S_YD_FREE_FEE_RATE," +
            "S_YC_NFC_FEE_RATE,S_YD_NFC_FEE_RATE,S_YC_QRCODE_FEE_RATE,S_YD_QRCODE_FEE_RATE,S_D0_FEE_RATE," +
            "S_C_FEE_RATIO,DQ_FEE_RATE,CQ_FEE_RATE,fee_type from B_AGENT_FEE a join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE " +
            "where a.agent_id=#{agent_id} ")
    AgentFeeRate findAgentFeeS(@Param("agent_id") String agent_id);
    @Select("select a.agent_id,C_FEE_RATIO,D0_FEE_RATE,S_D0_FEE_RATE,E_D0_FEE_RATE " +
            " from B_AGENT_FEE a " +
            " join B_AGENT_FEE_TEMP af on af.TEMP_CODE=a.FEE_TEMP_CODE" +
            " where a.agent_id=#{agent_id}")
    AgentFeeRate findAgentFeeD0(@Param("agent_id") String agent_id);

}
