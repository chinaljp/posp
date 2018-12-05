package com.chanjet.transnotice.sdk;

import com.alibaba.fastjson.JSONObject;
import com.chanjet.transnotice.entity.RecvMsg;
import okhttp3.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.concurrent.TimeUnit;

public class HttpClient {
    private static Logger logger = LoggerFactory.getLogger(HttpClient.class);

    public static RecvMsg post(String sendInfo,String sUrl) throws Exception {
        OkHttpClient okHttpClient = new OkHttpClient.Builder()
                .connectTimeout(10, TimeUnit.SECONDS)
                .writeTimeout(10, TimeUnit.SECONDS)
                .readTimeout(20, TimeUnit.SECONDS)
                .build();
        //String str = new String(sendInfo.getBytes("GBK"),"utf-8");

        RequestBody requestBody = FormBody.create(MediaType.parse("application/json; charset=GBK")
                , sendInfo);
        Request request = new Request.Builder()
                .url(sUrl)//请求的url
                .post(requestBody)
                .build();
        //创建/Call
        Call call = okHttpClient.newCall(request);
        //加入队列 异步操作
       /* call.enqueue(new Callback() {
            //请求错误回调方法
            @Override
            public void onFailure(Call call, IOException e) {
                System.out.println("连接失败");
            }
            @Override
            public void onResponse(Call call, Response response) throws IOException {
                System.out.println(response.body().string());
            }
        });*/

        RecvMsg recvMsg = new RecvMsg();
        try {
            Response response = call.execute();
            String json = response.body().string();
            logger.info("Respons : ["+ json +"]");
            /*将返回信息转换为json 对象*/
            JSONObject jsonObject = JSONObject.parseObject(json);
            /*将 json 对象 转换为 java 对象（实体）*/
            recvMsg = JSONObject.toJavaObject(jsonObject,RecvMsg.class);
        } catch (IOException e) {
            e.printStackTrace();
        }

        return recvMsg;
    }
}