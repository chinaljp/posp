package com.chanjet.vo;

import java.io.Serializable;

/**
 * Created by feng.gaoo on 2017/3/23.
 */
public class TokenRes implements Serializable {
    /**
     *
     */
    private static final long serialVersionUID = 3718683853567748901L;
    private String token;        //令牌

    /**
     * 令牌
     *
     * @return
     */
    public String getToken() {
        return token;
    }

    /**
     * 令牌
     *
     * @param token
     */
    public void setToken(String token) {
        this.token = token;
    }
}
