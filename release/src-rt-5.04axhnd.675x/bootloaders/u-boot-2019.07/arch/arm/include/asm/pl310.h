/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 * Aneesh V <aneesh@ti.com>
 */
#ifndef _PL310_H_
#define _PL310_H_

#include <linux/types.h>

/* Register bit fields */
#define PL310_AUX_CTRL_ASSOCIATIVITY_MASK	(1 << 16)
#define L2X0_DYNAMIC_CLK_GATING_EN		(1 << 1)
#define L2X0_STNDBY_MODE_EN			(1 << 0)
#define L2X0_CTRL_EN				1

#define L310_SHARED_ATT_OVERRIDE_ENABLE		(1 << 22)
#define L310_AUX_CTRL_DATA_PREFETCH_MASK	(1 << 28)
#define L310_AUX_CTRL_INST_PREFETCH_MASK	(1 << 29)
#define L310_LATENCY_CTRL_SETUP(n)		((n) << 0)
#define L310_LATENCY_CTRL_RD(n)			((n) << 4)
#define L310_LATENCY_CTRL_WR(n)			((n) << 8)

#define L2X0_CACHE_ID_PART_MASK     (0xf << 6)
#define L2X0_CACHE_ID_PART_L310     (3 << 6)
#define L2X0_CACHE_ID_RTL_MASK          0x3f
#define L2X0_CACHE_ID_RTL_R3P2          0x8

struct pl310_regs {
	u32 pl310_cache_id;
	u32 pl310_cache_type;
	u32 pad1[62];
	u32 pl310_ctrl;
	u32 pl310_aux_ctrl;
	u32 pl310_tag_latency_ctrl;
	u32 pl310_data_latency_ctrl;
	u32 pad2[60];
	u32 pl310_event_cnt_ctrl;
	u32 pl310_event_cnt1_cfg;
	u32 pl310_event_cnt0_cfg;
	u32 pl310_event_cnt1_val;
	u32 pl310_event_cnt0_val;
	u32 pl310_intr_mask;
	u32 pl310_masked_intr_stat;
	u32 pl310_raw_intr_stat;
	u32 pl310_intr_clear;
	u32 pad3[323];
	u32 pl310_cache_sync;
	u32 pad4[15];
	u32 pl310_inv_line_pa;
	u32 pad5[2];
	u32 pl310_inv_way;
	u32 pad6[12];
	u32 pl310_clean_line_pa;
	u32 pad7[1];
	u32 pl310_clean_line_idx;
	u32 pl310_clean_way;
	u32 pad8[12];
	u32 pl310_clean_inv_line_pa;
	u32 pad9[1];
	u32 pl310_clean_inv_line_idx;
	u32 pl310_clean_inv_way;
	u32 pad10[64];
	u32 pl310_lockdown_dbase;
	u32 pl310_lockdown_ibase;
	u32 pad11[190];
	u32 pl310_addr_filter_start;
	u32 pl310_addr_filter_end;
	u32 pad12[190];
	u32 pl310_test_operation;
	u32 pad13[3];
	u32 pl310_line_data;
	u32 pad14[7];
	u32 pl310_line_tag;
	u32 pad15[3];
	u32 pl310_debug_ctrl;
	u32 pad16[7];
	u32 pl310_prefetch_ctrl;
	u32 pad17[7];
	u32 pl310_power_ctrl;
};

void pl310_inval_all(void);
void pl310_clean_inval_all(void);
void pl310_inval_range(u32 start, u32 end);
void pl310_clean_inval_range(u32 start, u32 end);

#endif
