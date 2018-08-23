package com.chanjet.jobs;


import com.chanjet.service.autoquery.EposAutoQuery;
import com.chanjet.service.autoquery.EposAutoQueryTimeoutPro;
import com.chanjet.service.autoquery.QrcodeAutoQuery;
import com.chanjet.service.autoquery.QrcodeAutoQueryTimeoutPro;
import com.chanjet.service.feesplit.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import static com.chanjet.constants.Constants.ROWNUM;

@Component
public class ScheduledTasks {

    private static final Logger log = LoggerFactory.getLogger(ScheduledTasks.class);

    @Autowired
    private FeeSplitForEposService feeSplitForEposService;
    @Autowired
    private FeeSplitForEposHisService feeSplitForEposHisService;
    @Autowired
    private FeeSplitForQrcodeService feeSplitForQrcodeService;
    @Autowired
    private FeeSplitForQrcodeHisService feeSplitForQrcodeHisService;
    @Autowired
    private FeeSplitForD0Service feeSplitForD0Service;
    @Autowired
    private QrcodeAutoQuery qrcodeAutoQuery;
    @Autowired
    private QrcodeAutoQueryTimeoutPro qrcodeAutoQueryTimeoutPro;
    @Autowired
    private EposAutoQuery eposAutoQuery;
    @Autowired
    private EposAutoQueryTimeoutPro eposAutoQueryTimeoutPro;
    @Scheduled(fixedRate = 30000)
    public void feeSplitForEpos() throws Exception {
        feeSplitForEposService.feeSplitting(ROWNUM);
    }
    @Scheduled(fixedRate = 300000)
    public void feeSplitForEposHis() throws Exception {
        feeSplitForEposHisService.feeSplitting(ROWNUM);
    }
    @Scheduled(fixedRate = 30000)
    public void feeSplitForQrcode() throws Exception {
        feeSplitForQrcodeService.feeSplitting(ROWNUM);
    }
    @Scheduled(fixedRate = 300000)
    public void feeSplitForQrcodeHis() throws Exception {
        feeSplitForQrcodeHisService.feeSplitting(ROWNUM);
    }
    @Scheduled(fixedRate = 30000)
    public void feeSplitForD0() throws Exception {
        feeSplitForD0Service.feeSplitting(ROWNUM);
    }
    @Scheduled(fixedRate = 5000)
    public void qrcodeAutoQuery() throws Exception {
        qrcodeAutoQuery.doPost();
    }
    @Scheduled(fixedRate = 5000)
    public void qrcodeAutoQueryTimeoutPro() throws Exception {
        qrcodeAutoQueryTimeoutPro.getRet();
    }
    @Scheduled(fixedRate = 5000)
    public void eposAutoQuery() throws Exception {
        eposAutoQuery.doPost();
    }
    @Scheduled(fixedRate = 5000)
    public void eposAutoQueryRspTimeoutPro() throws Exception {
        eposAutoQueryTimeoutPro.getRet();
    }
}