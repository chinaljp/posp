package com.chanjet.service.autoquery;

import com.alibaba.fastjson.JSON;
import com.chanjet.entity.EposQueryDetailInfo;
import com.chanjet.entity.QrcodeQueryDetailInfo;
import com.chanjet.mapper.InlineJournalMapper;
import com.chanjet.mapper.SysParamMapper;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.redis.core.ListOperations;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import javax.annotation.Resource;
import java.util.*;
import java.util.concurrent.TimeUnit;

import static com.chanjet.constants.Constants.TIMEOUT;
import static com.chanjet.util.ByteUtil.convertHexString;
import static com.chanjet.util.ByteUtil.toHexString;

@Service
public class EposAutoQuery {
    private static final Logger logger = LoggerFactory.getLogger("eposQuery");
    @Autowired
    private InlineJournalMapper inlineJournalMapper;
    @Autowired
    private SysParamMapper sysParamMapper;
    @Autowired
    private RedisTemplate<String, String> redisTemplate;

    // inject the template as ListOperations
    @Resource(name="getRedisTemplate")
    private ListOperations<String, String> listOps;
    @Transactional
    public void doPost(){
        List<EposQueryDetailInfo> list = inlineJournalMapper.scanEposTrans();
        for (EposQueryDetailInfo eposQueryDetailInfo : list) {
            Map map = new HashMap();
            map.put("head","612100311000");
            //交易码转为查询交易码
            StringBuilder trans_code = new StringBuilder(eposQueryDetailInfo.getTrans_code());
            trans_code.replace(3,4,"3");

            map.put("trans_code",trans_code);
            map.put("merch_id",eposQueryDetailInfo.getMerch_id());
            map.put("traceNo",eposQueryDetailInfo.getTrace_no());
            map.put("o_rrn",eposQueryDetailInfo.getRrn());
            map.put("trans_date",eposQueryDetailInfo.getTrans_date());
            map.put("trans_time",eposQueryDetailInfo.getTrans_time());
            map.put("term_id",eposQueryDetailInfo.getTerm_id());
            map.put("tranTime",eposQueryDetailInfo.getTrans_date()+eposQueryDetailInfo.getTrans_time());
            map.put("amount",eposQueryDetailInfo.getAmount());

            logger.info("检索到Epos流水信息.");
            if(judgeTime(eposQueryDetailInfo.getCreate_time(),eposQueryDetailInfo.getCur_time(),eposQueryDetailInfo.getQuery_count()) > 0){
                //发起自动查询交易
                Map jsonMap = new TreeMap();
                jsonMap.put("data",map);
                jsonMap.put("svrid","eposQuery_P");
                jsonMap.put("key","eposQuery_"+eposQueryDetailInfo.getRrn());
                //jsonMap转换成json字符串
                String jsonString = JSON.toJSONString(jsonMap);
                logger.info("JSON:"+ jsonString);
                String hexString = toHexString(jsonString.getBytes());
                //发送到请求队列
                logger.info("请求消息队列名："+map.get("trans_code")+"_Q");
                logger.info("请求消息："+hexString);
                logger.info("LPUSH RET:"+listOps.leftPush(map.get("trans_code").toString()+"_Q",hexString));
                //更新查询次数
                inlineJournalMapper.updEposCnt(eposQueryDetailInfo.getRrn());
                //接收返回信息
                String hexStringRet = listOps.leftPop("eposQuery_P",TIMEOUT, TimeUnit.SECONDS);
                if(hexStringRet == null){
                    return;
                }
                //检查并打印返回信息
                logger.info("返回信息："+ new String(convertHexString(hexStringRet)));

            }

        }
    }

    public int judgeTime(Date createTime, Date curTime,Integer queryCount){
        //获取二维码自动查询间隔
        String queryInterval = sysParamMapper.findQRQueryInterval();
        String interval[] = queryInterval.split("[,]");

        //获取当前时间戳(秒级)
        Long timeStamp = curTime.getTime()/1000;
        logger.info("当前时间戳："+timeStamp);
        //交易时间与当前时间间隔
        logger.info("原交易时间戳:"+createTime.getTime()/1000);
        Long timeIntv = timeStamp - createTime.getTime()/1000;
        logger.info("当前时间戳-原交易时间戳:"+timeIntv);
        if(timeIntv <= Long.valueOf(interval[1]) && timeIntv > Long.valueOf(interval[0]) && queryCount < 1){
            logger.info("间隔参数0,1:"+Long.valueOf(interval[0]),Long.valueOf(interval[1]));
            return  1;
        }else if(timeIntv <= Long.valueOf(interval[2]) && timeIntv > Long.valueOf(interval[1]) && queryCount == 1){
            logger.info("间隔参数1,2:"+Long.valueOf(interval[1]),Long.valueOf(interval[2]));
            return  2;
        }else if(timeIntv <= Long.valueOf(interval[3]) && timeIntv > Long.valueOf(interval[2]) && queryCount == 2){
            logger.info("间隔参数2,3:"+Long.valueOf(interval[2]),Long.valueOf(interval[3]));
            return  3;
        }else if(timeIntv <= Long.valueOf(interval[4]) && timeIntv > Long.valueOf(interval[3]) && queryCount == 3){
            logger.info("间隔参数3,4:"+Long.valueOf(interval[3]),Long.valueOf(interval[4]));
            return  4;
        }else if(timeIntv <= Long.valueOf(interval[5]) && timeIntv > Long.valueOf(interval[4]) && queryCount == 4){
            logger.info("间隔参数4,5:"+Long.valueOf(interval[4]),Long.valueOf(interval[5]));
            return  5;
        }else if(timeIntv > Long.valueOf(interval[5]) && queryCount == 5) {
            logger.info("间隔参数5:" + Long.valueOf(interval[5]));
            return 6;
        }else {
            return 0;
        }
    }

}
