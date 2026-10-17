// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <cpu.h>
#include <display_options.h>
#include <dm.h>
#include <errno.h>

static const char *cpu_feature_name[CPU_FEAT_COUNT] = {
	"L1 cache",
	"MMU",
	"Microcode",
	"Device ID",
};

static int print_cpu_list(bool detail)
{
	struct udevice *dev;
	char buf[100];

	for (uclass_first_device(UCLASS_CPU, &dev);
		     dev;
		     uclass_next_device(&dev)) {
		struct cpu_plat *plat = dev_get_parent_plat(dev);
		struct cpu_info info;
		bool first = true;
		int ret, i;

		ret = cpu_get_desc(dev, buf, sizeof(buf));
		printf("%3d: %-10s %s\n", dev_seq(dev), dev->name,
		       ret ? "<no description>" : buf);
		if (!detail)
			continue;
		ret = cpu_get_info(dev, &info);
		if (ret) {
			printf("\t(no detail available");
			if (ret != -ENOSYS)
				printf(": err=%d", ret);
			printf(")\n");
			continue;
		}
		printf("\tID = %d, freq = ", plat->cpu_id);
		print_freq(info.cpu_freq, "");
		for (i = 0; i < CPU_FEAT_COUNT; i++) {
			if (info.features & (1 << i)) {
				printf("%s%s", first ? ": " : ", ",
				       cpu_feature_name[i]);
				first = false;
			}
		}
		printf("\n");
		if (info.features & (1 << CPU_FEAT_UCODE))
			printf("\tMicrocode version %#x\n",
			       plat->ucode_version);
		if (info.features & (1 << CPU_FEAT_DEVICE_ID))
			printf("\tDevice ID %#lx\n", plat->device_id);
	}

	return 0;
}

static int do_cpu_list(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	if (print_cpu_list(false))
		return CMD_RET_FAILURE;

	return 0;
}

static int do_cpu_detail(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	if (print_cpu_list(true))
		return CMD_RET_FAILURE;

	return 0;
}

U_BOOT_LONGHELP(cpu,
	"list	- list available CPUs\n"
	"cpu detail	- show CPU detail");

U_BOOT_CMD_WITH_SUBCMDS(cpu, "display information about CPUs", cpu_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_cpu_list),
	U_BOOT_SUBCMD_MKENT(detail, 1, 0, do_cpu_detail));
