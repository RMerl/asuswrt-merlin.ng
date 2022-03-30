/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 * Based on Atheros LSDK/QSDK
 */

#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <mach/ar71xx_regs.h>

#define MK_PLL_CONF(divint, refdiv, range, outdiv) \
     (((0x3F & divint) << 10) | \
     ((0x1F & refdiv) << 16) | \
     ((0x1 & range)   << 21) | \
     ((0x7 & outdiv)  << 23) )

#define MK_CLK_CNTL(cpudiv, ddrdiv, ahbdiv) \
    (((0x3 & (cpudiv - 1)) << 5)  | \
    ((0x3 & (ddrdiv - 1)) << 10) | \
    ((0x3 & (ahbdiv - 1)) << 15) )

#define SET_FIELD(name, v)      (((v) & QCA953X_##name##_MASK) << \
				 QCA953X_##name##_SHIFT)

#define DPLL2_KI(v)             SET_FIELD(SRIF_DPLL2_KI, v)
#define DPLL2_KD(v)             SET_FIELD(SRIF_DPLL2_KD, v)
#define DPLL2_PWD               QCA953X_SRIF_DPLL2_PWD
#define MK_DPLL2(ki, kd)        (DPLL2_KI(ki) | DPLL2_KD(kd) | DPLL2_PWD)

#define PLL_CPU_NFRAC(v)        SET_FIELD(PLL_CPU_CONFIG_NFRAC, v)
#define PLL_CPU_NINT(v)         SET_FIELD(PLL_CPU_CONFIG_NINT, v)
#define PLL_CPU_REFDIV(v)       SET_FIELD(PLL_CPU_CONFIG_REFDIV, v)
#define PLL_CPU_OUTDIV(v)       SET_FIELD(PLL_CPU_CONFIG_OUTDIV, v)
#define MK_PLL_CPU_CONF(frac, nint, ref, outdiv) \
				(PLL_CPU_NFRAC(frac) | \
				 PLL_CPU_NINT(nint) | \
				 PLL_CPU_REFDIV(ref) | \
				 PLL_CPU_OUTDIV(outdiv))

#define PLL_DDR_NFRAC(v)        SET_FIELD(PLL_DDR_CONFIG_NFRAC, v)
#define PLL_DDR_NINT(v)         SET_FIELD(PLL_DDR_CONFIG_NINT, v)
#define PLL_DDR_REFDIV(v)       SET_FIELD(PLL_DDR_CONFIG_REFDIV, v)
#define PLL_DDR_OUTDIV(v)       SET_FIELD(PLL_DDR_CONFIG_OUTDIV, v)
#define MK_PLL_DDR_CONF(frac, nint, ref, outdiv) \
				(PLL_DDR_NFRAC(frac) | \
				 PLL_DDR_REFDIV(ref) | \
				 PLL_DDR_NINT(nint) | \
				 PLL_DDR_OUTDIV(outdiv) | \
				 QCA953X_PLL_CONFIG_PWD)

#define PLL_CPU_CONF_VAL        MK_PLL_CPU_CONF(0, 26, 1, 0)
#define PLL_DDR_CONF_VAL        MK_PLL_DDR_CONF(0, 15, 1, 0)

#define PLL_CLK_CTRL_PLL_BYPASS (QCA953X_PLL_CLK_CTRL_CPU_PLL_BYPASS | \
				 QCA953X_PLL_CLK_CTRL_DDR_PLL_BYPASS | \
				 QCA953X_PLL_CLK_CTRL_AHB_PLL_BYPASS)

#define PLL_CLK_CTRL_CPU_DIV(v) SET_FIELD(PLL_CLK_CTRL_CPU_POST_DIV, v)
#define PLL_CLK_CTRL_DDR_DIV(v) SET_FIELD(PLL_CLK_CTRL_DDR_POST_DIV, v)
#define PLL_CLK_CTRL_AHB_DIV(v) SET_FIELD(PLL_CLK_CTRL_AHB_POST_DIV, v)
#define MK_PLL_CLK_CTRL(cpu, ddr, ahb) \
				(PLL_CLK_CTRL_CPU_DIV(cpu) | \
				 PLL_CLK_CTRL_DDR_DIV(ddr) | \
				 PLL_CLK_CTRL_AHB_DIV(ahb))
#define PLL_CLK_CTRL_VAL    (MK_PLL_CLK_CTRL(0, 0, 2) | \
			     PLL_CLK_CTRL_PLL_BYPASS | \
			     QCA953X_PLL_CLK_CTRL_CPUCLK_FROM_CPUPLL | \
			     QCA953X_PLL_CLK_CTRL_DDRCLK_FROM_DDRPLL)

#define PLL_DDR_DIT_FRAC_MAX(v)     SET_FIELD(PLL_DDR_DIT_FRAC_MAX, v)
#define PLL_DDR_DIT_FRAC_MIN(v)     SET_FIELD(PLL_DDR_DIT_FRAC_MIN, v)
#define PLL_DDR_DIT_FRAC_STEP(v)    SET_FIELD(PLL_DDR_DIT_FRAC_STEP, v)
#define PLL_DDR_DIT_UPD_CNT(v)      SET_FIELD(PLL_DDR_DIT_UPD_CNT, v)
#define PLL_CPU_DIT_FRAC_MAX(v)     SET_FIELD(PLL_CPU_DIT_FRAC_MAX, v)
#define PLL_CPU_DIT_FRAC_MIN(v)     SET_FIELD(PLL_CPU_DIT_FRAC_MIN, v)
#define PLL_CPU_DIT_FRAC_STEP(v)    SET_FIELD(PLL_CPU_DIT_FRAC_STEP, v)
#define PLL_CPU_DIT_UPD_CNT(v)      SET_FIELD(PLL_CPU_DIT_UPD_CNT, v)
#define MK_PLL_DDR_DIT_FRAC(max, min, step, cnt) \
				(QCA953X_PLL_DIT_FRAC_EN | \
				 PLL_DDR_DIT_FRAC_MAX(max) | \
				 PLL_DDR_DIT_FRAC_MIN(min) | \
				 PLL_DDR_DIT_FRAC_STEP(step) | \
				 PLL_DDR_DIT_UPD_CNT(cnt))
#define MK_PLL_CPU_DIT_FRAC(max, min, step, cnt) \
				(QCA953X_PLL_DIT_FRAC_EN | \
				 PLL_CPU_DIT_FRAC_MAX(max) | \
				 PLL_CPU_DIT_FRAC_MIN(min) | \
				 PLL_CPU_DIT_FRAC_STEP(step) | \
				 PLL_CPU_DIT_UPD_CNT(cnt))
#define PLL_CPU_DIT_FRAC_VAL    MK_PLL_CPU_DIT_FRAC(63, 0, 1, 15)
#define PLL_DDR_DIT_FRAC_VAL    MK_PLL_DDR_DIT_FRAC(763, 635, 1, 15)

    .text
    .set noreorder

LEAF(lowlevel_init)
	/* RTC Reset */
	li      t0, CKSEG1ADDR(AR71XX_RESET_BASE)
	lw      t1, QCA953X_RESET_REG_RESET_MODULE(t0)
	li      t2, 0x08000000
	or      t1, t1, t2
	sw      t1, QCA953X_RESET_REG_RESET_MODULE(t0)
	nop
	lw      t1, QCA953X_RESET_REG_RESET_MODULE(t0)
	li      t2, 0xf7ffffff
	and     t1, t1, t2
	sw      t1, QCA953X_RESET_REG_RESET_MODULE(t0)
	nop

	/* RTC Force Wake */
	li      t0, CKSEG1ADDR(QCA953X_RTC_BASE)
	li      t1, 0x01
	sw      t1, QCA953X_RTC_REG_SYNC_RESET(t0)
	nop
	nop

	/* Wait for RTC in on state */
1:
	lw      t1, QCA953X_RTC_REG_SYNC_STATUS(t0)
	andi    t1, t1, 0x02
	beqz    t1, 1b
	nop

	li      t0, CKSEG1ADDR(QCA953X_SRIF_BASE)
	li      t1, MK_DPLL2(2, 16)
	sw      t1, QCA953X_SRIF_BB_DPLL2_REG(t0)
	sw      t1, QCA953X_SRIF_PCIE_DPLL2_REG(t0)
	sw      t1, QCA953X_SRIF_DDR_DPLL2_REG(t0)
	sw      t1, QCA953X_SRIF_CPU_DPLL2_REG(t0)

	li      t0, CKSEG1ADDR(AR71XX_PLL_BASE)
	lw      t1, QCA953X_PLL_CLK_CTRL_REG(t0)
	ori     t1, PLL_CLK_CTRL_PLL_BYPASS
	sw      t1, QCA953X_PLL_CLK_CTRL_REG(t0)
	nop

	li      t1, PLL_CPU_CONF_VAL
	sw      t1, QCA953X_PLL_CPU_CONFIG_REG(t0)
	nop

	li      t1, PLL_DDR_CONF_VAL
	sw      t1, QCA953X_PLL_DDR_CONFIG_REG(t0)
	nop

	li      t1, PLL_CLK_CTRL_VAL
	sw      t1, QCA953X_PLL_CLK_CTRL_REG(t0)
	nop

	lw      t1, QCA953X_PLL_CPU_CONFIG_REG(t0)
	li      t2, ~QCA953X_PLL_CONFIG_PWD
	and     t1, t1, t2
	sw      t1, QCA953X_PLL_CPU_CONFIG_REG(t0)
	nop

	lw      t1, QCA953X_PLL_DDR_CONFIG_REG(t0)
	li      t2, ~QCA953X_PLL_CONFIG_PWD
	and     t1, t1, t2
	sw      t1, QCA953X_PLL_DDR_CONFIG_REG(t0)
	nop

	lw      t1, QCA953X_PLL_CLK_CTRL_REG(t0)
	li      t2, ~PLL_CLK_CTRL_PLL_BYPASS
	and     t1, t1, t2
	sw      t1, QCA953X_PLL_CLK_CTRL_REG(t0)
	nop

	li      t1, PLL_DDR_DIT_FRAC_VAL
	sw      t1, QCA953X_PLL_DDR_DIT_FRAC_REG(t0)
	nop

	li      t1, PLL_CPU_DIT_FRAC_VAL
	sw      t1, QCA953X_PLL_CPU_DIT_FRAC_REG(t0)
	nop

	li      t0, CKSEG1ADDR(AR71XX_RESET_BASE)
	lui     t1, 0x03fc
	sw      t1, 0xb4(t0)

	nop
	jr ra
	 nop
    END(lowlevel_init)
