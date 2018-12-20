#!/bin/sh
# environment variable: unit - modem unit.
# echo "This is a script to enable the modem."


if [ -z "$unit" ] || [ "$unit" -eq "0" ]; then
	prefix="usb_modem_"
else
	prefix="usb_modem${unit}_"
fi

modem_type=`nvram get ${prefix}act_type`
modem_vid=`nvram get ${prefix}act_vid`
modem_pid=`nvram get ${prefix}act_pid`
modem_dev=`nvram get ${prefix}act_dev`
modem_reg_time=`nvram get modem_reg_time`
wandog_interval=`nvram get wandog_interval`
atcmd=`nvram get modem_atcmd`
act_sim=`nvram get usb_modem_act_sim`

usb_gobi2=`nvram get usb_gobi2`
kernel_version=`uname -r`

if [ "$wandog_interval" == "" -o "$wandog_interval" == "0" ]; then
	wandog_interval=5
fi


# $1: ifname.
_get_wdm_by_usbnet(){
	rp1=`readlink -f /sys/class/net/$1/device 2>/dev/null`
	if [ "$rp1" == "" ]; then
		echo ""
		return
	fi

	rp2=
	i=0
	while [ $i -lt 5 ]; do
		ver_head=`echo -n $kernel_version |awk 'BEGIN{FS="."}{print $1}'`
		ver_2nd=`echo -n $kernel_version |awk 'BEGIN{FS="."}{print $2}'`
		if [ "$ver_head" -ge "4" ]; then
			rp2=`readlink -f /sys/class/usbmisc/cdc-wdm$i/device 2>/dev/null`
		elif [ "$ver_head" -eq "3" ] && [ "$ver_2nd" -ge "10" ]; then # ex: BlueCave
			rp2=`readlink -f /sys/class/usbmisc/cdc-wdm$i/device 2>/dev/null`
		else
			rp2=`readlink -f /sys/class/usb/cdc-wdm$i/device 2>/dev/null`
		fi
		if [ "$rp2" == "" ]; then
			i=`expr $i + 1`
			continue
		fi

		if [ "$rp1" == "$rp2" ]; then
			echo "/dev/cdc-wdm$i"
			return
		fi

		i=`expr $i + 1`
	done

	echo ""
}

# $1: ifname.
_get_qcqmi_by_usbnet(){
	rp1=`readlink -f /sys/class/net/$1/device 2>/dev/null`
	if [ "$rp1" == "" ]; then
		echo ""
		return
	fi

	rp2=
	i=0
	while [ $i -lt 10 ]; do
		rp2=`readlink -f /sys/class/GobiQMI/qcqmi$i/device 2>/dev/null`
		if [ "$rp2" == "" ]; then
			i=$((i+1))
			continue
		fi

		if [ "$rp1" == "$rp2" ]; then
			echo "qcqmi$i"
			return
		fi

		i=$((i+1))
	done

	echo ""
}


if [ "$modem_type" == "gobi" ]; then
	if [ "$usb_gobi2" == "1" ]; then
		echo "Gobi2: Pause the autoconnect."
		at_ret=`/usr/sbin/modem_at.sh "+CAUTOCONNECT=0" 2>&1`
		ret=`echo -n $at_ret |grep "OK" 2>/dev/null`
		if [ -z "$ret" ]; then
			echo "Gobi2: Fail to stop the autoconnect."
			exit 0
		fi
		at_ret=`/usr/sbin/modem_at.sh "+CWWAN=0" 2>&1`
		ret=`echo -n $at_ret |grep "OK" 2>/dev/null`
		if [ -z "$ret" ]; then
			echo "Gobi2: Fail to stop the connection."
			exit 0
		fi
	else
		qcqmi=`_get_qcqmi_by_usbnet $modem_dev`
		echo "Got qcqmi: $qcqmi."

		gobi_api $qcqmi SetEnhancedAutoconnect 0 1
		sleep 1

		# mainly kill gobi_api of IPv6
		killall gobi_api

		if [ -n "$act_sim" ] && [ "$act_sim" -ge "1" ]; then
			wait_time1=`expr $wandog_interval + $wandog_interval`
			wait_time=`expr $wait_time1 + $modem_reg_time`
			nvram set freeze_duck=$wait_time
			/usr/sbin/modem_at.sh '+COPS=2' "$modem_reg_time"
			if [ -z "$atcmd" ] || [ "$atcmd" != "1" ]; then
				/usr/sbin/modem_at.sh '' # clean the output of +COPS=2.
			fi
		fi
	fi
elif [ "$modem_type" == "qmi" ]; then
	wdm=`_get_wdm_by_usbnet $modem_dev`

	uqmi -d $wdm --keep-client-id wds --set-autoconnect disabled
	if [ "$?" != "0" ]; then
		echo "modem_stop: QMI($wdm): faile to disable autoconnect..."
	fi

	uqmi -d $wdm --keep-client-id wds --stop-network 4294967295
	if [ "$?" != "0" ]; then
		echo "modem_stop: QMI($wdm): faile to stop the network..."
	fi
fi

echo "$modem_type: Successfull to stop network."
