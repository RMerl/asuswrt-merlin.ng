#!/bin/sh

# $1 = trip 
# $2 = enable

log_file=/tmp/thermal.log
log_time=$(date '+%Y-%m-%d %H:%M:%S')

echo -n "$log_time " >> $log_file
echo -n "Trip=$1 Enable=$2 " >> $log_file

case $1 in
	0)
		case $2 in
		0)
			pwr config --physpeed off
		;;
		1)
			pwr config --physpeed on
		;;
		esac
		;;
	1)
		case $2 in
		0)
            pwr config --cpuspeed off
			;;
		1)
            pwr config --cpuspeed on
			;;
		esac
		;;
	2)
		case $2 in
		0)
			pwr config --wifi off
			;;
		1)
			pwr config --wifi on
			;;
		esac
		;;
	3)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	4)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	5)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	6)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	7)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	8)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	9)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	10)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	11)
		case $2 in
		0)
			;;
		1)
			;;
		esac
		;;
	*)
		echo -n "Unknown trip $1" >> $log_file
		;;
esac
echo "" >> $log_file
