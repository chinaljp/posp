package com.chanjet.transnotice.entity;

/**
 * Created by Gjq on 2018/8/8.
 */
public class SendMsg {
    private String amount;
    private String authCode;
    private String bankName;
    private String batchNo;
    private String cardNo;
    private String cardType;
    private String extData;
    private String feeType;
    private String inputMode;
    private String merchantId;
    private String merchantName;
    private String mobileNo;
    private String orderId;
    private String originalAuthCode;
    private String originalbatchNo;
    private String originalRrn;
    private String originaltraceNo;
    private String originalTranDate;
    private String platCode;
    private String rrn;
    private String settleAmount;
    private String settleDate;
    private String sysRespCode;
    private String sysRespDesc;
    private String sysTraceNo;
    private String termId;
    private String termModel;
    private String termSn;
    private String traceNo;
    private String tranCode;
    private String tranTime;
    private String sign;

    public String getAmount() {
        return amount;
    }

    public void setAmount(String amount) {
        this.amount = amount;
    }

    public String getAuthCode() {
        return authCode;
    }

    public void setAuthCode(String authCode) {
        this.authCode = authCode;
    }

    public String getBankName() {
        return bankName;
    }

    public void setBankName(String bankName) {
        this.bankName = bankName;
    }

    public String getBatchNo() {
        return batchNo;
    }

    public void setBatchNo(String batchNo) {
        this.batchNo = batchNo;
    }

    public String getCardNo() {
        return cardNo;
    }

    public void setCardNo(String cardNo) {
        this.cardNo = cardNo;
    }

    public String getCardType() {
        return cardType;
    }

    public void setCardType(String cardType) {
        this.cardType = cardType;
    }

    public String getExtData() {
        return extData;
    }

    public void setExtData(String extData) {
        this.extData = extData;
    }

    public String getFeeType() {
        return feeType;
    }

    public void setFeeType(String feeType) {
        this.feeType = feeType;
    }

    public String getInputMode() {
        return inputMode;
    }

    public void setInputMode(String inputMode) {
        this.inputMode = inputMode;
    }

    public String getMerchantId() {
        return merchantId;
    }

    public void setMerchantId(String merchantId) {
        this.merchantId = merchantId;
    }

    public String getMerchantName() {
        return merchantName;
    }

    public void setMerchantName(String merchantName) {
        this.merchantName = merchantName;
    }

    public String getMobileNo() { return mobileNo; }

    public void setMobileNo(String mobileNo) { this.mobileNo = mobileNo; }

    public String getOrderId() {
        return orderId;
    }

    public void setOrderId(String orderId) {
        this.orderId = orderId;
    }

    public String getOriginalAuthCode() {
        return originalAuthCode;
    }

    public void setOriginalAuthCode(String originalAuthCode) {this.originalAuthCode = originalAuthCode; }

    public String getOriginalbatchNo() {
        return originalbatchNo;
    }

    public void setOriginalbatchNo(String originalbatchNo) {
        this.originalbatchNo = originalbatchNo;
    }

    public String getOriginalRrn() {
        return originalRrn;
    }

    public void setOriginalRrn(String originalRrn) {
        this.originalRrn = originalRrn;
    }

    public String getOriginaltraceNo() {
        return originaltraceNo;
    }

    public void setOriginaltraceNo(String originaltraceNo) {
        this.originaltraceNo = originaltraceNo;
    }

    public String getOriginalTranDate() {
        return originalTranDate;
    }

    public void setOriginalTranDate(String originalTranDate) {
        this.originalTranDate = originalTranDate;
    }

    public String getPlatCode() {
        return platCode;
    }

    public void setPlatCode(String platCode) {
        this.platCode = platCode;
    }

    public String getRrn() {
        return rrn;
    }

    public void setRrn(String rrn) {
        this.rrn = rrn;
    }

    public String getSettleAmount() {
        return settleAmount;
    }

    public void setSettleAmount(String settleAmount) {
        this.settleAmount = settleAmount;
    }

    public String getSettleDate() {
        return settleDate;
    }

    public void setSettleDate(String settleDate) {
        this.settleDate = settleDate;
    }

    public String getSysRespCode() {
        return sysRespCode;
    }

    public void setSysRespCode(String sysRespCode) {
        this.sysRespCode = sysRespCode;
    }

    public String getSysRespDesc() {
        return sysRespDesc;
    }

    public void setSysRespDesc(String sysRespDesc) {
        this.sysRespDesc = sysRespDesc;
    }

    public String getSysTraceNo() {
        return sysTraceNo;
    }

    public void setSysTraceNo(String sysTraceNo) {
        this.sysTraceNo = sysTraceNo;
    }

    public String getTermId() {
        return termId;
    }

    public void setTermId(String termId) {
        this.termId = termId;
    }

    public String getTermModel() { return termModel; }

    public void setTermModel(String termModel) { this.termModel = termModel; }

    public String getTermSn() { return termSn; }

    public void setTermSn(String termSn) { this.termSn = termSn; }

    public String getTraceNo() {
        return traceNo;
    }

    public void setTraceNo(String traceNo) {
        this.traceNo = traceNo;
    }

    public String getTranCode() {
        return tranCode;
    }

    public void setTranCode(String tranCode) {
        this.tranCode = tranCode;
    }

    public String getTranTime() {
        return tranTime;
    }

    public void setTranTime(String tranTime) {
        this.tranTime = tranTime;
    }

    public String getSign() {
        return sign;
    }

    public void setSign(String sign) {
        this.sign = sign;
    }

    @Override
    public String toString() {
        return amount + authCode + bankName + batchNo + cardNo + cardType + extData +
                feeType + inputMode + merchantId + merchantName + mobileNo +
                orderId + originalAuthCode + originalbatchNo + originalRrn +
                originaltraceNo + originalTranDate + platCode + rrn + settleAmount +
                settleDate + sysRespCode + sysRespDesc + sysTraceNo + termId + termModel + termSn + traceNo + tranCode + tranTime;
    }
}
