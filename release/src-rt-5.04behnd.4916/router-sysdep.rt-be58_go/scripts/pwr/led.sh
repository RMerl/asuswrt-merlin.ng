#!/bin/sh

leds_path=/sys/class/leds
tmp_path=/tmp/led

case $1 in
suspend)
	mkdir $tmp_path
	for led in `ls $leds_path`
	do
		brightness=`cat $leds_path/$led/brightness`
		if [[ $brightness != "0" ]]; then
	        echo $brightness > $tmp_path/$led
			echo 0 > $leds_path/$led/brightness
        fi
	done
	;;
resume)
	for led in `ls $tmp_path`
	do
		echo $led
		cat $tmp_path/$led > $leds_path/$led/brightness
	done
	rm -rf $tmp_path
	;;
esac
