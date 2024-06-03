// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * U-Boot syscon driver for SiFive's Core Local Interruptor (CLINT).
 * The CLINT block holds memory-mapped control and status registers
 * associated with software and timer interrupts.
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/syscon.h>

/* MSIP registers */
#define MSIP_REG(base, hart)		((ulong)(base) + (hart) * 4)
/* mtime compare register */
#define MTIMECMP_REG(base, hart)	((ulong)(base) + 0x4000 + (hart) * 8)
/* mtime register */
#define MTIME_REG(base)			((ulong)(base) + 0xbff8)

DECLARE_GLOBAL_DATA_PTR;

#define CLINT_BASE_GET(void)						\
	do {								\
		long *ret;						\
									\
		if (!gd->arch.clint) {					\
			ret = syscon_get_first_range(RISCV_SYSCON_CLINT); \
			if (IS_ERR(ret))				\
				return PTR_ERR(ret);			\
			gd->arch.clint = ret;				\
		}							\
	} while (0)

int riscv_get_time(u64 *time)
{
	CLINT_BASE_GET();

	*time = readq((void __iomem *)MTIME_REG(gd->arch.clint));

	return 0;
}

int riscv_set_timecmp(int hart, u64 cmp)
{
	CLINT_BASE_GET();

	writeq(cmp, (void __iomem *)MTIMECMP_REG(gd->arch.clint, hart));

	return 0;
}

int riscv_send_ipi(int hart)
{
	CLINT_BASE_GET();

	writel(1, (void __iomem *)MSIP_REG(gd->arch.clint, hart));

	return 0;
}

int riscv_clear_ipi(int hart)
{
	CLINT_BASE_GET();

	writel(0, (void __iomem *)MSIP_REG(gd->arch.clint, hart));

	return 0;
}

static const struct udevice_id sifive_clint_ids[] = {
	{ .compatible = "riscv,clint0", .data = RISCV_SYSCON_CLINT },
	{ }
};

U_BOOT_DRIVER(sifive_clint) = {
	.name		= "sifive_clint",
	.id		= UCLASS_SYSCON,
	.of_match	= sifive_clint_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};
