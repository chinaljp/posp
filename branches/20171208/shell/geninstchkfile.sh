#!/bin/bash
source ${HOME}/.bash_profile
settle_date=`date -d "-1 day" +%Y%m%d`;
before_date=`date -d "-2 day" +%Y%m%d`;
if [ $1 ]
then
        settle_date=$1
	before_date=`date -d "yesterday $1" +%Y%m%d`
fi
echo $settle_date
logdir="/home/posp/log"
filedir="/xyjg"
#mkdir -p $filedir/$agent_id
for QUERYINST in `sqlplus -s posp/"SMjeNA0heeW*Hy8\$"@qianbao.yonyou <<EOF
set term off echo off feedback off heading off underline off; 
select agent_id from b_agent where channel_flag='1' and agent_id in('49000251' ,'49000455','49000780','49000199','49000525','49000445','49000501','49000870','49000546','49009929'); 
exit;
EOF`
do
mkdir -p $filedir/${QUERYINST}/$settle_date
#echo $QUERYINST
#echo $QUERYS
sqlplus posp/SMjeNA0heeW*Hy8\$@qianbao.yonyou >> ${logdir}/chkfileinst.log.${settle_date} <<EOF
set verify off;
set wrap off;
set echo off;
set term off;
set heading off;
set feedback off;
set pagesize 0;
set linesize 30000;
set trimspool on;
spool $filedir/${QUERYINST}/$settle_date/CJZF_${QUERYINST}_${settle_date}.csv;
select trace_no||','||trans_time||','||trans_date||','||card_no||','||to_char(amount,'FM999999990.00') ||','||decode(t.trans_code,'020000','0200,000000' \
,'U20000','0200,000000','M20000','0200,000000','020001','0220,200000','U20001','0220,200000','020002','0200,200000','M20002','0200,200000','020003','0400,000000','M20003','0400,000000','024100','0200,000000','024102','0200,200000','024103','0400,000000','020023','0400,200000','M20023','0400,200000','024123','0400,200000')||','||t.merch_id||','||t.term_id||','||m.merch_name||','||t.rrn||','||t.auth_code||','||t.o_trans_date||','||t.o_rrn||','||to_char(t.fee,'FM999999990.00') ||','||merch_order_no from B_POS_TRANS_DETAIL_HIS t \
    left join b_merch m on t.merch_id=m.merch_id join S_TRANS_CODE c on c.TRANS_CODE=t.TRANS_CODE and c.settle_flag='1'  where t.rrn in(select channel_rrn from b_cups_trans_detail where chk_flag in ('Y','B') and settle_date = $settle_date) and t.trans_date between $before_date and $settle_date and m.agent_id in( select agent_id from b_agent start with agent_id=$QUERYINST connect by prior agent_id= p_agent_id) ;
spool off;
spool $filedir/${QUERYINST}/$settle_date/CJZF_${QUERYINST}_${settle_date}.csv append;
select trace_no||','||trans_time||','||trans_date||','||card_no||','||to_char(amount,'FM999999990.00') ||','||decode(t.trans_code,'020000','0200,000000' \
,'U20000','0200,000000','M20000','0200,000000','020001','0220,200000','U20001','0220,200000','020002','0200,200000','M20002','0200,200000','020003','0400,000000','M20003','0400,000000','024100','0200,000000','024102','0200,200000','024103','0400,000000','020023','0400,200000','M20023','0400,200000','024123','0400,200000')||','||t.merch_id||','||t.term_id||','||m.merch_name||','||t.rrn||','||t.auth_code||','||t.o_trans_date||','||t.o_rrn||','||to_char(t.fee,'FM999999990.00') ||','||merch_order_no from B_POS_TRANS_DETAIL_HIS t \
    left join b_merch m on t.merch_id=m.merch_id join S_TRANS_CODE c on c.TRANS_CODE=t.TRANS_CODE and c.settle_flag='1'  where t.o_rrn in(select channel_rrn from b_cups_trans_detail where chk_flag in ('B') and settle_date = $settle_date) and t.agent_id in( select agent_id from b_agent start with agent_id=$QUERYINST connect by prior agent_id= p_agent_id) and substr(t.trans_code,6,1)='3' ;
spool off;
spool $filedir/${QUERYINST}/$settle_date/CJZF_${QUERYINST}_${settle_date}.csv append;
select trace_no||','||trans_time||','||trans_date||','||card_no||','||to_char(amount,'FM999999990.00') ||','||decode(t.trans_code,'020000','0200,000000' \
,'U20000','0200,000000' ,'M20000','0200,000000' ,'020001','0220,200000' ,'U20001','0220,200000' ,'020002','0200,200000' ,'M20002','0200,200000' ,'020003','0400,000000' ,'M20003','0400,000000' ,'024100','0200,000000' ,'024102','0200,200000' ,'024103','0400,000000' \
,'020023','0400,200000' ,'M20023','0400,200000' ,'024123','0400,200000')||','||t.merch_id||','||t.term_id||','||m.merch_name||','||t.rrn||','||t.auth_code||','||t.o_trans_date||','||t.o_rrn||','||to_char(t.fee,'FM999999990.00') ||','||merch_order_no from B_POS_TRANS_DETAIL_HIS t \
    left join b_merch m on t.merch_id=m.merch_id join S_TRANS_CODE c on c.TRANS_CODE=t.TRANS_CODE and c.settle_flag='1'  where t.rrn in(select rrn from b_spdb_trans_detail where chk_flag in ('Y','B') and settle_date = $settle_date) and t.trans_date between $before_date and $settle_date and m.agent_id in( select agent_id from b_agent start with agent_id=$QUERYINST connect by prior agent_id= p_agent_id);
spool off;
exit;
EOF
sed -i '/SQL/d' $filedir/${QUERYINST}/$settle_date/CJZF_${QUERYINST}_${settle_date}.csv
sed -i '1i\凭证号,交易时间,交易日期,主账号,交易金额(元),报文类型,交易处理码,商户号,终端号,商户名称,检索参考号,授权码,原交易日期,原始交易检索参考号,手续费(元),订单信息'  $filedir/${QUERYINST}/$settle_date/CJZF_${QUERYINST}_${settle_date}.csv 
done
