/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2023 Broadcom Ltd.
 */

/**********************************************************************
 *	
 * FAILSAFE boot implementation
 *	
 *********************************************************************	
 */

#include <linux/ctype.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include "bcm_bootstate.h"
#include <bcm_failsafe_boot.h>

static uint32_t fbr_current __attribute__((section(".data"))) ;

void bcm_failsafe_boot_check(void)
	
	{
	if (( bcmbca_get_reset_status() & SW_RESET_STATUS)) {
		u32 reset_reason =  bcmbca_get_boot_reason();
		if ( (reset_reason & BCM_BOOT_PHASE_MASK) == BCM_BOOT_PHASE_TPL || 
				(reset_reason & BCM_BOOT_PHASE_MASK) == BCM_BOOT_PHASE_SPL) {
			printf("!!! WARNING: FAILSAFE search offset 0%x\n", (fbr_current << 12));
			fbr_current = bcmbca_get_fbr();
		}
		printf("brom found image @ 0x%08x\n", bcmbca_get_fbr()*4096);
		if ((fbr_current*4096) < CONFIG_BCM_FAILSAFE_SEARCH_OFFSET) {
			bcmbca_set_fbr(CONFIG_BCM_FAILSAFE_SEARCH_OFFSET/4096);
		} else {
			bcmbca_set_fbr(fbr_current+1);
		}
	}
}

void bcm_failsafe_boot_clear(void)
{
	bcmbca_clr_fbr();
}

uint32_t bcm_failsafe_boot_offset(void)
{
	if (( bcmbca_get_reset_status() & SW_RESET_STATUS)) {
		return (fbr_current << 12);
	}
	return 0;
}

void bcm_failsafe_boot_init(void)
{
	fbr_current = 0;
}
