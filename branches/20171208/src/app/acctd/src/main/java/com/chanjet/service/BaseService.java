package com.chanjet.service;

import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.TypeReference;
import com.chanjet.vo.BaseResMessage;
import com.chanjet.vo.TokenRes;
import okhttp3.*;
import org.apache.log4j.Logger;

import java.io.IOException;

/**
 * @Author Gaofeng
 * @Date 2017/3/30 13:49
 */
public class BaseService {
    private static Logger logger = Logger.getLogger(BaseService.class);
    public static final MediaType JSON = MediaType.parse("application/json; charset=utf-8");

    public static BaseResMessage<Object> post(String url, String json) throws IOException {
        logger.error("请求内容 : " + url + ":" + json);
        OkHttpClient client = new OkHttpClient();
        RequestBody body = RequestBody.create(JSON, json);
        Request request = new Request.Builder()
                .url(url)
                .post(body)
                .build();
        Response response = client.newCall(request).execute();
        if (response.isSuccessful()) {
            String resBodyStr = response.body().string();
            BaseResMessage<Object> res = JSONObject.parseObject(resBodyStr, new TypeReference<BaseResMessage<Object>>() {
            });
            logger.debug(" 响应内容 : " + resBodyStr);
            logger.debug(res.getMessage());
            return res;
        } else {
            logger.error("响应码: " + response.code());
            logger.error("响应内容: " + response.body().string());
            throw new IOException("Unexpected code " + response);
        }
    }

    public static String getToken(String clientNo, String tokenType) throws IOException {
                /* 获取令牌 */
        TokenService tokenService = new TokenService();
        BaseResMessage<TokenRes> res = tokenService.getToken(clientNo, tokenType);
        String token = res.getData().getToken();
        logger.debug(clientNo + "-" + tokenType + ":获取令牌成功:" + token);
        return token;
    }

    public static String getToken2(String clientNo, String tokenType) throws IOException {
                /* 获取令牌 */
        TokenService tokenService = new TokenService();
        BaseResMessage<TokenRes> res = tokenService.getToken2(clientNo, tokenType);
        String token = res.getData().getToken();
        logger.debug(clientNo + "-" + tokenType + ":获取令牌成功:" + token);
        return token;
    }
}
