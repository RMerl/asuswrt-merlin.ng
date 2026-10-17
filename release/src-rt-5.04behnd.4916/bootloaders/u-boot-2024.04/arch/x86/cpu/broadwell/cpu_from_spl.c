// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bloblist.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <handoff.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/mtrr.h>

int misc_init_r(void)
{
	return 0;
}

int dram_init(void)
{
	struct spl_handoff *ho;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, sizeof(*ho));
	if (!ho)
		return log_msg_ret("Missing SPL hand-off info", -ENOENT);
	handoff_load_dram_size(ho);
#ifdef CONFIG_TPL
	/* TODO(sjg@chromium.org): MTRR cannot be adjusted without a hang */
	mtrr_add_request(MTRR_TYPE_WRBACK, 0, 2ULL << 30);
#else
	mtrr_add_request(MTRR_TYPE_WRBACK, 0, gd->ram_size);
	mtrr_commit(true);
#endif

	return 0;
}

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	return 0;
}

void board_debug_uart_init(void)
{
}

int dram_init_banksize(void)
{
	struct spl_handoff *ho;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, sizeof(*ho));
	if (!ho)
		return log_msg_ret("Missing SPL hand-off info", -ENOENT);
	handoff_load_dram_banks(ho);

	return 0;
}
