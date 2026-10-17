// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Sean Anderson <seanga2@gmail.com>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * U-Boot syscon driver for SiFive's Core Local Interruptor (CLINT).
 * The CLINT block holds memory-mapped control and status registers
 * associated with software and timer interrupts.
 */

#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/smp.h>
#include <asm/syscon.h>
#include <linux/err.h>

/* MSIP registers */
#define MSIP_REG(base, hart)		((ulong)(base) + (hart) * 4)

DECLARE_GLOBAL_DATA_PTR;

int riscv_init_ipi(void)
{
	int ret;
	struct udevice *dev;

	ret = uclass_get_device_by_driver(UCLASS_TIMER,
					  DM_DRIVER_GET(riscv_aclint_timer), &dev);
	if (ret)
		return ret;

	if (dev_get_driver_data(dev) != 0)
		gd->arch.aclint = dev_read_addr_ptr(dev);
	else
		gd->arch.aclint = syscon_get_first_range(RISCV_SYSCON_ACLINT);

	if (!gd->arch.aclint)
		return -EINVAL;

	return 0;
}

int riscv_send_ipi(int hart)
{
	writel(1, (void __iomem *)MSIP_REG(gd->arch.aclint, hart));

	return 0;
}

int riscv_clear_ipi(int hart)
{
	writel(0, (void __iomem *)MSIP_REG(gd->arch.aclint, hart));

	return 0;
}

int riscv_get_ipi(int hart, int *pending)
{
	*pending = readl((void __iomem *)MSIP_REG(gd->arch.aclint, hart));

	return 0;
}

static const struct udevice_id riscv_aclint_swi_ids[] = {
	{ .compatible = "riscv,aclint-mswi", .data = RISCV_SYSCON_ACLINT },
	{ }
};

U_BOOT_DRIVER(riscv_aclint_swi) = {
	.name		= "riscv_aclint_swi",
	.id		= UCLASS_SYSCON,
	.of_match	= riscv_aclint_swi_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};
