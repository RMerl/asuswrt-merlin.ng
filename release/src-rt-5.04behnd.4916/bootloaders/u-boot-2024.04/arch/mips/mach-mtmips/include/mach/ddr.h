/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTMIPS_DDR_H_
#define _MTMIPS_DDR_H_

#include <linux/io.h>
#include <linux/types.h>

enum mc_dram_size {
	DRAM_8MB,
	DRAM_16MB,
	DRAM_32MB,
	DRAM_64MB,
	DRAM_128MB,
	DRAM_256MB,

	__DRAM_SZ_MAX
};

struct mc_ddr_cfg {
	u32 cfg0;
	u32 cfg1;
	u32 cfg2;
	u32 cfg3;
	u32 cfg4;
};

typedef void (*mc_reset_t)(int assert);

struct mc_ddr_init_param {
	void __iomem *memc;

	u32 sdr_cfg0;
	u32 sdr_cfg1;

	u32 dq_dly;
	u32 dqs_dly;

	const struct mc_ddr_cfg *cfgs;
	mc_reset_t mc_reset;

	u32 memsize;
	u32 bus_width;
};

void sdr_init(struct mc_ddr_init_param *param);
void ddr1_init(struct mc_ddr_init_param *param);
void ddr2_init(struct mc_ddr_init_param *param);
void ddr_calibrate(void __iomem *memc, u32 memsize, u32 bw);

#endif /* _MTMIPS_DDR_H_ */
