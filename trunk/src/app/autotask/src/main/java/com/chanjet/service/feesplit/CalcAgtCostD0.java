package com.chanjet.service.feesplit;

import com.chanjet.entity.AgentInfo;
import com.chanjet.entity.D0DetailInfo;

public interface CalcAgtCostD0 {
    public void calcAgtCost(AgentInfo agentInfo, D0DetailInfo d0DetailInfo, String d0Type);
}
