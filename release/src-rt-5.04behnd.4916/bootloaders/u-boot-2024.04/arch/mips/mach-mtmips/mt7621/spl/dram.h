/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT7621_DRAM_H_
#define _MT7621_DRAM_H_

#define STAGE_LOAD_ADDR			0xBE108800

#ifndef __ASSEMBLY__
#include <linux/types.h>

#define DDR_PARAM_SIZE			24

struct stage_header {
	u32 jump_insn[2];
	u32 ep;
	u32 stage_size;
	u32 has_stage2;
	u32 next_ep;
	u32 next_size;
	u32 next_offset;
	u32 cpu_pll_cfg;
	u32 ddr_pll_cfg;
	u32 reserved2[6];
	char build_tag[32];
	u32 ddr3_act[DDR_PARAM_SIZE];
	u32 padding1[2];
	u32 ddr2_act[DDR_PARAM_SIZE];
	u32 padding2[2];
	u32 baudrate;
	u32 padding3;
};
#endif

#endif /* _MT7621_DRAM_H_ */
