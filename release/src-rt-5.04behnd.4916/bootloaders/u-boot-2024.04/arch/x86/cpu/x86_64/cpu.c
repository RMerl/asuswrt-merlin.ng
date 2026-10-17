// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <init.h>
#include <asm/cpu.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int cpu_has_64bit(void)
{
	return true;
}

void enable_caches(void)
{
	/* Not implemented */
}

void disable_caches(void)
{
	/* Not implemented */
}

int dcache_status(void)
{
	return true;
}

int x86_mp_init(void)
{
	/* Not implemented */
	return 0;
}

int x86_cpu_reinit_f(void)
{
	/* set the vendor to Intel so that native_calibrate_tsc() works */
	gd->arch.x86_vendor = X86_VENDOR_INTEL;
	gd->arch.has_mtrr = true;

	return 0;
}

int cpu_phys_address_size(void)
{
	return CONFIG_CPU_ADDR_BITS;
}

int x86_cpu_init_f(void)
{
	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	/* this was already done in SPL */
}
#endif
