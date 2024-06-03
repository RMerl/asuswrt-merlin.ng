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
#include <linux/of.h>
#include <linux/io.h>
#include <linux/of_fdt.h>
#include <linux/memblock.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <bcm_rsvmem.h>

#if defined(CONFIG_BCM_B15_MEGA_BARRIER)

extern int dt_get_memory_prop(unsigned long node, uint64_t* base, uint64_t* size);

static uint32_t preSOmemScratch;
static void *so_memory_virt_addr=&preSOmemScratch;

static phys_addr_t cma_phys_addr = 0;
static uint32_t cma_size = 0;

void BcmMegaBarrier(void) 
{
	writel_relaxed(so_memory_virt_addr, so_memory_virt_addr);
	dsb();
}

void __init board_map_io(void)
{
	struct map_desc desc;

	if (cma_phys_addr == 0)
		return;

	desc.virtual = (unsigned long)phys_to_virt(cma_phys_addr-SECTION_SIZE);
	desc.pfn = __phys_to_pfn(cma_phys_addr-SECTION_SIZE);
	desc.length = SECTION_SIZE;
	so_memory_virt_addr = (void*)desc.virtual;
	desc.type = MT_MEMORY_RW_SO;
	iotable_init(&desc, 1);

	soc_mb = BcmMegaBarrier;

	printk(KERN_INFO "creating a SO device at physical "
				"address of 0x%08lx to virtual address at "
				"0x%08lx with size of 0x%lx byte for b15 mega barrier\n",
				(unsigned long)(cma_phys_addr-SECTION_SIZE),
				desc.virtual, desc.length);
}

static int __init bcm_early_scan_cma(unsigned long node, const char *uname, int depth, void *data)
{
	int rc = 0;  
	uint64_t base = 0, size = 0;

	if ( strcmp(uname, "dt_reserved_cma") == 0 ) {
		rc = 1;
		if( dt_get_memory_prop(node, &base, &size) == 0 ) {
			cma_phys_addr = (phys_addr_t)base;
			cma_size = (uint32_t)size;
			if( base == 0 || size == 0) {
				printk("Error:incomplete rsvd mem entry base %lld size %lld for cma\n", base, size);
				cma_phys_addr = 0;
			}
		}
	}
	return rc;
}

static void __init board_reserve(void)
{
	of_scan_flat_dt(bcm_early_scan_cma, NULL);

	if (cma_phys_addr > SECTION_SIZE) {
	    memblock_remove(cma_phys_addr - SECTION_SIZE, SECTION_SIZE);
		printk(KERN_INFO "Reserved memory: phys 0x%08x size 0x%08x for b15 mega barrier\n", 
			   (uint32_t)(cma_phys_addr - SECTION_SIZE), SECTION_SIZE);
	}
}
#endif /*CONFIG_BCM_B15_MEGA_BARRIER*/

#if defined (CONFIG_BCM963148)

static const char * const bcm63xx_dt_compat[] = {
        "brcm,bcm963148",
        NULL
};

MACHINE_START(BCM963148, "BCM963148")
	/* Maintainer: Broadcom */
#if defined(CONFIG_BCM_B15_MEGA_BARRIER)
	.reserve	= board_reserve,
	.map_io		= board_map_io,
#endif
	.dt_compat      = bcm63xx_dt_compat,
MACHINE_END

#endif

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
