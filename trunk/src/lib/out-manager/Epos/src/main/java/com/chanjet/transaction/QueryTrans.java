package com.chanjet.transaction;

import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.TypeReference;
import com.chanjet.constants.Constants;
import com.chanjet.skeleton.MsgObject;
import com.chanjet.util.SignUtil;
import okhttp3.*;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.IOException;
import java.util.TreeMap;
import java.util.UUID;

public class QueryTrans {
	private static Logger logger = LogManager.getLogger(QueryTrans.class);
	public static final MediaType JSON = MediaType.parse("application/json; charset=utf-8");

	public  void doPost(MsgObject msgObject) {

		// 接口URL
		String url = Constants.getServerUrl() + "/gateway/channel/api/queryTrans";

		// 接入用户密钥
		String key = Constants.getKey();
		// 接入用户编号
		String accUsrNo = Constants.CLIENT_NO;
		String uuidStr = UUID.randomUUID().toString().replaceAll("-", "");

		// 请求参数
		String trxChnNo = "1010"; // 交易通道编号
		String trxTyp = "3001"; // 交易类型代码
		String trxId = uuidStr.substring(16); // 交易流水号
		String oriTrxTyp = "1001"; // 原交易类型代码
		String oriTrxId = msgObject.getData().getO_rrn(); // 原交易流水号
		String oriTrxDt = msgObject.getData().getO_trans_date(); // 原交易日期

		// 构建签名参数
		TreeMap<String, Object> signParams = new TreeMap<String, Object>();
		signParams.put("accUsrNo", accUsrNo);
		signParams.put("trxChnNo", trxChnNo);
		signParams.put("trxTyp", trxTyp);
		signParams.put("trxId", trxId);
		signParams.put("oriTrxTyp", oriTrxTyp);
		signParams.put("oriTrxId", oriTrxId);
		signParams.put("oriTrxDt", oriTrxDt);

		// 构建请求参数
		JSONObject jsonObj = new JSONObject();
		jsonObj.put("accUsrNo", accUsrNo);
		jsonObj.put("trxChnNo", trxChnNo);
		jsonObj.put("trxTyp", trxTyp);
		jsonObj.put("trxId", trxId);
		jsonObj.put("oriTrxTyp", oriTrxTyp);
		jsonObj.put("oriTrxId", oriTrxId);
		jsonObj.put("oriTrxDt", oriTrxDt);
		jsonObj.put("sign", SignUtil.signByMap(key, signParams));

		// 接口访问
		String jsonReq = jsonObj.toJSONString();
		logger.info("jsonReq: " + jsonReq);

		OkHttpClient client = new OkHttpClient();
		RequestBody body = RequestBody.create(JSON, jsonReq);
		Request request = new Request.Builder().url(url).post(body).build();
		Response response = null;
		try {
			response = client.newCall(request).execute();
		} catch (IOException e) {
			e.printStackTrace();
		}

		String jsonRsp = null;
		try {
			jsonRsp = response.body().string();
		} catch (IOException e) {
			e.printStackTrace();
		}
		logger.info("jsonRsp: " + jsonRsp);

		BaseResMessage<ChannelTransApiVo> res = null;
		if (response.isSuccessful()) {
			res = JSONObject.parseObject(jsonRsp, new TypeReference<BaseResMessage<ChannelTransApiVo>>() {
			});

			logger.info("\n接口响应内容：" + res.getData());
			if(res.getData() == null){
				msgObject.getData().setResp_code(res.getCode());
				msgObject.getData().setResp_desc(res.getMessage());
			}else {
				if(res.getData().getRpCd().equals("000000")){
					msgObject.getData().setResp_code("00");
					msgObject.getData().setResp_desc(res.getData().getRpDesc());
					msgObject.getData().setO_resp_desc(res.getData().getOriRpDesc());
					msgObject.getData().setTx_state(res.getData().getOriTrxStatus());

				}else {
					msgObject.getData().setResp_code("96");
					msgObject.getData().setResp_desc(res.getData().getRpDesc());
				}

			}
		} else {
			logger.info("响应码: " + response.code());
			msgObject.getData().setResp_code(String.valueOf(response.code()));
			msgObject.getData().setResp_desc(response.message());
			try {
				throw new IOException("Unexpected code " + response.message());
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}


}
