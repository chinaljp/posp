package com.chanjet.service.feesplit;

import com.chanjet.entity.*;
import com.chanjet.mapper.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

@Service
public class FeeSplitForD0Service extends BaseFeeSplitting implements CalcAgtCostD0 {
    private static final Logger logger = LoggerFactory.getLogger(FeeSplitForD0Service.class);
    @Autowired
    private D0JournalMapper d0JournalMapper;
    @Autowired
    private MerchMapper merchMapper;
    @Autowired
    private AgentMapper agentMapper;
    @Autowired
    private AppendTransDetailMapper appendTransDetailMapper;


    @Transactional
    public void feeSplitting(Integer rowNum) {

        //取出rowNum条待分润交易流水
        List<D0DetailInfo> list = this.d0JournalMapper.findJournalD0(rowNum);
        for(D0DetailInfo  d0DetailInfo:list){
            //从流水中找到商户号
            String merch_id = d0DetailInfo.getMerch_id();
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
                agentInfo.setAgentFeeRate(agentMapper.findAgentFeeD0(agt));
                agentInfo.setRatio(agentInfo.getAgentFeeRate().getC_fee_ratio());
                agtInfoList.add(agentInfo);
            }
            //反转agtInfoList
            Collections.reverse(agtInfoList);
            //初始化分润流水实体
            AppendTransDetail appendTransDetail = new AppendTransDetail(d0DetailInfo,merchInfo);
            appendTransDetail.setGroup_code("D0");

            //计算分润fee,cost_amount,ratio,
            logger.info("------------开始计算分润----------");
            for(int i = agtInfoList.size()-1; i >= 0; i-- ){
                //计算成本
                calcAgtCost(agtInfoList.get(i),d0DetailInfo,merchInfo.getD0_type());
                if( i == agtInfoList.size()-1){
                    //计算分润
                    calcMerchAgtFeeD0(agtInfoList.get(i), d0DetailInfo);
                }else {
                    calcAgtFeeD0(agtInfoList.get(i), agtInfoList.get(i+1), d0DetailInfo);
                }

            }
            logger.info("------------分润计算结束----------");
/*赋值appendTransDetail*/
            setAllAgent(appendTransDetail,agtInfoList);
            //插入分润流水
            appendTransDetailMapper.insertAppendTransDetail(appendTransDetail);
            //更新原流水Fee_flag
            d0JournalMapper.updFeeFlag(appendTransDetail.getRef_id());

        }

    }



    public void calcAgtCost(AgentInfo agentInfo, D0DetailInfo d0DetailInfo, String d0_type){
        Float cost_amount=0f;
        //epos
        if(d0DetailInfo.getBusi_type().equals("QUICKPAY")){
            //成本
            cost_amount = d0DetailInfo.getAmount()*agentInfo.getAgentFeeRate().getE_d0_fee_rate()/100;
            logger.info("代理商:"+agentInfo.getAgent_id()+" EPOS成本:"+cost_amount+" 费率E_D0_FEE_RATE:"+agentInfo.getAgentFeeRate().getE_d0_fee_rate());
        }
        //pos和qrcode
        else {
            if(d0_type.equals("1")){
                //成本
                cost_amount = d0DetailInfo.getAmount()*agentInfo.getAgentFeeRate().getS_d0_fee_rate()/100;
                logger.info("代理商:"+agentInfo.getAgent_id()+" D0成本:"+cost_amount+" 费率S_D0_FEE_RATE:"+agentInfo.getAgentFeeRate().getS_d0_fee_rate());
            }else {
                //成本
                cost_amount = d0DetailInfo.getAmount()*agentInfo.getAgentFeeRate().getD0_fee_rate()/100;
                logger.info("代理商:"+agentInfo.getAgent_id()+" D0成本:"+cost_amount+" 费率D0_FEE_RATE:"+agentInfo.getAgentFeeRate().getD0_fee_rate());
            }
        }
        agentInfo.setCost_amount(cost_amount);
    }
}
