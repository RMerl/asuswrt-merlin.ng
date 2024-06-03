/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sunxi A31 CPUCFG register definition.
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com
 */

#ifndef _SUNXI_CPUCFG_H
#define _SUNXI_CPUCFG_H

#include <linux/compiler.h>
#include <linux/types.h>

#ifndef __ASSEMBLY__

struct __packed sunxi_cpucfg_cpu {
	u32 rst;		/* base + 0x0 */
	u32 ctrl;		/* base + 0x4 */
	u32 status;		/* base + 0x8 */
	u8 res[0x34];		/* base + 0xc */
};

struct __packed sunxi_cpucfg_reg {
	u8 res0[0x40];		/* 0x000 */
	struct sunxi_cpucfg_cpu cpu[4];		/* 0x040 */
	u8 res1[0x44];		/* 0x140 */
	u32 gen_ctrl;		/* 0x184 */
	u32 l2_status;		/* 0x188 */
	u8 res2[0x4];		/* 0x18c */
	u32 event_in;		/* 0x190 */
	u8 res3[0xc];		/* 0x194 */
	u32 super_standy_flag;	/* 0x1a0 */
	u32 priv0;		/* 0x1a4 */
	u32 priv1;		/* 0x1a8 */
	u8 res4[0x4];		/* 0x1ac */
	u32 cpu1_pwr_clamp;	/* 0x1b0 sun7i only */
	u32 cpu1_pwroff;	/* 0x1b4 sun7i only */
	u8 res5[0x2c];		/* 0x1b8 */
	u32 dbg_ctrl1;		/* 0x1e4 */
	u8 res6[0x18];		/* 0x1e8 */
	u32 idle_cnt0_low;	/* 0x200 */
	u32 idle_cnt0_high;	/* 0x204 */
	u32 idle_cnt0_ctrl;	/* 0x208 */
	u8 res8[0x4];		/* 0x20c */
	u32 idle_cnt1_low;	/* 0x210 */
	u32 idle_cnt1_high;	/* 0x214 */
	u32 idle_cnt1_ctrl;	/* 0x218 */
	u8 res9[0x4];		/* 0x21c */
	u32 idle_cnt2_low;	/* 0x220 */
	u32 idle_cnt2_high;	/* 0x224 */
	u32 idle_cnt2_ctrl;	/* 0x228 */
	u8 res10[0x4];		/* 0x22c */
	u32 idle_cnt3_low;	/* 0x230 */
	u32 idle_cnt3_high;	/* 0x234 */
	u32 idle_cnt3_ctrl;	/* 0x238 */
	u8 res11[0x4];		/* 0x23c */
	u32 idle_cnt4_low;	/* 0x240 */
	u32 idle_cnt4_high;	/* 0x244 */
	u32 idle_cnt4_ctrl;	/* 0x248 */
	u8 res12[0x34];		/* 0x24c */
	u32 cnt64_ctrl;		/* 0x280 */
	u32 cnt64_low;		/* 0x284 */
	u32 cnt64_high;		/* 0x288 */
};

#endif /* __ASSEMBLY__ */
#endif /* _SUNXI_CPUCFG_H */
