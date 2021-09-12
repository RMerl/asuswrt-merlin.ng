#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

exec < <(exec wg show all dump)

printf '{'
while read -r -d $'\t' device; do
	if [[ $device != "$last_device" ]]; then
		[[ -z $last_device ]] && printf '\n' || printf '%s,\n' "$end"
		last_device="$device"
		read -r private_key public_key listen_port fwmark
		printf '\t"%s": {' "$device"
		delim=$'\n'
		[[ $private_key == "(none)" ]] || { printf '%s\t\t"privateKey": "%s"' "$delim" "$private_key"; delim=$',\n'; }
		[[ $public_key == "(none)" ]] || { printf '%s\t\t"publicKey": "%s"' "$delim" "$public_key"; delim=$',\n'; }
		[[ $listen_port == "0" ]] || { printf '%s\t\t"listenPort": %u' "$delim" $(( $listen_port )); delim=$',\n'; }
		[[ $fwmark == "off" ]] || { printf '%s\t\t"fwmark": %u' "$delim" $(( $fwmark )); delim=$',\n'; }
		printf '%s\t\t"peers": {' "$delim"; end=$'\n\t\t}\n\t}'
		delim=$'\n'
	else
		read -r public_key preshared_key endpoint allowed_ips latest_handshake transfer_rx transfer_tx persistent_keepalive
		printf '%s\t\t\t"%s": {' "$delim" "$public_key"
		delim=$'\n'
		[[ $preshared_key == "(none)" ]] || { printf '%s\t\t\t\t"presharedKey": "%s"' "$delim" "$preshared_key"; delim=$',\n'; }
		[[ $endpoint == "(none)" ]] || { printf '%s\t\t\t\t"endpoint": "%s"' "$delim" "$endpoint"; delim=$',\n'; }
		[[ $latest_handshake == "0" ]] || { printf '%s\t\t\t\t"latestHandshake": %u' "$delim" $(( $latest_handshake )); delim=$',\n'; }
		[[ $transfer_rx == "0" ]] || { printf '%s\t\t\t\t"transferRx": %u' "$delim" $(( $transfer_rx )); delim=$',\n'; }
		[[ $transfer_tx == "0" ]] || { printf '%s\t\t\t\t"transferTx": %u' "$delim" $(( $transfer_tx )); delim=$',\n'; }
		[[ $persistent_keepalive == "off" ]] || { printf '%s\t\t\t\t"persistentKeepalive": %u' "$delim" $(( $persistent_keepalive )); delim=$',\n'; }
		printf '%s\t\t\t\t"allowedIps": [' "$delim"
		delim=$'\n'
		if [[ $allowed_ips != "(none)" ]]; then
			old_ifs="$IFS"
			IFS=,
			for ip in $allowed_ips; do
				printf '%s\t\t\t\t\t"%s"' "$delim" "$ip"
				delim=$',\n'
			done
			IFS="$old_ifs"
			delim=$'\n'
		fi
		printf '%s\t\t\t\t]' "$delim"
		printf '\n\t\t\t}'
		delim=$',\n'
	fi


done
printf '%s\n' "$end"
printf '}\n'
