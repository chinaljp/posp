#!/bin/bash
settledate=`date -d "-1 day" +%Y%m%d`
impdate=$settledate
timestamp=`echo $[$(date +%s%N)/1000000]`
key=cb59e84a7f0545a9b55595df74dd3e02
partnerId=
merchidlist="822241053985003"
requestId=123456780
requestaddr=http://apis.gspay.org:443/gspay/api/interface

splitbill(){
rm -rf $HOME/shell/tmp3.txt
cat $chkfile |while read line  
do
	time1=`echo $line |awk -F '|' '{print $2}'`
	time3=`echo $[$time1/1000]`
	time2=`date -d @$time3 "+%Y%m%d %H%M%S"`
	time4=${time2:9:15}
	date=${time2:0:8}
	line=`echo $line|sed "s,$time1,$time4,g"`
        echo $date\|$line  >> $HOME/shell/tmp3.txt
done
return 0
}

chktrans(){
sqlplus -s posp/SMjeNA0heeW*Hy8\$@qianbao.yonyou <<EOF
set term off echo off feedback off heading off underline off; 
    merge into (select * from b_spdb_trans_detail  \
            where chk_flag ='N' ) h  \
            using B_pos_trans_Detail_his c  \
            on (c.channel_rrn=h.order_no and c.CHECK_FLAG='N' and c.trans_date=h.trans_date and c.istresp_code='00') \
             when matched then \
            update set  h.CHK_FLAG='Y',h.rrn=c.rrn;
     merge into (select * from B_pos_trans_Detail_his  \
            where check_flag ='N' ) h  \
            using b_spdb_trans_detail c  \
            on (c.order_no=h.channel_rrn and c.CHK_FLAG='Y' and c.trans_date=h.trans_date ) \
             when matched then \
            update set  h.CHECK_FLAG='Y',h.channel_fee=c.fee;
UPDATE B_SPDB_TRANS_DETAIL  set CHK_FLAG='L' where CHK_FLAG='N';
exit;
EOF
}

impchkfile(){
sqlldr posp/SMjeNA0heeW*Hy8\$@qianbao.yonyou control=$HOME/etc/impspdb.ctl log=$HOME/log/spdbsqlldrresult$impdate.log bad=$HOME/log/spdbsqlldrbad$impdate.txt &> $HOME/log/spdbsqlldr$impdate
}

downfile(){
sigdata="bizType=checkOrder&formDate=$settledate&partnerId=$partnerId&requestId=$requestId&timestamp=$timestamp&version=1.0&key=$key"
md5=`echo -n $sigdata|md5sum|cut -d ' ' -f1`
reqcontent="bizType=checkOrder&version=1.0&timestamp=$timestamp&partnerId=$partnerId&requestId=$requestId&formDate=$settledate&mac=$md5"
filename=`curl -X POST -d $reqcontent $requestaddr |iconv -f utf-8  -t gbk` 
downfileurl=`echo $filename|awk -F "dzurl" '/dzurl/{print$2}'|awk -F "}" '{print$1}'`
downfileurl=`echo ${downfileurl:3}`
downfileurl=`echo ${downfileurl%\"}`
downfileurl=`echo ${downfileurl/https/http}`
downfileurl=`echo ${downfileurl/org/org:443}`
echo $downfileurl
mkdir /duizhang/pufa/$impdate/
wget -O $chkfile $downfileurl
}
#更新分润流水
updappend(){
sqlplus -s posp/"SMjeNA0heeW*Hy8\$"@qianbao.yonyou <<EOF
set term off echo off feedback off heading off underline off; 
merge into (select * from B_append_trans_detail  \
            where check_flag ='N' and group_code='pos' ) h \
            using B_SPDB_TRANS_DETAIL c \
            on (c.rrn=h.rrn and c.CHK_FLAG='Y' and c.trans_date=h.trans_date ) \
             when matched then \
            update set  h.CHECK_FLAG='Y',h.channel_fee=c.fee,h.settle_date=c.settle_date; 
exit;
EOF
}
for partnerId in $merchidlist
do
chkfile=/duizhang/pufa/$impdate/${partnerId}_$impdate.txt
	downfile;
	splitbill;
	impchkfile;
done
chktrans;
updappend;
