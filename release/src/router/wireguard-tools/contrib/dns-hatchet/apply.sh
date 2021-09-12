#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

ME="$(readlink -f "$(dirname "$(readlink -f "$0")")")"
TOOLS="$ME/../../src"

sed -i "/~~ function override insertion point ~~/r $ME/hatchet.bash" "$TOOLS/wg-quick/linux.bash"
