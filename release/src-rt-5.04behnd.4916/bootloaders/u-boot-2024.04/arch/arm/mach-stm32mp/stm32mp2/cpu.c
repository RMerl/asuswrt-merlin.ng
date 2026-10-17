// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <clk.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <env_internal.h>
#include <init.h>
#include <misc.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <asm/system.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <dm/uclass.h>

/*
 * early TLB into the .data section so that it not get cleared
 * with 16kB alignment
 */
#define EARLY_TLB_SIZE 0xA000
u8 early_tlb[EARLY_TLB_SIZE] __section(".data") __aligned(0x4000);

/*
 * initialize the MMU and activate cache in U-Boot pre-reloc stage
 * MMU/TLB is updated in enable_caches() for U-Boot after relocation
 */
static void early_enable_caches(void)
{
	if (CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		return;

	if (!(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))) {
		gd->arch.tlb_size = EARLY_TLB_SIZE;
		gd->arch.tlb_addr = (unsigned long)&early_tlb;
	}
	/* enable MMU (default configuration) */
	dcache_enable();
}

/*
 * Early system init
 */
int arch_cpu_init(void)
{
	icache_enable();
	early_enable_caches();

	return 0;
}

void enable_caches(void)
{
	/* deactivate the data cache, early enabled in arch_cpu_init() */
	dcache_disable();
	/*
	 * Force the call of setup_all_pgtables() in mmu_setup() by clearing tlb_fillptr
	 * to update the TLB location udpated in board_f.c::reserve_mmu
	 */
	gd->arch.tlb_fillptr = 0;
	dcache_enable();
}

int arch_misc_init(void)
{
	setup_serial_number();
	setup_mac_address();

	return 0;
}

/*
 * Force data-section, as .bss will not be valid
 * when save_boot_params is invoked.
 */
static uintptr_t nt_fw_dtb __section(".data");

uintptr_t get_stm32mp_bl2_dtb(void)
{
	return nt_fw_dtb;
}

/*
 * Save the FDT address provided by TF-A in r2 at boot time
 * This function is called from start.S
 */
void save_boot_params(unsigned long r0, unsigned long r1, unsigned long r2,
		      unsigned long r3)
{
	nt_fw_dtb = r2;

	save_boot_params_ret();
}
