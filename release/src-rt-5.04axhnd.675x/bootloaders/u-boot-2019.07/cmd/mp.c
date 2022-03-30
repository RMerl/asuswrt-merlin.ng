// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>

static int cpu_status_all(void)
{
	unsigned long cpuid;

	for (cpuid = 0; ; cpuid++) {
		if (!is_core_valid(cpuid)) {
			if (cpuid == 0) {
				printf("Core num: %lu is not valid\n", cpuid);
				return 1;
			}
			break;
		}
		cpu_status(cpuid);
	}

	return 0;
}

static int
cpu_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long cpuid;

	if (argc == 2 && strncmp(argv[1], "status", 6) == 0)
		  return cpu_status_all();

	if (argc < 3)
		return CMD_RET_USAGE;

	cpuid = simple_strtoul(argv[1], NULL, 10);
	if (!is_core_valid(cpuid)) {
		printf ("Core num: %lu is not valid\n",	cpuid);
		return 1;
	}


	if (argc == 3) {
		if (strncmp(argv[2], "reset", 5) == 0)
			cpu_reset(cpuid);
		else if (strncmp(argv[2], "status", 6) == 0)
			cpu_status(cpuid);
		else if (strncmp(argv[2], "disable", 7) == 0)
			return cpu_disable(cpuid);
		else
			return CMD_RET_USAGE;

		return 0;
	}

	/* 4 or greater, make sure its release */
	if (strncmp(argv[2], "release", 7) != 0)
		return CMD_RET_USAGE;

	if (cpu_release(cpuid, argc - 3, argv + 3))
		return CMD_RET_USAGE;

	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char cpu_help_text[] =
	    "<num> reset                 - Reset cpu <num>\n"
	"cpu status                      - Status of all cpus\n"
	"cpu <num> status                - Status of cpu <num>\n"
	"cpu <num> disable               - Disable cpu <num>\n"
	"cpu <num> release <addr> [args] - Release cpu <num> at <addr> with [args]"
#ifdef CONFIG_PPC
	"\n"
	"                         [args] : <pir> <r3> <r6>\n" \
	"                                   pir - processor id (if writeable)\n" \
	"                                    r3 - value for gpr 3\n" \
	"                                    r6 - value for gpr 6\n" \
	"\n" \
	"     Use '-' for any arg if you want the default value.\n" \
	"     Default for r3 is <num> and r6 is 0\n" \
	"\n" \
	"     When cpu <num> is released r4 and r5 = 0.\n" \
	"     r7 will contain the size of the initial mapped area"
#endif
	"";
#endif

U_BOOT_CMD(
	cpu, CONFIG_SYS_MAXARGS, 1, cpu_cmd,
	"Multiprocessor CPU boot manipulation and release", cpu_help_text
);
