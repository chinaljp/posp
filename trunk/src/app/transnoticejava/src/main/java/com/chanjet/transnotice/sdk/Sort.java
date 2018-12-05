package com.chanjet.transnotice.sdk;

import java.lang.reflect.Field;
import java.util.Iterator;
import java.util.TreeMap;

public class Sort {
    public static String sort(Object object) throws Exception {
        /*TreeMap Ĭ��������򣺰���key���ֵ�˳������������*/
        TreeMap<String, String> map = new TreeMap<String, String>();
        /*��ʵ����תΪmap*/
        for (Field field : object.getClass().getDeclaredFields()) {
            field.setAccessible(true);
            //logger.info( field.getName() + ":" + field.get(object);
            map.put(field.getName(), (String) field.get(object));
            if ("sign".equals(field.getName())) {
                map.remove(field.getName());
            }
        }
        //System.out.println("map : [" + map.entrySet() + "]");
        /*����ȡ��keyֵ����ַ���*/
        String ss = null;
        Iterator<String> iter =  map.keySet().iterator();
        while (iter.hasNext()) {
            String value = map.get(iter.next());
            if( ss == null ) {
                ss = value;
            }
            else {
                ss = ss + value;
            }
        }
        /*ȥ���ַ����е�null*/
        String msg = ss.replaceAll("null","").trim();
        return msg;
    }
}
