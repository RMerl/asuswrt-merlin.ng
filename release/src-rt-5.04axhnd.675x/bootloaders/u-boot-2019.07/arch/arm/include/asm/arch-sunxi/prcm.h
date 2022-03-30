/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sunxi A31 Power Management Unit register definition.
 *
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 * http://linux-sunxi.org
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#ifndef _SUNXI_PRCM_H
#define _SUNXI_PRCM_H

#define __PRCM_CPUS_CFG_PRE(n) (((n) & 0x3) << 4)
#define PRCM_CPUS_CFG_PRE_MASK __PRCM_CPUS_CFG_PRE(0x3)
#define __PRCM_CPUS_CFG_PRE_DIV(n) (((n) >> 1) - 1)
#define PRCM_CPUS_CFG_PRE_DIV(n) \
	__PRCM_CPUS_CFG_PRE(__PRCM_CPUS_CFG_CLK_PRE(n))
#define __PRCM_CPUS_CFG_POST(n) (((n) & 0x1f) << 8)
#define PRCM_CPUS_CFG_POST_MASK __PRCM_CPUS_CFG_POST(0x1f)
#define __PRCM_CPUS_CFG_POST_DIV(n) ((n) - 1)
#define PRCM_CPUS_CFG_POST_DIV(n) \
	__PRCM_CPUS_CFG_POST_DIV(__PRCM_CPUS_CFG_POST_DIV(n))
#define __PRCM_CPUS_CFG_CLK_SRC(n) (((n) & 0x3) << 16)
#define PRCM_CPUS_CFG_CLK_SRC_MASK __PRCM_CPUS_CFG_CLK_SRC(0x3)
#define __PRCM_CPUS_CFG_CLK_SRC_LOSC 0x0
#define __PRCM_CPUS_CFG_CLK_SRC_HOSC 0x1
#define __PRCM_CPUS_CFG_CLK_SRC_PLL6 0x2
#define __PRCM_CPUS_CFG_CLK_SRC_PDIV 0x3
#define PRCM_CPUS_CFG_CLK_SRC_LOSC \
	__PRCM_CPUS_CFG_CLK_SRC(__PRCM_CPUS_CFG_CLK_SRC_LOSC)
#define PRCM_CPUS_CFG_CLK_SRC_HOSC \
	__PRCM_CPUS_CFG_CLK_SRC(__PRCM_CPUS_CFG_CLK_SRC_HOSC)
#define PRCM_CPUS_CFG_CLK_SRC_PLL6 \
	__PRCM_CPUS_CFG_CLK_SRC(__PRCM_CPUS_CFG_CLK_SRC_PLL6)
#define PRCM_CPUS_CFG_CLK_SRC_PDIV \
	__PRCM_CPUS_CFG_CLK_SRC(__PRCM_CPUS_CFG_CLK_SRC_PDIV)

#define __PRCM_APB0_RATIO(n) (((n) & 0x3) << 0)
#define PRCM_APB0_RATIO_DIV_MASK __PRCM_APB0_RATIO_DIV(0x3)
#define __PRCM_APB0_RATIO_DIV(n) (((n) >> 1) - 1)
#define PRCM_APB0_RATIO_DIV(n) \
	__PRCM_APB0_RATIO(__PRCM_APB0_RATIO_DIV(n))

#define PRCM_CPU_CFG_NEON_CLK_EN (0x1 << 0)
#define PRCM_CPU_CFG_CPU_CLK_EN (0x1 << 1)

#define PRCM_APB0_GATE_PIO (0x1 << 0)
#define PRCM_APB0_GATE_IR (0x1 << 1)
#define PRCM_APB0_GATE_TIMER01 (0x1 << 2)
#define PRCM_APB0_GATE_P2WI (0x1 << 3)		/* sun6i */
#define PRCM_APB0_GATE_RSB (0x1 << 3)		/* sun8i */
#define PRCM_APB0_GATE_UART (0x1 << 4)
#define PRCM_APB0_GATE_1WIRE (0x1 << 5)
#define PRCM_APB0_GATE_I2C (0x1 << 6)

#define PRCM_APB0_RESET_PIO (0x1 << 0)
#define PRCM_APB0_RESET_IR (0x1 << 1)
#define PRCM_APB0_RESET_TIMER01 (0x1 << 2)
#define PRCM_APB0_RESET_P2WI (0x1 << 3)
#define PRCM_APB0_RESET_UART (0x1 << 4)
#define PRCM_APB0_RESET_1WIRE (0x1 << 5)
#define PRCM_APB0_RESET_I2C (0x1 << 6)

#define PRCM_PLL_CTRL_PLL_BIAS (0x1 << 0)
#define PRCM_PLL_CTRL_HOSC_GAIN_ENH (0x1 << 1)
#define __PRCM_PLL_CTRL_USB_CLK_SRC(n) (((n) & 0x3) << 4)
#define PRCM_PLL_CTRL_USB_CLK_SRC_MASK \
	__PRCM_PLL_CTRL_USB_CLK_SRC(0x3)
#define __PRCM_PLL_CTRL_USB_CLK_0 0x0
#define __PRCM_PLL_CTRL_USB_CLK_1 0x1
#define __PRCM_PLL_CTRL_USB_CLK_2 0x2
#define __PRCM_PLL_CTRL_USB_CLK_3 0x3
#define PRCM_PLL_CTRL_USB_CLK_0 \
	__PRCM_PLL_CTRL_USB_CLK_SRC(__PRCM_PLL_CTRL_USB_CLK_0)
#define PRCM_PLL_CTRL_USB_CLK_1 \
	__PRCM_PLL_CTRL_USB_CLK_SRC(__PRCM_PLL_CTRL_USB_CLK_1)
#define PRCM_PLL_CTRL_USB_CLK_2 \
	__PRCM_PLL_CTRL_USB_CLK_SRC(__PRCM_PLL_CTRL_USB_CLK_2)
#define PRCM_PLL_CTRL_USB_CLK_3 \
	__PRCM_PLL_CTRL_USB_CLK_SRC(__PRCM_PLL_CTRL_USB_CLK_3)
#define __PRCM_PLL_CTRL_INT_PLL_IN_SEL(n) (((n) & 0x3) << 12)
#define PRCM_PLL_CTRL_INT_PLL_IN_SEL_MASK \
	__PRCM_PLL_CTRL_INT_PLL_IN_SEL(0x3)
#define PRCM_PLL_CTRL_INT_PLL_IN_SEL(n) \
	__PRCM_PLL_CTRL_INT_PLL_IN_SEL(n)
#define __PRCM_PLL_CTRL_HOSC_CLK_SEL(n) (((n) & 0x3) << 20)
#define PRCM_PLL_CTRL_HOSC_CLK_SEL_MASK \
	__PRCM_PLL_CTRL_HOSC_CLK_SEL(0x3)
#define __PRCM_PLL_CTRL_HOSC_CLK_0 0x0
#define __PRCM_PLL_CTRL_HOSC_CLK_1 0x1
#define __PRCM_PLL_CTRL_HOSC_CLK_2 0x2
#define __PRCM_PLL_CTRL_HOSC_CLK_3 0x3
#define PRCM_PLL_CTRL_HOSC_CLK_0 \
	__PRCM_PLL_CTRL_HOSC_CLK_SEL(__PRCM_PLL_CTRL_HOSC_CLK_0)
#define PRCM_PLL_CTRL_HOSC_CLK_1 \
	__PRCM_PLL_CTRL_HOSC_CLK_SEL(__PRCM_PLL_CTRL_HOSC_CLK_1)
#define PRCM_PLL_CTRL_HOSC_CLK_2 \
	__PRCM_PLL_CTRL_HOSC_CLK_SEL(__PRCM_PLL_CTRL_HOSC_CLK_2)
#define PRCM_PLL_CTRL_HOSC_CLK_3 \
	__PRCM_PLL_CTRL_HOSC_CLK_SEL(__PRCM_PLL_CTRL_HOSC_CLK_3)
#define PRCM_PLL_CTRL_PLL_TST_SRC_EXT (0x1 << 24)
#define PRCM_PLL_CTRL_LDO_DIGITAL_EN (0x1 << 0)
#define PRCM_PLL_CTRL_LDO_ANALOG_EN (0x1 << 1)
#define PRCM_PLL_CTRL_EXT_OSC_EN (0x1 << 2)
#define PRCM_PLL_CTRL_CLK_TST_EN (0x1 << 3)
#define PRCM_PLL_CTRL_IN_PWR_HIGH (0x1 << 15) /* 3.3 for hi 2.5 for lo */
#define __PRCM_PLL_CTRL_VDD_LDO_OUT(n) (((n) & 0x7) << 16)
#define PRCM_PLL_CTRL_LDO_OUT_MASK \
	__PRCM_PLL_CTRL_LDO_OUT(0x7)
/* When using the low voltage 20 mV steps, and high voltage 30 mV steps */
#define PRCM_PLL_CTRL_LDO_OUT_L(n) \
	__PRCM_PLL_CTRL_VDD_LDO_OUT((((n) - 1000) / 20) & 0x7)
#define PRCM_PLL_CTRL_LDO_OUT_H(n) \
	__PRCM_PLL_CTRL_VDD_LDO_OUT((((n) - 1160) / 30) & 0x7)
#define PRCM_PLL_CTRL_LDO_OUT_LV(n) \
	__PRCM_PLL_CTRL_VDD_LDO_OUT((((n) & 0x7) * 20) + 1000)
#define PRCM_PLL_CTRL_LDO_OUT_HV(n) \
	__PRCM_PLL_CTRL_VDD_LDO_OUT((((n) & 0x7) * 30) + 1160)
#define PRCM_PLL_CTRL_LDO_KEY (0xa7 << 24)
#define PRCM_PLL_CTRL_LDO_KEY_MASK (0xff << 24)

#define PRCM_CLK_1WIRE_GATE (0x1 << 31)

#define __PRCM_CLK_MOD0_M(n) (((n) & 0xf) << 0)
#define PRCM_CLK_MOD0_M_MASK __PRCM_CLK_MOD0_M(0xf)
#define __PRCM_CLK_MOD0_M_X(n) (n - 1)
#define PRCM_CLK_MOD0_M(n) __PRCM_CLK_MOD0_M(__PRCM_CLK_MOD0_M_X(n))
#define PRCM_CLK_MOD0_OUT_PHASE(n) (((n) & 0x7) << 8)
#define PRCM_CLK_MOD0_OUT_PHASE_MASK(n) PRCM_CLK_MOD0_OUT_PHASE(0x7)
#define _PRCM_CLK_MOD0_N(n) (((n) & 0x3) << 16)
#define PRCM_CLK_MOD0_N_MASK __PRCM_CLK_MOD_N(0x3)
#define __PRCM_CLK_MOD0_N_X(n) (((n) >> 1) - 1)
#define PRCM_CLK_MOD0_N(n) __PRCM_CLK_MOD0_N(__PRCM_CLK_MOD0_N_X(n))
#define PRCM_CLK_MOD0_SMPL_PHASE(n) (((n) & 0x7) << 20)
#define PRCM_CLK_MOD0_SMPL_PHASE_MASK PRCM_CLK_MOD0_SMPL_PHASE(0x7)
#define PRCM_CLK_MOD0_SRC_SEL(n) (((n) & 0x7) << 24)
#define PRCM_CLK_MOD0_SRC_SEL_MASK PRCM_CLK_MOD0_SRC_SEL(0x7)
#define PRCM_CLK_MOD0_GATE_EN (0x1 << 31)

#define PRCM_APB0_RESET_PIO (0x1 << 0)
#define PRCM_APB0_RESET_IR (0x1 << 1)
#define PRCM_APB0_RESET_TIMER01 (0x1 << 2)
#define PRCM_APB0_RESET_P2WI (0x1 << 3)
#define PRCM_APB0_RESET_UART (0x1 << 4)
#define PRCM_APB0_RESET_1WIRE (0x1 << 5)
#define PRCM_APB0_RESET_I2C (0x1 << 6)

#define __PRCM_CLK_OUTD_M(n) (((n) & 0x7) << 8)
#define PRCM_CLK_OUTD_M_MASK __PRCM_CLK_OUTD_M(0x7)
#define __PRCM_CLK_OUTD_M_X() ((n) - 1)
#define PRCM_CLK_OUTD_M(n) __PRCM_CLK_OUTD_M(__PRCM_CLK_OUTD_M_X(n))
#define __PRCM_CLK_OUTD_N(n) (((n) & 0x7) << 20)
#define PRCM_CLK_OUTD_N_MASK __PRCM_CLK_OUTD_N(0x7)
#define __PRCM_CLK_OUTD_N_X(n) (((n) >> 1) - 1)
#define PRCM_CLK_OUTD_N(n) __PRCM_CLK_OUTD_N(__PRCM_CLK_OUTD_N_X(n)
#define __PRCM_CLK_OUTD_SRC_SEL(n) (((n) & 0x3) << 24)
#define PRCM_CLK_OUTD_SRC_SEL_MASK __PRCM_CLK_OUTD_SRC_SEL(0x3)
#define __PRCM_CLK_OUTD_SRC_LOSC2 0x0
#define __PRCM_CLK_OUTD_SRC_LOSC 0x1
#define __PRCM_CLK_OUTD_SRC_HOSC 0x2
#define __PRCM_CLK_OUTD_SRC_ERR 0x3
#define PRCM_CLK_OUTD_SRC_LOSC2 \
#deifne __PRCM_CLK_OUTD_SRC_SEL(__PRCM_CLK_OUTD_SRC_LOSC2)
#define PRCM_CLK_OUTD_SRC_LOSC \
#deifne __PRCM_CLK_OUTD_SRC_SEL(__PRCM_CLK_OUTD_SRC_LOSC)
#define PRCM_CLK_OUTD_SRC_HOSC \
#deifne __PRCM_CLK_OUTD_SRC_SEL(__PRCM_CLK_OUTD_SRC_HOSC)
#define PRCM_CLK_OUTD_SRC_ERR \
#deifne __PRCM_CLK_OUTD_SRC_SEL(__PRCM_CLK_OUTD_SRC_ERR)
#define PRCM_CLK_OUTD_EN (0x1 << 31)

#define PRCM_CPU0_PWROFF (0x1 << 0)
#define PRCM_CPU1_PWROFF (0x1 << 1)
#define PRCM_CPU2_PWROFF (0x1 << 2)
#define PRCM_CPU3_PWROFF (0x1 << 3)
#define PRCM_CPU_ALL_PWROFF (0xf << 0)

#define PRCM_VDD_SYS_DRAM_CH0_PAD_HOLD_PWROFF (0x1 << 0)
#define PRCM_VDD_SYS_DRAM_CH1_PAD_HOLD_PWROFF (0x1 << 1)
#define PRCM_VDD_SYS_AVCC_A_PWROFF (0x1 << 2)
#define PRCM_VDD_SYS_CPU0_VDD_PWROFF (0x1 << 3)

#define PRCM_VDD_GPU_PWROFF (0x1 << 0)

#define PRCM_VDD_SYS_RESET (0x1 << 0)

#define PRCM_CPU1_PWR_CLAMP(n) (((n) & 0xff) << 0)
#define PRCM_CPU1_PWR_CLAMP_MASK PRCM_CPU1_PWR_CLAMP(0xff)

#define PRCM_CPU2_PWR_CLAMP(n) (((n) & 0xff) << 0)
#define PRCM_CPU2_PWR_CLAMP_MASK PRCM_CPU2_PWR_CLAMP(0xff)

#define PRCM_CPU3_PWR_CLAMP(n) (((n) & 0xff) << 0)
#define PRCM_CPU3_PWR_CLAMP_MASK PRCM_CPU3_PWR_CLAMP(0xff)

#define PRCM_SEC_SWITCH_APB0_CLK_NONSEC (0x1 << 0)
#define PRCM_SEC_SWITCH_PLL_CFG_NONSEC (0x1 << 1)
#define PRCM_SEC_SWITCH_PWR_GATE_NONSEC (0x1 << 2)

#ifndef __ASSEMBLY__
#include <linux/compiler.h>

struct __packed sunxi_prcm_reg {
	u32 cpus_cfg;		/* 0x000 */
	u8 res0[0x8];		/* 0x004 */
	u32 apb0_ratio;		/* 0x00c */
	u32 cpu0_cfg;		/* 0x010 */
	u32 cpu1_cfg;		/* 0x014 */
	u32 cpu2_cfg;		/* 0x018 */
	u32 cpu3_cfg;		/* 0x01c */
	u8 res1[0x8];		/* 0x020 */
	u32 apb0_gate;		/* 0x028 */
	u8 res2[0x14];		/* 0x02c */
	u32 pll_ctrl0;		/* 0x040 */
	u32 pll_ctrl1;		/* 0x044 */
	u8 res3[0x8];		/* 0x048 */
	u32 clk_1wire;		/* 0x050 */
	u32 clk_ir;		/* 0x054 */
	u8 res4[0x58];		/* 0x058 */
	u32 apb0_reset;		/* 0x0b0 */
	u8 res5[0x3c];		/* 0x0b4 */
	u32 clk_outd;		/* 0x0f0 */
	u8 res6[0xc];		/* 0x0f4 */
	u32 cpu_pwroff;		/* 0x100 */
	u8 res7[0xc];		/* 0x104 */
	u32 vdd_sys_pwroff;	/* 0x110 */
	u8 res8[0x4];		/* 0x114 */
	u32 gpu_pwroff;		/* 0x118 */
	u8 res9[0x4];		/* 0x11c */
	u32 vdd_pwr_reset;	/* 0x120 */
	u8 res10[0x1c];		/* 0x124 */
	u32 cpu_pwr_clamp[4];	/* 0x140 but first one is actually unused */
	u8 res11[0x30];		/* 0x150 */
	u32 dram_pwr;		/* 0x180 */
	u8 res12[0xc];		/* 0x184 */
	u32 dram_tst;		/* 0x190 */
	u8 res13[0x3c];		/* 0x194 */
	u32 prcm_sec_switch;	/* 0x1d0 */
};

void prcm_apb0_enable(u32 flags);
void prcm_apb0_disable(u32 flags);

#endif /* __ASSEMBLY__ */
#endif /* _PRCM_H */
