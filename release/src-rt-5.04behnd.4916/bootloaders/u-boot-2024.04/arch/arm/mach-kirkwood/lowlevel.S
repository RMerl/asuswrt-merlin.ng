/* SPDX-License-Identifier: GPL-2.0+ */

#include <config.h>
#include <linux/linkage.h>

ENTRY(arch_very_early_init)
	/* Move internal registers from KW_OFFSET_REG to KW_REGS_PHY_BASE */
	ldr	r0, =KW_REGS_PHY_BASE
	ldr	r1, =KW_OFFSET_REG
	str	r0, [r1]
	bx	lr
ENDPROC(arch_very_early_init)
