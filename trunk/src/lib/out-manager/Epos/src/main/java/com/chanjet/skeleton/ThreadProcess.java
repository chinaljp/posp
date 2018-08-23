package com.chanjet.skeleton;

import com.chanjet.constants.Constants;
import com.chanjet.transaction.Consume;
import com.chanjet.transaction.QueryTrans;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import redis.clients.jedis.Jedis;

import java.io.UnsupportedEncodingException;
import java.util.List;

import static com.chanjet.util.ByteUtil.convertHexString;
import static com.chanjet.util.ByteUtil.toHexString;

public class ThreadProcess implements Runnable{
    private static Logger logger = LogManager.getLogger(ThreadProcess.class);
    public void run() {
        String ReqJson = null;
        Jedis jedis = new Jedis(Constants.REDIS_IP,Constants.REDIS_PORT);
        jedis.connect();//连接
        Consume consume =  new Consume();
        QueryTrans queryTrans = new QueryTrans();
        while (true){
            List result = jedis.blpop(Constants.TIME_OUT,"QUICKPAY_Q");
//		解析取得的JSON数据
            if (!result.isEmpty()){
                try {
                    ReqJson = new String(convertHexString((String) result.get(1)),"GBK");
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
                MsgObject msgObject = com.alibaba.fastjson.JSON.parseObject(ReqJson, MsgObject.class);
				/*发送到支付网关*/
				if(msgObject.getData().getTrans_code().equals("0AQ000")){
				    //消费交易
                    consume.doPost(msgObject);
                }else if(msgObject.getData().getTrans_code().equals("0AQ300")){
				    //消费查询
                    queryTrans.doPost(msgObject);
                }

                String response = com.alibaba.fastjson.JSON.toJSONString(msgObject);
                //System.out.println(response);
                logger.info(response);

                String hexString = null;
                try {
                    //System.out.println(new String(response.getBytes("utf-8"),"GBK"));
                    hexString = toHexString(new String(response.getBytes("utf-8")).getBytes("GBK"));
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
                //System.out.println(hexString);

                /*发送返回信息到redis供c联机appsrv使用*/
                logger.info("交易码:"+msgObject.getData().getTrans_code());
                if(msgObject.getData().getTrans_code().equals("0AQ000")){
                    //消费交易
                    jedis.lpush("0AQ000_P", hexString);
                }else if(msgObject.getData().getTrans_code().equals("0AQ300")){
                    //消费查询
                    jedis.lpush("0AQ300_P", hexString);
                }
            }else {
                //ping,pongl
                logger.info("Server is running: "+jedis.ping());
            }
        }
//		jedis.disconnect();//断开连接
    }
}
