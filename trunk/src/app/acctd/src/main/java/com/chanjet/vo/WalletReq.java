package com.chanjet.vo;

/**
 * Created by feng.gaoo on 2017/3/25.
 */
public class WalletReq extends BaseReq {


    private static final long serialVersionUID = -8035323551251936329L;
    private String trans_code; //交易码
    private String accountUuid; //账户UUID
    private Integer frozenType; //出账状态 1 正常 2禁止出账（只进不出） 3 冻结（无法交易）
    private Integer walletType; //钱包类型 100 POS D0钱包状态 102 POS T1状态 200 二维码D0钱包状态 202 二维码T1钱包状态
    private String rrn;
    private Double amount;

    //账户修改使用属性
    private Integer changeType;        //修改类型：1为修改账户信息2为修改银行卡信息3为修改账户+银行卡信息

    //cp_ac_account部分
    private String realName;//真实姓名
    private String idCardNo;//身份证号
    private String companyName;//企业名称
    private Integer accountType;//账户类型1、个人 2、个体商户 3、企业

    //cp_ac_bankcard部分
    private String bankAccountNo;//卡号
    private String bankAccountName;//账户名（姓名)
    private Integer bankAccountType;//账户类型 1 对公 2 对私
    private String bankCode;//银行编码
    private String bankChannelNo;//银行行号
    private String bankName;//银行名称
    private String bankSubName;//银行支行名称
    private String bankProvince;//银行所属省
    private String bankCity;//银行所属市
    private String bankArea;//银行所属区域
    private String bankPhone;//绑定手机号
    private Integer bankCardType;//卡类型 1、借记卡 2、信用卡 3、准贷记卡 4、预付费卡
    private Integer bankCardOrg;//卡组织 1、银联 2、VISA 3、MasterCard （万事达） 4、JCB 5、America Express（美国运通） 6、Diners Club（大来）
    private String bankCardValidDate;//卡有效期 MM/YY

    private String merchantNo;            //商户号
    private String unionpayMerchantNo;    //银联商户号
    private String agentNo;                //代理商编号
    private Double withdrawDepositRate;        //提现费率
    private Double withdrawDepositSingleFee;        //提现单笔费用
    private String sys_trace; //流水号
    private String trans_date;
    private String trans_time;
    private String trans_type;
    private Double transAmount;        //交易金额
    private Double settleAmount;        //结算金额

    public String getSettle_date() {
        return settle_date;
    }

    public void setSettle_date(String settle_date) {
        this.settle_date = settle_date;
    }

    private String settle_date;

    public Integer getPostType() {
        return postType;
    }

    public void setPostType(Integer postType) {
        this.postType = postType;
    }

    private Integer postType;//入帐类型

    public String getTrans_type() {
        return trans_type;
    }

    public void setTrans_type(String trans_type) {
        this.trans_type = trans_type;
    }

    public String getTrans_date() {
        return trans_date;
    }

    public void setTrans_date(String trans_date) {
        this.trans_date = trans_date;
    }

    public String getTrans_time() {
        return trans_time;
    }

    public void setTrans_time(String trans_time) {
        this.trans_time = trans_time;
    }

    public String getSys_trace() {
        return sys_trace;
    }

    public void setSys_trace(String sys_trace) {
        this.sys_trace = sys_trace;
    }


    public Double getTransAmount() {
        return transAmount;
    }

    public void setTransAmount(Double transAmount) {
        this.transAmount = transAmount;
    }

    public Double getSettleAmount() {
        return settleAmount;
    }

    public void setSettleAmount(Double settleAmount) {
        this.settleAmount = settleAmount;
    }

    public Integer getChangeType() {
        return changeType;
    }

    public void setChangeType(Integer changeType) {
        this.changeType = changeType;
    }

    /**
     * 提现费率
     *
     * @param
     */
    public Double getWithdrawDepositRate() {
        return withdrawDepositRate;
    }

    /**
     * 提现费率
     *
     * @param withdrawDepositRate
     */
    public void setWithdrawDepositRate(Double withdrawDepositRate) {
        this.withdrawDepositRate = withdrawDepositRate;
    }

    /**
     * 提现单笔费用
     *
     * @param
     */
    public Double getWithdrawDepositSingleFee() {
        return withdrawDepositSingleFee;
    }

    /**
     * 提现单笔费用
     *
     * @param withdrawDepositSingleFee
     */
    public void setWithdrawDepositSingleFee(Double withdrawDepositSingleFee) {
        this.withdrawDepositSingleFee = withdrawDepositSingleFee;
    }

    /**
     * 真实姓名
     *
     * @return
     */
    public String getRealName() {
        return realName;
    }

    /**
     * 真实姓名
     *
     * @param realName
     */
    public void setRealName(String realName) {
        this.realName = realName;
    }

    /**
     * 身份证号
     *
     * @return
     */
    public String getIdCardNo() {
        return idCardNo;
    }

    /**
     * 身份证号
     *
     * @param idCardNo
     */
    public void setIdCardNo(String idCardNo) {
        this.idCardNo = idCardNo;
    }

    /**
     * 企业名称
     *
     * @return
     */
    public String getCompanyName() {
        return companyName;
    }

    /**
     * 企业名称
     *
     * @param companyName
     */
    public void setCompanyName(String companyName) {
        this.companyName = companyName;
    }

    /**
     * 账户类型1、个人 2、个体商户 3、企业
     *
     * @return
     */
    public Integer getAccountType() {
        return accountType;
    }

    /**
     * 账户类型1、个人 2、个体商户 3、企业
     *
     * @param accountType
     */
    public void setAccountType(Integer accountType) {
        this.accountType = accountType;
    }

    /**
     * 卡号
     *
     * @return
     */
    public String getBankAccountNo() {
        return bankAccountNo;
    }

    /**
     * 卡号
     *
     * @param bankAccountNo
     */
    public void setBankAccountNo(String bankAccountNo) {
        this.bankAccountNo = bankAccountNo;
    }

    /**
     * 账户名（姓名)
     *
     * @return
     */
    public String getBankAccountName() {
        return bankAccountName;
    }

    /**
     * 账户名（姓名)
     *
     * @param bankAccountName
     */
    public void setBankAccountName(String bankAccountName) {
        this.bankAccountName = bankAccountName;
    }

    /**
     * 账户类型 1 对公 2 对私
     *
     * @return
     */
    public Integer getBankAccountType() {
        return bankAccountType;
    }

    /**
     * 账户类型 1 对公 2 对私
     *
     * @param bankAccountType
     */
    public void setBankAccountType(Integer bankAccountType) {
        this.bankAccountType = bankAccountType;
    }

    /**
     * 银行编码
     *
     * @return
     */
    public String getBankCode() {
        return bankCode;
    }

    /**
     * 银行编码
     *
     * @param bankCode
     */
    public void setBankCode(String bankCode) {
        this.bankCode = bankCode;
    }

    /**
     * 银行行号
     *
     * @return
     */
    public String getBankChannelNo() {
        return bankChannelNo;
    }

    /**
     * 银行行号
     *
     * @param bankChannelNo
     */
    public void setBankChannelNo(String bankChannelNo) {
        this.bankChannelNo = bankChannelNo;
    }

    /**
     * 银行名称
     *
     * @return
     */
    public String getBankName() {
        return bankName;
    }

    /**
     * 银行名称
     *
     * @param bankName
     */
    public void setBankName(String bankName) {
        this.bankName = bankName;
    }

    /**
     * 银行支行名称
     *
     * @return
     */
    public String getBankSubName() {
        return bankSubName;
    }

    /**
     * 银行支行名称
     *
     * @param bankSubName
     */
    public void setBankSubName(String bankSubName) {
        this.bankSubName = bankSubName;
    }

    /**
     * 银行所属省
     *
     * @return
     */
    public String getBankProvince() {
        return bankProvince;
    }

    /**
     * 银行所属省
     *
     * @param bankProvince
     */
    public void setBankProvince(String bankProvince) {
        this.bankProvince = bankProvince;
    }

    /**
     * 银行所属市
     *
     * @return
     */
    public String getBankCity() {
        return bankCity;
    }

    /**
     * 银行所属市
     *
     * @param bankCity
     */
    public void setBankCity(String bankCity) {
        this.bankCity = bankCity;
    }

    /**
     * 银行所属区域
     *
     * @return
     */
    public String getBankArea() {
        return bankArea;
    }

    /**
     * 银行所属区域
     *
     * @param bankArea
     */
    public void setBankArea(String bankArea) {
        this.bankArea = bankArea;
    }

    /**
     * 绑定手机号
     *
     * @return
     */
    public String getBankPhone() {
        return bankPhone;
    }

    /**
     * 绑定手机号
     *
     * @param bankPhone
     */
    public void setBankPhone(String bankPhone) {
        this.bankPhone = bankPhone;
    }

    /**
     * 卡类型 1、借记卡 2、信用卡 3、准贷记卡 4、预付费卡
     *
     * @return
     */
    public Integer getBankCardType() {
        return bankCardType;
    }

    /**
     * 卡类型 1、借记卡 2、信用卡 3、准贷记卡 4、预付费卡
     *
     * @param bankCardType
     */
    public void setBankCardType(Integer bankCardType) {
        this.bankCardType = bankCardType;
    }

    /**
     * 卡组织 1、银联 2、VISA 3、MasterCard （万事达） 4、JCB 5、America Express（美国运通） 6、Diners Club（大来）
     *
     * @return
     */
    public Integer getBankCardOrg() {
        return bankCardOrg;
    }

    /**
     * 卡组织 1、银联 2、VISA 3、MasterCard （万事达） 4、JCB 5、America Express（美国运通） 6、Diners Club（大来）
     *
     * @param bankCardOrg
     */
    public void setBankCardOrg(Integer bankCardOrg) {
        this.bankCardOrg = bankCardOrg;
    }

    /**
     * 卡有效期 MM/YY
     *
     * @return
     */
    public String getBankCardValidDate() {
        return bankCardValidDate;
    }

    /**
     * 卡有效期 MM/YY
     *
     * @param bankCardValidDate
     */
    public void setBankCardValidDate(String bankCardValidDate) {
        this.bankCardValidDate = bankCardValidDate;
    }

    /**
     * 商户号
     *
     * @return
     */
    public String getMerchantNo() {
        return merchantNo;
    }

    /**
     * 商户号
     *
     * @param merchantNo
     */
    public void setMerchantNo(String merchantNo) {
        this.merchantNo = merchantNo;
    }

    /**
     * 银联商户号
     *
     * @return
     */
    public String getUnionpayMerchantNo() {
        return unionpayMerchantNo;
    }

    /**
     * 银联商户号
     *
     * @param unionpayMerchantNo
     */
    public void setUnionpayMerchantNo(String unionpayMerchantNo) {
        this.unionpayMerchantNo = unionpayMerchantNo;
    }

    /**
     * 代理商编号
     *
     * @return
     */
    public String getAgentNo() {
        return agentNo;
    }

    /**
     * 代理商编号
     *
     * @param agentNo
     */
    public void setAgentNo(String agentNo) {
        this.agentNo = agentNo;
    }

    public Double getAmount() {
        return amount;
    }

    public void setAmount(Double amount) {
        this.amount = amount;
    }

    public String getRrn() {
        return rrn;
    }

    public void setRrn(String rrn) {
        this.rrn = rrn;
    }


    public String getTrans_code() {
        return trans_code;
    }

    public void setTrans_code(String trans_code) {
        this.trans_code = trans_code;
    }

    /**
     * 获取uuid
     *
     * @param
     * @return uuid
     */
    public String getAccountUuid() {
        return accountUuid;
    }

    /**
     * 设置uuid
     *
     * @param accountUuid
     * @return void
     */
    public void setAccountUuid(String accountUuid) {
        this.accountUuid = accountUuid;
    }

    /**
     * 获取冻结类型
     *
     * @param
     * @return Integer
     */
    public Integer getFrozenType() {
        return frozenType;
    }

    /**
     * 设置冻结类型
     *
     * @param frozenType
     * @return void
     */
    public void setFrozenType(Integer frozenType) {
        this.frozenType = frozenType;
    }

    /**
     * 获取冻结钱包的类型
     *
     * @param
     * @return Integer
     */
    public Integer getWalletType() {
        return walletType;
    }

    /**
     * 设置冻结钱包类型
     *
     * @param walletType
     * @return void
     */
    public void setWalletType(Integer walletType) {
        this.walletType = walletType;
    }

    @Override
    public String toString() {
        return "WalletReq{" +
                "trans_code='" + trans_code + '\'' +
                ", accountUuid='" + accountUuid + '\'' +
                ", frozenType=" + frozenType +
                ", walletType=" + walletType +
                ", rrn='" + rrn + '\'' +
                ", amount=" + amount +
                ", changeType=" + changeType +
                ", realName='" + realName + '\'' +
                ", idCardNo='" + idCardNo + '\'' +
                ", companyName='" + companyName + '\'' +
                ", accountType=" + accountType +
                ", bankAccountNo='" + bankAccountNo + '\'' +
                ", bankAccountName='" + bankAccountName + '\'' +
                ", bankAccountType=" + bankAccountType +
                ", bankCode='" + bankCode + '\'' +
                ", bankChannelNo='" + bankChannelNo + '\'' +
                ", bankName='" + bankName + '\'' +
                ", bankSubName='" + bankSubName + '\'' +
                ", bankProvince='" + bankProvince + '\'' +
                ", bankCity='" + bankCity + '\'' +
                ", bankArea='" + bankArea + '\'' +
                ", bankPhone='" + bankPhone + '\'' +
                ", bankCardType=" + bankCardType +
                ", bankCardOrg=" + bankCardOrg +
                ", bankCardValidDate='" + bankCardValidDate + '\'' +
                ", merchantNo='" + merchantNo + '\'' +
                ", unionpayMerchantNo='" + unionpayMerchantNo + '\'' +
                ", agentNo='" + agentNo + '\'' +
                ", withdrawDepositRate=" + withdrawDepositRate +
                ", withdrawDepositSingleFee=" + withdrawDepositSingleFee +
                ", sys_trace='" + sys_trace + '\'' +
                ", trans_date='" + trans_date + '\'' +
                ", trans_time='" + trans_time + '\'' +
                ", trans_type='" + trans_type + '\'' +
                ", transAmount=" + transAmount +
                ", settleAmount=" + settleAmount +
                ", settle_date='" + settle_date + '\'' +
                ", postType=" + postType +
                '}';
    }
}
