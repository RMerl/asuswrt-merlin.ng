// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 * (C) Copyright 2019  Synamedia
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 */

#include <cpu_func.h>
#include <init.h>
#include <log.h>
#include <time.h>
#include <asm/global_data.h>
#include <linux/types.h>
#include <common.h>
#include <env.h>
#include <asm/io.h>
#include <asm/bootm.h>
#include <mach/timer.h>
#include <mmc.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

#define BCMSTB_DATA_SECTION __section(".data")

struct bcmstb_boot_parameters bcmstb_boot_parameters BCMSTB_DATA_SECTION;

phys_addr_t prior_stage_fdt_address BCMSTB_DATA_SECTION;

union reg_value_union {
	const char *data;
	const phys_addr_t *address;
};

int board_init(void)
{
	return 0;
}

void reset_cpu(void)
{
}

int print_cpuinfo(void)
{
	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	/*
	 * On this SoC, U-Boot is running as an ELF file.  Change the
	 * relocation address to CONFIG_TEXT_BASE, so that in
	 * setup_reloc, gd->reloc_off works out to 0, effectively
	 * disabling relocation.  Otherwise U-Boot hangs in the setup
	 * instructions just before relocate_code in
	 * arch/arm/lib/crt0.S.
	 */
	gd->relocaddr = CONFIG_TEXT_BASE;

	return 0;
}

void enable_caches(void)
{
	/*
	 * This port assumes that the prior stage bootloader has
	 * enabled I-cache and D-cache already.  Implementing this
	 * function silences the warning in the default function.
	 */
}

int timer_init(void)
{
	gd->arch.timer_rate_hz = readl(BCMSTB_TIMER_FREQUENCY);

	return 0;
}

ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}

uint64_t get_ticks(void)
{
	gd->timebase_h = readl(BCMSTB_TIMER_HIGH);
	gd->timebase_l = readl(BCMSTB_TIMER_LOW);

	return ((uint64_t)gd->timebase_h << 32) | gd->timebase_l;
}

int board_late_init(void)
{
	debug("Arguments from prior stage bootloader:\n");
	debug("General Purpose Register 0: 0x%x\n", bcmstb_boot_parameters.r0);
	debug("General Purpose Register 1: 0x%x\n", bcmstb_boot_parameters.r1);
	debug("General Purpose Register 2: 0x%x\n", bcmstb_boot_parameters.r2);
	debug("General Purpose Register 3: 0x%x\n", bcmstb_boot_parameters.r3);
	debug("Stack Pointer Register:     0x%x\n", bcmstb_boot_parameters.sp);
	debug("Link Register:              0x%x\n", bcmstb_boot_parameters.lr);
	debug("Assuming timer frequency register at: 0x%p\n",
	      (void *)BCMSTB_TIMER_FREQUENCY);
	debug("Read timer frequency (in Hz): %ld\n", gd->arch.timer_rate_hz);
	debug("Prior stage provided DTB at: 0x%p\n",
	      (void *)prior_stage_fdt_address);

	/*
	 * Set fdtcontroladdr in the environment so that scripts can
	 * refer to it, for example, to reuse it for fdtaddr.
	 */
	env_set_hex("fdtcontroladdr", prior_stage_fdt_address);

	/*
	 * Do not set machid to the machine identifier value provided
	 * by the prior stage bootloader (bcmstb_boot_parameters.r1)
	 * because we're using a device tree to boot Linux.
	 */

	return 0;
}

void *board_fdt_blob_setup(int *err)
{
	*err = 0;
	/* Stored the DTB address there during our init */
	return (void *)prior_stage_fdt_address;
}
