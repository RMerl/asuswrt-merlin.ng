// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Gabriel Huau <contact@huau-gabriel.fr>
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/imx-regs.h>

#define MAX_CPUS 4
static struct src *src = (struct src *)SRC_BASE_ADDR;

static uint32_t cpu_reset_mask[MAX_CPUS] = {
	0, /* We don't really want to modify the cpu0 */
	SRC_SCR_CORE_1_RESET_MASK,
	SRC_SCR_CORE_2_RESET_MASK,
	SRC_SCR_CORE_3_RESET_MASK
};

static uint32_t cpu_ctrl_mask[MAX_CPUS] = {
	0, /* We don't really want to modify the cpu0 */
	SRC_SCR_CORE_1_ENABLE_MASK,
	SRC_SCR_CORE_2_ENABLE_MASK,
	SRC_SCR_CORE_3_ENABLE_MASK
};

int cpu_reset(u32 nr)
{
	/* Software reset of the CPU N */
	src->scr |= cpu_reset_mask[nr];
	return 0;
}

int cpu_status(u32 nr)
{
	printf("core %d => %d\n", nr, !!(src->scr & cpu_ctrl_mask[nr]));
	return 0;
}

int cpu_release(u32 nr, int argc, char *const argv[])
{
	uint32_t boot_addr;

	boot_addr = hextoul(argv[0], NULL);

	switch (nr) {
	case 1:
		src->gpr3 = boot_addr;
		break;
	case 2:
		src->gpr5 = boot_addr;
		break;
	case 3:
		src->gpr7 = boot_addr;
		break;
	default:
		return 1;
	}

	/* CPU N is ready to start */
	src->scr |= cpu_ctrl_mask[nr];

	return 0;
}

int is_core_valid(unsigned int core)
{
	uint32_t nr_cores = get_nr_cpus();

	if (core > nr_cores)
		return 0;

	return 1;
}

int cpu_disable(u32 nr)
{
	/* Disable the CPU N */
	src->scr &= ~cpu_ctrl_mask[nr];
	return 0;
}
