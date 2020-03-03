/*
 * Emma Mobile EV2 processor support
 *
 * Copyright (C) 2012  Magnus Damm
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include "common.h"

static struct map_desc emev2_io_desc[] __initdata = {
#ifdef CONFIG_SMP
	/* 2M mapping for SCU + L2 controller */
	{
		.virtual	= 0xf0000000,
		.pfn		= __phys_to_pfn(0x1e000000),
		.length		= SZ_2M,
		.type		= MT_DEVICE
	},
#endif
};

static void __init emev2_map_io(void)
{
	iotable_init(emev2_io_desc, ARRAY_SIZE(emev2_io_desc));
}

static const char *const emev2_boards_compat_dt[] __initconst = {
	"renesas,emev2",
	NULL,
};

extern struct smp_operations emev2_smp_ops;

DT_MACHINE_START(EMEV2_DT, "Generic Emma Mobile EV2 (Flattened Device Tree)")
	.smp		= smp_ops(emev2_smp_ops),
	.map_io		= emev2_map_io,
	.init_early	= shmobile_init_delay,
	.init_late	= shmobile_init_late,
	.dt_compat	= emev2_boards_compat_dt,
MACHINE_END
