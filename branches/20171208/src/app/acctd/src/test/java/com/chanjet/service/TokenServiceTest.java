package com.chanjet.service;

import com.chanjet.vo.BaseResMessage;
import com.chanjet.vo.TokenRes;
import org.junit.Test;

/**
 * Created by feng.gaoo on 2017/3/23.
 */
public class TokenServiceTest {
    @Test
    public void getToken() {
        try {
            TokenService tokenService = new TokenService();
            BaseResMessage<TokenRes> res = tokenService.getToken("1000", "10");
            String token = res.getData().getToken();
            System.out.println("获取令牌成功:" + token);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}