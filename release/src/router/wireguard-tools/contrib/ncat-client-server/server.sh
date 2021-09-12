#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

if [[ -z $NCAT_REMOTE_ADDR ]]; then
	ip link del dev wg0 2>/dev/null
	set -e
	ip link add dev wg0 type wireguard
	ip address add 192.168.4.1/24 dev wg0
	wg set wg0 private-key <(wg genkey) listen-port 12912
	ip link set up dev wg0
	exec ncat -e "$(readlink -f "$0")" -k -l -p 42912 -v
fi
read -r public_key
[[ $(wg show wg0 peers | wc -l) -ge 253 ]] && wg set wg0 peer $(wg show wg0 latest-handshakes | sort -k 2 -b -n | head -n 1 | cut -f 1) remove
next_ip=$(all="$(wg show wg0 allowed-ips)"; for ((i=2; i<=254; i++)); do ip="192.168.4.$i"; [[ $all != *$ip/32* ]] && echo $ip && break; done)
wg set wg0 peer "$public_key" allowed-ips $next_ip/32 2>/dev/null && echo "OK:$(wg show wg0 private-key | wg pubkey):$(wg show wg0 listen-port):$next_ip" || echo ERROR
