package com.chanjet.entity;

public class AgentInfo {
    private String agent_id;
    private String agent_name;
    private String status;
    private String agent_level;
    private String grade_agent_level;
    private Float fee;
    private Float cost_amount;
    private Float ratio;
    private AgentFeeRate agentFeeRate;

    public Float getFee() {
        return fee;
    }

    public void setFee(Float fee) {
        this.fee = fee;
    }

    public Float getCost_amount() {
        return cost_amount;
    }

    public void setCost_amount(Float cost_amount) {
        this.cost_amount = cost_amount;
    }

    public Float getRatio() {
        return ratio;
    }

    public void setRatio(Float ratio) {
        this.ratio = ratio;
    }

    public AgentFeeRate getAgentFeeRate() {
        return agentFeeRate;
    }

    public void setAgentFeeRate(AgentFeeRate agentFeeRate) {
        this.agentFeeRate = agentFeeRate;
    }

    public String getAgent_id() {
        return agent_id;
    }

    public void setAgent_id(String agent_id) {
        this.agent_id = agent_id;
    }

    public String getAgent_name() {
        return agent_name;
    }

    public void setAgent_name(String agent_name) {
        this.agent_name = agent_name;
    }

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public String getAgent_level() {
        return agent_level;
    }

    public void setAgent_level(String agent_level) {
        this.agent_level = agent_level;
    }

    public String getGrade_agent_level() {
        return grade_agent_level;
    }

    public void setGrade_agent_level(String grade_agent_level) {
        this.grade_agent_level = grade_agent_level;
    }
}
