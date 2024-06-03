// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <cpu.h>

int cpu_sandbox_get_desc(struct udevice *dev, char *buf, int size)
{
	snprintf(buf, size, "LEG Inc. SuperMegaUltraTurbo CPU No. 1");

	return 0;
}

int cpu_sandbox_get_info(struct udevice *dev, struct cpu_info *info)
{
	info->cpu_freq = 42 * 42 * 42 * 42 * 42;
	info->features = 0x42424242;

	return 0;
}

int cpu_sandbox_get_count(struct udevice *dev)
{
	return 42;
}

int cpu_sandbox_get_vendor(struct udevice *dev, char *buf, int size)
{
	snprintf(buf, size, "Languid Example Garbage Inc.");

	return 0;
}

static const struct cpu_ops cpu_sandbox_ops = {
	.get_desc = cpu_sandbox_get_desc,
	.get_info = cpu_sandbox_get_info,
	.get_count = cpu_sandbox_get_count,
	.get_vendor = cpu_sandbox_get_vendor,
};

int cpu_sandbox_probe(struct udevice *dev)
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
	.probe          = cpu_sandbox_probe,
};
