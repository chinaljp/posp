
#!/bin/bash

svr_list="ingate  ingate_s manager appsvr safsvr outgate wallet-notice /usr/java/jdk1.7.0_80/bin/java qr_notice merch_notice addwallet ordernotice"
workdir=$HOME

stopapp()
{
	echo "app stop ..."
	for svrname in $svr_list 
	do
		kill -SIGUSR1 `ps -ef|grep $svrname|grep -v grep|awk -v _SVRNAME_=$svrname '{ if ( $8==_SVRNAME_ )  print $2 }'`
	done
	return 0
}

kstopapp()
{
	echo "app stop ..."
	for svrname in $svr_list 
	do
		kill -9 `ps -ef|grep $svrname|grep -v grep|awk -v _SVRNAME_=$svrname '{ if ( $8==_SVRNAME_ )  print $2 }'`
	done
	return 0
}

startapp()
{
	        echo "app start ..."
        #appsvr  -f 0G9200.conf -l cupssvr -s 0G9200 -b libadmin.so -n 1
        #appsvr  -f 0G9700.conf -l cupssvr -s 0G9700 -b libadmin.so -n 1
        #appsvr  -f 0G9900.conf -l cupssvr -s 0G9900 -b libadmin.so -n 1
        #appsvr  -f 0G9600.conf -l cupssvr -s 0G9600 -b libadmin.so -n 1
        appsvr  -f 020001.conf -l 020001 -s 020001 -b libaccount.so -n 1
        appsvr  -f 020000.conf -l 020000 -s 020000 -b libaccount.so -n 10
        appsvr  -f 020000.conf -l 020000 -s U20000 -b libaccount.so -n 1
        appsvr  -f 020003.conf -l 020000 -s 020003 -b libaccount.so -n 2
        appsvr  -f 020023.conf -l 020000 -s 020023 -b libaccount.so -n 1
        appsvr  -f 020002.conf -l 020000 -s 020002 -b libaccount.so -n 1
        appsvr  -f 024000.conf -l preauthsvr -s 024000 -b libaccount.so -n 1
        appsvr  -f 024002.conf -l preauthsvr -s 024002 -b libaccount.so -n 1
        appsvr  -f 024023.conf -l preauthsvr -s 024023 -b libaccount.so -n 1
        appsvr  -f 024003.conf -l preauthsvr -s 024003 -b libaccount.so -n 1
        appsvr  -f 024100.conf -l preauthsvr -s 024100 -b libaccount.so -n 1
        appsvr  -f 020002.conf -l preauthsvr -s 024102 -b libaccount.so -n 1
        appsvr  -f 020003.conf -l preauthsvr -s 024103 -b libaccount.so -n 1
        appsvr  -f 020023.conf -l preauthsvr -s 024123 -b libaccount.so -n 1
	appsvr  -f 021000.conf -l acctsvr -s 021000 -b libaccount.so -n 1 
	appsvr  -f 020400.conf -l acctsvr -s 020400 -b libaccount.so -n 1 
	appsvr  -f 022100.conf -l acctsvr -s 022100 -b libaccount.so -n 1
        appsvr  -f M20000.conf -l ordersvr -s M20000 -b libaccount.so -n 5
        appsvr  -f M20003.conf -l ordersvr -s M20003 -b libaccount.so -n 2
        appsvr  -f M20002.conf -l ordersvr -s M20002 -b libaccount.so -n 1
        appsvr  -f M20023.conf -l ordersvr -s M20023 -b libaccount.so -n 1
        appsvr  -f M21000.conf -l ordersvr -s M21000 -b libadmin.so -n 1
        appsvr  -f T20000.conf -l d0acctsvr -s T20000 -b libaccount.so -n 5
        appsvr  -f 020003.conf -l d0acctsvr -s T20003 -b libaccount.so -n 2
        appsvr  -f T20010.conf -l d0acctsvr -s T20010 -b libaccount.so -n 1
	appsvr  -f TA0010.conf -l d0acctsvr -s TA0010 -b libaccount.so -n 1
	appsvr  -f 02A000.conf -l authsvr -s 02A000 -b libjianquan.so -n 1
	appsvr  -f 0AA001.conf -l authsvr -s 0AA001 -b libjianquan.so -n 1
	appsvr  -f 0AA002.conf -l authsvr -s 0AA002 -b libjianquan.so -n 1
	appsvr  -f 02A100.conf -l authsvr -s 02A100 -b libjianquan.so -n 1
	appsvr  -f 0AA000.conf -l authsvr -s 0AA000 -b libjianquan.so -n 1
	appsvr  -f 0AA100.conf -l authsvr -s 0AA100 -b libjianquan.so -n 1
        appsvr  -f 02B100.conf -l qrcodesvr -s 02B100 -b libqrcode.so -n 1
        appsvr  -f 02W100.conf -l qrcodesvr -s 02W100 -b libqrcode.so -n 1
        appsvr  -f 02B200.conf -l qrcodesvr -s 02B200 -b libqrcode.so -n 1
        appsvr  -f 02B300.conf -l qrcodesvr -s 02B300 -b libqrcode.so -n 1
        appsvr  -f 02W200.conf -l qrcodesvr -s 02W200 -b libqrcode.so -n 1
        appsvr  -f 02W300.conf -l qrcodesvr -s 02W300 -b libqrcode.so -n 1
	appsvr  -f 0AB100.conf -l qrcodesvr -s 0AB100 -b libqrcode.so -n 1
        appsvr  -f 0AW100.conf -l qrcodesvr -s 0AW100 -b libqrcode.so -n 1
        appsvr  -f 0AB200.conf -l qrcodesvr -s 0AB200 -b libqrcode.so -n 1
        appsvr  -f 0AB300.conf -l qrcodesvr -s 0AB300 -b libqrcode.so -n 1
        appsvr  -f 0AW200.conf -l qrcodesvr -s 0AW200 -b libqrcode.so -n 1
        appsvr  -f 0AW300.conf -l qrcodesvr -s 0AW300 -b libqrcode.so -n 1
        appsvr  -f 029000.conf -l 029000 -s 029000 -b libadmin.so -n 1
        appsvr  -f 029300.conf -l adminsvr -s 029300 -b libadmin.so -n 1
        appsvr  -f 029500.conf -l adminsvr -s 029500 -b libadmin.so -n 1
        appsvr  -f 029400.conf -l adminsvr -s 029400 -b libadmin.so -n 1
        appsvr  -f 027800.conf -l adminsvr -s 027800 -b libadmin.so -n 1
        appsvr  -f 027900.conf -l adminsvr -s 027900 -b libadmin.so -n 1
        appsvr  -f 028100.conf -l adminsvr -s 028100 -b libadmin.so -n 1
        appsvr  -f 028110.conf -l adminsvr -s 028110 -b libadmin.so -n 1
        appsvr  -f 028200.conf -l adminsvr -s 028200 -b libadmin.so -n 1
        appsvr  -f 028210.conf -l adminsvr -s 028210 -b libadmin.so -n 1
        appsvr  -f 028300.conf -l adminsvr -s 028300 -b libadmin.so -n 1
        appsvr  -f 028310.conf -l adminsvr -s 028310 -b libadmin.so -n 1
        appsvr  -f 028400.conf -l adminsvr -s 028400 -b libadmin.so -n 1
        appsvr  -f 028410.conf -l adminsvr -s 028410 -b libadmin.so -n 1
        appsvr  -f 028500.conf -l adminsvr -s 028500 -b libadmin.so -n 1
        appsvr  -f 028510.conf -l adminsvr -s 028510 -b libadmin.so -n 1
        appsvr  -f 028600.conf -l adminsvr -s 028600 -b libadmin.so -n 1
        appsvr  -f 028610.conf -l adminsvr -s 028610 -b libadmin.so -n 1
        appsvr  -f 00T000.conf -l walletsvr -s 00T000 -b libwallet.so -n 5
        appsvr  -f 00T100.conf -l walletsvr -s 00T100 -b libwallet.so -n 5
        appsvr  -f 00T200.conf -l walletsvr -s 00T200 -b libwallet.so -n 5
        appsvr  -f 00T200.conf -l walletsvr -s 00T300 -b libwallet.so -n 5
        appsvr  -f 00T200.conf -l walletsvr -s 00TB00 -b libwallet.so -n 1
        appsvr  -f 00T200.conf -l walletsvr -s 00TC00 -b libwallet.so -n 1
        appsvr  -f 00T200.conf -l walletsvr -s 00T400 -b libwallet.so -n 5
        appsvr  -f 00T200.conf -l walletsvr -s 00T500 -b libwallet.so -n 5
        appsvr  -f 00T600.conf -l walletsvr -s 00T600 -b libwallet.so -n 5
        appsvr  -f 00T700.conf -l walletsvr -s 00T700 -b libwallet.so -n 5
        appsvr  -f 00T700.conf -l walletsvr -s 00T800 -b libwallet.so -n 5
        appsvr  -f 00T900.conf -l walletsvr -s 00T900 -b libwallet.so -n 5
        appsvr  -f 00T100.conf -l walletsvr -s 00TA00 -b libwallet.so -n 1
        ingate_s -Q WEBD_Q -l in_webd -s WEBD -i 10.21.203.121 -p 18677 -d 0
        manager  -l webd -s WEBD -b libwebd.so -n 2
	manager  -l posd_1.8 -s POS_1.8 -b libposd_1.8.so -n 3
        manager  -l posd_1.9 -s POS_1.9 -b libposd_1.9.so -n 3
        manager  -l appd -s APP -b libappd.so -n 2
        manager  -l authd  -s AUTH -b libauthd.so -n 2
        manager  -l scanpayd  -s SCANPAY -b libscanpayd.so -n 2
        manager  -l jnorderd  -s JNORDER -b libjnorderd.so -n 2
	ingate -Q POS_1.8_Q -l in_posd_1.8 -s INGATE121_1.8 -i 10.21.203.121 -p 19998 -d 0
	ingate -Q POS_1.8_Q -l in_posd_1.8.2 -s INGATE121_1.8.2 -i 10.21.203.121 -p 9998 -d 0
        ingate -Q POS_1.9_Q -l in_posd_1.9 -s INGATE121_1.9 -i 10.21.203.121 -p 26521 -d 0
        ingate_s -Q APP_Q -l ins_appd -s INGATE_S121 -i 10.21.203.121 -p 12530 -d 0
	manager  -l cupsd -s CUPS -b libcupsd.so -n 10
        outgate -Q SCANPAY_P -l out_scanpayd -s SCANPAY -I 10.21.203.121  -P 8082 -d 1 -t 1
        outgate -Q AUTH_P -l out_authd -s JIANQUAN -I 10.20.8.111 -P 8082 -d 1 -t 1
        #outgate -Q JNORDER_P -l out_jnorderd -s JNORDER -I 10.21.30.131 -P 8889 -d 1 -t 1
	qr_notice -l qr_notice -t 300
	addwallet -l addwallet -t 300 
	merch_notice -l merch_notice -t 120
	/usr/java/jdk1.7.0_80/bin/java -jar /home/posp/bin/acctd-2.0-SNAPSHOT.jar ACCTD_Q &
	/usr/java/jdk1.7.0_80/bin/java -jar /home/posp/bin/acctd-2.0-SNAPSHOT.jar ACCTD_Q &
	/usr/java/jdk1.7.0_80/bin/java -jar /home/posp/bin/acctd-2.0-SNAPSHOT.jar ACCTD_Q &
	/usr/java/jdk1.7.0_80/bin/java -jar /home/posp/bin/acctd-2.0-SNAPSHOT.jar ACCTD_Q &
	/usr/java/jdk1.7.0_80/bin/java -jar /home/posp/bin/acctd-2.0-SNAPSHOT.jar ACCTD_Q &
	/usr/java/jdk1.7.0_80/bin/java -jar /home/posp/bin/acctd-2.0-SNAPSHOT.jar TRANSD_Q &
	/usr/java/jdk1.7.0_80/bin/java -jar /home/posp/bin/acctd-2.0-SNAPSHOT.jar TRANSD_Q &
	ordernotice -l notice -s NOTICE -c 3  -n 1	

        return 0
}

statusapp()
{
	echo "app status ..."
        for svrname in $svr_list
        do
            ps -ef|grep ^"$USER "|grep $svrname|grep -v grep|awk -v _SVRNAME_=$svrname '{ if ( $8==_SVRNAME_ )  {printf "id[%5s] pid[%5s]  ", $2, $3;for ( i=8; i<=NF; i=i+1) {printf $i" " };printf "\n"}}'
        done 
	return 0
}

case "$1" in  
	status)    
		statusapp  
	;;  
	start)    
		startapp
                statusapp
	;;  
	kstop)    
		kstopapp
                statusapp
	;;  
	stop)    
		stopapp  
                sleep 5
                statusapp
	;;  
	reload|restart)    
		stopapp   
                sleep 5
                statusapp 
		startapp  
                statusapp
	;;  
	*)    
		echo "Usage:	`basename $0` status|start|stop|restart|reload"
		exit 1
esac
