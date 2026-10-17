/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 - 2018 Xilinx, Inc.
 */

#include <linux/build_bug.h>

enum {
	TCM_LOCK,
	TCM_SPLIT,
};

void initialize_tcm(bool mode);
void tcm_init(u8 mode);
void mem_map_fill(void);
