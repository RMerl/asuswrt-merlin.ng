// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for all histb boards
 *
 * (C) Copyright 2023 Yang Xiwen <forbidden405@outlook.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <init.h>
#include <asm/system.h>

int __weak board_init(void)
{
	return 0;
}

int __weak dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int __weak dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

void __weak reset_cpu(void)
{
	psci_system_reset();
}
