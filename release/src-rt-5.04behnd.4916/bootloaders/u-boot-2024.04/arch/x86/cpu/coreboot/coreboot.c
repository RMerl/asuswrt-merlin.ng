// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 */

#include <common.h>
#include <cpu_func.h>
#include <event.h>
#include <fdtdec.h>
#include <init.h>
#include <usb.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/cb_sysinfo.h>
#include <asm/arch/timestamp.h>
#include <dm/ofnode.h>

int arch_cpu_init(void)
{
	int ret;

	ret = IS_ENABLED(CONFIG_X86_RUN_64BIT) ? x86_cpu_reinit_f() :
		x86_cpu_init_f();
	if (ret)
		return ret;

	ret = get_coreboot_info(&lib_sysinfo);
	if (ret != 0) {
		printf("Failed to parse coreboot tables.\n");
		return ret;
	}

	timestamp_init();

	return 0;
}

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	return default_print_cpuinfo();
}

static void board_final_init(void)
{
	/*
	 * Un-cache the ROM so the kernel has one
	 * more MTRR available.
	 *
	 * Coreboot should have assigned this to the
	 * top available variable MTRR.
	 */
	u8 top_mtrr = (native_read_msr(MTRR_CAP_MSR) & 0xff) - 1;
	u8 top_type = native_read_msr(MTRR_PHYS_BASE_MSR(top_mtrr)) & 0xff;

	/* Make sure this MTRR is the correct Write-Protected type */
	if (top_type == MTRR_TYPE_WRPROT) {
		struct mtrr_state state;

		mtrr_open(&state, true);
		wrmsrl(MTRR_PHYS_BASE_MSR(top_mtrr), 0);
		wrmsrl(MTRR_PHYS_MASK_MSR(top_mtrr), 0);
		mtrr_close(&state, true);
	}

	if (!ofnode_conf_read_bool("u-boot,no-apm-finalize")) {
		/*
		 * Issue SMI to coreboot to lock down ME and registers
		 * when allowed via device tree
		 */
		printf("Finalizing coreboot\n");
		outb(0xcb, 0xb2);
	}
}

static int last_stage_init(void)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		return 0;

	board_final_init();

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);
