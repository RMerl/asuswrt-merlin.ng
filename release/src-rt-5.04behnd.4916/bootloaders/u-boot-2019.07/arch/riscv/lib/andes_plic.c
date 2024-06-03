// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019, Rick Chen <rick@andestech.com>
 *
 * U-Boot syscon driver for Andes's Platform Level Interrupt Controller (PLIC).
 * The PLIC block holds memory-mapped claim and pending registers
 * associated with software interrupt.
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/syscon.h>
#include <cpu.h>

/* pending register */
#define PENDING_REG(base, hart)	((ulong)(base) + 0x1000 + (hart) * 8)
/* enable register */
#define ENABLE_REG(base, hart)	((ulong)(base) + 0x2000 + (hart) * 0x80)
/* claim register */
#define CLAIM_REG(base, hart)	((ulong)(base) + 0x200004 + (hart) * 0x1000)

#define ENABLE_HART_IPI         (0x80808080)
#define SEND_IPI_TO_HART(hart)  (0x80 >> (hart))

DECLARE_GLOBAL_DATA_PTR;
static int init_plic(void);

#define PLIC_BASE_GET(void)						\
	do {								\
		long *ret;						\
									\
		if (!gd->arch.plic) {					\
			ret = syscon_get_first_range(RISCV_SYSCON_PLIC); \
			if (IS_ERR(ret))				\
				return PTR_ERR(ret);			\
			gd->arch.plic = ret;				\
			init_plic();					\
		}							\
	} while (0)

static int enable_ipi(int harts)
{
	int i;
	int en = ENABLE_HART_IPI;

	for (i = 0; i < harts; i++) {
		en = en >> i;
		writel(en, (void __iomem *)ENABLE_REG(gd->arch.plic, i));
	}

	return 0;
}

static int init_plic(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_find_first_device(UCLASS_CPU, &dev);
	if (ret)
		return ret;

	if (ret == 0 && dev) {
		ret = cpu_get_count(dev);
		if (ret < 0)
			return ret;

		enable_ipi(ret);
		return 0;
	}

	return -ENODEV;
}

int riscv_send_ipi(int hart)
{
	PLIC_BASE_GET();

	writel(SEND_IPI_TO_HART(hart),
	       (void __iomem *)PENDING_REG(gd->arch.plic, gd->arch.boot_hart));

	return 0;
}

int riscv_clear_ipi(int hart)
{
	u32 source_id;

	PLIC_BASE_GET();

	source_id = readl((void __iomem *)CLAIM_REG(gd->arch.plic, hart));
	writel(source_id, (void __iomem *)CLAIM_REG(gd->arch.plic, hart));

	return 0;
}

static const struct udevice_id andes_plic_ids[] = {
	{ .compatible = "riscv,plic1", .data = RISCV_SYSCON_PLIC },
	{ }
};

U_BOOT_DRIVER(andes_plic) = {
	.name		= "andes_plic",
	.id		= UCLASS_SYSCON,
	.of_match	= andes_plic_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};
