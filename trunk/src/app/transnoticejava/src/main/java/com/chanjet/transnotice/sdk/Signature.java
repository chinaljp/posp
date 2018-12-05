package com.chanjet.transnotice.sdk;

import com.chanjet.transnotice.entity.RecvMsg;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.security.NoSuchAlgorithmException;

public class Signature {
    private static final Logger logger = LoggerFactory.getLogger(Signature.class);
    public static String getMD5(String sKey, String sMsg) {
        String src = sKey + sMsg;
        byte[] source = src.getBytes();
        String s = null;
        char hexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'a', 'b', 'c', 'd', 'e', 'f' };// �������ֽ�ת����16���Ʊ�ʾ���ַ�
        java.security.MessageDigest md = null;
        try {
            md = java.security.MessageDigest.getInstance("MD5");
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        md.update(source);
        byte tmp[] = md.digest();// MD5 �ļ�������һ�� 128 λ�ĳ�������
        // ���ֽڱ�ʾ���� 16 ���ֽ�
        char str[] = new char[16 * 2];// ÿ���ֽ��� 16 ���Ʊ�ʾ�Ļ���ʹ�������ַ��� ���Ա�ʾ�� 16
        // ������Ҫ 32 ���ַ�
        int k = 0;// ��ʾת������ж�Ӧ���ַ�λ��
        for (int i = 0; i < 16; i++) {// �ӵ�һ���ֽڿ�ʼ���� MD5 ��ÿһ���ֽ�// ת���� 16
            // �����ַ���ת��
            byte byte0 = tmp[i];// ȡ�� i ���ֽ�
            str[k++] = hexDigits[byte0 >>> 4 & 0xf];// ȡ�ֽ��и� 4 λ������ת��,// >>>
            // Ϊ�߼����ƣ�������λһ������
            str[k++] = hexDigits[byte0 & 0xf];// ȡ�ֽ��е� 4 λ������ת��

        }
        s = new String(str);// ����Ľ��ת��Ϊ�ַ���
        return s;
    }

    /*��֤ǩ��*/
    public static boolean verificationSign(String key, RecvMsg recvMsg) throws Exception {
        String localSign = getMD5(key,Sort.sort(recvMsg));
        String oriSign = recvMsg.getSign();
        logger.info("ԭǩ������: ["+ oriSign +"],����ǩ������: ["+ localSign +"]");
        return ignoreCaseEquals(localSign,oriSign);
    }

    /*����Ӣ����ĸ��Сд�Ƚ�*/
    public static boolean ignoreCaseEquals(String str1,String str2){
        return str1 == null ? str2 == null :str1.equalsIgnoreCase(str2);
    }
}