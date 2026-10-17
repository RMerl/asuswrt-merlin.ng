#!/bin/sh


# Check if profile id is passed
if [ "$#" -ne 1 ]; then
    echo "Error: profile_id is required"
    echo "Usage: $0 <profile_id>"
    exit 1
fi

echo "running script for BNL profile = $1"

profile_id=profile_$1

if [ ! -f "/proc/bnl/$profile_id/activelist" ]; then
    echo "Error: BNL profile $1 does not exist"
    exit 1
fi

print_header_only_once=true

val_print_fmt_singleline=0
val_activelist_clearOnread=0
val_print_only_nonzero=0

init_func() {

	val_print_fmt_singleline=$(cat /proc/sys/bnl/$profile_id/print_fmt_singleline)
	val_activelist_clearOnread=$(cat /proc/sys/bnl/$profile_id/activelist_clearOnread)
	val_activelist_print_only_nonzero=$(cat /proc/sys/bnl/$profile_id/activelist_print_only_nonzero)

	echo "1" > /proc/sys/bnl/$profile_id/print_fmt_singleline
	echo "1" > /proc/sys/bnl/$profile_id/activelist_clearOnread
	echo "1" > /proc/sys/bnl/$profile_id/activelist_print_only_nonzero
}

cleanup_func() {
	echo $val_print_fmt_singleline > /proc/sys/bnl/$profile_id/print_fmt_singleline
	echo $val_activelist_clearOnread > /proc/sys/bnl/$profile_id/activelist_clearOnread
	echo $val_activelist_print_only_nonzero > /proc/sys/bnl/$profile_id/activelist_print_only_nonzero

	exit 0
}

trap cleanup_func SIGINT

append_timestamp() {
	hdr_lines=0;

	# get current timestamp
	curr_ts=$(date "+%Y-%m-%d %H:%M:%S")

	while IFS= read -r line; do
		if [[ $hdr_lines -lt 2 ]] && $print_header_only_once; then
			if $header_not_printed; then
				echo "$line"
			fi
			hdr_lines=$((hdr_lines+1))
		elif [[ ! -z "$line" ]]; then
			echo "$line $curr_ts"
		fi
	done < /proc/bnl/$profile_id/activelist
}

init_func
header_not_printed=true
append_timestamp
header_not_printed=false

# keep reading atcivelist every second till ctrl+c
while true; do
	sleep 1
	append_timestamp
done
