package com.chanjet.service;

import com.alibaba.fastjson.JSONObject;
import com.chanjet.constants.KeyConstants;
import com.chanjet.constants.ServerConstants;
import com.chanjet.util.EncryptUtil;
import com.chanjet.util.SignUtil;
import com.chanjet.util.WebParamUtils;
import com.chanjet.vo.*;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.util.TreeMap;

/**
 * @Author Gaofeng
 * @Date 2017/3/25 17:15
 * @comment 钱包操作
 */
public class WalletService extends BaseService {
    private static Logger logger = Logger.getLogger(WalletService.class);

    public static BaseResMessage<Object> openAccount(WalletReq walletReq) throws Exception {
        String tokenType = WebParamUtils.getStringValue("10");                    //开通账户10 令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码
        String url = ServerConstants.getServerUrl() + "/v1/account/openAccountForPosp";

        logger.debug("url:" + url);
        /* 获取令牌 */
        String token = getToken(clientNo, tokenType);
        logger.info("注册获取令牌成功:" + url + "," + token);

        String merchantNoSource = WebParamUtils.getStringValue(walletReq.getMerchantNo());        //商户号
        String unionpayMerchantNoSource = WebParamUtils.getStringValue(walletReq.getMerchantNo());//银联商户号
        String agentNoSource = WebParamUtils.getStringValue(walletReq.getAgentNo());            //代理商编号
        String realNameSource = WebParamUtils.getStringValue(walletReq.getRealName());                //真实姓名
        String companyNameSource = WebParamUtils.getStringValue(walletReq.getCompanyName());        //企业名称
        Integer accountType = WebParamUtils.getIntegerValue(walletReq.getAccountType());                    //账户类型 1 个人 2 个体商户 3企业
        String idCardNoSource = WebParamUtils.getStringValue(walletReq.getIdCardNo());        //身份证号
        String bankAccountNoSource = WebParamUtils.getStringValue(walletReq.getBankAccountNo());        //卡号
        String bankAccountNameSource = WebParamUtils.getStringValue(walletReq.getBankAccountName());    //账户名
        Integer bankAccountType = WebParamUtils.getIntegerValue(walletReq.getBankAccountType());                //银行账户类型 1 对公 2 对私
        String bankCode = WebParamUtils.getStringValue(walletReq.getBankCode());                    //银行编码
        String bankChannelNo = WebParamUtils.getStringValue(walletReq.getBankChannelNo());                //银行行号
        String bankName = WebParamUtils.getStringValue(walletReq.getBankName());                //银行名称
        String bankSubName = WebParamUtils.getStringValue(walletReq.getBankSubName());                //银行支行
        String bankProvince = WebParamUtils.getStringValue(walletReq.getBankProvince());                //银行所属省
        String bankCity = WebParamUtils.getStringValue(walletReq.getBankCity());                    //银行所属市
        String bankArea = WebParamUtils.getStringValue(walletReq.getBankArea());                    //银行所属区域
        Double withdrawDepositRate = WebParamUtils.getDoubleValue(walletReq.getWithdrawDepositRate());        //提现费率
        Double withdrawDepositSingleFee = WebParamUtils.getDoubleValue(walletReq.getWithdrawDepositSingleFee());        //提现单笔费用

        //2、DES加密参数
        String key = KeyConstants.getKey();
        String merchantNo = null;
        if (StringUtils.isNotEmpty(merchantNoSource)) {
            merchantNo = EncryptUtil.desEncrypt(merchantNoSource, key);
        }
        String unionpayMerchantNo = null;
        if (StringUtils.isNotEmpty(unionpayMerchantNoSource)) {
            unionpayMerchantNo = EncryptUtil.desEncrypt(unionpayMerchantNoSource, key);
        }
        String agentNo = null;
        if (StringUtils.isNotEmpty(agentNoSource)) {
            agentNo = EncryptUtil.desEncrypt(agentNoSource, key);
        }
        String realName = null;
        if (StringUtils.isNotEmpty(realNameSource)) {
            realName = EncryptUtil.desEncrypt(realNameSource, key);
        }
        String companyName = null;
        if (StringUtils.isNotEmpty(companyNameSource)) {
            companyName = EncryptUtil.desEncrypt(companyNameSource, key);
        }
        String idCardNo = null;
        if (StringUtils.isNotEmpty(idCardNoSource)) {
            idCardNo = EncryptUtil.desEncrypt(idCardNoSource, key);
        }
        String bankAccountNo = null;
        if (StringUtils.isNotEmpty(bankAccountNoSource)) {
            bankAccountNo = EncryptUtil.desEncrypt(bankAccountNoSource, key);
        }
        String bankAccountName = null;
        if (StringUtils.isNotEmpty(bankAccountNameSource)) {
            bankAccountName = EncryptUtil.desEncrypt(bankAccountNameSource, key);
        }
        //2、构建签名参数
        TreeMap<String, Object> accountSignParams = new TreeMap<String, Object>();
        accountSignParams.put("token", token);
        accountSignParams.put("clientNo", clientNo);
        accountSignParams.put("merchantNo", merchantNoSource);
        accountSignParams.put("unionpayMerchantNo", unionpayMerchantNoSource);
        accountSignParams.put("agentNo", agentNoSource);
        accountSignParams.put("realName", realNameSource);
        accountSignParams.put("idCardNo", idCardNoSource);
        accountSignParams.put("companyName", companyNameSource);
        accountSignParams.put("accountType", accountType);
        accountSignParams.put("bankAccountNo", bankAccountNoSource);
        accountSignParams.put("bankAccountName", bankAccountNameSource);
        accountSignParams.put("bankAccountType", bankAccountType);
        accountSignParams.put("bankCode", bankCode);
        accountSignParams.put("bankChannelNo", bankChannelNo);
        accountSignParams.put("bankName", bankName);
        accountSignParams.put("bankSubName", bankSubName);
        accountSignParams.put("bankProvince", bankProvince);
        accountSignParams.put("bankCity", bankCity);
        accountSignParams.put("bankArea", bankArea);
        accountSignParams.put("withdrawDepositRate", withdrawDepositRate);
        accountSignParams.put("withdrawDepositSingleFee", withdrawDepositSingleFee);

        JSONObject jsonAccount = new JSONObject();
        jsonAccount.put("sign", SignUtil.signByMap(KeyConstants.getKey(), accountSignParams));

        jsonAccount.put("token", token);
        jsonAccount.put("clientNo", clientNo);
        jsonAccount.put("merchantNo", merchantNo);
        jsonAccount.put("idCardNo", idCardNo);
        jsonAccount.put("unionpayMerchantNo", unionpayMerchantNo);
        jsonAccount.put("agentNo", agentNo);
        jsonAccount.put("realName", realName);
        jsonAccount.put("companyName", companyName);
        jsonAccount.put("accountType", accountType);
        jsonAccount.put("bankAccountNo", bankAccountNo);
        jsonAccount.put("bankAccountName", bankAccountName);
        jsonAccount.put("bankAccountType", bankAccountType);
        jsonAccount.put("bankCode", bankCode);
        jsonAccount.put("bankChannelNo", bankChannelNo);
        jsonAccount.put("bankName", bankName);
        jsonAccount.put("bankSubName", bankSubName);
        jsonAccount.put("bankProvince", bankProvince);
        jsonAccount.put("bankCity", bankCity);
        jsonAccount.put("bankArea", bankArea);
        jsonAccount.put("withdrawDepositRate", withdrawDepositRate);
        jsonAccount.put("withdrawDepositSingleFee", withdrawDepositSingleFee);
        String accountJson = jsonAccount.toJSONString();

        return post(url, accountJson);
    }

    public static BaseResMessage<Object> changeAccount(WalletReq walletReq) throws Exception {
        String tokenType = WebParamUtils.getStringValue("11");    //账户信息变更 11 令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码
        //1、调用API
        String url = ServerConstants.getServerUrl() + "/v1/account/changeAccount";
                /* 获取令牌 */
        String token = getToken(clientNo, tokenType);

        Integer changeType = WebParamUtils.getIntegerValue(walletReq.getChangeType());                //变更类型 1为修改账户信息2为修改银行卡信息3为修改账户+银行卡信息
        String reqFlowNo = WebParamUtils.getStringValue(walletReq.getRrn());        //请求流水号
        String accountUuid = WebParamUtils.getStringValue(walletReq.getAccountUuid());//账户UUID
        Integer accountType = WebParamUtils.getIntegerValue(walletReq.getAccountType());                //账户类型 1、个人 2、个体工商户 3、企业
        Integer bankAccountType = WebParamUtils.getIntegerValue(walletReq.getBankAccountType());            //银行账户类型 1、对公 2、对私
        String bankCode = WebParamUtils.getStringValue(walletReq.getBankCode());            //银行编码
        String bankChannelNo = WebParamUtils.getStringValue(walletReq.getBankChannelNo());            //银行行号
        String bankName = WebParamUtils.getStringValue(walletReq.getBankName());            //银行名称
        String bankSubName = WebParamUtils.getStringValue(walletReq.getBankSubName());    //银行支行名称
        String bankProvince = WebParamUtils.getStringValue(walletReq.getBankProvince());            //银行所属省
        String bankCity = WebParamUtils.getStringValue(walletReq.getBankCity());                //银行所属市
        String bankArea = WebParamUtils.getStringValue(walletReq.getBankArea());                //银行所属区域
        String realNameSource = WebParamUtils.getStringValue(walletReq.getRealName());        //真实姓名
        String idCardNoSource = WebParamUtils.getStringValue(walletReq.getIdCardNo());    //身份证号码
        String companyNameSource = WebParamUtils.getStringValue(walletReq.getCompanyName());        //企业名称
        String bankAccountNoSource = WebParamUtils.getStringValue(walletReq.getBankAccountNo());    //卡号
        String bankAccountNameSource = WebParamUtils.getStringValue(walletReq.getBankAccountName());//账户名

        //2、DES加密参数
        String key = KeyConstants.getKey();
        String realName = null;
        if (StringUtils.isNotEmpty(realNameSource)) {
            realName = EncryptUtil.desEncrypt(realNameSource, key);
        }
        String idCardNo = null;
        if (StringUtils.isNotEmpty(idCardNoSource)) {
            idCardNo = EncryptUtil.desEncrypt(idCardNoSource, key);
        }
        String companyName = null;
        if (StringUtils.isNotEmpty(companyNameSource)) {
            companyName = EncryptUtil.desEncrypt(companyNameSource, key);
        }
        String bankAccountNo = null;
        if (StringUtils.isNotEmpty(bankAccountNoSource)) {
            bankAccountNo = EncryptUtil.desEncrypt(bankAccountNoSource, key);
        }
        String bankAccountName = null;
        if (StringUtils.isNotEmpty(bankAccountNameSource)) {
            bankAccountName = EncryptUtil.desEncrypt(bankAccountNameSource, key);
        }
        //3、构建签名参数
        TreeMap<String, Object> changgeAccountSignParams = new TreeMap<String, Object>();
        changgeAccountSignParams.put("token", token);
        changgeAccountSignParams.put("clientNo", clientNo);
        changgeAccountSignParams.put("changeType", changeType);
        changgeAccountSignParams.put("reqFlowNo", reqFlowNo);
        changgeAccountSignParams.put("accountUuid", accountUuid);
        changgeAccountSignParams.put("realName", realNameSource);
        changgeAccountSignParams.put("idCardNo", idCardNoSource);
        changgeAccountSignParams.put("companyName", companyNameSource);
        changgeAccountSignParams.put("accountType", accountType);
        changgeAccountSignParams.put("bankAccountNo", bankAccountNoSource);
        changgeAccountSignParams.put("bankAccountName", bankAccountNameSource);
        changgeAccountSignParams.put("bankAccountType", bankAccountType);
        changgeAccountSignParams.put("bankCode", bankCode);
        changgeAccountSignParams.put("bankChannelNo", bankChannelNo);
        changgeAccountSignParams.put("bankName", bankName);
        changgeAccountSignParams.put("bankSubName", bankSubName);
        changgeAccountSignParams.put("bankProvince", bankProvince);
        changgeAccountSignParams.put("bankCity", bankCity);
        changgeAccountSignParams.put("bankArea", bankArea);

        JSONObject jsonChanggeAccount = new JSONObject();
        jsonChanggeAccount.put("sign", SignUtil.signByMap(KeyConstants.getKey(), changgeAccountSignParams));
        jsonChanggeAccount.put("token", token);
        jsonChanggeAccount.put("clientNo", clientNo);
        jsonChanggeAccount.put("changeType", changeType);
        jsonChanggeAccount.put("reqFlowNo", reqFlowNo);
        jsonChanggeAccount.put("accountUuid", accountUuid);
        jsonChanggeAccount.put("realName", realName);
        jsonChanggeAccount.put("idCardNo", idCardNo);
        jsonChanggeAccount.put("companyName", companyName);
        jsonChanggeAccount.put("accountType", accountType);//账户类型1、个人 2、个体商户 3、企业
        jsonChanggeAccount.put("bankAccountNo", bankAccountNo);
        jsonChanggeAccount.put("bankAccountName", bankAccountName);
        jsonChanggeAccount.put("bankAccountType", bankAccountType);
        jsonChanggeAccount.put("bankCode", bankCode);
        jsonChanggeAccount.put("bankChannelNo", bankChannelNo);
        jsonChanggeAccount.put("bankName", bankName);
        jsonChanggeAccount.put("bankSubName", bankSubName);
        jsonChanggeAccount.put("bankProvince", bankProvince);
        jsonChanggeAccount.put("bankCity", bankCity);
        jsonChanggeAccount.put("bankArea", bankArea);
        String changgeAccountJson = jsonChanggeAccount.toJSONString();
        return post(url, changgeAccountJson);
    }

    /**
     * @param walletReq
     * @return BaseResMessage<Object>
     * @function 冻结钱包
     */
    public static BaseResMessage<Object> frozenWallet(WalletReq walletReq) throws Exception {
        String tokenType = WebParamUtils.getStringValue("12");    //状态变更冻结 11 令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码

        /* 获取令牌 */
        /* 获取令牌 */
        String token = getToken(clientNo, tokenType);
        logger.info("冻结钱包获取令牌成功:" + token);

        Integer walletType = WebParamUtils.getIntegerValue(walletReq.getWalletType());//钱包类型 100 POS D0钱包状态 102 POS T1状态 200 二维码D0钱包状态 202 二维码T1钱包状态
        Integer frozenType = WebParamUtils.getIntegerValue(walletReq.getFrozenType());    //出账状态 1 正常 2禁止出账（只进不出） 3 冻结（无法交易）
        String accountUuid = WebParamUtils.getStringValue(walletReq.getAccountUuid());//账户UUID

        String url = ServerConstants.getServerUrl() + "/v1/wallet/frozenWallet";
        //2、构建签名参数
        TreeMap<String, Object> frozenWalletSignParams = new TreeMap<String, Object>();
        frozenWalletSignParams.put("token", token);
        frozenWalletSignParams.put("clientNo", clientNo);
        frozenWalletSignParams.put("walletType", walletType);
        frozenWalletSignParams.put("frozenType", frozenType);
        frozenWalletSignParams.put("accountUuid", accountUuid);

        JSONObject jsonFrozenWallet = new JSONObject();
        jsonFrozenWallet.put("sign", SignUtil.signByMap(KeyConstants.getKey(), frozenWalletSignParams));
        jsonFrozenWallet.put("token", token);
        jsonFrozenWallet.put("clientNo", clientNo);
        jsonFrozenWallet.put("walletType", walletType);
        jsonFrozenWallet.put("frozenType", frozenType);
        jsonFrozenWallet.put("accountUuid", accountUuid);
        String arozenWalletJson = jsonFrozenWallet.toJSONString();
        return post(url, arozenWalletJson);
    }

    /**
     * @param walletReq
     * @return BaseResMessage<Object>
     * @function 解冻钱包
     */
    public static BaseResMessage<Object> activeWallet(WalletReq walletReq) throws Exception {

        //1、获取参数
        String tokenType = WebParamUtils.getStringValue("14");    //状态变更 解冻钱包 14 令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码

        Integer walletType = WebParamUtils.getIntegerValue(walletReq.getWalletType());//钱包类型 100 POS D0钱包状态 102 POS T1状态 200 二维码D0钱包状态 202 二维码T1钱包状态
        String accountUuid = WebParamUtils.getStringValue(walletReq.getAccountUuid());//账户UUID

                /* 获取令牌 */
        /* 获取令牌 */
        String token = getToken(clientNo, tokenType);
        logger.info("冻结钱包获取令牌成功:" + token);

        String url = ServerConstants.getServerUrl() + "/v1/wallet/activeWallet";
        //2、构建签名参数
        TreeMap<String, Object> activeWalletSignParams = new TreeMap<String, Object>();
        activeWalletSignParams.put("token", token);
        activeWalletSignParams.put("clientNo", clientNo);
        activeWalletSignParams.put("walletType", walletType);
        activeWalletSignParams.put("accountUuid", accountUuid);
        JSONObject jsonActiveWallet = new JSONObject();
        jsonActiveWallet.put("sign", SignUtil.signByMap(KeyConstants.getKey(), activeWalletSignParams));
        jsonActiveWallet.put("token", token);
        jsonActiveWallet.put("clientNo", clientNo);
        jsonActiveWallet.put("walletType", walletType);
        jsonActiveWallet.put("accountUuid", accountUuid);
        String activeWalletJson = jsonActiveWallet.toJSONString();
        return post(url, activeWalletJson);
    }

    /**
     * @param walletReq
     * @return BaseResMessage<Object>
     * @function 资金冻结
     */
    public static BaseResMessage<Object> frozenWalletBalance(WalletReq walletReq) throws Exception {
        //1、获取参数
        String tokenType = WebParamUtils.getStringValue("18");        //18 冻结资金    令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码

                /* 获取令牌 */
        /* 获取令牌 */
        String token = getToken(clientNo, tokenType);
        logger.info("资金冻结获取令牌成功:" + token);

        Integer frozenType = WebParamUtils.getIntegerValue(walletReq.getFrozenType());        //冻结类型  10、全部资金冻结 20、POS资金冻结 21、POS D0资金冻结 22、POS T1资金冻结 30、二维码资金冻结 31、二维码 D0资金冻结 32、二维码 T1资金冻结
        String reqFlowNo = WebParamUtils.getStringValue(walletReq.getRrn());//请求流水号
        Double amount = WebParamUtils.getDoubleValue(walletReq.getAmount());            //金额
        String accountUuid = WebParamUtils.getStringValue(walletReq.getAccountUuid());//账户UUID

        String url = ServerConstants.getServerUrl() + "/v1/wallet/frozenWalletBalance";
        //2、构建签名参数
        TreeMap<String, Object> frozenWalletBalanceSignParams = new TreeMap<String, Object>();

        frozenWalletBalanceSignParams.put("token", token);
        frozenWalletBalanceSignParams.put("clientNo", clientNo);
        frozenWalletBalanceSignParams.put("frozenType", frozenType);
        frozenWalletBalanceSignParams.put("reqFlowNo", reqFlowNo);
        frozenWalletBalanceSignParams.put("amount", amount);
        frozenWalletBalanceSignParams.put("accountUuid", accountUuid);

        JSONObject jsonFrozenWalletBalance = new JSONObject();
        jsonFrozenWalletBalance.put("sign", SignUtil.signByMap(KeyConstants.getKey(), frozenWalletBalanceSignParams));
        jsonFrozenWalletBalance.put("token", token);
        jsonFrozenWalletBalance.put("clientNo", clientNo);
        jsonFrozenWalletBalance.put("frozenType", frozenType);
        jsonFrozenWalletBalance.put("reqFlowNo", reqFlowNo);
        jsonFrozenWalletBalance.put("amount", amount);
        jsonFrozenWalletBalance.put("accountUuid", accountUuid);
        String frozenWalletBalanceJson = jsonFrozenWalletBalance.toJSONString();
        return post(url, frozenWalletBalanceJson);
    }

    /**
     * @param walletReq
     * @return BaseResMessage<Object>
     * @function 资金解冻
     */
    public static BaseResMessage<Object> activeWalletBalance(WalletReq walletReq) throws Exception {

        //1、获取参数
        String tokenType = WebParamUtils.getStringValue("19");            //解冻资金 19  令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码

        /* 获取令牌 */
        String token = getToken(clientNo, tokenType);
        logger.info("解冻资金获取令牌成功:" + token);

        Integer activeType = WebParamUtils.getIntegerValue(10);            //冻结类型  10、全部资金冻结 20、POS资金冻结 21、POS D0资金冻结 22、POS T1资金冻结 30、二维码资金冻结 31、二维码 D0资金冻结 32、二维码 T1资金冻结
        String reqFlowNo = WebParamUtils.getStringValue(walletReq.getRrn());    //请求流水号
        Double amount = WebParamUtils.getDoubleValue(walletReq.getAmount());
        String accountUuid = WebParamUtils.getStringValue(walletReq.getAccountUuid());//账户UUID
        //令牌值
        String url = ServerConstants.getServerUrl() + "/v1/wallet/activeWalletBalance";
        //2、构建签名参数
        TreeMap<String, Object> activeWalletBalanceSignParams = new TreeMap<String, Object>();
        activeWalletBalanceSignParams.put("token", token);
        activeWalletBalanceSignParams.put("clientNo", clientNo);
        activeWalletBalanceSignParams.put("activeType", activeType);
        activeWalletBalanceSignParams.put("reqFlowNo", reqFlowNo);
        activeWalletBalanceSignParams.put("amount", amount);
        activeWalletBalanceSignParams.put("accountUuid", accountUuid);
        JSONObject jsonActiveWalletBalance = new JSONObject();
        jsonActiveWalletBalance.put("sign", SignUtil.signByMap(KeyConstants.getKey(), activeWalletBalanceSignParams));
        jsonActiveWalletBalance.put("token", token);
        jsonActiveWalletBalance.put("clientNo", clientNo);
        jsonActiveWalletBalance.put("activeType", activeType);
        jsonActiveWalletBalance.put("reqFlowNo", reqFlowNo);
        jsonActiveWalletBalance.put("amount", amount);
        jsonActiveWalletBalance.put("accountUuid", accountUuid);
        String activeWalletBalanceJson = jsonActiveWalletBalance.toJSONString();
        return post(url, activeWalletBalanceJson);
    }

    /**
     * @param walletReq
     * @return BaseResMessage<Object>
     * @function *加款/减款
     */
    public static BaseResMessage<Object> addWallet(WalletReq walletReq) throws Exception {
        //1、获取参数
        String tokenType = WebParamUtils.getStringValue("16");            //加款/减款 16 令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码
        String url = ServerConstants.getServerUrl2() + "/v1/wallet/addWalletFlow";
        /* 获取令牌 */
        String token = getToken2(clientNo, tokenType);
        logger.info("充值获取令牌成功:" + url + "," + token);

        String reqFlowNo = WebParamUtils.getStringValue(walletReq.getRrn());            //请求流水号
        Double settleAmount = WebParamUtils.getDoubleValue(walletReq.getSettleAmount());        //结算金额，精度2位
        Double transAmount = WebParamUtils.getDoubleValue(walletReq.getTransAmount());        //交易金额，精度2位

        String accountUuid = WebParamUtils.getStringValue(walletReq.getAccountUuid());//账户UUID
        Integer postType = WebParamUtils.getIntegerValue(walletReq.getPostType());
        //入账方式：  1000、POS消费入账 1010、POS预授权完成入账 1020、POS差错款加款  1100、清算失败退回 、
        // 2000、POS撤销入账 2001、POS冲正入账 2002、POS退货入账 2021、POS差错款减款  2010、POS预授权完成撤销入账
        // 2011、POS预授权完成冲正入账  2100、清算扣减 2101、清算差错扣减
        // 1200二维码消费入账 1101、清算差错退回   2200、二维码退货入账
        Integer walletType = WebParamUtils.getIntegerValue(walletReq.getWalletType());
        //账户钱包类型 100 D0钱包 101 D0待审钱包 200 D0二维码钱包

        String posRefNo = null;
        String posTransFlowNo = null;
        String posMerchantNoSource = null;
        String posChnMerchantNoSource = null;
        String posTransTime = null;
        String posSettleDate = null;
        String qrCodeRefNo = null;
        String qrCodeTransFlowNo = null;
        String qrCodeMerchantNoSource = null;
        String qrCodeChnMerchantNoSource = null;
        String qrCodeTransTime = null;
        String qrCodeSettleDate = null;
        /* 交易类型 */
        if (walletReq.getTrans_type().equals("POS")) {
            posRefNo = WebParamUtils.getStringValue(walletReq.getRrn());            //POS交易系统参考号
            posTransFlowNo = WebParamUtils.getStringValue(walletReq.getSys_trace());    //POS交易流水号
            posMerchantNoSource = WebParamUtils.getStringValue(walletReq.getMerchantNo());//POS交易商户号
            posChnMerchantNoSource = WebParamUtils.getStringValue(walletReq.getMerchantNo());    //POS交易渠道商户号
            posTransTime = WebParamUtils.getStringValue(walletReq.getTrans_date() + walletReq.getTrans_time());        //POS交易交易时间
            posSettleDate = WebParamUtils.getStringValue(walletReq.getSettle_date());        //POS交易结算时间
        } else {
            qrCodeRefNo = WebParamUtils.getStringValue(walletReq.getRrn());        //二维码交易系统参考号
            qrCodeTransFlowNo = WebParamUtils.getStringValue(walletReq.getSys_trace());    //二维码交易流水号
            qrCodeMerchantNoSource = WebParamUtils.getStringValue(walletReq.getMerchantNo());    //二维码交易商户号，3DES加密
            qrCodeChnMerchantNoSource = WebParamUtils.getStringValue(walletReq.getMerchantNo());//二维码交易渠道商户号
            qrCodeTransTime = WebParamUtils.getStringValue(walletReq.getTrans_date() + walletReq.getTrans_time());//二维码交易渠道商户号
            qrCodeSettleDate = WebParamUtils.getStringValue(walletReq.getSettle_date());    //二维码交易结算时间
        }
        //2、DES加密参数
        String key = KeyConstants.getKey();
        String posMerchantNo = null;
        if (StringUtils.isNotEmpty(posMerchantNoSource)) {
            posMerchantNo = EncryptUtil.desEncrypt(posMerchantNoSource, key);
        }
        String posChnMerchantNo = null;
        if (StringUtils.isNotEmpty(posChnMerchantNoSource)) {
            posChnMerchantNo = EncryptUtil.desEncrypt(posChnMerchantNoSource, key);
        }
        String qrCodeMerchantNo = null;
        if (StringUtils.isNotEmpty(qrCodeMerchantNoSource)) {
            qrCodeMerchantNo = EncryptUtil.desEncrypt(qrCodeMerchantNoSource, key);
        }
        String qrCodeChnMerchantNo = null;
        if (StringUtils.isNotEmpty(qrCodeChnMerchantNoSource)) {
            qrCodeChnMerchantNo = EncryptUtil.desEncrypt(qrCodeChnMerchantNoSource, key);
        }
        //2、构建签名参数
        TreeMap<String, Object> addWalletFlowSignParams = new TreeMap<String, Object>();
        addWalletFlowSignParams.put("token", token);
        addWalletFlowSignParams.put("clientNo", clientNo);
        addWalletFlowSignParams.put("reqFlowNo", reqFlowNo);
        addWalletFlowSignParams.put("settleAmount", settleAmount);
        addWalletFlowSignParams.put("transAmount", transAmount);
        addWalletFlowSignParams.put("accountUuid", accountUuid);
        addWalletFlowSignParams.put("postType", postType);
        addWalletFlowSignParams.put("walletType", walletType);

        addWalletFlowSignParams.put("posRefNo", posRefNo);
        addWalletFlowSignParams.put("posTransFlowNo", posTransFlowNo);
        addWalletFlowSignParams.put("posMerchantNo", posMerchantNoSource);
        addWalletFlowSignParams.put("posChnMerchantNo", posChnMerchantNoSource);
        addWalletFlowSignParams.put("posTransTime", posTransTime);
        addWalletFlowSignParams.put("posSettleDate", posSettleDate);

        addWalletFlowSignParams.put("qrCodeRefNo", qrCodeRefNo);
        addWalletFlowSignParams.put("qrCodeTransFlowNo", qrCodeTransFlowNo);
        addWalletFlowSignParams.put("qrCodeMerchantNo", qrCodeMerchantNoSource);
        addWalletFlowSignParams.put("qrCodeChnMerchantNo", qrCodeChnMerchantNoSource);
        addWalletFlowSignParams.put("qrCodeTransTime", qrCodeTransTime);
        addWalletFlowSignParams.put("qrCodeSettleDate", qrCodeSettleDate);

        JSONObject jsonAddWalletFlow = new JSONObject();
        logger.debug("key:" + KeyConstants.getKey());
        jsonAddWalletFlow.put("sign", SignUtil.signByMap(KeyConstants.getKey(), addWalletFlowSignParams));

        jsonAddWalletFlow.put("token", token);
        jsonAddWalletFlow.put("clientNo", clientNo);
        jsonAddWalletFlow.put("reqFlowNo", reqFlowNo);
        jsonAddWalletFlow.put("settleAmount", settleAmount);
        jsonAddWalletFlow.put("transAmount", transAmount);
        jsonAddWalletFlow.put("accountUuid", accountUuid);
        jsonAddWalletFlow.put("postType", postType);
        jsonAddWalletFlow.put("walletType", walletType);

        jsonAddWalletFlow.put("posRefNo", posRefNo);
        jsonAddWalletFlow.put("posTransFlowNo", posTransFlowNo);
        jsonAddWalletFlow.put("posMerchantNo", posMerchantNo);
        jsonAddWalletFlow.put("posChnMerchantNo", posChnMerchantNo);
        jsonAddWalletFlow.put("posTransTime", posTransTime);
        jsonAddWalletFlow.put("posSettleDate", posSettleDate);

        jsonAddWalletFlow.put("qrCodeRefNo", qrCodeRefNo);
        jsonAddWalletFlow.put("qrCodeTransFlowNo", qrCodeTransFlowNo);
        jsonAddWalletFlow.put("qrCodeMerchantNo", qrCodeMerchantNo);
        jsonAddWalletFlow.put("qrCodeChnMerchantNo", qrCodeChnMerchantNo);
        jsonAddWalletFlow.put("qrCodeTransTime", qrCodeTransTime);
        jsonAddWalletFlow.put("qrCodeSettleDate", qrCodeSettleDate);

        String addWalletFlowJson = jsonAddWalletFlow.toJSONString();
        return post(url, addWalletFlowJson);
    }

    /**
     * @param walletReq
     * @return BaseResMessage<Object>
     * @function 提现
     */
    public static BaseResMessage<Object> withdrawDeposit(WalletReq walletReq) throws Exception {
        //1、获取参数
        String tokenType = WebParamUtils.getStringValue("20");        //提现 20 令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码

        /* 获取令牌 */
        String token = getToken(clientNo, tokenType);
        logger.info("提现获取令牌成功:" + token);

        Integer walletType = WebParamUtils.getIntegerValue(walletReq.getWalletType());    //账户类型 100、POS D0可提现钱包 200、二维码 D0可提现钱包
        String reqFlowNo = WebParamUtils.getStringValue(walletReq.getRrn());//请求流水号
        Double amount = WebParamUtils.getDoubleValue(walletReq.getSettleAmount());            //金额
        String accountUuid = WebParamUtils.getStringValue(walletReq.getAccountUuid());//账户UUID

        String url = ServerConstants.getServerUrl() + "/v1/wallet/withdrawDeposit";
        //2、构建签名参数
        TreeMap<String, Object> withdrawDepositSignParams = new TreeMap<String, Object>();
        withdrawDepositSignParams.put("token", token);
        withdrawDepositSignParams.put("clientNo", clientNo);
        withdrawDepositSignParams.put("walletType", walletType);
        withdrawDepositSignParams.put("reqFlowNo", reqFlowNo);
        withdrawDepositSignParams.put("amount", amount);
        withdrawDepositSignParams.put("accountUuid", accountUuid);
        JSONObject jsonWithdrawDeposit = new JSONObject();
        jsonWithdrawDeposit.put("sign", SignUtil.signByMap(KeyConstants.getKey(), withdrawDepositSignParams));

        jsonWithdrawDeposit.put("token", token);
        jsonWithdrawDeposit.put("clientNo", clientNo);
        jsonWithdrawDeposit.put("walletType", walletType);
        jsonWithdrawDeposit.put("reqFlowNo", reqFlowNo);
        jsonWithdrawDeposit.put("amount", amount);
        jsonWithdrawDeposit.put("accountUuid", accountUuid);
        String withdrawDepositJson = jsonWithdrawDeposit.toJSONString();
        return post(url, withdrawDepositJson);
    }

    /**
     * @param walletReq
     * @return BaseResMessage<Object>
     * @function 日结消费
     */
    public static BaseResMessage<Object> d0WithdrawDeposit(WalletReq walletReq) throws Exception {
        //1、获取参数
        String tokenType = WebParamUtils.getStringValue("21");        //提现 20 令牌类型
        String clientNo = ServerConstants.getClientNo();    //客户端编码

        /* 获取令牌 */
        String token = getToken2(clientNo, tokenType);
        logger.info("日结消费获取令牌成功:" + token);

        String reqFlowNo = WebParamUtils.getStringValue(walletReq.getRrn());//请求流水号
        Double amount = WebParamUtils.getDoubleValue(walletReq.getSettleAmount());            //金额
        String accountUuid = WebParamUtils.getStringValue(walletReq.getAccountUuid());//账户UUID
        Double settleAmount = WebParamUtils.getDoubleValue(walletReq.getSettleAmount());        //结算金额，精度2位
        Double transAmount = WebParamUtils.getDoubleValue(walletReq.getTransAmount());        //交易金额，精度2位
        String posRefNo = WebParamUtils.getStringValue(walletReq.getRrn());            //POS交易系统参考号
        String posTransFlowNo = WebParamUtils.getStringValue(walletReq.getSys_trace());    //POS交易流水号
        String posMerchantNoSource = WebParamUtils.getStringValue(walletReq.getMerchantNo());//POS交易商户号
        String posChnMerchantNoSource = WebParamUtils.getStringValue(walletReq.getMerchantNo());    //POS交易渠道商户号
        String posTransTime = WebParamUtils.getStringValue(walletReq.getTrans_date() + walletReq.getTrans_time());        //POS交易交易时间
        String posSettleDate = WebParamUtils.getStringValue(walletReq.getSettle_date());        //POS交易结算时间

        //2、DES加密参数
        String key = KeyConstants.getKey();
        String posMerchantNo = null;
        if (StringUtils.isNotEmpty(posMerchantNoSource)) {
            posMerchantNo = EncryptUtil.desEncrypt(posMerchantNoSource, key);
        }
        String posChnMerchantNo = null;
        if (StringUtils.isNotEmpty(posChnMerchantNoSource)) {
            posChnMerchantNo = EncryptUtil.desEncrypt(posChnMerchantNoSource, key);
        }

        String url = ServerConstants.getServerUrl2() + "/v1/wallet/addFlowAndWithdrawDeposit";
    //2、构建签名参数
        TreeMap<String, Object> addWalletFlowSignParams = new TreeMap<String, Object>();
        addWalletFlowSignParams.put("token", token);
        addWalletFlowSignParams.put("clientNo", clientNo);
        addWalletFlowSignParams.put("reqFlowNo", reqFlowNo);
        addWalletFlowSignParams.put("settleAmount", settleAmount);
        addWalletFlowSignParams.put("transAmount", transAmount);
        addWalletFlowSignParams.put("accountUuid", accountUuid);

        addWalletFlowSignParams.put("posRefNo", posRefNo);
        addWalletFlowSignParams.put("posTransFlowNo", posTransFlowNo);
        addWalletFlowSignParams.put("posMerchantNo", posMerchantNoSource);
        addWalletFlowSignParams.put("posChnMerchantNo", posChnMerchantNoSource);
        addWalletFlowSignParams.put("posTransTime", posTransTime);
        addWalletFlowSignParams.put("posSettleDate", posSettleDate);

        JSONObject jsonAddWalletFlow = new JSONObject();
        logger.debug("key:" + KeyConstants.getKey());
        jsonAddWalletFlow.put("sign", SignUtil.signByMap(KeyConstants.getKey(), addWalletFlowSignParams));

        jsonAddWalletFlow.put("token", token);
        jsonAddWalletFlow.put("clientNo", clientNo);
        jsonAddWalletFlow.put("reqFlowNo", reqFlowNo);
        jsonAddWalletFlow.put("settleAmount", settleAmount);
        jsonAddWalletFlow.put("transAmount", transAmount);
        jsonAddWalletFlow.put("accountUuid", accountUuid);

        jsonAddWalletFlow.put("posRefNo", posRefNo);
        jsonAddWalletFlow.put("posTransFlowNo", posTransFlowNo);
        jsonAddWalletFlow.put("posMerchantNo", posMerchantNo);
        jsonAddWalletFlow.put("posChnMerchantNo", posChnMerchantNo);
        jsonAddWalletFlow.put("posTransTime", posTransTime);
        jsonAddWalletFlow.put("posSettleDate", posSettleDate);
        String withdrawDepositJson = jsonAddWalletFlow.toJSONString();
        return post(url, withdrawDepositJson);
    }
}
