// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019, Rick Chen <rick@andestech.com>
 *
 * U-Boot syscon driver for Andes's Platform Level Machine Timer (PLMT).
 * The PLMT block holds memory-mapped mtime register
 * associated with timer tick.
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/syscon.h>

/* mtime register */
#define MTIME_REG(base)			((ulong)(base))

DECLARE_GLOBAL_DATA_PTR;

#define PLMT_BASE_GET(void)						\
	do {								\
		long *ret;						\
									\
		if (!gd->arch.plmt) {					\
			ret = syscon_get_first_range(RISCV_SYSCON_PLMT); \
			if (IS_ERR(ret))				\
				return PTR_ERR(ret);			\
			gd->arch.plmt = ret;				\
		}							\
	} while (0)

int riscv_get_time(u64 *time)
{
	PLMT_BASE_GET();

	*time = readq((void __iomem *)MTIME_REG(gd->arch.plmt));

	return 0;
}

static const struct udevice_id andes_plmt_ids[] = {
	{ .compatible = "riscv,plmt0", .data = RISCV_SYSCON_PLMT },
	{ }
};

U_BOOT_DRIVER(andes_plmt) = {
	.name		= "andes_plmt",
	.id		= UCLASS_SYSCON,
	.of_match	= andes_plmt_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};
