package com.chanjet.transaction;

import java.io.Serializable;

/**
 * 无卡支付渠道API响应结构
 * 
 * @author tinn
 *
 */
public class ChannelTransApiVo implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	private String rpCd; // 交易应答码
	private String rpDesc; // 交易应答描述
	private String oriRpCd; // 原交易应答码
	private String oriRpDesc; // 原交易应答描述
	private Integer oriTrxStatus; // 原交易状态

	public String getRpCd() {
		return rpCd;
	}

	public void setRpCd(String rpCd) {
		this.rpCd = rpCd;
	}

	public String getRpDesc() {
		return rpDesc;
	}

	public void setRpDesc(String rpDesc) {
		this.rpDesc = rpDesc;
	}

	public String getOriRpCd() {
		return oriRpCd;
	}

	public void setOriRpCd(String oriRpCd) {
		this.oriRpCd = oriRpCd;
	}

	public String getOriRpDesc() {
		return oriRpDesc;
	}

	public void setOriRpDesc(String oriRpDesc) {
		this.oriRpDesc = oriRpDesc;
	}

	public Integer getOriTrxStatus() {
		return oriTrxStatus;
	}

	public void setOriTrxStatus(Integer oriTrxStatus) {
		this.oriTrxStatus = oriTrxStatus;
	}

	@Override
	public String toString() {
		return new StringBuilder().append("QuickPayChannelGatewayVo [rpCd=").append(rpCd).append(", rpDesc=").append(rpDesc).append(", oriRpCd=")
				.append(oriRpCd).append(", oriRpDesc=").append(oriRpDesc).append(", oriTrxStatus=").append(oriTrxStatus).append("]").toString();
	}

}
