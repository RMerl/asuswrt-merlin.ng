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
	struct {
		u64 start;
		u64 size;
	} ram_bank[CONFIG_NR_DRAM_BANKS];
};

void handoff_save_dram(struct spl_handoff *ho);
void handoff_load_dram_size(struct spl_handoff *ho);
void handoff_load_dram_banks(struct spl_handoff *ho);

/**
 * handoff_arch_save() - Save arch-specific info into the handoff area
 *
 * This is defined to an empty function by default, but arch-specific code can
 * define it to write to spi_handoff->arch. It is called from
 * write_spl_handoff().
 *
 * @ho: Handoff area to fill in
 * Return: 0 if OK, -ve on error
 */
int handoff_arch_save(struct spl_handoff *ho);

#endif

#endif
