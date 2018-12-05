import os
from lib.proc import *

import pandas as pd

#将银联对账文件转成csv格式
#交易流水文件
settleDate=genSettleDate()
genCsvFile(g_aiAcom,'/duizhang/yinlianHQ/49000000/'+settleDate+'/IND'+settleDate[2:]+'01ACOM',os.getenv('HOME')+'/tmp/acom.csv')
#品牌服务费交易流水文件
genCsvFile(g_aiAFee,'/duizhang/yinlianHQ/49000000/'+settleDate+'/IND'+settleDate[2:]+'99ALFEE',os.getenv('HOME')+'/tmp/alfee.csv')



#合并交易流水和品牌费
df1 = pd.read_csv(os.getenv('HOME')+'/tmp/acom.csv',error_bad_lines=False,dtype=str,names=acomHeader)
df2 = pd.read_csv(os.getenv('HOME')+'/tmp/alfee.csv',dtype=str,names=alfeeHeader)

result = pd.merge(df1,df2,how='left',on=['系统跟踪号','交易传输时间','交易金额','主账号']).fillna('0')

#添加交易码列
result['交易码']=result.apply(lambda x: getTranType(x.报文类型_x, x.交易类型码,x.服务点条件码_x), axis = 1)
#添加渠道号列
result['渠道号']=result.apply(lambda x: getChannelId(), axis = 1)
#添加清算日期列
result['交易清算日期']=result.apply(lambda x: genSettleDate(), axis = 1)
#添加交易日期列
result['交易日期']=result.apply(lambda x: genTransDate(x.交易清算日期,x.交易传输时间), axis = 1)
#添加交易时间列
result['交易时间']=result.apply(lambda x: genTransTime(x.交易传输时间), axis = 1)
#整理借贷记卡标志列
result['借贷记卡标志']=result.apply(lambda x: genCardType(x.借贷记卡标志), axis = 1)
#交易金额格式转换
result['交易金额']=result.apply(lambda x: int(x.交易金额), axis = 1)
#获取转接服务费
result['转接服务费']=result.apply(lambda x: genTransferFee(x.转接服务费), axis = 1)
#获取品牌费
result['品牌服务费']=result.apply(lambda x: genTransferFee(x.品牌服务费), axis = 1)
#发卡方手续费
result['发卡方手续费']=result.apply(lambda x: genIssFee(x.受理方应收手续费,x.受理方应付手续费), axis = 1)
#获取支付类型,对账标识，调整标识
result['支付类型']=result.apply(lambda x:"0004", axis = 1)
result['对账标识']=result.apply(lambda x:  'N', axis = 1)
result['调整标识']=result.apply(lambda x:  'Y', axis = 1)
#获取渠道手续费
result['渠道手续费']=result.apply(lambda x:  genChannelFee(x.发卡方手续费,x.转接服务费,x.品牌服务费), axis = 1)

#生成最终待入库的文件
result.to_csv(os.getenv('HOME')+'/tmp/result.csv',index=False,sep=',',encoding="gb2312",columns=resultHeader)

#调用sqlloader导入数据库

#cmd_str="sqlldr posp/SMjeNA0heeW*Hy8\$@qianbao.yonyou control=$HOME/etc/impcups.ctl log=$HOME/log/sqlldrresult$impdate.log bad=$HOME/log/sqlldrbad$impdate.txt &> $HOME/log/sqlldr$impdate.log skip=1"
# cmd_str="sqlldr posp/posp@paydb control=$HOME/etc/impcups.ctl log=$HOME/log/sqlldrresult$impdate.log bad=$HOME/log/sqlldrbad$impdate.txt &> $HOME/log/sqlldr$impdate.log skip=1"
# call(["/u01/app/oracle/product/11.2.0/client/bin/sqlldr", "posp/posp@paydb control=$HOME/etc/impcups.ctl log=$HOME/log/sqlldrresult$impdate.log bad=$HOME/log/sqlldrbad$impdate.txt &> $HOME/log/sqlldr$impdate.log skip=1"], shell=True)
# print(os.system(cmd_str))