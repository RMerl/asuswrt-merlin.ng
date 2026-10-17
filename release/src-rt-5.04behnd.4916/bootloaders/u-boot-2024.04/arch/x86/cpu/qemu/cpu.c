// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Miao Yan <yanmiaobest@gmail.com>
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <qfw.h>
#include <asm/cpu.h>

int cpu_qemu_get_desc(const struct udevice *dev, char *buf, int size)
{
	if (size < CPU_MAX_NAME_LEN)
		return -ENOSPC;

	cpu_get_name(buf);

	return 0;
}

static int cpu_qemu_get_count(const struct udevice *dev)
{
	int ret;
	struct udevice *qfw_dev;

	ret = qfw_get_dev(&qfw_dev);
	if (ret)
		return ret;

	return qfw_online_cpus(qfw_dev);
}

static const struct cpu_ops cpu_qemu_ops = {
	.get_desc	= cpu_qemu_get_desc,
	.get_count	= cpu_qemu_get_count,
};

static const struct udevice_id cpu_qemu_ids[] = {
	{ .compatible = "cpu-qemu" },
	{ }
};

U_BOOT_DRIVER(cpu_qemu_drv) = {
	.name		= "cpu_qemu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_qemu_ids,
	.ops		= &cpu_qemu_ops,
};
