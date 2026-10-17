// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2020 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#include <common.h>
#include <init.h>
#include <soc.h>

int print_cpuinfo(void)
{
	struct udevice *soc;
	char name[SOC_MAX_STR_SIZE];
	int ret;

	ret = soc_get(&soc);
	if (ret) {
		printf("CPU:   UNKNOWN\n");
		return 0;
	}

	ret = soc_get_family(soc, name, SOC_MAX_STR_SIZE);
	if (ret)
		printf("CPU:   %s\n", name);

	ret = soc_get_revision(soc, name, SOC_MAX_STR_SIZE);
	if (ret)
		printf("Silicon: %s\n", name);

	ret = soc_get_machine(soc, name, SOC_MAX_STR_SIZE);
	if (ret)
		printf("Chip:  %s\n", name);

	return 0;
}
