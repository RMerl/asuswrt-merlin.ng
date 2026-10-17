// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <command.h>
#include <cyclic.h>
#include <div64.h>
#include <malloc.h>
#include <linux/delay.h>

struct cyclic_demo_info {
	uint delay_us;
};

static void cyclic_demo(void *ctx)
{
	struct cyclic_demo_info *info = ctx;

	/* Just a small dummy delay here */
	udelay(info->delay_us);
}

static int do_cyclic_demo(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct cyclic_demo_info *info;
	struct cyclic_info *cyclic;
	uint time_ms;

	if (argc < 3)
		return CMD_RET_USAGE;

	info = malloc(sizeof(struct cyclic_demo_info));
	if (!info) {
		printf("out of memory\n");
                return CMD_RET_FAILURE;
	}

	time_ms = simple_strtoul(argv[1], NULL, 0);
	info->delay_us = simple_strtoul(argv[2], NULL, 0);

	/* Register demo cyclic function */
	cyclic = cyclic_register(cyclic_demo, time_ms * 1000, "cyclic_demo",
				 info);
	if (!cyclic)
		printf("Registering of cyclic_demo failed\n");

	printf("Registered function \"%s\" to be executed all %dms\n",
	       "cyclic_demo", time_ms);

	return 0;
}

static int do_cyclic_list(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct cyclic_info *cyclic;
	struct hlist_node *tmp;
	u64 cnt, freq;

	hlist_for_each_entry_safe(cyclic, tmp, cyclic_get_list(), list) {
		cnt = cyclic->run_cnt * 1000000ULL * 100ULL;
		freq = lldiv(cnt, timer_get_us() - cyclic->start_time_us);
		printf("function: %s, cpu-time: %lld us, frequency: %lld.%02d times/s\n",
		       cyclic->name, cyclic->cpu_time_us,
		       lldiv(freq, 100), do_div(freq, 100));
	}

	return 0;
}

U_BOOT_LONGHELP(cyclic,
	"demo <cycletime_ms> <delay_us> - register cyclic demo function\n"
	"cyclic list - list cyclic functions\n");

U_BOOT_CMD_WITH_SUBCMDS(cyclic, "Cyclic", cyclic_help_text,
	U_BOOT_SUBCMD_MKENT(demo, 3, 1, do_cyclic_demo),
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_cyclic_list));
