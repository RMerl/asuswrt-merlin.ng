/*
<:copyright-BRCM:2015:GPL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#include <linux/init.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>
#include <linux/sched.h>
#include <linux/sizes.h>
#include <linux/dma-mapping.h>
#include <linux/smp.h>
#include <asm/cp15.h>
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#include <board.h>
#include <bcm_rsvmem.h>
#include <asm/mach/map.h>
#include <linux/memblock.h>
#include <bcm_pinmux.h>
#include "pmc_core_api.h"
#include "pmc_neon.h"

#if defined(CONFIG_BCM_BOOTSTATE)
#include "bcm_bootstate.h"
#endif

#if defined(CONFIG_BCM96846)
#include <linux/init.h>
#include "board.h"
#include <asm/mach/map.h>
#endif

#if defined(CONFIG_BCM_GLB_COHERENCY)
extern void bcm_coherency_init(void);
#endif
#ifdef CONFIG_OF
extern unsigned long getMemorySize(void);
extern void check_if_rootfs_is_set(char *cmdline);
extern void check_if_ikosboot(char *cmdline);
extern int bcm_early_scan_dt(unsigned long node, const char *uname, int depth, void *data);
#ifdef CONFIG_BCM_STRAP
extern int init_strap_register(void);
#endif
#endif

static struct notifier_block restart_handler_struct;

static void bcm_sys_restart(enum reboot_mode reboot_mode, const char *cmd)
{
#if !defined(CONFIG_BRCM_IKOS)
	kerSysSoftReset();
#endif
}

static int _reset_restart_handler(struct notifier_block *nb,
    unsigned long reboot_mode, void *data)
{
    bcm_sys_restart(reboot_mode, data);
    return NOTIFY_DONE;
}

#if defined(CONFIG_BCM_BOOTSTATE)
static int reboot_handler(struct notifier_block *self, unsigned long val, void*data)
{

	if((bcmbca_get_boot_reason() & BCM_BOOT_REASON_ACTIVATE) == 0)
	{
		bcmbca_set_boot_reason(0);
	}
	return 0;
}
static struct notifier_block reboot_notifier = {
	.notifier_call = reboot_handler,
};
#endif


int __init bcm_arch_early_init(void)

{
	memset(&restart_handler_struct, '\0', sizeof(restart_handler_struct));
	restart_handler_struct.notifier_call = _reset_restart_handler;
	restart_handler_struct.priority = 255;
	register_restart_handler(&restart_handler_struct);

#if defined(CONFIG_BCM_BOOTSTATE)
	register_reboot_notifier(&reboot_notifier);
#endif

#ifdef CONFIG_OF
	of_scan_flat_dt(bcm_early_scan_dt, NULL);
#ifdef CONFIG_BCM_STRAP
	init_strap_register();
#endif
#endif
#if defined (CONFIG_BCM96878) || defined(CONFIG_BCM96855)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
/*
There are no users of init_dma_coherent_pool_size() left due to
387870f ("mm: dmapool: use provided gfp flags for all
dma_alloc_coherent() calls")

dmapool always calls dma_alloc_coherent() with GFP_ATOMIC flag,
regardless the flags provided by the caller. This causes excessive
pruning of emergency memory pools without any good reason. Additionaly,
on ARM architecture any driver which is using dmapools will sooner or
later  trigger the following error: 
"ERROR: 256 KiB atomic DMA coherent pool is too small!
Please increase it with coherent_pool= kernel parameter!".
Increasing the coherent pool size usually doesn't help much and only
delays such error, because all GFP_ATOMIC DMA allocations are always
served from the special, very limited memory pool.

This patch changes the dmapool code to correctly use gfp flags provided
by the dmapool caller.

Increase it with coherent_pool= kernel parameter in the dts file:
e.g. bootargs = "coherent_pool=2M"
*/
#else
    init_dma_coherent_pool_size(SZ_2M);
#endif
#endif

	check_if_rootfs_is_set(boot_command_line);

#if defined(CONFIG_BCM_GLB_COHERENCY)
	bcm_coherency_init();
#endif

	check_if_ikosboot(boot_command_line);
#if !defined(CONFIG_BRCM_IKOS)
	kerSysEarlyFlashInit();
	kerSysFlashInit();
#endif

	return 0;
}

early_initcall(bcm_arch_early_init);

#if defined (CONFIG_BCM96878) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM963138)
static void __init neon_enable(void *data)
{
#ifndef CONFIG_BRCM_QEMU
	u32 access;
	(void)data;
	
	access = get_copro_access();

	/*
	 * Enable full access to VFP (cp10 and cp11)
	 */
	set_copro_access(access | CPACC_FULL(10) | CPACC_FULL(11));

	/* mov r0, 0x40000000; vmsr fpexc, r0 */
	asm volatile ("mov r0, #0x40000000; .word 0xeee80a10" : : : "r0" );
#endif
}

static int __init bcm_arm_neon_fixup(void)
{
#if defined(CONFIG_BCM963138)
	pmc_neon_power_up();
#endif
	smp_call_function_single(0, neon_enable, NULL, 1);
	return 0;
}
late_initcall(bcm_arm_neon_fixup);
#endif
