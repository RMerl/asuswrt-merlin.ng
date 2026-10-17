// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */

#include <common.h>
#include <init.h>
#include <asm/io.h>

#define U8500_BOOTROM_BASE	0x90000000
#define U8500_ASIC_ID_LOC_V2	(U8500_BOOTROM_BASE + 0x1DBF4)

int print_cpuinfo(void)
{
	/* Convert ASIC ID to display string, e.g. 0x8520A0 => DB8520 V1.0 */
	u32 asicid = readl(U8500_ASIC_ID_LOC_V2);
	u32 cpu = (asicid >> 8) & 0xffff;
	u32 rev = asicid & 0xff;

	/* 0xA0 => 0x10 (V1.0) */
	if (rev >= 0xa0)
		rev -= 0x90;

	printf("CPU: ST-Ericsson DB%x V%d.%d\n", cpu, rev >> 4, rev & 0xf);
	return 0;
}
