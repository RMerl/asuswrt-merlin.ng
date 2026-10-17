// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Cache support: switch on or off, get status
 */
#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <linux/compiler.h>

static int parse_argv(const char *);

void __weak invalidate_icache_all(void)
{
	/* please define arch specific invalidate_icache_all */
	puts("No arch specific invalidate_icache_all available!\n");
}

__weak void noncached_set_region(void)
{
}

static int do_icache(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	switch (argc) {
	case 2:			/* on / off / flush */
		switch (parse_argv(argv[1])) {
		case 0:
			icache_disable();
			break;
		case 1:
			icache_enable();
			break;
		case 2:
			invalidate_icache_all();
			break;
		default:
			return CMD_RET_USAGE;
		}
		break;
	case 1:			/* get status */
		printf("Instruction Cache is %s\n",
			icache_status() ? "ON" : "OFF");
		return 0;
	default:
		return CMD_RET_USAGE;
	}
	return 0;
}

void __weak flush_dcache_all(void)
{
	puts("No arch specific flush_dcache_all available!\n");
	/* please define arch specific flush_dcache_all */
}

static int do_dcache(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	switch (argc) {
	case 2:			/* on / off / flush */
		switch (parse_argv(argv[1])) {
		case 0:
			dcache_disable();
			break;
		case 1:
			dcache_enable();
			noncached_set_region();
			break;
		case 2:
			flush_dcache_all();
			break;
		default:
			return CMD_RET_USAGE;
		}
		break;
	case 1:			/* get status */
		printf("Data (writethrough) Cache is %s\n",
			dcache_status() ? "ON" : "OFF");
		return 0;
	default:
		return CMD_RET_USAGE;
	}
	return 0;
}

static int parse_argv(const char *s)
{
	if (strcmp(s, "flush") == 0)
		return 2;
	else if (strcmp(s, "on") == 0)
		return 1;
	else if (strcmp(s, "off") == 0)
		return 0;

	return -1;
}


U_BOOT_CMD(
	icache,   2,   1,     do_icache,
	"enable or disable instruction cache",
	"[on, off, flush]\n"
	"    - enable, disable, or flush instruction cache"
);

U_BOOT_CMD(
	dcache,   2,   1,     do_dcache,
	"enable or disable data cache",
	"[on, off, flush]\n"
	"    - enable, disable, or flush data (writethrough) cache"
);
