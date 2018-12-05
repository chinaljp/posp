# sMsgType;       /* 消息类型码       */
# sProcCode;      /* 交易处理码       */
# sProcCodeEx;    /* 扩展的交易处理码 */
# sNmiCode;       /* 网络管理信息码   */
# sTransCode;     /* 交易类型码       */
# sOldTransCode;  /* 原交易类型码     */
# sOOldTransCode; /* 原原交易类型码   */
import datetime

staTranTbl=[
    {'sMsgType': "0200", 'sProcCode': "300000", 'sProcCodeEx': "00", 'sNmiCode': "", 'sTransCode': "021000", 'sOldTransCode': "", 'sOOldTransCode': ""},  # POS查余额
    {'sMsgType': "0200", 'sProcCode': "000000", 'sProcCodeEx': "00", 'sNmiCode': "", 'sTransCode': "020000", 'sOldTransCode': "", 'sOOldTransCode': ""},  # POS消费
    {'sMsgType': "0220", 'sProcCode': "000000", 'sProcCodeEx': "00", 'sNmiCode': "", 'sTransCode': "022000", 'sOldTransCode': "", 'sOOldTransCode': ""},  # POS脱机消费
    {'sMsgType': "0420", 'sProcCode': "000000", 'sProcCodeEx': "00", 'sNmiCode': "", 'sTransCode': "020300", 'sOldTransCode': "020000", 'sOOldTransCode': ""}, #POS消费冲正
    {'sMsgType': "0200", 'sProcCode': "200000", 'sProcCodeEx': "00", 'sNmiCode': "", 'sTransCode': "020200", 'sOldTransCode': "020000", 'sOOldTransCode': ""}, #POS消费撤销
    {'sMsgType': "0420", 'sProcCode': "200000", 'sProcCodeEx': "00", 'sNmiCode': "", 'sTransCode': "020300", 'sOldTransCode': "020200", 'sOOldTransCode': "020000"}, #POS消费撤销冲正
    {'sMsgType': "0220", 'sProcCode': "200000", 'sProcCodeEx': "00", 'sNmiCode': "", 'sTransCode': "020100", 'sOldTransCode': "020000", 'sOOldTransCode': ""}, #POS退货
    {'sMsgType': "0220", 'sProcCode': "200000", 'sProcCodeEx': "69", 'sNmiCode': "", 'sTransCode': "020100", 'sOldTransCode': "022000", 'sOOldTransCode': ""}, #POS脱机消费退货
    {'sMsgType': "0100", 'sProcCode': "030000", 'sProcCodeEx': "06", 'sNmiCode': "", 'sTransCode': "024000", 'sOldTransCode': "", 'sOOldTransCode': ""}, #POS预授权
    {'sMsgType': "0420", 'sProcCode': "030000", 'sProcCodeEx': "06", 'sNmiCode': "", 'sTransCode': "020300", 'sOldTransCode': "024000", 'sOOldTransCode': ""}, #POS预授权冲正
    {'sMsgType': "0100", 'sProcCode': "200000", 'sProcCodeEx': "06", 'sNmiCode': "", 'sTransCode': "020200", 'sOldTransCode': "024000", 'sOOldTransCode': ""}, #POS预授权撤销
    {'sMsgType': "0420", 'sProcCode': "200000", 'sProcCodeEx': "06", 'sNmiCode': "", 'sTransCode': "020300", 'sOldTransCode': "020200", 'sOOldTransCode': "024000"}, #POS预授权撤销冲正
    {'sMsgType': "0200", 'sProcCode': "000000", 'sProcCodeEx': "06", 'sNmiCode': "", 'sTransCode': "024100", 'sOldTransCode': "024000", 'sOOldTransCode': ""}, #POS预授权完成
    {'sMsgType': "0420", 'sProcCode': "000000", 'sProcCodeEx': "06", 'sNmiCode': "", 'sTransCode': "020300", 'sOldTransCode': "024100", 'sOOldTransCode': "024000"}, #POS预授权完成冲正
    {'sMsgType': "0200", 'sProcCode': "200000", 'sProcCodeEx': "06", 'sNmiCode': "", 'sTransCode': "020200", 'sOldTransCode': "024100", 'sOOldTransCode': "024000"}, #POS预授权完成撤销
    {'sMsgType': "0420", 'sProcCode': "200000", 'sProcCodeEx': "06", 'sNmiCode': "", 'sTransCode': "020300", 'sOldTransCode': "020200", 'sOOldTransCode': "024100"}, #POS预授权完成撤销冲正
]

#一般交易流水文件记录格式各域长度
g_aiAcom = [11, 11, 6, 10, 19, 12, 12, 12, 4, 6, 4, 8, 15, 12, 2, 6, 11, 6, 2, 3, 12, 12, 12, 1, 3, 1, 1, 10, 11, 1, 2, 2, 12]
#品牌服务费交易流水文件记录格式各域长度
g_aiAFee = [11, 1, 2, 2, 1, 1, 4, 11, 11, 11, 11, 11, 11, 4, 6, 2, 6, 10, 19, 28, 28, 42, 8, 15, 12, 12, 12, 12, 12, 12, 1]

#待导入库表对账文件表头
resultHeader=['交易码','渠道号','交易清算日期','系统跟踪号','交易日期','交易时间','交易传输时间', '主账号','借贷记卡标志','交易金额','渠道手续费','报文类型_x','交易类型码','商户类型_x',\
'受卡方标识码_x','受卡机终端标识码','检索参考号_x','授权应答码','接收机构标识码','原始交易的系统\
跟踪号','交易返回码','服务点输入方式','发卡方手续费','转接服务费','品牌服务费','原始交易日期时\
间','服务点条件码_x','交易地域标志_x','发卡机构标识码','支付类型','对账标识','调整标识']
#一般交易流水文件表头
acomHeader=['代理机构标识码', '发送机构标识码', '系统跟踪号', '交易传输时间', '主账号','交易金额','部分代收时的承兑金额','持卡人交易手续\
费','报文类型','交易类型码','商户类型','受卡机终端标识码','受卡方标识码','检索参考号','服务点条件码','授权应答码','接收机构标识码','原始交易的系统\
跟踪号','交易返回码','服务点输入方式','受理方应收手续费','受理方应付手续费','转接服务费','单双转换标志','卡片序列号','终端读取能力','IC 卡条件代码','原始交易日期时\
间','发卡机构标识码','交易地域标志','终端类型','ECI 标志','分期付款附加手续费','其他信息']
#品牌服务费交易流水文件表头
alfeeHeader=['受理方一级分行代码', '是否银联标准卡', '借贷记卡标志', '终端类型', '卡介质','交易地域标志','商户类型','受理方二级分行代码', \
             '受理机构代码','转发机构代码','发卡机构代码','接收机构代码','收单机构代码','报文类型','交易处理码','服务点条件码','系统跟踪号','交易传输时间',\
             '主账号','转出卡','转入卡','原始交易信息','受卡方终端标识码','受卡方标识码','检索参考号','交易金额','受理方手续费','品牌服务费','保留使用','净金额','交易发起方式','保留使用']


#将银联对账文件转换成csv文件
def genCsvFile(isoTable, srcFile, desFile):
    with open(desFile, 'w') as f1:
        with open(srcFile, 'r') as f:
            for line in f:
                iOffset = 0
                for i in range(len(isoTable)):
                    iOffset += isoTable[i]
                    line = line[:iOffset] + ',' + line[iOffset + 1:]
                    iOffset += 1
                f1.write(line)

#获取交易码
def getTranType(msgType,procCode,procCodeEx):
    transCode = ""
    for i in range(len(staTranTbl)):
        if staTranTbl[i]['sMsgType'] == msgType and staTranTbl[i]['sProcCode'][0:2] == procCode[0:2] and staTranTbl[i]['sProcCodeEx'] == procCodeEx:
            transCode = staTranTbl[i]['sTransCode']
    return transCode

#获取渠道号
def getChannelId():
    return '49000000'

#获取清算日期
def genSettleDate():
    today=datetime.date.today()
    oneday=datetime.timedelta(days=1)
    yesterday=today-oneday
    # 转换为其他字符串格式:
    yesterday = yesterday.strftime("%Y%m%d")
    return yesterday

#获取交易日期
def genTransDate(settleDate,transmitTime):
    if(settleDate[4:8] == '0101' and transmitTime[0:4] == '1231'):
        this_year_start = datetime.datetime(datetime.datetime.now().year, 1, 1)
        last_year_end = this_year_start - datetime.timedelta(days=1)
        transDate = last_year_end.strftime("%Y")+transmitTime[0:4]
    else:
        transDate = settleDate[0:4] + transmitTime[0:4]
    return transDate

#获取交易时间
def genTransTime(transmitTime):
    return transmitTime[4:10]

#整理借贷记卡标志
def genCardType(oldCardType):
    if(oldCardType == '01'):
        cardType = '0'
    elif(oldCardType == '02' or oldCardType == '03' ):
        cardType = '1'
    else:
        cardType = 'X'
    return cardType


#服务费转换
def genTransferFee(oldTransferFee):
    if(oldTransferFee == '0'):
        transferFee = 0
    elif(oldTransferFee[0:1] == 'C'):
        transferFee = -int(oldTransferFee[1:])
    else:
        transferFee = int(oldTransferFee[1:])
    return   str(transferFee)

#获取发卡方手续费
def genIssFee(issReciveFee,issPayFee):
    issFee = -int(issReciveFee)
    if(int(issFee) == 0):
        issFee = int(issPayFee)
    return issFee

#获取渠道手续费
def genChannelFee(issFee,transferFee,brandFee):
    return int(issFee)+int(transferFee)+int(brandFee)
