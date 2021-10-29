#!/bin/sh
#
# Copyright (C) 2020, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# <<Broadcom-WL-IPTag/Proprietary,Open:.*>>
#
# debug_loop_acs_dfs.sh <WiFi interface name> <Band: 2|5|6> <Driver mode: nic|dhd>

if [ $# -lt 3 ]; then
	echo -e "Usage:\n\t" $0 "<WiFi interface name> <Band: 2|5|6> <Driver mode: nic|dhd>"
	echo -e "Eg.:\n\t" $0 wl0 5 dhd
	exit
fi
set -x
WlGetDriverCfg.sh $1 $2 $3
echo 7 > /proc/sys/kernel/printk
wl -i $1 msglevel +dfs +regulatory
wl -i $1 phymsglevel +radar
wl -i $1 spect
wl -i $1 radarthrs2
wl -i $1 radarargs
wl -i $1 radar
wl -i $1 clear_radar_status

if [ $3 == "dhd" ]; then
	#dhd -i $1 msglevel +s_iov
	dhd -i $1 dconpoll 250
fi

while true; do
	wl -i $1 chanspec
	wl -i $1 chanim_stats
	wl -i $1 chan_info
	wl -i $1 dfs_status_all
	wl -i $1 dfs_ap_move
	wl -i $1 radar_status
	wl -i $1 radar_sc_status
	wl -i $1 counters
	wl -i $1 bs_data -noreset

	acs_cli2 -i $1 -R

	if [ $3 == "dhd" ]; then
		dhd -i $1 cons mu
		dhd -i $1 dump
	fi

	dmesg -c
	free

	sleep 5
done
