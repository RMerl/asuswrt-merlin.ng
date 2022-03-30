// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <asm/cpu.h>

DECLARE_GLOBAL_DATA_PTR;

int cpu_x86_bind(struct udevice *dev)
{
	struct cpu_platdata *plat = dev_get_parent_platdata(dev);
	struct cpuid_result res;

	plat->cpu_id = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				      "intel,apic-id", -1);
	plat->family = gd->arch.x86;
	res = cpuid(1);
	plat->id[0] = res.eax;
	plat->id[1] = res.edx;

	return 0;
}

int cpu_x86_get_vendor(struct udevice *dev, char *buf, int size)
{
	const char *vendor = cpu_vendor_name(gd->arch.x86_vendor);

	if (size < (strlen(vendor) + 1))
		return -ENOSPC;

	strcpy(buf, vendor);

	return 0;
}

int cpu_x86_get_desc(struct udevice *dev, char *buf, int size)
{
	char *ptr;

	if (size < CPU_MAX_NAME_LEN)
		return -ENOSPC;

	ptr = cpu_get_name(buf);
	if (ptr != buf)
		strcpy(buf, ptr);

	return 0;
}

static int cpu_x86_get_count(struct udevice *dev)
{
	int node, cpu;
	int num = 0;

	node = fdt_path_offset(gd->fdt_blob, "/cpus");
	if (node < 0)
		return -ENOENT;

	for (cpu = fdt_first_subnode(gd->fdt_blob, node);
	     cpu >= 0;
	     cpu = fdt_next_subnode(gd->fdt_blob, cpu)) {
		const char *device_type;

		device_type = fdt_getprop(gd->fdt_blob, cpu,
					  "device_type", NULL);
		if (!device_type)
			continue;
		if (strcmp(device_type, "cpu") == 0)
			num++;
	}

	return num;
}

static const struct cpu_ops cpu_x86_ops = {
	.get_desc	= cpu_x86_get_desc,
	.get_count	= cpu_x86_get_count,
	.get_vendor	= cpu_x86_get_vendor,
};

static const struct udevice_id cpu_x86_ids[] = {
	{ .compatible = "cpu-x86" },
	{ }
};

U_BOOT_DRIVER(cpu_x86_drv) = {
	.name		= "cpu_x86",
	.id		= UCLASS_CPU,
	.of_match	= cpu_x86_ids,
	.bind		= cpu_x86_bind,
	.ops		= &cpu_x86_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
