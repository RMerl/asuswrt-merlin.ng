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
#include <linux/memblock.h>
#include <linux/bootmem.h>
#include <asm/bootinfo.h>
#include <linux/kthread.h>

#include "bcm_map_part.h"
#include "bcm_mm.h"
#include "bcm_rsvmem.h"

#ifdef CONFIG_OF

extern unsigned long memsize;
extern unsigned long getMemorySize(void);
extern void check_if_rootfs_is_set(char *cmdline);

int __init bcm_reserve_memory(void)
{
	uint32_t phys_offs = getMemorySize();
        unsigned long phys_addr;
	int i;
	/* phys_offs is maxed to 256M due to adsl and dhd runner restrictions*/
	if (phys_offs > SZ_256M) {
#ifdef CONFIG_BCM_512MB_DDR

#if !defined HIGH_MEM_PHYS_ADDRESS
#error "High memory physical address is undefined (HIGH_MEM_PHYS_ADDRESS), memory over 256MB cannot be mapped"
#endif
		add_memory_region(HIGH_MEM_PHYS_ADDRESS, phys_offs - SZ_256M, BOOT_MEM_RAM);
#else
		memsize = SZ_256M;
#endif        
		phys_offs = SZ_256M;
	}

	for(i = 0, phys_addr = reserve_mem[0].phys_addr; i < rsvd_mem_cnt; i++ ) {
		reserve_mem[i].virt_addr = (void*)PHYS_TO_UNCACHED(reserve_mem[i].phys_addr);
		printk(KERN_INFO "Reserved memory: phys %pa virt 0x%p size 0x%08x for %s\n", 
				&reserve_mem[i].phys_addr,
				reserve_mem[i].virt_addr,
				(uint32_t)reserve_mem[i].size, 
				reserve_mem[i].name);
		add_memory_region(reserve_mem[i].phys_addr, (uint32_t)reserve_mem[i].size, BOOT_MEM_RESERVED);
                if (phys_addr > reserve_mem[i].phys_addr) {
                    /* get lowest offset where free ram would be*/
                    phys_addr = reserve_mem[i].phys_addr; 
                } 
                
	}

        if (rsvd_mem_cnt == 0) {
           phys_addr = phys_offs; 
        }

	add_memory_region(0, phys_addr, BOOT_MEM_RAM);
	return 0;
}

EXPORT_SYMBOL(bcm_reserve_memory);

int __init bcm_dt_postinit(void)
{
	check_if_rootfs_is_set(boot_command_line);
	return 0;
}
EXPORT_SYMBOL(bcm_dt_postinit);
#endif

/* thread will be using up core 1, making sure init process will be scheduled on core 0 */
static int fixup_func(void * arg) 
{
    static int found = 0;

    while (1)
    {
        struct task_struct *p;

        read_lock(&tasklist_lock);
        for_each_process(p)
        {
            if (strstr(p->comm, "init") == p->comm)
            {
                if (!found)
                    found = 1;
            }
        }
        read_unlock(&tasklist_lock);

        if (found)
            return 0;
    }

    return 0;
}

static int bcm_mips_fix(void)
{
    struct task_struct *thread;
    struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };
    unsigned int cpu = 1;

    thread = kthread_create(fixup_func, NULL, "dummy");
    if (!thread)
    {
        printk("Failed to polling create thread.\n");
        return -1;
    }

    if (!IS_ERR(thread))
        kthread_bind(thread, cpu);

    sched_setscheduler(thread, SCHED_FIFO, &param);

    if (!IS_ERR(thread))
        wake_up_process(thread);

    return !!thread;
}

late_initcall(bcm_mips_fix);

