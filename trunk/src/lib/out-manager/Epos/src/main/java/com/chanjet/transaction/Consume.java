package com.chanjet.transaction;


import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.TypeReference;
import com.chanjet.constants.Constants;
import com.chanjet.skeleton.MsgObject;
import com.chanjet.util.EncryptUtil;
import com.chanjet.util.SignUtil;
import okhttp3.*;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import java.io.IOException;
import java.util.TreeMap;
import java.util.UUID;



public class Consume{
	private static Logger logger = LogManager.getLogger(Consume.class);
	public static final MediaType JSON = MediaType.parse("application/json; charset=utf-8");
	public void doPost(MsgObject msgObject){
		// 接口URL
		String url = Constants.getServerUrl() + "/gateway/channel/api/consume";

		// 接入用户密钥
		String key = Constants.getKey();
		// 接入用户编号
		String accUsrNo = Constants.CLIENT_NO;
		String uuidStr = UUID.randomUUID().toString().replaceAll("-", "");

		/** ----------------------------- 新无卡-消费 Start --------------------------- */
		// ------------------------------------------------------------------ 签约消费
		// 请求参数
		String trxChnNo = "1010"; // 交易通道编号
		String trxTyp = "1001"; // 交易类型代码
		String trxId = msgObject.getData().getRrn();// 交易流水号
		StringBuffer stringBuffer = new StringBuffer(msgObject.getData().getAmount().toString());
		if(stringBuffer.length()==1){
			stringBuffer.insert(0,"00");
		}else if(stringBuffer.length()==2){
			stringBuffer.insert(0,"0");
		}
		String trxAmt = stringBuffer.insert(stringBuffer.length()-2,'.').toString(); // 交易金额
		String mrchntNo = msgObject.getData().getChannel_merch_id(); // 商户编号
		String mrchntNm = msgObject.getData().getChannel_merch_name(); // 商户名称
		String mrchntTpId = msgObject.getData().getChannel_merch_id().substring(7,11); // 商户类别
		String cardNoText = msgObject.getData().getCard_no(); // 银行卡号
		String expDt = msgObject.getData().getExpDt(); //银行卡有效期


		String subMrchntNo = null; // 二级商户编号
		String subMrchntNm = null; // 二级商户名称
		String idTp = null; // 证件类型
		String idNoText = null; // 证件号码
		String mobNoText = null; // 预留手机号
		String cardNm = null; // 银行账户姓名
		String cvnNoText = null; // 银行卡安全码
//		String expDt = null; // 银行卡有效期
		String authMsg = null; // 动态验证码
		String oriTrxTyp = null; // 原交易类型代码
		String oriTrxId = null; // 原交易流水号

		// ------------------------------------------------------------------ 动态验证码消费
		// 请求参数
		// String trxChnNo = "1100"; // 交易通道编号
		// String trxTyp = "1002"; // 交易类型代码
		// String trxId = uuidStr.substring(16); // 交易流水号
		// String trxAmt = "100.00"; // 交易金额
		// String mrchntNo = "900100054110006"; // 商户编号
		// String mrchntNm = "acbd123"; // 商户名称
		// String mrchntTpId = "5411"; // 商户类别
		// String cardNoText = "6212143000000000037"; // 银行卡号
		// String authMsg = "123456"; // 动态验证码
		// String oriTrxTyp = "0011"; // 原交易类型代码
		// String oriTrxId = "82cdee69725c0000"; // 原交易流水号
		//
		// String subMrchntNo = null; // 二级商户编号
		// String subMrchntNm = null; // 二级商户名称
		// String idTp = null; // 证件类型
		// String idNoText = null; // 证件号码
		// String mobNoText = null; // 预留手机号
		// String cardNm = null; // 银行账户姓名
		// String cvnNoText = null; // 银行卡安全码
		// String expDt = null; // 银行卡有效期
		/** ----------------------------- 新无卡-消费 End ----------------------------- */

		/** ----------------------------- 全渠道（商户接入）-消费 Start ---------------- */
		// ------------------------------------------------------------------ 签约消费
		// 请求参数
		// String trxChnNo = "1000"; // 交易通道编号
		// String trxTyp = "1001"; // 交易类型代码
		// String trxId = uuidStr.substring(16); // 交易流水号
		// String trxAmt = "100.00"; // 交易金额
		// String mrchntNo = "900110045119997"; // 商户编号
		// String cardNoText = "6216261000000000018"; // 银行卡号
		//
		// String mrchntNm = null; // 商户名称
		// String mrchntTpId = null; // 商户类别
		// String subMrchntNo = null; // 二级商户编号
		// String subMrchntNm = null; // 二级商户名称
		// String idTp = null; // 证件类型
		// String idNoText = null; // 证件号码
		// String mobNoText = null; // 预留手机号
		// String cardNm = null; // 银行账户姓名
		// String cvnNoText = null; // 银行卡安全码
		// String expDt = null; // 银行卡有效期
		// String authMsg = null; // 动态验证码
		// String oriTrxTyp = null; // 原交易类型代码
		// String oriTrxId = null; // 原交易流水号

		// ------------------------------------------------------------------ 动态验证码签约消费
		// 请求参数
		// String trxChnNo = "1000"; // 交易通道编号
		// String trxTyp = "1004"; // 交易类型代码
		// String trxId = uuidStr.substring(16); // 交易流水号
		// String trxAmt = "100.00"; // 交易金额
		// String mrchntNo = "900110045119997"; // 商户编号
		// String cardNoText = "6216261000000000018"; // 银行卡号
		// String authMsg = "111111"; // 动态验证码
		// String oriTrxTyp = "0011"; // 原交易类型代码
		// String oriTrxId = "8680b5ad99a1b95e"; // 原交易流水号
		//
		// String mrchntNm = null; // 商户名称
		// String mrchntTpId = null; // 商户类别
		// String subMrchntNo = null; // 二级商户编号
		// String subMrchntNm = null; // 二级商户名称
		// String idTp = null; // 证件类型
		// String idNoText = null; // 证件号码
		// String mobNoText = null; // 预留手机号
		// String cardNm = null; // 银行账户姓名
		// String cvnNoText = null; // 银行卡安全码
		// String expDt = null; // 银行卡有效期
		/** ----------------------------- 全渠道（商户接入）-消费 End ------------------ */

		/** ----------------------------- 全渠道（机构接入）-消费 Start ---------------- */
		// ------------------------------------------------------------------ 签约消费
		// 请求参数
		// String trxChnNo = "1010"; // 交易通道编号
		// String trxTyp = "1001"; // 交易类型代码
		// String trxId = uuidStr.substring(16); // 交易流水号
		// String trxAmt = "100.00"; // 交易金额
		// String mrchntNo = "900475849000222"; // 商户编号
		// String mrchntNm = "acbd123"; // 商户名称
		// String mrchntTpId = "5411"; // 商户类别
		// String cardNoText = "6216261000000000018"; // 银行卡号
		// String expDt = "0719"; // 银行卡有效期
		//
		// String subMrchntNo = null; // 二级商户编号
		// String subMrchntNm = null; // 二级商户名称
		// String idTp = null; // 证件类型
		// String idNoText = null; // 证件号码
		// String mobNoText = null; // 预留手机号
		// String cardNm = null; // 银行账户姓名
		// String cvnNoText = null; // 银行卡安全码
		// String authMsg = null; // 动态验证码
		// String oriTrxTyp = null; // 原交易类型代码
		// String oriTrxId = null; // 原交易流水号
		/** ----------------------------- 全渠道（机构接入）-消费 End ------------------ */

		// 敏感数据3DES加密
		String cardNo = null;
		String mobNo = null;
		String idNo = null;
		String cvnNo = null;
		try {
			cardNo = EncryptUtil.desEncrypt(cardNoText, key);
			mobNo = EncryptUtil.desEncrypt(mobNoText, key);
			idNo = EncryptUtil.desEncrypt(idNoText, key);
			if (StringUtils.isNotEmpty(cvnNoText))
				cvnNo = EncryptUtil.desEncrypt(cvnNoText, key);
		} catch (Exception e) {
			e.printStackTrace();
		}

		// 构建签名参数
		TreeMap<String, Object> signParams = new TreeMap<String, Object>();
		signParams.put("accUsrNo", accUsrNo);
		signParams.put("trxChnNo", trxChnNo);
		signParams.put("trxTyp", trxTyp);
		signParams.put("trxId", trxId);
		signParams.put("trxAmt", trxAmt);
		signParams.put("mrchntNo", mrchntNo);
		signParams.put("mrchntNm", mrchntNm);
		signParams.put("mrchntTpId", mrchntTpId);
		signParams.put("subMrchntNo", subMrchntNo);
		signParams.put("subMrchntNm", subMrchntNm);
		signParams.put("idTp", idTp);
		signParams.put("idNo", idNoText);
		signParams.put("mobNo", mobNoText);
		signParams.put("cardNm", cardNm);
		signParams.put("cardNo", cardNoText);
		signParams.put("cvnNo", cvnNoText);
		signParams.put("expDt", expDt);
		signParams.put("authMsg", authMsg);
		signParams.put("oriTrxTyp", oriTrxTyp);
		signParams.put("oriTrxId", oriTrxId);

		// 构建请求参数
		JSONObject jsonObj = new JSONObject();
		jsonObj.put("accUsrNo", accUsrNo);
		jsonObj.put("trxChnNo", trxChnNo);
		jsonObj.put("trxTyp", trxTyp);
		jsonObj.put("trxId", trxId);
		jsonObj.put("trxAmt", trxAmt);
		jsonObj.put("mrchntNo", mrchntNo);
		jsonObj.put("mrchntNm", mrchntNm);
		jsonObj.put("mrchntTpId", mrchntTpId);
		jsonObj.put("subMrchntNo", subMrchntNo);
		jsonObj.put("subMrchntNm", subMrchntNm);
		jsonObj.put("idTp", idTp);
		jsonObj.put("idNo", idNo);
		jsonObj.put("mobNo", mobNo);
		jsonObj.put("cardNm", cardNm);
		jsonObj.put("cardNo", cardNo);
		jsonObj.put("cvnNo", cvnNo);
		jsonObj.put("expDt", expDt);
		jsonObj.put("authMsg", authMsg);
		jsonObj.put("oriTrxTyp", oriTrxTyp);
		jsonObj.put("oriTrxId", oriTrxId);
		jsonObj.put("sign", SignUtil.signByMap(key, signParams));


		// 接口访问
		String jsonReq = jsonObj.toJSONString();
		System.out.println("jsonReq: " + jsonReq);
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
		//System.out.println("jsonRsp: " + jsonRsp);
		logger.info("jsonRsp: " + jsonRsp);

		msgObject.getData().setChannel_merch_name(null);
		msgObject.getData().setMerch_name(null);
		msgObject.getData().setMerch_p_name(null);
		BaseResMessage<ChannelTransApiVo> res = null;
		if (response.isSuccessful()) {
			res = JSONObject.parseObject(jsonRsp, new TypeReference<BaseResMessage<ChannelTransApiVo>>() {
			});

			//System.out.println("接口响应内容：" + res.getData());
			logger.info("接口响应内容：" + res.getData());
			if(res.getData() == null){
				msgObject.getData().setResp_code(res.getCode());
				msgObject.getData().setResp_desc(res.getMessage());
			}else {
				msgObject.getData().setResp_code(res.getData().getRpCd());
				msgObject.getData().setResp_desc(res.getData().getRpDesc());
			}

		} else {
			//System.out.println("响应码: " + response.code());
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
