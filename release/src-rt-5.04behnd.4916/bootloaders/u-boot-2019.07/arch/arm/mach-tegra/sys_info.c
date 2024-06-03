// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <linux/ctype.h>

static void upstring(char *s)
{
	while (*s) {
		*s = toupper(*s);
		s++;
	}
}

/* Print CPU information */
int print_cpuinfo(void)
{
	char soc_name[10];

	strncpy(soc_name, CONFIG_SYS_SOC, 10);
	upstring(soc_name);
	puts(soc_name);
	puts("\n");

	/* TBD: Add printf of major/minor rev info, stepping, etc. */
	return 0;
}
