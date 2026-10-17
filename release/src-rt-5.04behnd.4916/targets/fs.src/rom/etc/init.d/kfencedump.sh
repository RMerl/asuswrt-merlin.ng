#!/bin/sh

CORE_DIR_NEW=$1
CORE_DIR_LAST=$2
KFENCE_STATS_DIR=/sys/kernel/debug/kfence/stats
alias err_cnt="cat ${KFENCE_STATS_DIR} | grep 'total bugs' | awk -F ':' '{print \$NF}' "

kfence_report_poll()
{
	last_err_cnt=0
	curr_err_cnt=0

	if [ ! -e $KFENCE_STATS_DIR ]; then
		return;
	fi

	echo "enter_kfence_message_poll:$1, $2"

	if [ -f $CORE_DIR_NEW/kfence_log.tar.gz ]; then
		mv $CORE_DIR_NEW/kfence_log.tar.gz $CORE_DIR_LAST/
	fi

	last_err_cnt=$(err_cnt)

	while true
	do
		curr_err_cnt=$(err_cnt)

		if [ $last_err_cnt != $curr_err_cnt ]; then
			# delete old kfence log file
			if [ -f $CORE_DIR_NEW/kfence_log.tar.gz ]; then
				rm $CORE_DIR_NEW/kfence_log.tar.gz
			fi

			dmesg > /tmp/kfence_log.txt
			tar -czf $CORE_DIR_NEW/kfence_log.tar.gz -C /tmp kfence_log.txt

			rm /tmp/kfence_log.txt
		fi

		last_err_cnt=$curr_err_cnt
		# poll the kfence stats every 30s.
		sleep 30
	done
}

kfence_report_poll &
