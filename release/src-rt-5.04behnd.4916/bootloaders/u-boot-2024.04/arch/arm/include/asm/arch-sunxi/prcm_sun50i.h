/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sunxi H6 Power Management Unit register definition.
 *
 * (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#ifndef _SUN50I_PRCM_H
#define _SUN50I_PRCM_H

#ifndef __ASSEMBLY__
#include <linux/compiler.h>

struct sunxi_prcm_reg {
	u32 cpus_cfg;		/* 0x000 */
	u8 res0[0x8];		/* 0x004 */
	u32 apbs1_cfg;		/* 0x00c */
	u32 apbs2_cfg;		/* 0x010 */
	u8 res1[0x108];		/* 0x014 */
	u32 tmr_gate_reset;	/* 0x11c */
	u8 res2[0xc];		/* 0x120 */
	u32 twd_gate_reset;	/* 0x12c */
	u8 res3[0xc];		/* 0x130 */
	u32 pwm_gate_reset;	/* 0x13c */
	u8 res4[0x4c];		/* 0x140 */
	u32 uart_gate_reset;	/* 0x18c */
	u8 res5[0xc];		/* 0x190 */
	u32 twi_gate_reset;	/* 0x19c */
	u8 res6[0x1c];		/* 0x1a0 */
	u32 rsb_gate_reset;	/* 0x1bc */
	u32 cir_cfg;		/* 0x1c0 */
	u8 res7[0x8];		/* 0x1c4 */
	u32 cir_gate_reset;	/* 0x1cc */
	u8 res8[0x10];		/* 0x1d0 */
	u32 w1_cfg;		/* 0x1e0 */
	u8 res9[0x8];		/* 0x1e4 */
	u32 w1_gate_reset;	/* 0x1ec */
	u8 res10[0x1c];		/* 0x1f0 */
	u32 rtc_gate_reset;	/* 0x20c */
	u8 res11[0x34];		/* 0x210 */
	u32 pll_ldo_cfg;	/* 0x244 */
	u8 res12[0x8];		/* 0x248 */
	u32 sys_pwroff_gating;	/* 0x250 */
	u8 res13[0xbc];		/* 0x254 */
	u32 res_cal_ctrl;	/* 0x310 */
	u32 ohms200;		/* 0x314 */
	u32 ohms240;		/* 0x318 */
	u32 res_cal_status;	/* 0x31c */
};
check_member(sunxi_prcm_reg, rtc_gate_reset, 0x20c);
check_member(sunxi_prcm_reg, res_cal_status, 0x31c);

#define PRCM_TWI_GATE		(1 << 0)
#define PRCM_TWI_RESET		(1 << 16)

#endif /* __ASSEMBLY__ */
#endif /* _PRCM_H */
