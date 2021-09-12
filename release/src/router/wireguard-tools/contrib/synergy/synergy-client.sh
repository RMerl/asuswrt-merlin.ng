#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

set -ex
if [[ $UID == 0 ]]; then
	ip link del dev synergy || true
	ip link add dev synergy type wireguard
	ip address add 10.193.125.39/32 peer 10.193.125.38/32 dev synergy
	wg set synergy \
		listen-port 29184 \
		private-key <(echo oNcsXA5Ma56q9xHmvvKuzLfwXYy7Uqy+bTmmXg/XtVs=) \
		peer m321UMZXoJ6qw8Jli2spbAVBc2MdOzV/EHDKfZQy0g0= \
			allowed-ips 10.193.125.38/32 \
			endpoint 10.10.10.100:29184
	ip link set up dev synergy
else
	sudo "$(readlink -f "$0")"
	killall synergyc || true
	synergyc 10.193.125.38:38382
fi
