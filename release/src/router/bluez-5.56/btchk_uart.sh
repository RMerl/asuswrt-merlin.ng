#!/bin/sh

hot_plug_file="/tmp/plug_bluetooth"
cmd_timeoutfile="/sys/class/bluetooth/hci0/cmd_timeout"
exec_name=$0
stage=$1
bt_gpio=$2
bt_dev=$3
bt_dbg=0

prn(){
	echo "[BT:($stage),($btchk_round),($bt_reset_count)]: $*" | logger -c
}

reset_bt(){
	prn "reset BT"
	echo -n "0" > /sys/class/gpio/gpio${bt_gpio}/value
	sleep 2
	echo -n "1" > /sys/class/gpio/gpio${bt_gpio}/value
	bt_reset_count="$(($bt_reset_count + 1))"
	nvram set bt_reset_cnt=$bt_reset_count

	ps | grep 'hciattach' >& /dev/null
	if [ $? != "0" ] ; then
		killall hciattach
	fi

	execute_bt_bscp
	export bt_reset_count
}

if [ "$#" != "3" ]; then
	echo "Usage: $0 wait_plug|hci_check|qis_wait gpio device"
	exit
fi

if [ $stage == "t1" ]; then
	reset_bt
	exit
elif [ $stage == "t2" ]; then
	if [ $(cat ${cmd_timeoutfile}) != 0 ]; then
		echo "CMD_timeout != 0?"
	else
		echo "CMD_timeout == 0?"
	fi
	exit
elif [ $stage == "t3" ]; then
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
				exec $exec_name hci_check ${bt_gpio} ${bt_dev}
			fi
			sleep 1
	                c="$(($c + 1))"
		done
		prn "timeout"
		if [ $btchk_round -ge 3 ]; then
			prn "no BT interface, reset again..."
			#reboot
			#exit
		elif [ $btchk_round -ge 2 ]; then
			prn "no BT interface, reset..."
		fi
		reset_bt
		exec $exec_name wait_plug ${bt_gpio} ${bt_dev}
		;;
	hci_check)
		echo "waiting..."
		sleep 15 
		hciconfig hci0 | grep 'UP RUNNING' >& /dev/null
		bt_state=$?
		hciconfig hci0 name | grep 'ASUS' >& /dev/null
		bt_name=$?
		if [ $bt_state != "0" ] || [ $bt_name != "0" ] ; then
			prn "hci0 check fail"
			killall bluetoothd
			rm -f $hot_plug_file

			prn "fail to init BT!! reset again..."
			reset_bt

			exec $exec_name wait_plug ${bt_gpio} ${bt_dev}
		else
			# check if bluetootd is running
			ps -w | grep bluetoothd | grep aqis > /dev/null
			if [ $? != "0" ] ; then
				prn "Bluetoothd is not running, reset again..."
				reset_bt
				exec $exec_name wait_plug ${bt_gpio} ${bt_dev}
			fi

			if [ ! -f $cmd_timeoutfile ] || [ $(cat ${cmd_timeoutfile}) != 0 ]; then
				prn "BT cmd timeout1... reset again"
				reset_bt
				exec $exec_name wait_plug ${bt_gpio} ${bt_dev}
			fi

			if [ $bt_dbg == "1" ]; then
				hciconfig -a
				echo -n "NVRAM: bt_reset_cnt "
				nvram get bt_reset_cnt
			fi
			sleep 1
			prn "Init OK!!"
			exec $exec_name qis_wait ${bt_gpio} ${bt_dev}
		fi
		;;
	qis_wait)
		c=0
		nvram set bt_inf_state=1
		while [ $bt_dbg == "1" ]; do
			if [ $c -ge 10 ]; then
				bt_reset_tmp=$(nvram get bt_reset_${bt_reset_count})
				if [ $? == "" ]; then
					nvram set bt_reset_${bt_reset_count}=1
				else
	                		bt_reset_tmp="$((${bt_reset_tmp} + 1))"
					nvram set bt_reset_${bt_reset_count}=$bt_reset_tmp
				fi
				echo "====================================="
				echo "BT: aginging reboot"
				nvram get bt_inf_state
				nvram show | grep 'bt_reset'
				nvram set bt_inf_state=0
				nvram commit
				reboot
				break;
			fi
		        c="$(($c + 1))"
	        done
		;;
	*)
		stage=""
		prn "invalid parameter!"
		;;
esac
