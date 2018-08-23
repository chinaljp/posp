package com.chanjet.service.feesplit;

import com.chanjet.entity.AgentInfo;
import com.chanjet.entity.D0DetailInfo;
import com.chanjet.entity.InlineDetailInfo;

public interface CalcAgtCost {
    public void calcAgtCost(AgentInfo agentInfo, InlineDetailInfo inlineDetailInfo);
}
