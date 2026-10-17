// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019, Rick Chen <rick@andestech.com>
 *
 * U-Boot syscon driver for Andes' PLICSW
 * The PLICSW block is an Andes-specific design for software interrupts,
 * contains memory-mapped priority, enable, claim and pending registers
 * similar to RISC-V PLIC.
 */

#include <dm.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/syscon.h>
#include <cpu.h>
#include <linux/err.h>

/* pending register */
#define PENDING_REG(base, hart)	((ulong)(base) + 0x1000 + 4 * (((hart) + 1) / 32))
/* enable register */
#define ENABLE_REG(base, hart)	((ulong)(base) + 0x2000 + (hart) * 0x80 + 4 * (((hart) + 1) / 32))
/* claim register */
#define CLAIM_REG(base, hart)	((ulong)(base) + 0x200004 + (hart) * 0x1000)
/* priority register */
#define PRIORITY_REG(base)	((ulong)(base) + PLICSW_PRIORITY_BASE)

/* Bit 0 of PLIC-SW pending array is hardwired to zero, so we start from bit 1 */
#define PLICSW_PRIORITY_BASE        0x4

DECLARE_GLOBAL_DATA_PTR;

static int enable_ipi(int hart)
{
	u32 enable_bit = (hart + 1) % 32;

	writel(BIT(enable_bit), (void __iomem *)ENABLE_REG(gd->arch.plicsw, hart));

	return 0;
}

static void init_priority_ipi(int hart_num)
{
	u32 *priority = (void *)PRIORITY_REG(gd->arch.plicsw);

	for (int i = 0; i < hart_num; i++)
		writel(1, &priority[i]);

	return;
}

int riscv_init_ipi(void)
{
	int ret;
	int hart_num = 0;
	long *base = syscon_get_first_range(RISCV_SYSCON_PLICSW);
	ofnode node;
	struct udevice *dev;
	u32 reg;

	if (IS_ERR(base))
		return PTR_ERR(base);
	gd->arch.plicsw = base;

	ret = uclass_find_first_device(UCLASS_CPU, &dev);
	if (ret)
		return ret;
	if (!dev)
		return -ENODEV;

	ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
		const char *device_type;

		device_type = ofnode_read_string(node, "device_type");
		if (!device_type)
			continue;

		if (strcmp(device_type, "cpu"))
			continue;

		/* skip if hart is marked as not available */
		if (!ofnode_is_enabled(node))
			continue;

		/* read hart ID of CPU */
		ret = ofnode_read_u32(node, "reg", &reg);
		if (ret == 0)
			enable_ipi(reg);
		hart_num++;
	}

	init_priority_ipi(hart_num);
	return 0;
}

int riscv_send_ipi(int hart)
{
	u32 interrupt_id = hart + 1;
	u32 pending_bit  = interrupt_id % 32;

	writel(BIT(pending_bit), (void __iomem *)PENDING_REG(gd->arch.plicsw, hart));

	return 0;
}

int riscv_clear_ipi(int hart)
{
	u32 source_id;

	source_id = readl((void __iomem *)CLAIM_REG(gd->arch.plicsw, hart));
	writel(source_id, (void __iomem *)CLAIM_REG(gd->arch.plicsw, hart));

	return 0;
}

int riscv_get_ipi(int hart, int *pending)
{
	u32 interrupt_id = hart + 1;
	u32 pending_bit  = interrupt_id % 32;

	*pending = readl((void __iomem *)PENDING_REG(gd->arch.plicsw, hart));
	*pending = !!(*pending & BIT(pending_bit));

	return 0;
}

static const struct udevice_id andes_plicsw_ids[] = {
	{ .compatible = "andestech,plicsw", .data = RISCV_SYSCON_PLICSW },
	{ }
};

U_BOOT_DRIVER(andes_plicsw) = {
	.name		= "andes_plicsw",
	.id		= UCLASS_SYSCON,
	.of_match	= andes_plicsw_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};
