package com.chanjet.service.feesplit;

import com.chanjet.entity.*;
import com.chanjet.mapper.AgentMapper;
import com.chanjet.mapper.AppendTransDetailMapper;
import com.chanjet.mapper.InlineJournalMapper;
import com.chanjet.mapper.MerchMapper;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

@Service
public class FeeSplitForEposService extends BaseFeeSplitting implements CalcAgtCost {
    private static final Logger logger = LoggerFactory.getLogger(FeeSplitForEposService.class);
    @Autowired
    private InlineJournalMapper inlineJournalMapper;
    @Autowired
    private MerchMapper merchMapper;
    @Autowired
    private AgentMapper agentMapper;
    @Autowired
    private AppendTransDetailMapper appendTransDetailMapper;


    @Transactional
    public void feeSplitting(Integer rowNum) {

        //取出rowNum条待分润交易流水
        List<InlineDetailInfo> list = this.inlineJournalMapper.findJournalEpos(rowNum);
        for(InlineDetailInfo  inlineDetailInfo:list){
            //从流水中找到商户号
            String merch_id = inlineDetailInfo.getMerch_id();
            //查找商户名称
            MerchInfo merchInfo = merchMapper.findMerchInfo(merch_id);
            //根据商户号查找对账代理商号
            String  agent_id = merchMapper.findMerchAgent(merch_id).getAgent_id();
            //查找全部代理商
            String agent_organization = agentMapper.findAllAgentId(agent_id);
            logger.info("代理商组织关系"+agent_organization);
            String agent[] = agent_organization.split("[,]");
            //查找各代理商费率放到list中
            List<AgentInfo> agtInfoList = new ArrayList<AgentInfo>();
            //List<AgentFeeRate> agtFeeRateList = null;
            for(int i = agent.length -1; i>=0; i--){
                String agt=agent[i];
                AgentInfo agentInfo = agentMapper.findAgentInfo(agt);
                if(i == 0 && agtInfoList.get(agent.length -2).getAgentFeeRate().getFee_type().equals("1") ){
                    agentInfo.setAgentFeeRate(agentMapper.findAgentFeeS(agt));
                }else {
                    agentInfo.setAgentFeeRate(agentMapper.findAgentFee(agt));
                }
                agentInfo.setRatio(agentInfo.getAgentFeeRate().getC_fee_ratio());
                agtInfoList.add(agentInfo);
            }
            //反转agtInfoList
            Collections.reverse(agtInfoList);
            //初始化分润流水实体
            AppendTransDetail appendTransDetail = new AppendTransDetail(inlineDetailInfo,merchInfo);
            appendTransDetail.setGroup_code("epos");

            //计算分润fee,cost_amount,ratio,
            logger.info("------------开始计算分润----------");
            for(int i = agtInfoList.size()-1; i >= 0; i-- ){
                //计算成本
                calcAgtCost(agtInfoList.get(i),inlineDetailInfo);
                if( i == agtInfoList.size()-1){
                    //计算分润
                    calcMerchAgtFee(agtInfoList.get(i), inlineDetailInfo);
                }else {
                    calcAgtFee(agtInfoList.get(i), agtInfoList.get(i+1), inlineDetailInfo);
                }

            }
            logger.info("------------分润计算结束----------");
/*赋值appendTransDetail*/
            setAllAgent(appendTransDetail,agtInfoList);
            //插入分润流水
            appendTransDetailMapper.insertAppendTransDetail(appendTransDetail);
            //更新原流水Fee_flag
            inlineJournalMapper.updFeeFlag(appendTransDetail.getRef_id());

        }

    }



    public void calcAgtCost(AgentInfo agentInfo, InlineDetailInfo inlineDetailInfo){
        Float cost_amount=0f;
        //借记卡
        if(inlineDetailInfo.getCard_type().equals("0")){
            //成本
            cost_amount = inlineDetailInfo.getAmount()*agentInfo.getAgentFeeRate().getDq_fee_rate()/100;
            logger.info("代理商:"+agentInfo.getAgent_id()+" 借记卡成本:"+cost_amount+" 费率DQ_FEE_RATE:"+agentInfo.getAgentFeeRate().getDq_fee_rate());
        }
        if(inlineDetailInfo.getCard_type().equals("1")){
            //成本
            cost_amount = inlineDetailInfo.getAmount()*agentInfo.getAgentFeeRate().getCq_fee_rate()/100;
            logger.info("代理商:"+agentInfo.getAgent_id()+" 贷记卡成本:"+cost_amount+" 费率CQ_FEE_RATE:"+agentInfo.getAgentFeeRate().getCq_fee_rate());
        }
        agentInfo.setCost_amount(cost_amount);
    }
}
