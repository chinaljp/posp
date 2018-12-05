package com.chanjet.transnotice.jobs;

import com.chanjet.transnotice.service.SendMsgInline;
import com.chanjet.transnotice.service.SendMsgInlineFail;
import com.chanjet.transnotice.service.SendMsgPos;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

/**
 * Created by Gjq on 2018/8/8.
 */
@Component
public class ScheduledTasks {

    private static final Logger logger = LoggerFactory.getLogger(ScheduledTasks.class);

    @Autowired
    private SendMsgPos sendMsgPos;

    @Autowired
    private SendMsgInline sendMsgInline;

    @Autowired
    private SendMsgInlineFail sendMsgInlineFail;

    @Scheduled(fixedDelay = 5000) /**fixedDelay 当一个线程的此任务执行完毕后 才会新启一个线程再次执行此任务*/
    public void sendMsgSplitForPos() throws Exception {
        sendMsgPos.sendMsgPosSplitting();
    }

    @Scheduled(fixedDelay = 5000)
    public void sendMsgSplitForInline() throws Exception {
        sendMsgInline.sendMsgInlineSplitting();
    }

    @Scheduled(fixedDelay = 5000)
    public void sendMsgSplitForInlineFail() throws Exception {
        sendMsgInlineFail.sendMsgInlineFailSplitting();
    }

}
