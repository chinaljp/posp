package com.chanjet.action;

import com.chanjet.service.WalletService;
import com.chanjet.util.WebParamUtils;
import com.chanjet.vo.BaseResMessage;
import com.chanjet.vo.WalletReq;
import org.apache.log4j.Logger;
import org.junit.Test;

import java.io.UnsupportedEncodingException;

/**
 * Created by feng.gaoo on 2017/3/25.
 */
public class OpenAccoutActionTest {
    private static Logger logger = Logger.getLogger(OpenAccoutActionTest.class);
    @Test
    public void openAccount() throws UnsupportedEncodingException {
        WalletReq accountReq = new WalletReq();
                    /* 组开户请求数据 */
        accountReq.setMerchantNo(WebParamUtils.getStringValue("900690054220022"));        //商户号
        accountReq.setUnionpayMerchantNo(WebParamUtils.getStringValue("900690054220022"));//银联商户号
        accountReq.setAgentNo(WebParamUtils.getStringValue("49000002"));            //代理商编号
        accountReq.setRealName(WebParamUtils.getStringValue("高峰"));                //真实姓名
        accountReq.setCompanyName(WebParamUtils.getStringValue("畅捷支付"));//企业名称
        accountReq.setAccountType(WebParamUtils.getIntegerValue(2)); //账户类型 1 个人 2 个体商户 3企业
        accountReq.setIdCardNo(WebParamUtils.getStringValue("210405197201010215X"));   //身份证号
        accountReq.setBankAccountNo(WebParamUtils.getStringValue("6214441000011010"));  //卡号
        accountReq.setBankAccountName(WebParamUtils.getStringValue("高峰"));  //账户名
        accountReq.setBankAccountType(WebParamUtils.getIntegerValue(2));//银行账户类型 1 对公 2 对私
        accountReq.setBankCode(WebParamUtils.getStringValue("105"));    //银行编码
        accountReq.setBankChannelNo(WebParamUtils.getStringValue("105110039436"));  //银行行号
        accountReq.setBankName(WebParamUtils.getStringValue("中国建设银行")); //银行名称
        accountReq.setBankSubName(WebParamUtils.getStringValue("中国建设银行股份有限公司天津明华支行"));  //银行支行
        accountReq.setBankProvince(WebParamUtils.getStringValue("天津市"));     //银行所属省
        accountReq.setBankCity(WebParamUtils.getStringValue("天津市"));     //银行所属市
        accountReq.setBankArea(WebParamUtils.getStringValue("天津市"));       //银行所属区域
        accountReq.setWithdrawDepositRate(WebParamUtils.getDoubleValue(0.02));   //提现费率
        accountReq.setWithdrawDepositSingleFee(WebParamUtils.getDoubleValue(2));     //提现单笔费用
        logger.debug("json:"+accountReq.toString());
        try {
            BaseResMessage<Object> resMessage = WalletService.openAccount(accountReq);
            if (resMessage == null) {
                return;
            }
            logger.debug("返回：" + resMessage.getCode() + "," + resMessage.getMessage());

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}