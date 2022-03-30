/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#include <asm/arch-ag101/ag101.h>
#include <linux/linkage.h>

.text

#ifndef CONFIG_SKIP_TRUNOFF_WATCHDOG
ENTRY(turnoff_watchdog)

#define WD_CR		0xC
#define WD_ENABLE	0x1

	! Turn off the watchdog, according to Faraday FTWDT010 spec
	li 	$p0, (CONFIG_FTWDT010_BASE+WD_CR)	! Get the addr of WD CR
	lwi	$p1, [$p0]				! Get the config of WD
	andi	$p1, $p1, 0x1f				! Wipe out useless bits
	li	$r0, ~WD_ENABLE
	and	$p1, $p1, $r0				! Set WD disable
	sw	$p1, [$p0]				! Write back to WD CR

	! Disable Interrupts by clear GIE in $PSW reg
	setgie.d

	ret

ENDPROC(turnoff_watchdog)
#endif
