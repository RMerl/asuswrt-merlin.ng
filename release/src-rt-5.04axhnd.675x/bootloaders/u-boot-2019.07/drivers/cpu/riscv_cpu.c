// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

DECLARE_GLOBAL_DATA_PTR;

static int riscv_cpu_get_desc(struct udevice *dev, char *buf, int size)
{
	const char *isa;

	isa = dev_read_string(dev, "riscv,isa");
	if (size < (strlen(isa) + 1))
		return -ENOSPC;

	strcpy(buf, isa);

	return 0;
}

static int riscv_cpu_get_info(struct udevice *dev, struct cpu_info *info)
{
	const char *mmu;

	dev_read_u32(dev, "clock-frequency", (u32 *)&info->cpu_freq);

	mmu = dev_read_string(dev, "mmu-type");
	if (!mmu)
		info->features |= BIT(CPU_FEAT_MMU);

	return 0;
}

static int riscv_cpu_get_count(struct udevice *dev)
{
	ofnode node;
	int num = 0;

	ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
		const char *device_type;

		device_type = ofnode_read_string(node, "device_type");
		if (!device_type)
			continue;
		if (strcmp(device_type, "cpu") == 0)
			num++;
	}

	return num;
}

static int riscv_cpu_bind(struct udevice *dev)
{
	struct cpu_platdata *plat = dev_get_parent_platdata(dev);
	struct driver *drv;
	int ret;

	/* save the hart id */
	plat->cpu_id = dev_read_addr(dev);
	/* first examine the property in current cpu node */
	ret = dev_read_u32(dev, "timebase-frequency", &plat->timebase_freq);
	/* if not found, then look at the parent /cpus node */
	if (ret)
		dev_read_u32(dev->parent, "timebase-frequency",
			     &plat->timebase_freq);

	/*
	 * Bind riscv-timer driver on boot hart.
	 *
	 * We only instantiate one timer device which is enough for U-Boot.
	 * Pass the "timebase-frequency" value as the driver data for the
	 * timer device.
	 *
	 * Return value is not checked since it's possible that the timer
	 * driver is not included.
	 */
	if (plat->cpu_id == gd->arch.boot_hart && plat->timebase_freq) {
		drv = lists_driver_lookup_name("riscv_timer");
		if (!drv) {
			debug("Cannot find the timer driver, not included?\n");
			return 0;
		}

		device_bind_with_driver_data(dev, drv, "riscv_timer",
					     plat->timebase_freq, ofnode_null(),
					     NULL);
	}

	return 0;
}

static const struct cpu_ops riscv_cpu_ops = {
	.get_desc	= riscv_cpu_get_desc,
	.get_info	= riscv_cpu_get_info,
	.get_count	= riscv_cpu_get_count,
};

static const struct udevice_id riscv_cpu_ids[] = {
	{ .compatible = "riscv" },
	{ }
};

U_BOOT_DRIVER(riscv_cpu) = {
	.name = "riscv_cpu",
	.id = UCLASS_CPU,
	.of_match = riscv_cpu_ids,
	.bind = riscv_cpu_bind,
	.ops = &riscv_cpu_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
