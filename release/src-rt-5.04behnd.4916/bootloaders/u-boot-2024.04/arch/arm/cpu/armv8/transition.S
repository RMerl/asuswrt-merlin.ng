/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

.pushsection .text.armv8_switch_to_el2, "ax"
ENTRY(armv8_switch_to_el2)
	switch_el x6, 1f, 0f, 0f
0:
	cmp x5, #ES_TO_AARCH64
	b.eq 2f
	/*
	 * When loading 32-bit kernel, it will jump
	 * to secure firmware again, and never return.
	 */
	bl armv8_el2_to_aarch32
2:
	/*
	 * x4 is kernel entry point or switch_to_el1
	 * if CONFIG_ARMV8_SWITCH_TO_EL1 is defined.
         * When running in EL2 now, jump to the
	 * address saved in x4.
	 */
	br x4
1:	armv8_switch_to_el2_m x4, x5, x6
ENDPROC(armv8_switch_to_el2)
.popsection

.pushsection .text.armv8_switch_to_el1, "ax"
ENTRY(armv8_switch_to_el1)
	switch_el x6, 0f, 1f, 0f
0:
	/* x4 is kernel entry point. When running in EL1
	 * now, jump to the address saved in x4.
	 */
	br x4
1:	armv8_switch_to_el1_m x4, x5, x6, x7
ENDPROC(armv8_switch_to_el1)
.popsection

.pushsection .text.armv8_el2_to_aarch32, "ax"
WEAK(armv8_el2_to_aarch32)
	ret
ENDPROC(armv8_el2_to_aarch32)
.popsection
