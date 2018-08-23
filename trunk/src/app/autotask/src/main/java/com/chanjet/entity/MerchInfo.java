package com.chanjet.entity;

public class MerchInfo {
    private String merch_id;
    private String merch_name;
    private String user_code;
    private String d0_type;

    public String getD0_type() {
        return d0_type;
    }

    public void setD0_type(String d0_type) {
        this.d0_type = d0_type;
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
}
