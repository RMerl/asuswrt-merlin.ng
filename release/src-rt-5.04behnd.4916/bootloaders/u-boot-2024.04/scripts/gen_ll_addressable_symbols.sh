#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2020 Marek Behún <kabel@kernel.org>

# Generate __ADDRESSABLE(symbol) for every linker list entry symbol, so that LTO
# does not optimize these symbols away

# The expected parameter of this script is the command requested to have
# the U-Boot symbols to parse, for example: $(NM) $(u-boot-main)

set -e

echo '#include <common.h>'
$@ 2>/dev/null | grep -oe '_u_boot_list_2_[a-zA-Z0-9_]*_2_[a-zA-Z0-9_]*' | \
	sort -u | sed -e 's/^\(.*\)/extern char \1[];\n__ADDRESSABLE(\1);/'
