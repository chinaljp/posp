package com.chanjet.action;

import com.chanjet.service.WalletService;
import com.chanjet.vo.WalletReq;
import org.junit.Test;

/**
 * @Author Gaofeng
 * @Date 2017/3/25 17:35
 */
public class FrozenWalletActionTest {
    @Test
    public void frozenWallet() throws Exception {
        WalletReq walletReq = new WalletReq();

        walletReq.setAccountUuid("1b73ca6c-638c-49d0-9ce1-6dc3c75e519e");
        walletReq.setFrozenType(100);
        walletReq.setWalletType(2);

        try {
            WalletService.frozenWallet(walletReq);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}