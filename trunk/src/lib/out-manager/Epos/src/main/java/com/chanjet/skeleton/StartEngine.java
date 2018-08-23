package com.chanjet.skeleton;

import com.chanjet.constants.Constants;
import com.chanjet.transaction.Consume;
import com.chanjet.util.EncryptUtil;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import redis.clients.jedis.Jedis;

import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Created by Pay on 2017/2/16.
 */
public class StartEngine {

    private static Logger logger = LogManager.getLogger(StartEngine.class);
    public static void main(String[] args) throws Exception {


        ExecutorService cachedThreadPool = Executors.newFixedThreadPool(Constants.MAX_THREAD);
        for (int i = 0; i < Constants.MAX_THREAD; i++){
            cachedThreadPool.execute(new ThreadProcess());
        }

        logger.debug(" ...");

    }
}
