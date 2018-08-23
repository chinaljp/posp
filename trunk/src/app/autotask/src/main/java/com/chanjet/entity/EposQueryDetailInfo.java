package com.chanjet.entity;

import java.util.Date;

public class EposQueryDetailInfo {
    private String trans_code;
    private Float amount;
    private String trans_date;
    private String trans_time;
    private String merch_id;
    private String term_id;
    private String trace_no;
    private String rrn;
    private String card_type;
    private String agent_id;
    private String merch_order_no;
    private String order_url;
    private Integer query_count;
    private Date create_time;
    private Date cur_time;

    public Date getCur_time() {
        return cur_time;
    }

    public void setCur_time(Date cur_time) {
        this.cur_time = cur_time;
    }

    public Date getCreate_time() {
        return create_time;
    }

    public void setCreate_time(Date create_time) {
        this.create_time = create_time;
    }

    public Integer getQuery_count() {
        return query_count;
    }

    public void setQuery_count(Integer query_count) {
        this.query_count = query_count;
    }

    public String getTrans_code() {
        return trans_code;
    }

    public void setTrans_code(String trans_code) {
        this.trans_code = trans_code;
    }

    public Float getAmount() {
        return amount;
    }

    public void setAmount(Float amount) {
        this.amount = amount;
    }

    public String getTrans_date() {
        return trans_date;
    }

    public void setTrans_date(String trans_date) {
        this.trans_date = trans_date;
    }

    public String getTrans_time() {
        return trans_time;
    }

    public void setTrans_time(String trans_time) {
        this.trans_time = trans_time;
    }

    public String getMerch_id() {
        return merch_id;
    }

    public void setMerch_id(String merch_id) {
        this.merch_id = merch_id;
    }

    public String getTerm_id() {
        return term_id;
    }

    public void setTerm_id(String term_id) {
        this.term_id = term_id;
    }

    public String getTrace_no() {
        return trace_no;
    }

    public void setTrace_no(String trace_no) {
        this.trace_no = trace_no;
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

    public String getAgent_id() {
        return agent_id;
    }

    public void setAgent_id(String agent_id) {
        this.agent_id = agent_id;
    }

    public String getMerch_order_no() {
        return merch_order_no;
    }

    public void setMerch_order_no(String merch_order_no) {
        this.merch_order_no = merch_order_no;
    }

    public String getOrder_url() {
        return order_url;
    }

    public void setOrder_url(String order_url) {
        this.order_url = order_url;
    }

}
