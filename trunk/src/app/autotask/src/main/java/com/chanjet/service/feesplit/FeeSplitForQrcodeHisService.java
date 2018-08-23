package com.chanjet.service.feesplit;

import com.chanjet.entity.AgentInfo;
import com.chanjet.entity.AppendTransDetail;
import com.chanjet.entity.InlineDetailInfo;
import com.chanjet.entity.MerchInfo;
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
public class FeeSplitForQrcodeHisService extends FeeSplitForQrcodeService {
    private static final Logger logger = LoggerFactory.getLogger(FeeSplitForQrcodeHisService.class);
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
        List<InlineDetailInfo> list = this.inlineJournalMapper.findJournalQrcodeHis(rowNum);
        for (InlineDetailInfo inlineDetailInfo : list) {
            //从流水中找到商户号
            String merch_id = inlineDetailInfo.getMerch_id();
            //查找商户名称
            MerchInfo merchInfo = merchMapper.findMerchInfo(merch_id);
            //根据商户号查找对应代理商号
            String agent_id = merchMapper.findMerchAgent(merch_id).getAgent_id();
            //查找全部代理商
            String agent_organization = agentMapper.findAllAgentId(agent_id);
            logger.info("代理商组织关系" + agent_organization);
            String agent[] = agent_organization.split("[,]");
            //查找各代理商费率放到list中
            List<AgentInfo> agtInfoList = new ArrayList<AgentInfo>();
            //List<AgentFeeRate> agtFeeRateList = null;
            for (int i = agent.length - 1; i >= 0; i--) {
                String agt = agent[i];
                AgentInfo agentInfo = agentMapper.findAgentInfo(agt);
                if (i == 0 && agtInfoList.get(agent.length - 2).getAgentFeeRate().getFee_type().equals("1")) {
                    agentInfo.setAgentFeeRate(agentMapper.findAgentFeeS(agt));
                } else {
                    agentInfo.setAgentFeeRate(agentMapper.findAgentFee(agt));
                }
                agentInfo.setRatio(agentInfo.getAgentFeeRate().getC_fee_ratio());
                agtInfoList.add(agentInfo);
            }
            //反转agtInfoList
            Collections.reverse(agtInfoList);
            //初始化分润流水实体
            AppendTransDetail appendTransDetail = new AppendTransDetail(inlineDetailInfo, merchInfo);
            appendTransDetail.setGroup_code("qrcode");


            //计算分润fee,cost_amount,ratio,
            logger.info("------------开始计算分润----------");
            for (int i = agtInfoList.size() - 1; i >= 0; i--) {
                //计算成本
                calcAgtCost(agtInfoList.get(i), inlineDetailInfo);
                if (i == agtInfoList.size() - 1) {
                    calcMerchAgtFee(agtInfoList.get(i), inlineDetailInfo);
                } else {
                    calcAgtFee(agtInfoList.get(i), agtInfoList.get(i + 1), inlineDetailInfo);
                }

            }
            logger.info("------------分润计算结束----------");
/*赋值appendTransDetail*/
            setAllAgent(appendTransDetail, agtInfoList);

            //插入分润流水
            appendTransDetailMapper.insertAppendTransDetail(appendTransDetail);
            //更新原流水Fee_flag
            inlineJournalMapper.updFeeFlagHis(appendTransDetail.getRef_id());

        }

    }
}

