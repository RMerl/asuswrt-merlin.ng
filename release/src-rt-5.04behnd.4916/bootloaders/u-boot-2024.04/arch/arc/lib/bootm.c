// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#include <bootm.h>
#include <bootstage.h>
#include <env.h>
#include <image.h>
#include <irq_func.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int cleanup_before_linux(void)
{
	disable_interrupts();
	sync_n_cleanup_cache_all();

	return 0;
}

__weak int board_prep_linux(struct bootm_headers *images) { return 0; }

/* Subcommand: PREP */
static int boot_prep_linux(struct bootm_headers *images)
{
	int ret;

	if (IS_ENABLED(CONFIG_LMB)) {
		ret = image_setup_linux(images);
		if (ret)
			return ret;
	}

	return board_prep_linux(images);
}

/* Generic implementation for single core CPU */
__weak void board_jump_and_run(ulong entry, int zero, int arch, uint params)
{
	void (*kernel_entry)(int zero, int arch, uint params);

	kernel_entry = (void (*)(int, int, uint))entry;

	kernel_entry(zero, arch, params);
}

/* Subcommand: GO */
static void boot_jump_linux(struct bootm_headers *images, int flag)
{
	ulong kernel_entry;
	unsigned int r0, r2;
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);

	kernel_entry = images->ep;

	debug("## Transferring control to Linux (at address %08lx)...\n",
	      kernel_entry);
	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	printf("\nStarting kernel ...%s\n\n", fake ?
	       "(fake run for tracing)" : "");
	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_HANDOFF, "start_kernel");

	if (CONFIG_IS_ENABLED(OF_LIBFDT) && images->ft_len) {
		r0 = 2;
		r2 = (unsigned int)images->ft_addr;
	} else {
		r0 = 1;
		r2 = (unsigned int)env_get("bootargs");
	}

	cleanup_before_linux();

	if (!fake)
		board_jump_and_run(kernel_entry, r0, 0, r2);
}

int do_bootm_linux(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;

	/* No need for those on ARC */
	if ((flag & BOOTM_STATE_OS_BD_T) || (flag & BOOTM_STATE_OS_CMDLINE))
		return -1;

	if (flag & BOOTM_STATE_OS_PREP)
		return boot_prep_linux(images);

	if (flag & (BOOTM_STATE_OS_GO | BOOTM_STATE_OS_FAKE_GO)) {
		boot_jump_linux(images, flag);
		return 0;
	}

	return -1;
}
