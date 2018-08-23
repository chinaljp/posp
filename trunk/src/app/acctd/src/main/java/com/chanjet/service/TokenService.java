package com.chanjet.service;

import com.alibaba.fastjson.TypeReference;
import com.chanjet.constants.KeyConstants;
import com.chanjet.constants.ServerConstants;
import com.chanjet.util.SignUtil;
import com.chanjet.vo.BaseResMessage;
import com.chanjet.vo.TokenRes;

import com.alibaba.fastjson.JSONObject;
import okhttp3.*;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.security.Key;
import java.util.TreeMap;

/**
 * Created by feng.gaoo on 2017/3/23.
 */
public class TokenService {
    private static Logger logger = Logger.getLogger(TokenService.class);
    public static final MediaType JSON = MediaType.parse("application/json; charset=utf-8");

    public BaseResMessage<TokenRes> getToken(String clientNo, String tokenType) throws IOException {
        String url = ServerConstants.getServerUrl() + "/v1/account/getToken";
        //2、构建签名参数
        TreeMap<String, Object> signParams = new TreeMap<String, Object>();
        signParams.put("clientNo", clientNo);
        signParams.put("tokenType", tokenType);
        JSONObject jsonObj = new JSONObject();
        jsonObj.put("clientNo", clientNo);
        jsonObj.put("tokenType", tokenType);
        jsonObj.put("sign", SignUtil.signByMap(KeyConstants.getKey(), signParams));
        String tokenJson = jsonObj.toJSONString();
        OkHttpClient client = new OkHttpClient();
        RequestBody body = RequestBody.create(JSON, tokenJson);
        Request request = new Request.Builder()
                .url(url)
                .post(body)
                .build();
        Response response = client.newCall(request).execute();
        String tokenBody = response.body().string();
        logger.debug(tokenBody);
        BaseResMessage<TokenRes> res = null;
        if (response.isSuccessful()) {
            res = JSONObject.parseObject(tokenBody, new TypeReference<BaseResMessage<TokenRes>>() {
            });
        } else {
            logger.debug("响应码: " + response.code());
            logger.debug("请求内容: " + response.body().string());
            throw new IOException("Unexpected code " + response);
        }
        return res;
    }

    /* 获取POS/二维码、转账、日结消费的token */
    public BaseResMessage<TokenRes> getToken2(String clientNo, String tokenType) throws IOException {
        String url = ServerConstants.getServerUrl2() + "/v1/account/getToken";
        //2、构建签名参数
        TreeMap<String, Object> signParams = new TreeMap<String, Object>();
        signParams.put("clientNo", clientNo);
        signParams.put("tokenType", tokenType);
        JSONObject jsonObj = new JSONObject();
        jsonObj.put("clientNo", clientNo);
        jsonObj.put("tokenType", tokenType);
        jsonObj.put("sign", SignUtil.signByMap(KeyConstants.getKey(), signParams));
        String tokenJson = jsonObj.toJSONString();
        OkHttpClient client = new OkHttpClient();
        RequestBody body = RequestBody.create(JSON, tokenJson);
        Request request = new Request.Builder()
                .url(url)
                .post(body)
                .build();
        Response response = client.newCall(request).execute();
        String tokenBody = response.body().string();
        logger.debug(tokenBody);
        BaseResMessage<TokenRes> res = null;
        if (response.isSuccessful()) {
            res = JSONObject.parseObject(tokenBody, new TypeReference<BaseResMessage<TokenRes>>() {
            });
        } else {
            logger.debug("响应码: " + response.code());
            logger.debug("请求内容: " + response.body().string());
            throw new IOException("Unexpected code " + response);
        }
        return res;
    }
}
