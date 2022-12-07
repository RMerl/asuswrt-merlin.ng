#!/bin/sh
PATH=/usr/bin:/bin:/usr/sbin:/sbin

hot_plug_file="/tmp/plug_bluetooth"
cmd_timeoutfile="/sys/class/bluetooth/hci0/cmd_timeout"
stage=$1
exec_name=$0
bt_gpio=$2
bt_dbg=0

prn(){
	echo "[BT:($stage),($btchk_round),($bt_reset_count)]: $*" | logger -c
}

reset_3012(){
	echo -n "c" > $cmd_timeoutfile
	prn "reset AR3012"
	echo -n "1" > /sys/class/gpio/gpio${bt_gpio}/value
	sleep 1
	echo -n "0" > /sys/class/gpio/gpio${bt_gpio}/value
	sleep 1
	echo -n "1" > /sys/class/gpio/gpio${bt_gpio}/value
	bt_reset_count="$(($bt_reset_count + 1))"
	nvram set bt_reset_cnt=$bt_reset_count
	export bt_reset_count
}

reload_module(){
	local mode=$1
	# echo -n "c" > $cmd_timeoutfile # it will be cleared when module reloaded
	rmmod ath3k
	rmmod btusb
	rmmod xhci_hcd
	if [ "$mode" == "full" ]; then
		rmmod dwc3-ipq40xx
		rmmod dwc3
		rmmod phy-qca-uniphy
		rmmod phy-qca-baldur
		modprobe phy-qca-baldur
		modprobe phy-qca-uniphy
		modprobe dwc3
		modprobe dwc3-ipq40xx
	fi
	modprobe btusb
	modprobe ath3k
	modprobe xhci_hcd
	bt_reset_count="$(($bt_reset_count + 1))"
	nvram set bt_reset_cnt=$bt_reset_count
	export bt_reset_count
}

if [ "$#" != "2" ]; then
	echo "Usage: $0 wait_plug|hci_check|qis_wait gpio"
	exit
fi

if [ $stage == "t1" ]; then
	reload_module full
	exit
elif [ $stage == "t2" ]; then
	reload_module simple
	exit
elif [ $stage == "t3" ]; then
	reset_3012
	exit
elif [ $stage == "t4" ]; then
	if [ $(cat ${cmd_timeoutfile}) != 0 ]; then
		echo "CMD_timeout != 0?"
	else
		echo "CMD_timeout == 0?"
	fi
	exit
elif [ $stage == "t5" ]; then
	echo "clear $cmd_timeoutfile"
	echo -n "c" > $cmd_timeoutfile
	exit
fi

nvram get x_Setting | grep -q 1
if [ $? == "0" ]; then
	echo "BT: workaround byebye~~~"
	exit 0
fi

if [ "$btchk_round" == "" ]; then
	btchk_round=1
	bt_reset_count=0
	nvram set bt_reset_cnt=$bt_reset_count
	export bt_reset_count
else
	btchk_round="$(($btchk_round + 1))"
fi
export btchk_round

case "$stage" in
	wait_plug)
		timeout=35
		c=1
		while [ $c -le $timeout ]; do
			if [ -f $hot_plug_file ]; then
				exec $exec_name hci_check ${bt_gpio}
			fi
			sleep 1
	                c="$(($c + 1))"
		done
		prn "timeout"
		if [ $btchk_round -ge 3 ]; then
			prn "no hotplug, reset again..."
			#reboot
			#exit
		elif [ $btchk_round -ge 2 ]; then
			prn "no hotplug, reset..."
			reload_module full
		fi
		reset_3012
		exec $exec_name wait_plug ${bt_gpio}
		;;
	hci_check)
		echo "waiting..."
		sleep 30
		hciconfig hci0 | grep 'UP RUNNING' >& /dev/null
		if [ $? != "0" ] ; then
			prn "hci0 check fail"
			killall bluetoothd
			rm -f $hot_plug_file
			if [ $btchk_round -ge 8 ]; then
				prn "fail to init BT!! reset again..."
				reset_3012
			elif [ $btchk_round -ge 6 ]; then
				prn "reset BT 3"
				reload_module full
				reset_3012
			elif [ $btchk_round -ge 4 ]; then
				prn "reset BT 2"
				reset_3012
			elif [ $btchk_round -ge 2 ]; then
				prn "reset BT 1"
				reload_module basic
			fi
			exec $exec_name wait_plug ${bt_gpio}
		else
			# check if bluetootd is running
			ps -w | grep bluetoothd | grep aqis > /dev/null
			if [ $? != "0" ] ; then
				prn "Bluetoothd is not running, reset again..."
				reset_3012
				exec $exec_name wait_plug ${bt_gpio}
			fi

			if [ ! -f $cmd_timeoutfile ] || [ $(cat ${cmd_timeoutfile}) != 0 ]; then
				prn "BT cmd timeout1... reset again"
				reset_3012
				exec $exec_name wait_plug ${bt_gpio}
			fi

			prn "Init OK!!"
			if [ $bt_dbg == "1" ]; then
				hciconfig -a
				echo -n "NVRAM: bt_reset_cnt "
				nvram get bt_reset_cnt
			fi
			sleep 1
			exec $exec_name qis_wait ${bt_gpio}
		fi
		;;
	qis_wait)
		c=0
		while [ 1 ]; do
			nvram get x_Setting | grep -q 1
			if [ $? == "0" ]; then
				echo "BT: workaround byebye~~"
				break;
			elif [ ! -f $cmd_timeoutfile ] || [ $(cat ${cmd_timeoutfile}) != 0 ]; then
				prn "BT cmd timeout2... reset again"
				reset_3012
				exec $exec_name wait_plug ${bt_gpio}
			else
				sleep 2
				if [ $bt_dbg == "1" ] && [ $c -ge 10 ]; then
					echo "BT: aginging reboot"
					reboot
					break;
				fi
		                c="$(($c + 1))"
			fi
	        done
		;;
	*)
		stage=""
		prn "invalid parameter!"
		;;
esac
