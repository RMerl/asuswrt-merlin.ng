// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <cpu.h>

static int cpu_sandbox_get_desc(const struct udevice *dev, char *buf, int size)
{
	snprintf(buf, size, "LEG Inc. SuperMegaUltraTurbo CPU No. 1");

	return 0;
}

static int cpu_sandbox_get_info(const struct udevice *dev,
				struct cpu_info *info)
{
	info->cpu_freq = 42 * 42 * 42 * 42 * 42;
	info->features = 0x42424242;
	info->address_width = IS_ENABLED(CONFIG_PHYS_64BIT) ? 64 : 32;

	return 0;
}

static int cpu_sandbox_get_count(const struct udevice *dev)
{
	return 42;
}

static int cpu_sandbox_get_vendor(const struct udevice *dev, char *buf,
				  int size)
{
	snprintf(buf, size, "Languid Example Garbage Inc.");

	return 0;
}

static const char *cpu_current = "cpu@1";

void cpu_sandbox_set_current(const char *name)
{
	cpu_current = name;
}

static int cpu_sandbox_is_current(struct udevice *dev)
{
	if (!strcmp(dev->name, cpu_current))
		return 1;

	return 0;
}

static const struct cpu_ops cpu_sandbox_ops = {
	.get_desc = cpu_sandbox_get_desc,
	.get_info = cpu_sandbox_get_info,
	.get_count = cpu_sandbox_get_count,
	.get_vendor = cpu_sandbox_get_vendor,
	.is_current = cpu_sandbox_is_current,
};

static int cpu_sandbox_bind(struct udevice *dev)
{
	int ret;
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	/* first examine the property in current cpu node */
	ret = dev_read_u32(dev, "timebase-frequency", &plat->timebase_freq);
	/* if not found, then look at the parent /cpus node */
	if (ret)
		ret = dev_read_u32(dev->parent, "timebase-frequency",
				   &plat->timebase_freq);

	return ret;
}

static int cpu_sandbox_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id cpu_sandbox_ids[] = {
	{ .compatible = "sandbox,cpu_sandbox" },
	{ }
};

U_BOOT_DRIVER(cpu_sandbox) = {
	.name           = "cpu_sandbox",
	.id             = UCLASS_CPU,
	.ops		= &cpu_sandbox_ops,
	.of_match       = cpu_sandbox_ids,
	.bind		= cpu_sandbox_bind,
	.probe          = cpu_sandbox_probe,
};
