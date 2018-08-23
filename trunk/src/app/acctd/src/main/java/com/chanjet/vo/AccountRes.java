package com.chanjet.vo;

import java.io.Serializable;

/**
 * Created by feng.gaoo on 2017/3/23.
 */
public class AccountRes implements Serializable {
    private static final long serialVersionUID = -8094224829177731699L;
    private String accountUuid;//账户UUID

    /**
     * //账户UUID
     *
     * @return
     */
    public String getAccountUuid() {
        return accountUuid;
    }

    /**
     * //账户UUID
     *
     * @param accountUuid
     */
    public void setAccountUuid(String accountUuid) {
        this.accountUuid = accountUuid;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("AccountRes [accountUuid=");
        builder.append(accountUuid);
        builder.append("]");
        return builder.toString();
    }
}
