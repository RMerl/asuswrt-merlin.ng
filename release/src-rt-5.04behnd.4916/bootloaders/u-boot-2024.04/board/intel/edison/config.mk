# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
# Copyright (c) 2017 Intel Corporation
#

# Add 4096 bytes of zeroes to u-boot.bin
quiet_cmd_mkalign_eds = EDSALGN $@
cmd_mkalign_eds =							\
	dd if=$^ of=$@ bs=4k seek=1 2>/dev/null &&			\
	mv $@ $^

INPUTS-y += u-boot-align.bin
u-boot-align.bin: u-boot.bin
	$(call if_changed,mkalign_eds)

HOSTCFLAGS_autoconf.mk.dep = -Wno-variadic-macros
