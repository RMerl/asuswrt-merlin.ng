/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 * Based on Atheros LSDK/QSDK and u-boot_mod project
 */

#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <mach/ar71xx_regs.h>

#define SET_BIT(val, bit)   ((val) | (1 << (bit)))
#define SET_PLL_PD(val)     SET_BIT(val, 30)
#define AHB_DIV_TO_4(val)   SET_BIT(SET_BIT(val, 15), 16)
#define PLL_BYPASS(val)     SET_BIT(val, 2)

#define MK_PLL_CONF(divint, refdiv, range, outdiv) \
     (((0x3F & divint) << 10) | \
     ((0x1F & refdiv) << 16) | \
     ((0x1 & range)   << 21) | \
     ((0x7 & outdiv)  << 23) )

#define MK_CLK_CNTL(cpudiv, ddrdiv, ahbdiv) \
    (((0x3 & (cpudiv - 1)) << 5)  | \
    ((0x3 & (ddrdiv - 1)) << 10) | \
    ((0x3 & (ahbdiv - 1)) << 15) )

/*
 * PLL_CPU_CONFIG_VAL
 *
 * Bit30 is set (CPU_PLLPWD = 1 -> power down control for CPU PLL)
 * After PLL configuration we need to clear this bit
 *
 * Values written into CPU PLL Configuration (CPU_PLL_CONFIG)
 *
 * bits 10..15  (6bit)  DIV_INT (Integer part of the DIV to CPU PLL)
 *                      =>  32  (0x20)  VCOOUT = XTAL * DIV_INT
 * bits 16..20  (5bit)  REFDIV  (Reference clock divider)
 *                      =>  1   (0x1)   [Must start at values 1]
 * bits 21      (1bit)  RANGE   (VCO frequency range of the CPU PLL)
 *                      =>  0   (0x0)   [Doesn't impact clock values]
 * bits 23..25  (3bit)  OUTDIV  (Ratio between VCO and PLL output)
 *                      =>  1   (0x1)   [0 is illegal!]
 *                              PLLOUT = VCOOUT * (1/2^OUTDIV)
 */
/* DIV_INT=32 (25MHz*32/2=400MHz), REFDIV=1, RANGE=0, OUTDIV=1 */
#define PLL_CPU_CONFIG_VAL_40M  MK_PLL_CONF(20, 1, 0, 1)
/* DIV_INT=20 (40MHz*20/2=400MHz), REFDIV=1, RANGE=0, OUTDIV=1 */
#define PLL_CPU_CONFIG_VAL_25M  MK_PLL_CONF(32, 1, 0, 1)

/*
 * PLL_CLK_CONTROL_VAL
 *
 * In PLL_CLK_CONTROL_VAL bit 2 is set (BYPASS = 1 -> bypass PLL)
 * After PLL configuration we need to clear this bit
 *
 * Values written into CPU Clock Control Register CLOCK_CONTROL
 *
 * bits 2       (1bit)  BYPASS (Bypass PLL. This defaults to 1 for test.
 *                      Software must enable the CPU PLL for normal and
 *                      then set this bit to 0)
 * bits 5..6    (2bit)  CPU_POST_DIV    =>  0   (DEFAULT, Ratio = 1)
 *                      CPU_CLK = PLLOUT / CPU_POST_DIV
 * bits 10..11  (2bit)  DDR_POST_DIV    =>  0   (DEFAULT, Ratio = 1)
 *                      DDR_CLK = PLLOUT / DDR_POST_DIV
 * bits 15..16  (2bit)  AHB_POST_DIV    =>  1   (DEFAULT, Ratio = 2)
 *                      AHB_CLK = PLLOUT / AHB_POST_DIV
 *
 */
#define PLL_CLK_CONTROL_VAL MK_CLK_CNTL(1, 1, 2)

    .text
    .set noreorder

LEAF(lowlevel_init)
	/* These three WLAN_RESET will avoid original issue */
	li      t3, 0x03
1:
	li      t0, CKSEG1ADDR(AR71XX_RESET_BASE)
	lw      t1, AR933X_RESET_REG_RESET_MODULE(t0)
	ori     t1, t1, 0x0800
	sw      t1, AR933X_RESET_REG_RESET_MODULE(t0)
	nop
	lw      t1, AR933X_RESET_REG_RESET_MODULE(t0)
	li      t2, 0xfffff7ff
	and     t1, t1, t2
	sw      t1, AR933X_RESET_REG_RESET_MODULE(t0)
	nop
	addi    t3, t3, -1
	bnez    t3, 1b
	nop

	li      t2, 0x20
2:
	beqz    t2, 1b
	nop
	addi    t2, t2, -1
	lw      t5, AR933X_RESET_REG_BOOTSTRAP(t0)
	andi    t1, t5, 0x10
	bnez    t1, 2b
	nop

	li      t1, 0x02110E
	sw      t1, AR933X_RESET_REG_BOOTSTRAP(t0)
	nop

	/* RTC Force Wake */
	li      t0, CKSEG1ADDR(AR933X_RTC_BASE)
	li      t1, 0x03
	sw      t1, AR933X_RTC_REG_FORCE_WAKE(t0)
	nop
	nop

	/* RTC Reset */
	li      t1, 0x00
	sw      t1, AR933X_RTC_REG_RESET(t0)
	nop
	nop

	li      t1, 0x01
	sw      t1, AR933X_RTC_REG_RESET(t0)
	nop
	nop

	/* Wait for RTC in on state */
1:
	lw      t1, AR933X_RTC_REG_STATUS(t0)
	andi    t1, t1, 0x02
	beqz    t1, 1b
	nop

	/* Program ki/kd */
	li      t0, CKSEG1ADDR(AR933X_SRIF_BASE)
	andi    t1, t5, 0x01            # t5 BOOT_STRAP
	bnez    t1, 1f
	nop
	li      t1, 0x19e82f01
	b       2f
	nop
1:
	li      t1, 0x18e82f01
2:
	sw      t1, AR933X_SRIF_DDR_DPLL2_REG(t0)

	/* Program phase shift */
	lw      t1, AR933X_SRIF_DDR_DPLL3_REG(t0)
	li      t2, 0xc07fffff
	and     t1, t1, t2
	li      t2, 0x800000
	or      t1, t1, t2
	sw      t1, AR933X_SRIF_DDR_DPLL3_REG(t0)
	nop

	/* in some cases, the SoC doesn't start with higher clock on AHB */
	li      t0, CKSEG1ADDR(AR71XX_PLL_BASE)
	li      t1, AHB_DIV_TO_4(PLL_BYPASS(PLL_CLK_CONTROL_VAL))
	sw      t1, AR933X_PLL_CLK_CTRL_REG(t0)
	nop

	/* Set SETTLE_TIME in CPU PLL */
	andi    t1, t5, 0x01            # t5 BOOT_STRAP
	bnez    t1, 1f
	nop
	li      t1, 0x0352
	b       2f
	nop
1:
	li      t1, 0x0550
2:
	sw      t1, AR71XX_PLL_REG_SEC_CONFIG(t0)
	nop

	/* Set nint, frac, refdiv, outdiv, range according to xtal */
0:
	andi    t1, t5, 0x01            # t5 BOOT_STRAP
	bnez    t1, 1f
	nop
	li      t1, SET_PLL_PD(PLL_CPU_CONFIG_VAL_25M)
	b       2f
	nop
1:
	li      t1, SET_PLL_PD(PLL_CPU_CONFIG_VAL_40M)
2:
	sw      t1, AR933X_PLL_CPU_CONFIG_REG(t0)
	nop
1:
	lw      t1, AR933X_PLL_CPU_CONFIG_REG(t0)
	li      t2, 0x80000000
	and     t1, t1, t2
	bnez    t1, 1b
	nop

	/* Put frac bit19:10 configuration */
	li      t1, 0x1003E8
	sw      t1, AR933X_PLL_DITHER_FRAC_REG(t0)
	nop

	/* Clear PLL power down bit in CPU PLL configuration */
	andi    t1, t5, 0x01            # t5 BOOT_STRAP
	bnez    t1, 1f
	nop
	li      t1, PLL_CPU_CONFIG_VAL_25M
	b       2f
	nop
1:
	li      t1, PLL_CPU_CONFIG_VAL_40M
2:
	sw      t1, AR933X_PLL_CPU_CONFIG_REG(t0)
	nop

	/* Wait for PLL update -> bit 31 in CPU_PLL_CONFIG should be 0 */
1:
	lw      t1, AR933X_PLL_CPU_CONFIG_REG(t0)
	li      t2, 0x80000000
	and     t1, t1, t2
	bnez    t1, 1b
	nop

	/* Confirm DDR PLL lock */
	li      t3, 100
	li      t4, 0

2:
	addi    t4, t4, 1
	bgt     t4, t3, 0b
	nop

	li      t3, 5
3:
	/* Clear do_meas */
	li      t0, CKSEG1ADDR(AR933X_SRIF_BASE)
	lw      t1, AR933X_SRIF_DDR_DPLL3_REG(t0)
	li      t2, 0xBFFFFFFF
	and     t1, t1, t2
	sw      t1, AR933X_SRIF_DDR_DPLL3_REG(t0)
	nop

	li      t2, 10
1:
	subu    t2, t2, 1
	bnez    t2, 1b
	nop

	/* Set do_meas */
	li      t2, 0x40000000
	or      t1, t1, t2
	sw      t1, AR933X_SRIF_DDR_DPLL3_REG(t0)
	nop

	/* Check meas_done */
1:
	lw      t1, AR933X_SRIF_DDR_DPLL4_REG(t0)
	andi    t1, t1, 0x8
	beqz    t1, 1b
	nop

	lw      t1, AR933X_SRIF_DDR_DPLL3_REG(t0)
	li      t2, 0x007FFFF8
	and     t1, t1, t2
	srl     t1, t1, 3
	li      t2, 0x4000
	bgt     t1, t2, 2b
	nop
	addi    t3, t3, -1
	bnez    t3, 3b
	nop

	/* clear PLL bypass (bit 2) in CPU CLOCK CONTROL register */
	li      t0, CKSEG1ADDR(AR71XX_PLL_BASE)
	li      t1, PLL_CLK_CONTROL_VAL
	sw      t1, AR933X_PLL_CLK_CTRL_REG(t0)
	nop

	nop
	jr ra
	nop
    END(lowlevel_init)
