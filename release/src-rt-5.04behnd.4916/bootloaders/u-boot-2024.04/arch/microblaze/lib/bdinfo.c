// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022, Ovidiu Panait <ovpanait@gmail.com>
 */
#include <init.h>
#include <asm/cpuinfo.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void arch_print_bdinfo(void)
{
	struct microblaze_cpuinfo *ci = gd_cpuinfo();

	if (ci->icache_size) {
		bdinfo_print_size("icache", ci->icache_size);
		bdinfo_print_size("icache line", ci->icache_line_length);
	}

	if (ci->dcache_size) {
		bdinfo_print_size("dcache", ci->dcache_size);
		bdinfo_print_size("dcache line", ci->dcache_line_length);
	}
}
