package com.chanjet.action;

import com.alibaba.fastjson.JSONObject;
import com.chanjet.service.WalletService;
import com.chanjet.util.StringUtil;
import com.chanjet.vo.BaseResMessage;
import com.chanjet.vo.WalletReq;
import org.apache.log4j.Logger;
import redis.clients.jedis.Jedis;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.List;


/**
 * Created by feng.gaoo on 2017/3/24.
 */
public class mainAction {
    private static Logger logger = Logger.getLogger(mainAction.class);

    public static void main(String[] args) {
        Jedis jedis = new Jedis("127.0.0.1", 6379);
        jedis.del("test");
        logger.debug("主进程运行:" + "队列[" + args[0] + "]");
        while (jedis.isConnected()) {
            logger.debug("等待消息中......");
            List<String> list = jedis.brpop(0, args[0]);
            for (String s :
                    list) {
                if (!s.equals(args[0])) {
                    //logger.debug("msg:" + s);
                     /* 接收消息 */
                    byte[] bytes = new byte[0];
                    try {
                        bytes = StringUtil.asc2Bcd(s.getBytes("gbk"), s.length());
                    } catch (UnsupportedEncodingException e) {
                        e.printStackTrace();
                        break;
                    }
                    String ss = null;
                    try {
                        ss = new String(bytes, "gbk");
                    } catch (UnsupportedEncodingException e) {
                        e.printStackTrace();
                        break;
                    }

                    JSONObject jsonMsg = jsonMsg = JSONObject.parseObject(ss);
                    //logger.error("json:" + jsonMsg.toString());
                    JSONObject jsonObject = jsonMsg.getJSONObject("data");
                    logger.debug("json:" + jsonObject.toString());

                    WalletReq walletReq = jsonObject.toJavaObject(WalletReq.class);

                    logger.debug("json:" + walletReq.toString());
                    String transCode = walletReq.getTrans_code();
                    logger.debug("trans_code:" + transCode);
                    if (transCode == null) {
                        break;
                    }
                    BaseResMessage<Object> resMessage = null;
                    /* 判断是什么交易
                    00T000:账户开户
                    00T100:账户变更
                    00T200:账户冻结
                    00T300:账户解冻
                    00T400:资金冻结
                    00T500:资金解冻
                    00T600:账户充值
                    */
                    try {
                        logger.debug("开始处理交易:" + transCode);
                        /* 账户开户 */
                        if (transCode.equals("00T000")) {
                            resMessage = WalletService.openAccount(walletReq);
                        }
                        /* 账户变更 */
                        else if (transCode.equals("00T100")) {
                            resMessage = WalletService.changeAccount(walletReq);
                        }
                          /* 账户冻结 */
                        else if (transCode.equals("00T200")) {
                            resMessage = WalletService.frozenWallet(walletReq);
                        }
                    /* 账户解冻 */
                        else if (transCode.equals("00T300")) {
                            resMessage = WalletService.activeWallet(walletReq);
                        }
                     /* 资金冻结 */
                        else if (transCode.equals("00T400")) {
                            resMessage = WalletService.frozenWalletBalance(walletReq);
                        }
                    /* 资金解冻 */
                        else if (transCode.equals("00T500")) {
                            resMessage = WalletService.activeWalletBalance(walletReq);
                        }
                        /* +／-资金 */
                        else if (transCode.equals("00T600")) {
                            resMessage = WalletService.addWallet(walletReq);
                        } else if (transCode.equals("00T700")) {
                            resMessage = WalletService.withdrawDeposit(walletReq);
                        } else if (transCode.equals("00T800")) {
                            resMessage = WalletService.d0WithdrawDeposit(walletReq);
                        } else {
                            logger.error("不支持的交易:" + transCode);
                            break;
                        }
                        if (resMessage == null) {
                            break;
                        }
                        logger.debug("返回：" + resMessage.getCode() + "," + resMessage.getMessage());

                        String svrId = jsonMsg.getString("svrid");
                        String traceNo = jsonObject.getString("sys_trace");
                        jsonMsg.remove("data");

                        JSONObject resJson = new JSONObject();
                        resJson.put("resp_code", resMessage.getCode());
                        resJson.put("resp_desc", resMessage.getMessage());
                        if (transCode.equals("00T000")
                                && resMessage.getCode().equals("00")) {
                            resJson.put("data", resMessage.getData().toString());
                        }

                        resJson.put("sys_trace", traceNo);
                        jsonMsg.put("data", resJson);
                        logger.debug(jsonMsg.toJSONString());

                        String resMsg = StringUtil.bcd2Str(jsonMsg.toJSONString().getBytes());
                        jedis.lpush(svrId, resMsg);
                        logger.debug("res:" + resMsg);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        logger.debug("主进程退出");
    }
}
