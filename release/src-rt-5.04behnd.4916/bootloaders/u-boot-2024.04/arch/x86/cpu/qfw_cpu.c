// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <malloc.h>
#include <qfw.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <dm/root.h>

int qemu_cpu_fixup(void)
{
	int ret;
	int cpu_num;
	int cpu_online;
	struct uclass *uc;
	struct udevice *dev, *pdev, *qfwdev;
	struct cpu_plat *plat;
	char *cpu;

	/* This will cause the CPUs devices to be bound */
	ret = uclass_get(UCLASS_CPU, &uc);
	if (ret)
		return ret;

	/* first we need to find '/cpus' */
	for (device_find_first_child(dm_root(), &pdev);
	     pdev;
	     device_find_next_child(&pdev)) {
		if (!strcmp(pdev->name, "cpus"))
			break;
	}
	if (!pdev) {
		printf("unable to find cpus device\n");
		return -ENODEV;
	}

	/* get qfw dev */
	ret = qfw_get_dev(&qfwdev);
	if (ret) {
		printf("unable to find qfw device\n");
		return ret;
	}

	/* calculate cpus that are already bound */
	cpu_num = 0;
	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		cpu_num++;
	}

	/* get actual cpu number */
	cpu_online = qfw_online_cpus(qfwdev);
	if (cpu_online < 0) {
		printf("unable to get online cpu number: %d\n", cpu_online);
		return cpu_online;
	}

	/* bind addtional cpus */
	dev = NULL;
	for (; cpu_num < cpu_online; cpu_num++) {
		/*
		 * allocate device name here as device_bind_driver() does
		 * not copy device name, 8 bytes are enough for
		 * sizeof("cpu@") + 3 digits cpu number + '\0'
		 */
		cpu = malloc(8);
		if (!cpu) {
			printf("unable to allocate device name\n");
			return -ENOMEM;
		}
		sprintf(cpu, "cpu@%d", cpu_num);
		ret = device_bind_driver(pdev, "cpu_qemu", cpu, &dev);
		if (ret) {
			printf("binding cpu@%d failed: %d\n", cpu_num, ret);
			return ret;
		}
		plat = dev_get_parent_plat(dev);
		plat->cpu_id = cpu_num;
	}
	return 0;
}
