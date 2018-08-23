package com.chanjet.skeleton;

/**
 * Created by Pay on 2017/2/8.
 */
public class MsgObject {
    private String svrid;
    private String key;
    private Data data;

    public String getSvrid() {
        return svrid;
    }

    public void setSvrid(String svrid) {
        this.svrid = svrid;
    }

    public String getKey() {
        return key;
    }

    public void setKey(String key) {
        this.key = key;
    }

    public Data getData() {
        return data;
    }

    public void setData(Data data) {
        this.data = data;
    }
}
