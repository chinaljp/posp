package com.chanjet.constants;

/**
 * 
 * 类描述：常量
 * 
 * @author lxl
 * @since 1.0, 2016年12月27日
 */
public final class Constants {

	private Constants() {

	}
	public static final String REDIS_IP = "localhost";
	public static final Integer REDIS_PORT = 6379;
	public static final Integer TIME_OUT = 3;
	public static final Integer MAX_THREAD = 10;

	public static final String DEVELOP_MODE = "0";
	public static final String CLIENT_NO = "4003";
	public static final String TEST_SERVER_URL = "http://10.255.0.113:7185";
	public static final String TEST_KEY = "0123456789ABCDEFFEDCBA9876543210";

	public static final String ONLINE_SERVER_URL = "https://qkchannel.chanpay.com:17080";
	//public static final String ONLINE_KEY = "FEDCBA98765432100123456789ABCDEF";
	public static final String ONLINE_KEY = "855C6C3E16EC9F6C855C6C3E16EC9F6C855C6C3E16EC9F6C";

	public static String getServerUrl() {
		// 生产模式
		if ("0".equals(DEVELOP_MODE)) {
			return ONLINE_SERVER_URL;
		}
		// 开发模式
		else {
			return TEST_SERVER_URL;
		}
	}

	public static String getKey() {
		// 生产模式
		if ("0".equals(DEVELOP_MODE)) {
			return ONLINE_KEY;
		}
		// 开发模式
		else {
			return TEST_KEY;
		}
	}

}
