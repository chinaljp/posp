#!/bin/bash
source ${HOME}/.bash_profile

impdate=`date -d "-1 day" +%Y%m%d`
impcups -l impcups -c 49000000 -t $impdate
sleep 30s
sqlldr posp/posp@qianbao.yonyou control=$HOME/etc/impcups.ctl log=$HOME/log/sqlldrresult$impdate.log bad=$HOME/log/sqlldrbad$impdate.txt &> $HOME/log/sqlldr$impdate.log
