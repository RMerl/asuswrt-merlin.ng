#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:GPL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

/*
 * Generic board routine for Broadcom 963xx ARM boards
 */
#include <linux/types.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clkdev.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/irqchip.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include <mach/hardware.h>
#include <mach/memory.h>
#include <mach/smp.h>

#include <plat/bsp.h>
#if defined(CONFIG_BCM963138)
#include <plat/ca9mpcore.h>
#elif defined(CONFIG_BCM963148)
#include <plat/b15core.h>
#endif
#include <bcm_map_part.h>
#include <board.h>
#include <bcm_rsvmem.h>
#include <asm/mach/map.h>
#include <linux/memblock.h>
#include <bcm_pinmux.h>

#ifdef CONFIG_OF
extern int __init bcm_scan_fdt(void);
extern int __init bcm_reserve_memory(void);
extern int __init bcm_dt_postinit(void);
#endif
extern int __init brcm_legacy_init(struct device_node *np);

#define BOOT_ATAGS 0
#define BOOT_FDT 1

static int boot_param_status = BOOT_ATAGS;

extern unsigned long getMemorySize(void);

#if defined(CONFIG_BCM_B15_MEGA_BARRIER)
static uint32_t preSOmemScratch;
static void *so_memory_virt_addr=&preSOmemScratch;
#endif

		
/*
*****************************************************************************
** FUNCTION:   allocDspModBuffers
**
** PURPOSE:    Allocates buffers for the init and core sections of the DSP
**             module. This module is special since it has to be allocated
**             in the 0x800.. memory range which is not mapped by the TLB.
**
** PARAMETERS: None
** RETURNS:    Nothing
*****************************************************************************
*/
void __init allocDspModBuffers(void)
{
}

#ifdef CONFIG_BCM_B15_MEGA_BARRIER
void BcmMegaBarrier(void) 
{
	__asm__("dsb");
	writel_relaxed(so_memory_virt_addr, so_memory_virt_addr);
	__asm__("dsb");
}
EXPORT_SYMBOL(BcmMegaBarrier);
#endif /*CONFIG_BCM_B15_MEGA_BARRIER*/

void __init board_map_io(void)
{
	struct map_desc desc[TOTAL_RESERVE_MEM_NUM+1];
	int i = 0;

	/* Map SoC specific I/O */
	soc_map_io();

	for(i = 0; i < rsvd_mem_cnt; i++ ) {
		desc[i].virtual = (unsigned long)phys_to_virt(reserve_mem[i].phys_addr);
		desc[i].pfn = __phys_to_pfn(reserve_mem[i].phys_addr);
		desc[i].length = reserve_mem[i].size;
#if defined (CONFIG_BCM_B15_MEGA_BARRIER)
		if (strcmp(B15_MEGA_BARRIER, reserve_mem[i].name)==0) {
			so_memory_virt_addr = (void*)desc[i].virtual;
			desc[i].type = MT_MEMORY_RW_SO;
		} else {
			desc[i].type = MT_MEMORY_RWX_NONCACHED;
		}	
#else
		desc[i].type = MT_MEMORY_RWX_NONCACHED;
#endif
		reserve_mem[i].virt_addr = (void*)desc[i].virtual;
		printk(KERN_INFO "creating a %s device at physical "
				"address of 0x%08lx to virtual address at "
				"0x%08lx with size of 0x%lx byte for %s\n",
				(desc[i].type == MT_MEMORY_RWX_NONCACHED)?"MT_MEMORY_NONCACHED":"MT_MEMORY", 
				(unsigned long)reserve_mem[i].phys_addr,
				desc[i].virtual, desc[i].length,
				reserve_mem[i].name);
	}

	if( i > 0 )
		iotable_init(desc, i);

	if (getMemorySize() <= SZ_32M)
		printk("WARNING! System is with 0x%0lx memory, might not "
				"boot successfully.\n"
				"\tcheck ATAG or CMDLINE\n", getMemorySize());
}

static int __init bcm_setup(void)
{
	if(kerSysIsIkosBootSet() == 0)
	{
		kerSysEarlyFlashInit();
		kerSysFlashInit();
	}
	bcm_init_pinmux();
	return 0;
}
early_initcall(bcm_setup);

void __init board_init_early(void)
{
	soc_init_early();

}

void __init board_init_irq(void)
{
#if !defined (CONFIG_OF)
	soc_init_irq();

#ifdef CONFIG_CACHE_L2X0
	/* cache initialization */
	soc_l2_cache_init();
#endif
#else
	irqchip_init();
	brcm_legacy_init(of_get_child_by_name(of_root, "brcm-legacy"));
#endif
}

static void __init dt_fixup(void)
{
	boot_param_status = BOOT_FDT;
}

void __init board_init_machine(void)
{
	/*
	 * Add common platform devices that do not have board dependent HW
	 * configurations
	 */
	soc_add_devices();

#if defined (CONFIG_OF)
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
#endif

	return;
}

static void board_restart(enum reboot_mode mode, const char *cmd)
{
	if(kerSysIsIkosBootSet() == 0)
		kerSysSoftReset();
}


static void __init board_reserve(void)
{
	if (boot_param_status == BOOT_FDT){
		bcm_scan_fdt();
		bcm_reserve_memory();
		bcm_dt_postinit();
	}
}


#if defined (CONFIG_BCM963138)

#if defined (CONFIG_OF)
static const char * const bcm63xx_dt_compat[] = {
        "brcm,bcm963138",
        NULL
};
#endif

MACHINE_START(BCM963138, "BCM963138")
	/* Maintainer: Broadcom */
	.reserve	= board_reserve,
	.map_io		= board_map_io,	
	.init_early	= board_init_early,
	.init_irq	= board_init_irq,
	.init_machine	= board_init_machine,
	.restart	= board_restart,
#ifdef CONFIG_ZONE_DMA
	/* If enable CONFIG_ZONE_DMA, it will reserve the given size of
	 * memory from SDRAM and use it exclusively for DMA purpose.
	 * This ensures the device driver can allocate enough memory. */
#if defined(CONFIG_OPTEE) || defined(CONFIG_KASAN)
	.dma_zone_size	= SZ_32M,	/* must be multiple of 2MB */
#else
	.dma_zone_size	= SZ_16M,	/* must be multiple of 2MB */
#endif
#endif
#if defined (CONFIG_OF)
	.dt_compat      = bcm63xx_dt_compat,
	.dt_fixup       = dt_fixup,
	.l2c_aux_val	= BCM_L2C_AUX_VAL,
	.l2c_aux_mask	= BCM_L2C_AUX_MSK,
#endif
MACHINE_END

#endif


#if defined (CONFIG_BCM963148)

#ifdef CONFIG_OF
static const char * const bcm63xx_dt_compat[] = {
        "brcm,bcm963148",
        NULL
};
#endif


MACHINE_START(BCM963148, "BCM963148")
	/* Maintainer: Broadcom */
	.reserve	= board_reserve,
	.map_io		= board_map_io,	
	.init_early	= board_init_early,
	.init_irq	= board_init_irq,
	.init_machine	= board_init_machine,
	.restart	= board_restart,
#ifdef CONFIG_ZONE_DMA
	/* If enable CONFIG_ZONE_DMA, it will reserve the given size of
	 * memory from SDRAM and use it exclusively for DMA purpose.
	 * This ensures the device driver can allocate enough memory. */
#if defined(CONFIG_OPTEE) || defined(CONFIG_KASAN)
	.dma_zone_size	= SZ_32M,	/* must be multiple of 2MB */
#else
	.dma_zone_size	= SZ_16M,	/* must be multiple of 2MB */
#endif
#endif
#if defined (CONFIG_OF)
	.dt_compat      = bcm63xx_dt_compat,
	.dt_fixup       = dt_fixup, 
#endif
MACHINE_END

#endif

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
