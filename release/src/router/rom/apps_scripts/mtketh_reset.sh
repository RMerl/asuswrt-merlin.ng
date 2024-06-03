#!/bin/sh
num=0
while true
do
	#regs d 1b100100
	par_q=`regs d 1b100100|grep 1b100100|awk 'BEGIN{FS=":"} {print $2}'|awk 'BEGIN{FS=" "} {print $1}'`
	par_w=`regs d 1b100100|grep 1b100110|awk 'BEGIN{FS=":"} {print $2}'|awk 'BEGIN{FS=" "} {print $3}'`


	Free_Queue_Page_Counter=0x`echo ${par_q:0:2}`

	Low_Threshold=0x`echo ${par_q:6:2}`

	Output_Queue_Count=0x`echo ${par_w:6:2}`

	FQ=`printf %d $Free_Queue_Page_Counter`
	LT=`printf %d $Low_Threshold`
	OQ=`printf %d $Output_Queue_Count`
	kk=`printf %d 0x80`

	if [ $FQ -lt $LT ]  && [ $OQ -gt $kk ]; then
		num=$(($num+1))
	fi

	if [ $num -eq 2 ]; then
		echo 1 > /sys/kernel/debug/mtketh/reset
		num=0
	fi 
	sleep 3
done
