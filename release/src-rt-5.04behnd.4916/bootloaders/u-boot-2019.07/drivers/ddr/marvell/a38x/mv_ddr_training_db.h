/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef _MV_DDR_TRAINING_DB_H
#define _MV_DDR_TRAINING_DB_H

#include "mv_ddr_topology.h"

/* in ns */
#define TREFI_LOW	7800
#define TREFI_HIGH	3900

enum mv_ddr_page_size {
	MV_DDR_PAGE_SIZE_1K = 1,
	MV_DDR_PAGE_SIZE_2K
};

struct mv_ddr_page_element {
	/* 8-bit bus width page size */
	enum mv_ddr_page_size page_size_8bit;
	/* 16-bit bus width page size */
	enum mv_ddr_page_size page_size_16bit;
};

/* cas latency value per frequency */
struct mv_ddr_cl_val_per_freq {
	unsigned int cl_val[MV_DDR_FREQ_LAST];
};

u32 mv_ddr_rfc_get(u32 mem);
unsigned int *mv_ddr_freq_tbl_get(void);
u32 mv_ddr_freq_get(enum mv_ddr_freq freq);
u32 mv_ddr_page_size_get(enum mv_ddr_dev_width bus_width, enum mv_ddr_die_capacity mem_size);
unsigned int mv_ddr_speed_bin_timing_get(enum mv_ddr_speed_bin index, enum mv_ddr_speed_bin_timing element);
u32 mv_ddr_cl_val_get(u32 index, u32 freq);
u32 mv_ddr_cwl_val_get(u32 index, u32 freq);

#endif /* _MV_DDR_TRAINING_DB_H */
