package com.chanjet.service.autoquery;

import com.alibaba.fastjson.JSON;
import com.chanjet.entity.EposQueryDetailInfo;
import com.chanjet.mapper.InlineJournalMapper;
import com.chanjet.mapper.SysParamMapper;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.redis.core.ListOperations;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;

import javax.annotation.Resource;
import java.util.*;
import java.util.concurrent.TimeUnit;

import static com.chanjet.constants.Constants.TIMEOUT;
import static com.chanjet.util.ByteUtil.convertHexString;
import static com.chanjet.util.ByteUtil.toHexString;

@Service
public class EposAutoQueryTimeoutPro {
    private static final Logger logger = LoggerFactory.getLogger("eposQuery");
    @Autowired
    private InlineJournalMapper inlineJournalMapper;
    @Autowired
    private RedisTemplate<String, String> redisTemplate;

    // inject the template as ListOperations
    @Resource(name = "getRedisTemplate")
    private ListOperations<String, String> listOps;

    public void getRet() {
        //接收返回信息
        String hexStringRet = listOps.leftPop("eposQuery_P", TIMEOUT, TimeUnit.SECONDS);
        if (hexStringRet == null) {
            return;
        }
        //检查并打印返回信息
        logger.info("返回信息：" + new String(convertHexString(hexStringRet)));
    }
}
