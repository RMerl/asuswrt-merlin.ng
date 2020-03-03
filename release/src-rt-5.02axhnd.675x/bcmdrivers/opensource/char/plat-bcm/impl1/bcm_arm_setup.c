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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>
#include <linux/sched.h>
#include <linux/sizes.h>
#include <linux/dma-mapping.h>
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#include <bcm_map_part.h>
#include <board.h>
#include <bcm_rsvmem.h>
#include <asm/mach/map.h>
#include <linux/memblock.h>
#include <bcm_extirq.h>
#if !defined(CONFIG_BCM96858)
#include <pmc_drv.h>
#endif
#include <bcm_pinmux.h>

#if defined (CONFIG_BCM947189) || defined(CONFIG_BCM96846)
#include <linux/init.h>
#include "board.h"
#include <asm/mach/map.h>
#endif
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878)
#include <bcm_ubus4.h>
extern void __init create_mapping(struct map_desc *md);
#endif

#if defined(CONFIG_BCM_GLB_COHERENCY)
/* Before kernel has CCI port control for ARMv8 support, we need 
   this function to enable snoop. This can be done in CFE but then
   we can't control it with CONFIG option.
*/
static void cci_coherent_enable(void) 
{
#if defined(CCI500)
	/* enable snoop in the cpu cluster interface */
	CCI500->si[SLAVEINTF_CPU_CLUSTER].snoop_ctrl |= SNOOP_CTRL_ENABLE_SNOOP;
	while (readl_relaxed(&CCI500->status) & STATUS_CHANGE_PENDING)
		;

	printk("CCI hardware cache coherency enabled\n");
#endif
	return;
}
#endif

#ifdef CONFIG_OF
extern unsigned long getMemorySize(void);
extern void check_if_rootfs_is_set(char *cmdline);
extern void check_if_ikosboot(char *cmdline);
extern int bcm_early_scan_dt(unsigned long node, const char *uname, int depth, void *data);

#if defined(CONFIG_BCM_B15_MEGA_BARRIER)
static int __init bcm_reserve_mem_add_list(char* rsrv_name, uint32_t phys_offs, uint32_t size)
{
	if (reserved_mem_total + size > phys_offs) {
		return -1;
	}
	if (rsvd_mem_cnt >= TOTAL_RESERVE_MEM_NUM) {
		printk("reserved memory count %d reached the total memory reserve count %d!!!", rsvd_mem_cnt, TOTAL_RESERVE_MEM_NUM);
		return -1; 
	}
	strcpy(reserve_mem[rsvd_mem_cnt].name, rsrv_name);
	reserve_mem[rsvd_mem_cnt].phys_addr = phys_offs - size; 
	reserve_mem[rsvd_mem_cnt].size = size;
	memblock_remove(reserve_mem[rsvd_mem_cnt].phys_addr, reserve_mem[rsvd_mem_cnt].size);
	reserved_mem_total += size;
	rsvd_mem_cnt++;
	return 0;	
}
#endif

int __init bcm_dt_postinit(void)
{
	check_if_rootfs_is_set(boot_command_line);
	check_if_ikosboot(boot_command_line);
	return 0;
}

int __init bcm_reserve_memory(void)
{
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878)
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
    struct map_desc desc;
    void* virt;
    int i;

    printk(KERN_INFO "     Reserved memory mapping...\n");
    printk(KERN_INFO "                phys          virt       size\n");
    for(i = 0; i < rsvd_mem_cnt; i++ )
    {
        virt = phys_to_virt(reserve_mem[i].phys_addr);
        reserve_mem[i].virt_addr = virt;
        if (reserve_mem[i].mapped == 0)
        {
            desc.virtual = (unsigned long)virt;
            desc.pfn = __phys_to_pfn(reserve_mem[i].phys_addr);
            desc.length = reserve_mem[i].size;
            desc.type = MT_MEMORY_RWX_NONCACHED;
            create_mapping(&desc);
        }
        printk("   %s   %pa  0x%08x  0x%08x\n", reserve_mem[i].name, &reserve_mem[i].phys_addr, (unsigned int)reserve_mem[i].virt_addr, (unsigned int)reserve_mem[i].size);
    }
#endif
#else
	uint32_t phys_offs = getMemorySize();
	int i;
	if (phys_offs > SZ_256M) {
		phys_offs = SZ_256M;
	}

#if defined(CONFIG_BCM_CMA_RSVMEM)
	phys_offs = (uint32_t)cma_phys_addr;
#else
	for(i = 0 ; i < rsvd_mem_cnt; i++ ) {
		/*memblock_remove(reserve_mem[i].phys_addr, reserve_mem[i].size);*/
		if (phys_offs > reserve_mem[i].phys_addr) {
			phys_offs = reserve_mem[i].phys_addr;
		}  
	}
#endif

#if defined(CONFIG_BCM_B15_MEGA_BARRIER)
	if (bcm_reserve_mem_add_list(B15_MEGA_BARRIER, 
					phys_offs, 
					SECTION_SIZE)) {
		return -1;
	}
#endif
	for(i = 0 ; i < rsvd_mem_cnt; i++ ) {
		printk(KERN_INFO "Reserved memory: phys %pa size 0x%08x for %s\n", 
				&reserve_mem[i].phys_addr, 
				(uint32_t)reserve_mem[i].size, 
				reserve_mem[i].name);
	}
#endif

	return 0;
}

EXPORT_SYMBOL(bcm_reserve_memory);

#endif

#if defined(CONFIG_BCM947189) || defined(CONFIG_BCM96846) ||defined(CONFIG_BCM947622) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM96878)
static void bcm_sys_restart(enum reboot_mode reboot_mode, const char *cmd)
{
	if(kerSysIsIkosBootSet() == 0)
		kerSysSoftReset();
}

int __init bcm_arch_early_init(void)

{
	arm_pm_restart = bcm_sys_restart;	

#ifdef CONFIG_OF
	memset(reserve_mem, 0x0, sizeof(reserve_mem_t)*TOTAL_RESERVE_MEM_NUM);
	of_scan_flat_dt(bcm_early_scan_dt, NULL);
#if defined (CONFIG_BCM96846) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878)
	bcm_reserve_memory();
#endif
#endif
#ifdef CONFIG_BCM96878
    init_dma_coherent_pool_size(SZ_2M);
#endif

	check_if_rootfs_is_set(boot_command_line);

#if defined(CONFIG_BCM_GLB_COHERENCY)
	cci_coherent_enable();
#endif

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878)
	pmc_init();
#endif

	check_if_ikosboot(boot_command_line);
	if(kerSysIsIkosBootSet() == 0)
	{
		kerSysEarlyFlashInit();
		kerSysFlashInit();

		/* Setup external irqs */
		bcm_extirq_init();
	}

#if !defined(CONFIG_BCM947189)
	bcm_init_pinmux();

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878)
	ubus_master_port_init();
#endif
#endif // !defined(CONFIG_BRCM_IKOS)

	return 0;
}

early_initcall(bcm_arch_early_init);
#endif

