/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <asm/asm.h>
#include <asm/regdef.h>

#define BASE_MACRO      0x600a0000
#define REG_OFFSET(t, o) (t + (o*4))
#define REG_MACRO(x) REG_OFFSET(BASE_MACRO, x)
#define BIT(nr)			(1 << (nr))

#define MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0 REG_MACRO(6)
#define MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_LOCK_STATUS BIT(0)
#define MACRO_CTRL_PLL5G_CFG_PLL5G_CFG2 REG_MACRO(2)
#define MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 REG_MACRO(0)
#define MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV (0x3F << 6)
#define MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV_ENC(x) (x << 6)

    .set noreorder
LEAF(pll_init)
	/* Make sure PLL is locked */
	lw	v0, MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0
	andi	v1, v0, MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_LOCK_STATUS
	bne	v1, zero, 1f
	 nop

	/* Black magic from frontend */
	li	v1, 0x00610400
	sw	v1, MACRO_CTRL_PLL5G_CFG_PLL5G_CFG2

	li	v1, 0x00610c00
	sw	v1, MACRO_CTRL_PLL5G_CFG_PLL5G_CFG2

	li	v1, 0x00610800
	sw	v1, MACRO_CTRL_PLL5G_CFG_PLL5G_CFG2

	li	v1, 0x00610000
	sw	v1, MACRO_CTRL_PLL5G_CFG_PLL5G_CFG2

	/* Wait for lock */
2:	lw	v0, MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0
	andi	v1, v0, MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_LOCK_STATUS
	/* Keep looping if zero (no lock bit yet) */
	beq	v1, zero, 2b
	 nop

	/* Setup PLL CPU clock divider for 416MHz */
1:	lw	v0, MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0

	/* Keep reserved bits */
	li	v1, ~MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV
	and	v0, v0, v1

	/* Set code 6 ~ 416.66 MHz */
	ori	v0, v0, MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV_ENC(6)

	sw	v0, MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0
	jr      ra
	 nop
	END(pll_init)
