package com.chanjet.action;

import com.chanjet.service.WalletService;
import com.chanjet.util.WebParamUtils;
import com.chanjet.vo.WalletReq;
import org.junit.Test;

/**
 * Created by feng.gaoo on 2017/3/25.
 */
public class ChangeAccountActionTest {
    @Test
    public void changeAccount() throws Exception {
        WalletReq changeAccountReq = new WalletReq();
        changeAccountReq.setAccountUuid("1b73ca6c-638c-49d0-9ce1-6dc3c75e519e");

        changeAccountReq.setRealName(WebParamUtils.getStringValue("李伟"));                //真实姓名
        changeAccountReq.setCompanyName(WebParamUtils.getStringValue("畅捷支付"));//企业名称
        changeAccountReq.setAccountType(WebParamUtils.getIntegerValue(2)); //账户类型 1 个人 2 个体商户 3企业
        changeAccountReq.setIdCardNo(WebParamUtils.getStringValue("210405197201010215X"));   //身份证号
        changeAccountReq.setBankAccountNo(WebParamUtils.getStringValue("6214441000011010"));  //卡号
        changeAccountReq.setBankAccountName(WebParamUtils.getStringValue("李伟"));  //账户名
        changeAccountReq.setBankAccountType(WebParamUtils.getIntegerValue(2));//银行账户类型 1 对公 2 对私
        changeAccountReq.setBankCode(WebParamUtils.getStringValue("105"));    //银行编码
        changeAccountReq.setBankChannelNo(WebParamUtils.getStringValue("105110039436"));  //银行行号
        changeAccountReq.setBankName(WebParamUtils.getStringValue("中国建设银行")); //银行名称
        changeAccountReq.setBankSubName(WebParamUtils.getStringValue("中国建设银行股份有限公司天津明华支行"));  //银行支行
        changeAccountReq.setBankProvince(WebParamUtils.getStringValue("天津市"));     //银行所属省
        changeAccountReq.setBankCity(WebParamUtils.getStringValue("天津市"));     //银行所属市
        changeAccountReq.setBankArea(WebParamUtils.getStringValue("天津市"));       //银行所属区域

        changeAccountReq.setChangeType(3);

        WalletService.changeAccount(changeAccountReq);
    }

}