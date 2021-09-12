#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
# Copyright (C) 2017-2018 Peter Wu <peter@lekensteyn.nl>. All Rights Reserved.

set -e

ME_DIR="${BASH_SOURCE[0]}"
ME_DIR="${ME_DIR%/*}"
source "$ME_DIR/offsets.include" || { echo "Did you forget to run make?" >&2; exit 1; }

case "$(uname -m)" in
	x86_64) ARGUMENT_REGISTER="%si" ;;
	i386|i686) ARGUMENT_REGISTER="%dx" ;;
	aarch64) ARGUMENT_REGISTER="%x1" ;;
	arm) ARGUMENT_REGISTER="%r1" ;;
	*) echo "ERROR: Unknown architecture" >&2; exit 1 ;;
esac

ARGS=( )
REGEX=".*: idxadd: .*"
for key in "${!OFFSETS[@]}"; do
	values="${OFFSETS[$key]}"
	values=( ${values//,/ } )
	for i in {0..3}; do
		value="$ARGUMENT_REGISTER"
		for indirection in "${values[@]:1}"; do
			value="+$indirection($value)"
		done
		value="+$((i * 8 + values[0]))($value)"
		ARGS+=( "${key,,}$i=$value:x64" )
		REGEX="$REGEX ${key,,}$i=0x([0-9a-f]+)"
	done
done

turn_off() {
	set +e
	[[ -f /sys/kernel/debug/tracing/events/wireguard/idxadd/enable ]] || exit
	echo 0 > /sys/kernel/debug/tracing/events/wireguard/idxadd/enable
	echo "-:wireguard/idxadd" >> /sys/kernel/debug/tracing/kprobe_events
	exit
}

trap turn_off INT TERM EXIT
echo "p:wireguard/idxadd index_hashtable_insert ${ARGS[*]}" >> /sys/kernel/debug/tracing/kprobe_events
echo 1 > /sys/kernel/debug/tracing/events/wireguard/idxadd/enable

unpack_u64() {
	local i expanded="$1"
	if [[ $ENDIAN == big ]]; then
		printf -v expanded "%.*s$expanded" $((16 - ${#expanded})) 0000000000000000
		for i in {0..7}; do
			echo -n "\\x${expanded:(i * 2):2}"
		done
	elif [[ $ENDIAN == little ]]; then
		(( ${#expanded} % 2 == 1 )) && expanded="0$expanded"
		expanded="${expanded}0000000000000000"
		for i in {0..7}; do
			echo -n "\\x${expanded:((7 - i) * 2):2}"
		done
	else
		echo "ERROR: Unable to determine endian" >&2
		exit 1
	fi
}

while read -r line; do
	[[ $line =~ $REGEX ]] || continue
	echo "New handshake session:"
	j=1
	for key in "${!OFFSETS[@]}"; do
		bytes=""
		for i in {0..3}; do
			bytes="$bytes$(unpack_u64 "${BASH_REMATCH[j]}")"
			((++j))
		done
		echo "  $key = $(printf "$bytes" | base64)"
	done
done < /sys/kernel/debug/tracing/trace_pipe
