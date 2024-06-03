/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ifndef __HANDOFF_H
#define __HANDOFF_H

#if CONFIG_IS_ENABLED(HANDOFF)

#include <asm/handoff.h>

/**
 * struct spl_handoff - information passed from SPL to U-Boot proper
 *
 * @ram_size: Value to use for gd->ram_size
 */
struct spl_handoff {
	struct arch_spl_handoff arch;
	u64 ram_size;
#ifdef CONFIG_NR_DRAM_BANKS
	struct {
		u64 start;
		u64 size;
	} ram_bank[CONFIG_NR_DRAM_BANKS];
#endif
};

void handoff_save_dram(struct spl_handoff *ho);
void handoff_load_dram_size(struct spl_handoff *ho);
void handoff_load_dram_banks(struct spl_handoff *ho);
#endif

#endif
