package com.chanjet.constants;

/**
 * Created by feng.gaoo on 2017/3/23.
 */
public class ServerConstants {
    public static final String SERVER_URL = "http://123.103.9.207:7180";
    public static final String SERVER2_URL = "http://123.103.9.207:7182";
    //public static final String SERVER_URL = "http://10.20.8.109:7180";
    //public static final String SERVER2_URL = "http://10.20.8.109:7182";
    public static final String CLIENT_NO = "1000";

    public static String getClientNo() {
        return CLIENT_NO;
    }

    public static String getServerUrl() {
        return SERVER_URL;
    }
    public static String getServerUrl2() {
        return SERVER2_URL;
    }
}
