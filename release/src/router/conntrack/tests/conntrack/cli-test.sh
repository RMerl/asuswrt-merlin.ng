CONNTRACK=conntrack

SRC=1.1.1.1
DST=2.2.2.2
SPORT=2005
DPORT=21

case $1 in
	dump)
		echo "Dumping conntrack table"
		$CONNTRACK -L
		;;
	flush)
		echo "Flushing conntrack table"
		$CONNTRACK -F
		;;
	new)
		echo "creating a new conntrack"
		$CONNTRACK -I --orig-src $SRC --orig-dst $DST \
		 --reply-src $DST --reply-dst $SRC -p tcp \
		 --orig-port-src $SPORT  --orig-port-dst $DPORT \
		 --reply-port-src $DPORT --reply-port-dst $SPORT \
		--state LISTEN -u SEEN_REPLY -t 50
		;;
	new-simple)
		echo "creating a new conntrack (simplified)"
		$CONNTRACK -I -s $SRC -d $DST \
		-p tcp --sport $SPORT  --dport $DPORT \
		--state LISTEN -u SEEN_REPLY -t 50
		;;
	new-nat)
		echo "creating a new conntrack (NAT)"
		$CONNTRACK -I -s $SRC -d $DST \
		-p tcp --sport $SPORT  --dport $DPORT \
		--state LISTEN -u SEEN_REPLY -t 50 --dst-nat 8.8.8.8
		;;
	get)
		echo "getting a conntrack"
		$CONNTRACK -G -s $SRC -d $DST \
		-p tcp --sport $SPORT --dport $DPORT
		;;
	change)
		echo "change a conntrack"
		$CONNTRACK -U -s $SRC -d $DST \
		-p tcp --sport $SPORT --dport $DPORT \
		--state TIME_WAIT -u ASSURED,SEEN_REPLY -t 500
		;;
	delete)
		$CONNTRACK -D -s $SRC -d $DST \
		-p tcp --sport $SPORT --dport $DPORT
		;;
	output)
		proc=$(cat /proc/net/ip_conntrack | wc -l)
		netl=$($CONNTRACK -L | wc -l)
		count=$(cat /proc/sys/net/ipv4/netfilter/ip_conntrack_count)
		if [ $proc -ne $netl ]; then
			echo "proc is $proc and netl is $netl and count is $count"
		else
			if [ $proc -ne $count ]; then
				echo "proc is $proc and netl is $netl and count is $count"
			else
				echo "now $proc"
			fi
		fi
		;;
	dump-expect)
		$CONNTRACK -L expect
		;;
	flush-expect)
		$CONNTRACK -F expect
		;;
	create-expect)
		# requires modprobe ip_conntrack_ftp
		$CONNTRACK -I expect --orig-src $SRC --orig-dst $DST \
		--tuple-src 4.4.4.4 --tuple-dst 5.5.5.5 \
		--mask-src 255.255.255.0 --mask-dst 255.255.255.255 \
		-p tcp --orig-port-src $SPORT --orig-port-dst $DPORT \
		-t 200 --tuple-port-src 10240 --tuple-port-dst 10241\
		--mask-port-src 10 --mask-port-dst 300
		;;
	get-expect)
		$CONNTRACK -G expect --orig-src 4.4.4.4 --orig-dst 5.5.5.5 \
		--p tcp --orig-port-src 10240 --orig-port-dst 10241
		;;
	delete-expect)
		$CONNTRACK -D expect --orig-src 4.4.4.4 \
		--orig-dst 5.5.5.5 -p tcp --orig-port-src 10240 \
		--orig-port-dst 10241
		;;
	*)
		echo "Usage: $0 [dump"
		echo "		|new"
		echo "		|new-simple"
		echo "		|new-nat"
		echo "		|get"
		echo "		|change"
		echo "		|delete"
		echo "		|output"
		echo "		|flush"
		echo "		|dump-expect"
		echo "		|flush-expect"
		echo "		|create-expect"
		echo "		|get-expect"
		echo "		|delete-expect]"
		;;
esac
