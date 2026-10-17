// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Xilinx, Inc. - Michal Simek
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <linux/err.h>
#include <linux/stringify.h>

static int microblaze_sysreset_request(struct udevice *dev,
				       enum sysreset_t type)
{
	puts("Microblaze soft reset sysreset\n");
	__asm__ __volatile__ (
	    "mts rmsr, r0;" \
	    "brai " __stringify(CONFIG_XILINX_MICROBLAZE0_VECTOR_BASE_ADDR));

	return -EINPROGRESS;
}

static struct sysreset_ops microblaze_sysreset = {
	.request = microblaze_sysreset_request,
};

U_BOOT_DRIVER(sysreset_microblaze) = {
	.id	= UCLASS_SYSRESET,
	.name	= "mb_soft_reset",
	.ops	= &microblaze_sysreset,
};
