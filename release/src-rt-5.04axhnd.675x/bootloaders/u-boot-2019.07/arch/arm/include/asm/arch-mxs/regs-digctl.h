/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 DIGCTL Register Definitions
 *
 * Copyright (C) 2012 Robert Delien <robert@delien.nl>
 */

#ifndef __MX28_REGS_DIGCTL_H__
#define __MX28_REGS_DIGCTL_H__

#include <asm/mach-imx/regs-common.h>

#ifndef	__ASSEMBLY__
struct mxs_digctl_regs {
	mxs_reg_32(hw_digctl_ctrl)				/* 0x000 */
	mxs_reg_32(hw_digctl_status)				/* 0x010 */
	mxs_reg_32(hw_digctl_hclkcount)			/* 0x020 */
	mxs_reg_32(hw_digctl_ramctrl)				/* 0x030 */
	mxs_reg_32(hw_digctl_emi_status)			/* 0x040 */
	mxs_reg_32(hw_digctl_read_margin)			/* 0x050 */
	uint32_t	hw_digctl_writeonce;			/* 0x060 */
	uint32_t	reserved_writeonce[3];
	mxs_reg_32(hw_digctl_bist_ctl)				/* 0x070 */
	mxs_reg_32(hw_digctl_bist_status)			/* 0x080 */
	uint32_t	hw_digctl_entropy;			/* 0x090 */
	uint32_t	reserved_entropy[3];
	uint32_t	hw_digctl_entropy_latched;		/* 0x0a0 */
	uint32_t	reserved_entropy_latched[3];

	uint32_t	reserved1[4];

	mxs_reg_32(hw_digctl_microseconds)			/* 0x0c0 */
	uint32_t	hw_digctl_dbgrd;			/* 0x0d0 */
	uint32_t	reserved_hw_digctl_dbgrd[3];
	uint32_t	hw_digctl_dbg;				/* 0x0e0 */
	uint32_t	reserved_hw_digctl_dbg[3];

	uint32_t	reserved2[4];

	mxs_reg_32(hw_digctl_usb_loopback)			/* 0x100 */
	mxs_reg_32(hw_digctl_ocram_status0)			/* 0x110 */
	mxs_reg_32(hw_digctl_ocram_status1)			/* 0x120 */
	mxs_reg_32(hw_digctl_ocram_status2)			/* 0x130 */
	mxs_reg_32(hw_digctl_ocram_status3)			/* 0x140 */
	mxs_reg_32(hw_digctl_ocram_status4)			/* 0x150 */
	mxs_reg_32(hw_digctl_ocram_status5)			/* 0x160 */
	mxs_reg_32(hw_digctl_ocram_status6)			/* 0x170 */
	mxs_reg_32(hw_digctl_ocram_status7)			/* 0x180 */
	mxs_reg_32(hw_digctl_ocram_status8)			/* 0x190 */
	mxs_reg_32(hw_digctl_ocram_status9)			/* 0x1a0 */
	mxs_reg_32(hw_digctl_ocram_status10)			/* 0x1b0 */
	mxs_reg_32(hw_digctl_ocram_status11)			/* 0x1c0 */
	mxs_reg_32(hw_digctl_ocram_status12)			/* 0x1d0 */
	mxs_reg_32(hw_digctl_ocram_status13)			/* 0x1e0 */

	uint32_t	reserved3[36];

	uint32_t	hw_digctl_scratch0;			/* 0x280 */
	uint32_t	reserved_hw_digctl_scratch0[3];
	uint32_t	hw_digctl_scratch1;			/* 0x290 */
	uint32_t	reserved_hw_digctl_scratch1[3];
	uint32_t	hw_digctl_armcache;			/* 0x2a0 */
	uint32_t	reserved_hw_digctl_armcache[3];
	mxs_reg_32(hw_digctl_debug_trap)			/* 0x2b0 */
	uint32_t	hw_digctl_debug_trap_l0_addr_low;	/* 0x2c0 */
	uint32_t	reserved_hw_digctl_debug_trap_l0_addr_low[3];
	uint32_t	hw_digctl_debug_trap_l0_addr_high;	/* 0x2d0 */
	uint32_t	reserved_hw_digctl_debug_trap_l0_addr_high[3];
	uint32_t	hw_digctl_debug_trap_l3_addr_low;	/* 0x2e0 */
	uint32_t	reserved_hw_digctl_debug_trap_l3_addr_low[3];
	uint32_t	hw_digctl_debug_trap_l3_addr_high;	/* 0x2f0 */
	uint32_t	reserved_hw_digctl_debug_trap_l3_addr_high[3];
	uint32_t	hw_digctl_fsl;				/* 0x300 */
	uint32_t	reserved_hw_digctl_fsl[3];
	uint32_t	hw_digctl_chipid;			/* 0x310 */
	uint32_t	reserved_hw_digctl_chipid[3];

	uint32_t	reserved4[4];

	uint32_t	hw_digctl_ahb_stats_select;		/* 0x330 */
	uint32_t	reserved_hw_digctl_ahb_stats_select[3];

	uint32_t	reserved5[12];

	uint32_t	hw_digctl_l1_ahb_active_cycles;		/* 0x370 */
	uint32_t	reserved_hw_digctl_l1_ahb_active_cycles[3];
	uint32_t	hw_digctl_l1_ahb_data_stalled;		/* 0x380 */
	uint32_t	reserved_hw_digctl_l1_ahb_data_stalled[3];
	uint32_t	hw_digctl_l1_ahb_data_cycles;		/* 0x390 */
	uint32_t	reserved_hw_digctl_l1_ahb_data_cycles[3];
	uint32_t	hw_digctl_l2_ahb_active_cycles;		/* 0x3a0 */
	uint32_t	reserved_hw_digctl_l2_ahb_active_cycles[3];
	uint32_t	hw_digctl_l2_ahb_data_stalled;		/* 0x3b0 */
	uint32_t	reserved_hw_digctl_l2_ahb_data_stalled[3];
	uint32_t	hw_digctl_l2_ahb_data_cycles;		/* 0x3c0 */
	uint32_t	reserved_hw_digctl_l2_ahb_data_cycles[3];
	uint32_t	hw_digctl_l3_ahb_active_cycles;		/* 0x3d0 */
	uint32_t	reserved_hw_digctl_l3_ahb_active_cycles[3];
	uint32_t	hw_digctl_l3_ahb_data_stalled;		/* 0x3e0 */
	uint32_t	reserved_hw_digctl_l3_ahb_data_stalled[3];
	uint32_t	hw_digctl_l3_ahb_data_cycles;		/* 0x3f0 */
	uint32_t	reserved_hw_digctl_l3_ahb_data_cycles[3];

	uint32_t	reserved6[64];

	uint32_t	hw_digctl_mpte0_loc;			/* 0x500 */
	uint32_t	reserved_hw_digctl_mpte0_loc[3];
	uint32_t	hw_digctl_mpte1_loc;			/* 0x510 */
	uint32_t	reserved_hw_digctl_mpte1_loc[3];
	uint32_t	hw_digctl_mpte2_loc;			/* 0x520 */
	uint32_t	reserved_hw_digctl_mpte2_loc[3];
	uint32_t	hw_digctl_mpte3_loc;			/* 0x530 */
	uint32_t	reserved_hw_digctl_mpte3_loc[3];
	uint32_t	hw_digctl_mpte4_loc;			/* 0x540 */
	uint32_t	reserved_hw_digctl_mpte4_loc[3];
	uint32_t	hw_digctl_mpte5_loc;			/* 0x550 */
	uint32_t	reserved_hw_digctl_mpte5_loc[3];
	uint32_t	hw_digctl_mpte6_loc;			/* 0x560 */
	uint32_t	reserved_hw_digctl_mpte6_loc[3];
	uint32_t	hw_digctl_mpte7_loc;			/* 0x570 */
	uint32_t	reserved_hw_digctl_mpte7_loc[3];
	uint32_t	hw_digctl_mpte8_loc;			/* 0x580 */
	uint32_t	reserved_hw_digctl_mpte8_loc[3];
	uint32_t	hw_digctl_mpte9_loc;			/* 0x590 */
	uint32_t	reserved_hw_digctl_mpte9_loc[3];
	uint32_t	hw_digctl_mpte10_loc;			/* 0x5a0 */
	uint32_t	reserved_hw_digctl_mpte10_loc[3];
	uint32_t	hw_digctl_mpte11_loc;			/* 0x5b0 */
	uint32_t	reserved_hw_digctl_mpte11_loc[3];
	uint32_t	hw_digctl_mpte12_loc;			/* 0x5c0 */
	uint32_t	reserved_hw_digctl_mpte12_loc[3];
	uint32_t	hw_digctl_mpte13_loc;			/* 0x5d0 */
	uint32_t	reserved_hw_digctl_mpte13_loc[3];
	uint32_t	hw_digctl_mpte14_loc;			/* 0x5e0 */
	uint32_t	reserved_hw_digctl_mpte14_loc[3];
	uint32_t	hw_digctl_mpte15_loc;			/* 0x5f0 */
	uint32_t	reserved_hw_digctl_mpte15_loc[3];
};
#endif

/* Product code identification */
#define HW_DIGCTL_CHIPID_MASK	(0xffff << 16)
#define HW_DIGCTL_CHIPID_MX23	(0x3780 << 16)
#define HW_DIGCTL_CHIPID_MX28	(0x2800 << 16)

#endif /* __MX28_REGS_DIGCTL_H__ */
