// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */
#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

/* Default is s5pc100 */
unsigned int s5p_cpu_id = 0xC100;
/* Default is EVT1 */
unsigned int s5p_cpu_rev = 1;

#ifdef CONFIG_ARCH_CPU_INIT
int arch_cpu_init(void)
{
	s5p_set_cpu_id();

	return 0;
}
#endif

u32 get_device_type(void)
{
	return s5p_cpu_id;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	const char *cpu_model;
	int len;

	/* For SoC with no real CPU ID in naming convention. */
	cpu_model = fdt_getprop(gd->fdt_blob, 0, "cpu-model", &len);
	if (cpu_model)
		printf("CPU:   %.*s @ ", len, cpu_model);
	else
		printf("CPU:   %s%X @ ", s5p_get_cpu_name(), s5p_cpu_id);

	print_freq(get_arm_clk(), "\n");

	return 0;
}
#endif
