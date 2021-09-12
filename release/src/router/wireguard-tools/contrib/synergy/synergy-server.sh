#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

set -ex
if [[ $UID == 0 ]]; then
	ip link del dev synergy || true
	ip link add dev synergy type wireguard
	ip address add 10.193.125.38/32 peer 10.193.125.39/32 dev synergy
	wg set synergy \
		listen-port 29184 \
		private-key <(echo 2InSrlZA5eQfI/MvnvPieqNTBo9cd+udc3SOO9yFpXo=) \
		peer CBnoidQLjlbRsrqrI56WQbANWwkll41w/rVUIW9zISI= \
			allowed-ips 10.193.125.39/32
	ip link set up dev synergy
else
	sudo "$(readlink -f "$0")"
	killall synergys || true
	synergys -a 10.193.125.38:38382
fi
