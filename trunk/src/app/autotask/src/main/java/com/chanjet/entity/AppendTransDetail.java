package com.chanjet.entity;

public class AppendTransDetail {
    private String agent_id0;
    private String agent_name0;
    private Float fee0 = 0f;
    private Float cost_amount0;
    private Float ratio0;
    private String grade_agent_level0;
    private String agent_id1;
    private String agent_name1;
    private Float fee1 = 0f;
    private Float cost_amount1;
    private Float ratio1;
    private String grade_agent_level1;
    private String agent_id2;
    private String agent_name2;
    private Float fee2 = 0f;
    private Float cost_amount2;
    private Float ratio2;
    private String grade_agent_level2;
    private String agent_id3;
    private String agent_name3;
    private Float fee3 = 0f;
    private Float cost_amount3;
    private Float ratio3;
    private String grade_agent_level3;
    private String agent_id4;
    private String agent_name4;
    private Float fee4 = 0f;
    private Float cost_amount4;
    private Float ratio4;
    private String grade_agent_level4;
    private String agent_id5;
    private String agent_name5;
    private Float fee5 = 0f ;
    private Float cost_amount5;
    private Float ratio5;
    private String grade_agent_level5;
    private String agent_id6;
    private String agent_name6;
    private Float fee6 = 0f;
    private Float cost_amount6;
    private Float ratio6;
    private String grade_agent_level6;
    private String agent_id7;
    private String agent_name7;
    private Float fee7 = 0f;
    private Float cost_amount7;
    private Float ratio7;
    private String grade_agent_level7;
    private String agent_id8;
    private String agent_name8;
    private Float fee8 = 0f;
    private Float cost_amount8;
    private Float ratio8;
    private String grade_agent_level8;
    private String ref_id;
    private String group_code;
    private String trans_date;
    private String user_code;
    private String merch_id;
    private String merch_name;
    private String trans_time;
    private Float amount;
    private Float merch_fee;
    private String rrn;
    private String card_type;
    private String merch_order_no;
    private String settle_date;
    private String check_flag;


    public AppendTransDetail(InlineDetailInfo inlineDetailInfo, MerchInfo merchInfo){
        this.ref_id = inlineDetailInfo.getId();
        this.merch_id = inlineDetailInfo.getMerch_id();
        this.user_code = inlineDetailInfo.getUser_code();
        this.merch_name = merchInfo.getMerch_name();
        this.card_type = inlineDetailInfo.getCard_type();
        this.trans_date = inlineDetailInfo.getTrans_date();
        this.trans_time = inlineDetailInfo.getTrans_time();
        this.rrn = inlineDetailInfo.getRrn();
        this.amount = inlineDetailInfo.getAmount();
        this.merch_fee = inlineDetailInfo.getFee();
        this.merch_order_no = inlineDetailInfo.getMerch_order_no();
        this.check_flag = "N";

    }

    public AppendTransDetail(D0DetailInfo d0DetailInfo, MerchInfo merchInfo){
        this.ref_id = d0DetailInfo.getId();
        this.merch_id = d0DetailInfo.getMerch_id();
        this.user_code = merchInfo.getUser_code();
        this.merch_name = merchInfo.getMerch_name();
        this.card_type = "X";
        this.trans_date = d0DetailInfo.getTrans_date();
        this.settle_date = d0DetailInfo.getTrans_date();
        this.amount = d0DetailInfo.getAmount();
        this.merch_fee = d0DetailInfo.getP_fee();
        this.check_flag = "Y";

    }

    public String getCheck_flag() {
        return check_flag;
    }

    public void setCheck_flag(String check_flag) {
        this.check_flag = check_flag;
    }

    public String getSettle_date() {
        return settle_date;
    }

    public void setSettle_date(String settle_date) {
        this.settle_date = settle_date;
    }

    public String getMerch_order_no() {
        return merch_order_no;
    }

    public void setMerch_order_no(String merch_order_no) {
        this.merch_order_no = merch_order_no;
    }

    public String getAgent_id0() {
        return agent_id0;
    }

    public void setAgent_id0(String agent_id0) {
        this.agent_id0 = agent_id0;
    }

    public String getAgent_name0() {
        return agent_name0;
    }

    public void setAgent_name0(String agent_name0) {
        this.agent_name0 = agent_name0;
    }

    public Float getFee0() {
        return fee0;
    }

    public void setFee0(Float fee0) {
        this.fee0 = fee0;
    }

    public Float getCost_amount0() {
        return cost_amount0;
    }

    public void setCost_amount0(Float cost_amount0) {
        this.cost_amount0 = cost_amount0;
    }

    public Float getRatio0() {
        return ratio0;
    }

    public void setRatio0(Float ratio0) {
        this.ratio0 = ratio0;
    }

    public String getGrade_agent_level0() {
        return grade_agent_level0;
    }

    public void setGrade_agent_level0(String grade_agent_level0) {
        this.grade_agent_level0 = grade_agent_level0;
    }

    public String getAgent_id1() {
        return agent_id1;
    }

    public void setAgent_id1(String agent_id1) {
        this.agent_id1 = agent_id1;
    }

    public String getAgent_name1() {
        return agent_name1;
    }

    public void setAgent_name1(String agent_name1) {
        this.agent_name1 = agent_name1;
    }

    public Float getFee1() {
        return fee1;
    }

    public void setFee1(Float fee1) {
        this.fee1 = fee1;
    }

    public Float getCost_amount1() {
        return cost_amount1;
    }

    public void setCost_amount1(Float cost_amount1) {
        this.cost_amount1 = cost_amount1;
    }

    public Float getRatio1() {
        return ratio1;
    }

    public void setRatio1(Float ratio1) {
        this.ratio1 = ratio1;
    }

    public String getGrade_agent_level1() {
        return grade_agent_level1;
    }

    public void setGrade_agent_level1(String grade_agent_level1) {
        this.grade_agent_level1 = grade_agent_level1;
    }

    public String getAgent_id2() {
        return agent_id2;
    }

    public void setAgent_id2(String agent_id2) {
        this.agent_id2 = agent_id2;
    }

    public String getAgent_name2() {
        return agent_name2;
    }

    public void setAgent_name2(String agent_name2) {
        this.agent_name2 = agent_name2;
    }

    public Float getFee2() {
        return fee2;
    }

    public void setFee2(Float fee2) {
        this.fee2 = fee2;
    }

    public Float getCost_amount2() {
        return cost_amount2;
    }

    public void setCost_amount2(Float cost_amount2) {
        this.cost_amount2 = cost_amount2;
    }

    public Float getRatio2() {
        return ratio2;
    }

    public void setRatio2(Float ratio2) {
        this.ratio2 = ratio2;
    }

    public String getGrade_agent_level2() {
        return grade_agent_level2;
    }

    public void setGrade_agent_level2(String grade_agent_level2) {
        this.grade_agent_level2 = grade_agent_level2;
    }

    public String getAgent_id3() {
        return agent_id3;
    }

    public void setAgent_id3(String agent_id3) {
        this.agent_id3 = agent_id3;
    }

    public String getAgent_name3() {
        return agent_name3;
    }

    public void setAgent_name3(String agent_name3) {
        this.agent_name3 = agent_name3;
    }

    public Float getFee3() {
        return fee3;
    }

    public void setFee3(Float fee3) {
        this.fee3 = fee3;
    }

    public Float getCost_amount3() {
        return cost_amount3;
    }

    public void setCost_amount3(Float cost_amount3) {
        this.cost_amount3 = cost_amount3;
    }

    public Float getRatio3() {
        return ratio3;
    }

    public void setRatio3(Float ratio3) {
        this.ratio3 = ratio3;
    }

    public String getGrade_agent_level3() {
        return grade_agent_level3;
    }

    public void setGrade_agent_level3(String grade_agent_level3) {
        this.grade_agent_level3 = grade_agent_level3;
    }

    public String getAgent_id4() {
        return agent_id4;
    }

    public void setAgent_id4(String agent_id4) {
        this.agent_id4 = agent_id4;
    }

    public String getAgent_name4() {
        return agent_name4;
    }

    public void setAgent_name4(String agent_name4) {
        this.agent_name4 = agent_name4;
    }

    public Float getFee4() {
        return fee4;
    }

    public void setFee4(Float fee4) {
        this.fee4 = fee4;
    }

    public Float getCost_amount4() {
        return cost_amount4;
    }

    public void setCost_amount4(Float cost_amount4) {
        this.cost_amount4 = cost_amount4;
    }

    public Float getRatio4() {
        return ratio4;
    }

    public void setRatio4(Float ratio4) {
        this.ratio4 = ratio4;
    }

    public String getGrade_agent_level4() {
        return grade_agent_level4;
    }

    public void setGrade_agent_level4(String grade_agent_level4) {
        this.grade_agent_level4 = grade_agent_level4;
    }

    public String getAgent_id5() {
        return agent_id5;
    }

    public void setAgent_id5(String agent_id5) {
        this.agent_id5 = agent_id5;
    }

    public String getAgent_name5() {
        return agent_name5;
    }

    public void setAgent_name5(String agent_name5) {
        this.agent_name5 = agent_name5;
    }

    public Float getFee5() {
        return fee5;
    }

    public void setFee5(Float fee5) {
        this.fee5 = fee5;
    }

    public Float getCost_amount5() {
        return cost_amount5;
    }

    public void setCost_amount5(Float cost_amount5) {
        this.cost_amount5 = cost_amount5;
    }

    public Float getRatio5() {
        return ratio5;
    }

    public void setRatio5(Float ratio5) {
        this.ratio5 = ratio5;
    }

    public String getGrade_agent_level5() {
        return grade_agent_level5;
    }

    public void setGrade_agent_level5(String grade_agent_level5) {
        this.grade_agent_level5 = grade_agent_level5;
    }

    public String getAgent_id6() {
        return agent_id6;
    }

    public void setAgent_id6(String agent_id6) {
        this.agent_id6 = agent_id6;
    }

    public String getAgent_name6() {
        return agent_name6;
    }

    public void setAgent_name6(String agent_name6) {
        this.agent_name6 = agent_name6;
    }

    public Float getFee6() {
        return fee6;
    }

    public void setFee6(Float fee6) {
        this.fee6 = fee6;
    }

    public Float getCost_amount6() {
        return cost_amount6;
    }

    public void setCost_amount6(Float cost_amount6) {
        this.cost_amount6 = cost_amount6;
    }

    public Float getRatio6() {
        return ratio6;
    }

    public void setRatio6(Float ratio6) {
        this.ratio6 = ratio6;
    }

    public String getGrade_agent_level6() {
        return grade_agent_level6;
    }

    public void setGrade_agent_level6(String grade_agent_level6) {
        this.grade_agent_level6 = grade_agent_level6;
    }

    public String getAgent_id7() {
        return agent_id7;
    }

    public void setAgent_id7(String agent_id7) {
        this.agent_id7 = agent_id7;
    }

    public String getAgent_name7() {
        return agent_name7;
    }

    public void setAgent_name7(String agent_name7) {
        this.agent_name7 = agent_name7;
    }

    public Float getFee7() {
        return fee7;
    }

    public void setFee7(Float fee7) {
        this.fee7 = fee7;
    }

    public Float getCost_amount7() {
        return cost_amount7;
    }

    public void setCost_amount7(Float cost_amount7) {
        this.cost_amount7 = cost_amount7;
    }

    public Float getRatio7() {
        return ratio7;
    }

    public void setRatio7(Float ratio7) {
        this.ratio7 = ratio7;
    }

    public String getGrade_agent_level7() {
        return grade_agent_level7;
    }

    public void setGrade_agent_level7(String grade_agent_level7) {
        this.grade_agent_level7 = grade_agent_level7;
    }

    public String getAgent_id8() {
        return agent_id8;
    }

    public void setAgent_id8(String agent_id8) {
        this.agent_id8 = agent_id8;
    }

    public String getAgent_name8() {
        return agent_name8;
    }

    public void setAgent_name8(String agent_name8) {
        this.agent_name8 = agent_name8;
    }

    public Float getFee8() {
        return fee8;
    }

    public void setFee8(Float fee8) {
        this.fee8 = fee8;
    }

    public Float getCost_amount8() {
        return cost_amount8;
    }

    public void setCost_amount8(Float cost_amount8) {
        this.cost_amount8 = cost_amount8;
    }

    public Float getRatio8() {
        return ratio8;
    }

    public void setRatio8(Float ratio8) {
        this.ratio8 = ratio8;
    }

    public String getGrade_agent_level8() {
        return grade_agent_level8;
    }

    public void setGrade_agent_level8(String grade_agent_level8) {
        this.grade_agent_level8 = grade_agent_level8;
    }

    public String getRef_id() {
        return ref_id;
    }

    public void setRef_id(String ref_id) {
        this.ref_id = ref_id;
    }

    public String getGroup_code() {
        return group_code;
    }

    public void setGroup_code(String group_code) {
        this.group_code = group_code;
    }

    public String getTrans_date() {
        return trans_date;
    }

    public void setTrans_date(String trans_date) {
        this.trans_date = trans_date;
    }

    public String getUser_code() {
        return user_code;
    }

    public void setUser_code(String user_code) {
        this.user_code = user_code;
    }

    public String getMerch_id() {
        return merch_id;
    }

    public void setMerch_id(String merch_id) {
        this.merch_id = merch_id;
    }

    public String getMerch_name() {
        return merch_name;
    }

    public void setMerch_name(String merch_name) {
        this.merch_name = merch_name;
    }

    public String getTrans_time() {
        return trans_time;
    }

    public void setTrans_time(String trans_time) {
        this.trans_time = trans_time;
    }

    public Float getAmount() {
        return amount;
    }

    public void setAmount(Float amount) {
        this.amount = amount;
    }

    public Float getMerch_fee() {
        return merch_fee;
    }

    public void setMerch_fee(Float merch_fee) {
        this.merch_fee = merch_fee;
    }

    public String getRrn() {
        return rrn;
    }

    public void setRrn(String rrn) {
        this.rrn = rrn;
    }

    public String getCard_type() {
        return card_type;
    }

    public void setCard_type(String card_type) {
        this.card_type = card_type;
    }
}

