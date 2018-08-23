package com.chanjet.entity;

public class D0DetailInfo {
    private String id;
    private Float amount;
    private String merch_id;
    private Float p_fee;
    private String trans_date;
    private Float cnt;
    private String busi_type;

    public String getBusi_type() {
        return busi_type;
    }

    public void setBusi_type(String busi_type) {
        this.busi_type = busi_type;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public Float getAmount() {
        return amount;
    }

    public void setAmount(Float amount) {
        this.amount = amount;
    }

    public String getMerch_id() {
        return merch_id;
    }

    public void setMerch_id(String merch_id) {
        this.merch_id = merch_id;
    }

    public Float getP_fee() {
        return p_fee;
    }

    public void setP_fee(Float p_fee) {
        this.p_fee = p_fee;
    }

    public String getTrans_date() {
        return trans_date;
    }

    public void setTrans_date(String trans_date) {
        this.trans_date = trans_date;
    }

    public Float getCnt() {
        return cnt;
    }

    public void setCnt(Float cnt) {
        this.cnt = cnt;
    }
}
