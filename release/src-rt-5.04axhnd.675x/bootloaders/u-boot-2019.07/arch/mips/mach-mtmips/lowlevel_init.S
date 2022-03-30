/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2018 Stefan Roese <sr@denx.de>
 *
 * This code is mostly based on the code extracted from this MediaTek
 * github repository:
 *
 * https://github.com/MediaTek-Labs/linkit-smart-uboot.git
 *
 * I was not able to find a specific license or other developers
 * copyrights here, so I can't add them here.
 */

#include <config.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/asm.h>
#include "mt76xx.h"

#ifndef BIT
#define BIT(nr)			(1 << (nr))
#endif

#define DELAY_USEC(us)		((us) / 100)

#define DDR_CFG1_CHIP_WIDTH_MASK (0x3 << 16)
#define DDR_CFG1_BUS_WIDTH_MASK	(0x3 << 12)

#if defined(CONFIG_ONBOARD_DDR2_SIZE_256MBIT)
#define DDR_CFG1_SIZE_VAL	0x222e2323
#define DDR_CFG4_SIZE_VAL	7
#endif
#if defined(CONFIG_ONBOARD_DDR2_SIZE_512MBIT)
#define DDR_CFG1_SIZE_VAL	0x22322323
#define DDR_CFG4_SIZE_VAL	9
#endif
#if defined(CONFIG_ONBOARD_DDR2_SIZE_1024MBIT)
#define DDR_CFG1_SIZE_VAL	0x22362323
#define DDR_CFG4_SIZE_VAL	9
#endif
#if defined(CONFIG_ONBOARD_DDR2_SIZE_2048MBIT)
#define DDR_CFG1_SIZE_VAL	0x223a2323
#define DDR_CFG4_SIZE_VAL	9
#endif

#if defined(CONFIG_ONBOARD_DDR2_CHIP_WIDTH_8BIT)
#define DDR_CFG1_CHIP_WIDTH_VAL	(0x1 << 16)
#endif
#if defined(CONFIG_ONBOARD_DDR2_CHIP_WIDTH_16BIT)
#define DDR_CFG1_CHIP_WIDTH_VAL	(0x2 << 16)
#endif

#if defined(CONFIG_ONBOARD_DDR2_BUS_WIDTH_16BIT)
#define DDR_CFG1_BUS_WIDTH_VAL	(0x2 << 12)
#endif
#if defined(CONFIG_ONBOARD_DDR2_BUS_WIDTH_32BIT)
#define DDR_CFG1_BUS_WIDTH_VAL	(0x3 << 12)
#endif

	.set noreorder

LEAF(lowlevel_init)

	/* Load base addresses as physical addresses for later usage */
	li	s0, CKSEG1ADDR(MT76XX_SYSCTL_BASE)
	li	s1, CKSEG1ADDR(MT76XX_MEMCTRL_BASE)
	li	s2, CKSEG1ADDR(MT76XX_RGCTRL_BASE)

	/* polling CPLL is ready */
	li	t1, DELAY_USEC(1000000)
	la	t5, MT76XX_ROM_STATUS_REG
1:
	lw	t2, 0(t5)
	andi	t2, t2, 0x1
	bnez	t2, CPLL_READY
	subu	t1, t1, 1
	bgtz	t1, 1b
	nop
	la      t0, MT76XX_CLKCFG0_REG
	lw      t3, 0(t0)
	ori	t3, t3, 0x1
	sw	t3, 0(t0)
	b	CPLL_DONE
	nop
CPLL_READY:
	la	t0, MT76XX_CLKCFG0_REG
	lw	t1, 0(t0)
	li	t2, ~0x0c
	and	t1, t1, t2
	ori	t1, t1, 0xc
	sw	t1, 0(t0)
	la	t0, MT76XX_DYN_CFG0_REG
	lw	t3, 0(t0)
	li	t5, ~((0x0f << 8) | (0x0f << 0))
	and	t3, t3, t5
	li	t5, (10 << 8) | (1 << 0)
	or	t3, t3, t5
	sw	t3, 0(t0)
	la	t0, MT76XX_CLKCFG0_REG
	lw	t3, 0(t0)
	li	t4, ~0x0F
	and     t3, t3, t4
	ori	t3, t3, 0xc
	sw	t3, 0(t0)
	lw	t3, 0(t0)
	ori	t3, t3, 0x08
	sw	t3, 0(t0)

CPLL_DONE:
	/* Reset MC */
	lw	t2, 0x34(s0)
	ori	t2, BIT(10)
	sw	t2, 0x34(s0)
	nop

	/*
	 * SDR and DDR initialization: delay 200us
	 */
	li	t0, DELAY_USEC(200 + 40)
	li	t1, 0x1
1:
	sub	t0, t0, t1
	bnez	t0, 1b
	nop

	/* set DRAM IO PAD for MT7628IC */
	/* DDR LDO Enable  */
	lw	t4, 0x100(s2)
	li	t2, BIT(31)
	or	t4, t4, t2
	sw	t4, 0x100(s2)
	lw	t4, 0x10c(s2)
	j	LDO_1P8V
	nop
LDO_1P8V:
	li	t2, ~BIT(6)
	and	t4, t4, t2
	sw	t4, 0x10c(s2)
	j	DDRLDO_SOFT_START
LDO_2P5V:
	/* suppose external DDR1 LDO 2.5V */
	li	t2, BIT(6)
	or	t4, t4, t2
	sw	t4, 0x10c(s2)

DDRLDO_SOFT_START:
	lw	t2, 0x10c(s2)
	li	t3, BIT(16)
	or	t2, t2, t3
	sw	t2, 0x10c(s2)
	li	t3, DELAY_USEC(250*50)
LDO_DELAY:
	subu	t3, t3, 1
	bnez	t3, LDO_DELAY
	nop

	lw	t2, 0x10c(s2)
	li	t3, BIT(18)
	or	t2, t2, t3
	sw	t2, 0x10c(s2)

SET_RG_BUCK_FPWM:
	lw	t2, 0x104(s2)
	ori	t2, t2, BIT(10)
	sw	t2, 0x104(s2)

DDR_PAD_CFG:
	/* clean CLK PAD */
	lw	t2, 0x704(s2)
	li	t8, 0xfffff0f0
	and	t2, t2, t8
	/* clean CMD PAD */
	lw	t3, 0x70c(s2)
	li	t8, 0xfffff0f0
	and	t3, t3, t8
	/* clean DQ IPAD */
	lw	t4, 0x710(s2)
	li	t8, 0xfffff8ff
	and	t4, t4, t8
	/* clean DQ OPAD */
	lw	t5, 0x714(s2)
	li	t8, 0xfffff0f0
	and	t5, t5, t8
	/* clean DQS IPAD */
	lw	t6, 0x718(s2)
	li	t8, 0xfffff8ff
	and	t6, t6, t8
	/* clean DQS OPAD */
	lw	t7, 0x71c(s2)
	li	t8, 0xfffff0f0
	and	t7, t7, t8

	lw	t9, 0xc(s0)
	srl	t9, t9, 16
	andi	t9, t9, 0x1
	bnez	t9, MT7628_AN_DDR1_PAD
MT7628_KN_PAD:
	li	t8, 0x00000303
	or	t2, t2, t8
	or	t3, t3, t8
	or	t5, t5, t8
	or	t7, t7, t8
	li	t8, 0x00000000
	or	t4, t4, t8
	or	t6, t6, t8
	j	SET_PAD_CFG
MT7628_AN_DDR1_PAD:
	lw	t1, 0x10(s0)
	andi	t1, t1, 0x1
	beqz	t1, MT7628_AN_DDR2_PAD
	li	t8, 0x00000c0c
	or	t2, t2, t8
	li	t8, 0x00000202
	or	t3, t3, t8
	li	t8, 0x00000707
	or	t5, t5, t8
	li	t8, 0x00000c0c
	or	t7, t7, t8
	li	t8, 0x00000000
	or	t4, t4, t8
	or	t6, t6, t8
	j	SET_PAD_CFG
MT7628_AN_DDR2_PAD:
	li	t8, 0x00000c0c
	or	t2, t2, t8
	li	t8, 0x00000202
	or	t3, t3, t8
	li	t8, 0x00000404
	or	t5, t5, t8
	li	t8, 0x00000c0c
	or	t7, t7, t8
	li	t8, 0x00000000		/* ODT off */
	or	t4, t4, t8
	or	t6, t6, t8

SET_PAD_CFG:
	sw	t2, 0x704(s2)
	sw	t3, 0x70c(s2)
	sw	t4, 0x710(s2)
	sw	t5, 0x714(s2)
	sw	t6, 0x718(s2)
	sw	t7, 0x71c(s2)

	/*
	 * DDR initialization: reset pin to 0
	 */
	lw	t2, 0x34(s0)
	and	t2, ~BIT(10)
	sw	t2, 0x34(s0)
	nop

	/*
	 * DDR initialization: wait til reg DDR_CFG1 bit 21 equal to 1 (ready)
	 */
DDR_READY:
	li	t1, DDR_CFG1_REG
	lw	t0, 0(t1)
	nop
	and	t2, t0, BIT(21)
	beqz	t2, DDR_READY
	nop

	/*
	 * DDR initialization
	 *
	 * Only DDR2 supported right now. DDR2 support can be added, once
	 * boards using it will get added to mainline U-Boot.
	 */
	li	t1, DDR_CFG2_REG
	lw	t0, 0(t1)
	nop
	and	t0, ~BIT(30)
	and	t0, ~(7 << 4)
	or	t0, (4 << 4)
	or	t0, BIT(30)
	or	t0, BIT(11)
	sw	t0, 0(t1)
	nop

	li	t1, DDR_CFG3_REG
	lw	t2, 0(t1)
	/* Disable ODT; reference board ok, ev board fail */
	and	t2, ~BIT(6)
	or	t2, BIT(2)
	li	t0, DDR_CFG4_REG
	lw	t1, 0(t0)
	li	t2, ~(0x01f | 0x0f0)
	and	t1, t1, t2
	ori	t1, t1, DDR_CFG4_SIZE_VAL
	sw	t1, 0(t0)
	nop

	/*
	 * DDR initialization: config size and width on reg DDR_CFG1
	 */
	li	t6, DDR_CFG1_SIZE_VAL

	and	t6, ~DDR_CFG1_CHIP_WIDTH_MASK
	or	t6, DDR_CFG1_CHIP_WIDTH_VAL

	/* CONFIG DDR_CFG1[13:12] about TOTAL WIDTH */
	and	t6, ~DDR_CFG1_BUS_WIDTH_MASK
	or	t6, DDR_CFG1_BUS_WIDTH_VAL

	li	t5, DDR_CFG1_REG
	sw	t6, 0(t5)
	nop

	/*
	 * DDR: enable self auto refresh for power saving
	 * enable it by default for both RAM and ROM version (for CoC)
	 */
	lw	t1, 0x14(s1)
	nop
	and	t1, 0xff000000
	or	t1, 0x01
	sw	t1, 0x14(s1)
	nop
	lw	t1, 0x10(s1)
	nop
	or	t1, 0x10
	sw	t1, 0x10(s1)
	nop

	jr	ra
	nop
	END(lowlevel_init)
