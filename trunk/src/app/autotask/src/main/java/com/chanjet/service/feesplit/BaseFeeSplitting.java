package com.chanjet.service.feesplit;

import com.chanjet.entity.AgentInfo;
import com.chanjet.entity.AppendTransDetail;
import com.chanjet.entity.D0DetailInfo;
import com.chanjet.entity.InlineDetailInfo;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

public abstract class BaseFeeSplitting {
    private static final Logger logger = LoggerFactory.getLogger(BaseFeeSplitting.class);
    public void setAgent0(AppendTransDetail appendTransDetail, AgentInfo agentInfo){
        //分公司
        appendTransDetail.setAgent_id0(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name0(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level0(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee0(agentInfo.getFee());
        appendTransDetail.setCost_amount0(agentInfo.getCost_amount());
        appendTransDetail.setRatio0(agentInfo.getRatio());
    }

    public void setAgent1(AppendTransDetail appendTransDetail, AgentInfo agentInfo) {
        //1代
        appendTransDetail.setAgent_id1(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name1(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level1(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee1(agentInfo.getFee());
        appendTransDetail.setCost_amount1(agentInfo.getCost_amount());
        appendTransDetail.setRatio1(agentInfo.getRatio());
    }

    public void setAgent2(AppendTransDetail appendTransDetail, AgentInfo agentInfo) {
        //2代
        appendTransDetail.setAgent_id2(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name2(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level2(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee2(agentInfo.getFee());
        appendTransDetail.setCost_amount2(agentInfo.getCost_amount());
        appendTransDetail.setRatio2(agentInfo.getRatio());
    }
    public void setAgent3(AppendTransDetail appendTransDetail, AgentInfo agentInfo) {
        //3代
        appendTransDetail.setAgent_id3(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name3(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level3(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee3(agentInfo.getFee());
        appendTransDetail.setCost_amount3(agentInfo.getCost_amount());
        appendTransDetail.setRatio3(agentInfo.getRatio());
    }

    public void setAgent4(AppendTransDetail appendTransDetail, AgentInfo agentInfo) {
        //4代
        appendTransDetail.setAgent_id4(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name4(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level4(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee4(agentInfo.getFee());
        appendTransDetail.setCost_amount4(agentInfo.getCost_amount());
        appendTransDetail.setRatio4(agentInfo.getRatio());
    }


    public void setAgent5(AppendTransDetail appendTransDetail, AgentInfo agentInfo) {
        //5代
        appendTransDetail.setAgent_id5(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name5(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level5(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee5(agentInfo.getFee());
        appendTransDetail.setCost_amount5(agentInfo.getCost_amount());
        appendTransDetail.setRatio5(agentInfo.getRatio());
    }

    public void setAgent6(AppendTransDetail appendTransDetail, AgentInfo agentInfo) {
        //6代
        appendTransDetail.setAgent_id6(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name6(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level6(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee6(agentInfo.getFee());
        appendTransDetail.setCost_amount6(agentInfo.getCost_amount());
        appendTransDetail.setRatio6(agentInfo.getRatio());
    }

    public void setAgent7(AppendTransDetail appendTransDetail, AgentInfo agentInfo) {
        //7代
        appendTransDetail.setAgent_id7(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name7(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level7(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee7(agentInfo.getFee());
        appendTransDetail.setCost_amount7(agentInfo.getCost_amount());
        appendTransDetail.setRatio7(agentInfo.getRatio());
    }

    public void setAgent8(AppendTransDetail appendTransDetail, AgentInfo agentInfo) {
        //8代
        appendTransDetail.setAgent_id8(agentInfo.getAgent_id());
        appendTransDetail.setAgent_name8(agentInfo.getAgent_name());
        appendTransDetail.setGrade_agent_level8(agentInfo.getGrade_agent_level());
        appendTransDetail.setFee8(agentInfo.getFee());
        appendTransDetail.setCost_amount8(agentInfo.getCost_amount());
        appendTransDetail.setRatio8(agentInfo.getRatio());
    }

    public void setAllAgent(AppendTransDetail appendTransDetail, List<AgentInfo> agtInfoList){
        /*赋值appendTransDetail*/
//分公司
        setAgent0(appendTransDetail,agtInfoList.get(0));
        //1代
        setAgent1(appendTransDetail,agtInfoList.get(1));

        //2代
        if(agtInfoList.size() > 2) setAgent2(appendTransDetail,agtInfoList.get(2));
        //3代
        if(agtInfoList.size() > 3) setAgent3(appendTransDetail,agtInfoList.get(3));
        //4代
        if(agtInfoList.size() > 4 )setAgent4(appendTransDetail,agtInfoList.get(4));
        //5代
        if(agtInfoList.size() > 5) setAgent5(appendTransDetail,agtInfoList.get(5));
        //6代
        if(agtInfoList.size() > 6) setAgent6(appendTransDetail,agtInfoList.get(6));
        //7代
        if(agtInfoList.size() > 7) setAgent7(appendTransDetail,agtInfoList.get(7));
        //8代
        if(agtInfoList.size() > 8) setAgent8(appendTransDetail,agtInfoList.get(8));
    }

//计算商户非直属代理商分润
    public void calcAgtFee(AgentInfo agentInfo, AgentInfo lowAgentInfo, InlineDetailInfo inlineDetailInfo){
        //分润
        Float fee = (inlineDetailInfo.getFee()-agentInfo.getCost_amount())*agentInfo.getRatio()/100 -(inlineDetailInfo.getFee()-lowAgentInfo.getCost_amount())*lowAgentInfo.getRatio()/100;
        logger.info("ID:"+inlineDetailInfo.getId()+" 交易金额:"+ inlineDetailInfo.getAmount() +" 代理商分润:"+fee);
        agentInfo.setFee(fee);
    }
    //计算商户直属代理商分润
    public void calcMerchAgtFee(AgentInfo agentInfo, InlineDetailInfo inlineDetailInfo){
        Float fee = (inlineDetailInfo.getFee()-agentInfo.getCost_amount())*agentInfo.getRatio()/100;
        logger.info("ID:"+inlineDetailInfo.getId()+" 交易金额:"+ inlineDetailInfo.getAmount() +" 代理商分润:"+fee);
        agentInfo.setFee(fee);
    }

    //计算D0商户非直属代理商分润
    public void calcAgtFeeD0(AgentInfo agentInfo, AgentInfo lowAgentInfo, D0DetailInfo d0DetailInfo){
        //分润
        Float fee = (d0DetailInfo.getP_fee()-agentInfo.getCost_amount())*agentInfo.getRatio()/100 -(d0DetailInfo.getP_fee()-lowAgentInfo.getCost_amount())*lowAgentInfo.getRatio()/100;
        logger.info("ID:"+d0DetailInfo.getId()+" 交易金额:"+ d0DetailInfo.getAmount() +" 代理商分润:"+fee);
        agentInfo.setFee(fee);
    }
    //计算D0商户直属代理商分润
    public void calcMerchAgtFeeD0(AgentInfo agentInfo, D0DetailInfo d0DetailInfo){
        Float fee = (d0DetailInfo.getP_fee()-agentInfo.getCost_amount())*agentInfo.getRatio()/100;
        logger.info("ID:"+d0DetailInfo.getId()+" 交易金额:"+ d0DetailInfo.getAmount() +" 代理商分润:"+fee);
        agentInfo.setFee(fee);
    }



}
