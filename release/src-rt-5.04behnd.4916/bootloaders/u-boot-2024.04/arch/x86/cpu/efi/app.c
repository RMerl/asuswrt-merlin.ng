// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <cpu_func.h>
#include <fdtdec.h>
#include <init.h>
#include <netdev.h>

int arch_cpu_init(void)
{
	return x86_cpu_init_f();
}

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	return default_print_cpuinfo();
}

void board_final_init(void)
{
}

int misc_init_r(void)
{
	return 0;
}
