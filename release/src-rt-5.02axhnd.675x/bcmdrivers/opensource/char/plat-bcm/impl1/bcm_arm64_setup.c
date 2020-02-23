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
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#include <bcm_map_part.h>
#include <bcm_extirq.h>
#include <board.h>
#include <bcm_rsvmem.h>
#include <pmc_drv.h>
#if !defined(CONFIG_BCM94908)
#include <bcm_ubus4.h>
#endif
#include <bcm_pinmux.h>

extern void check_if_rootfs_is_set(char *cmdline);
extern void check_if_ikosboot(char *cmdline);
extern int bcm_early_scan_dt(unsigned long node, const char *uname, int depth, void *data);

#ifdef CONFIG_OF

static int __init bcm_create_reserved_memory_mapping(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
    int i;
    void* virt;
    uint32_t size;
    phys_addr_t phys;

    for(i = 0; i < rsvd_mem_cnt; i++ ) {
        phys = reserve_mem[i].phys_addr;
        virt = phys_to_virt(phys);
        reserve_mem[i].virt_addr = virt;

        if (reserve_mem[i].mapped == 0) {
            size = reserve_mem[i].size;
            create_pgd_mapping(&init_mm, phys, (unsigned long)virt, size,  __pgprot(PROT_NORMAL_NC));
            printk("creating mapping for reserved memory phys %pa virt 0x%p size 0x%08x for %s\n", &phys, virt, (uint32_t)size, reserve_mem[i].name);
        } else {
            /* kernel mapped as cached memory already */
            printk("Do not need to create mapping for reserved memory phys "
                   "%pa virt 0x%p size 0x%08x for %s\n", &reserve_mem[i].phys_addr, virt,
                   reserve_mem[i].size, reserve_mem[i].name);
        }
    }
#endif
    return 0;
}

#endif

static void bcm_sys_restart(enum reboot_mode reboot_mode, const char *cmd)
{
#if !defined(CONFIG_BRCM_IKOS)
    kerSysSoftReset();
#endif
}

#if defined(CONFIG_BCM_GLB_COHERENCY) && defined(CONFIG_BCM963158)
/* Before kernel has CCI port control for ARMv8 support, we need 
   this function to enable snoop. This can be done in CFE but then
   we can't control it with CONFIG option.
*/
static void cci_coherent_enable(void) 
{
    /* enable snoop in the cpu cluster interface */
    CCI500->si[SLAVEINTF_CPU_CLUSTER].snoop_ctrl |= SNOOP_CTRL_ENABLE_SNOOP;
    while (readl_relaxed(&CCI500->status) & STATUS_CHANGE_PENDING)
        ;

    printk("CCI hardware cache coherency enabled\n");

    return;
}
#endif

static int __init bcm_arch_early_init(void)
{
    /* replace with PSCI in future release */
    arm_pm_restart = bcm_sys_restart;

    memset((void*)bcm_get_blparms(), 0x0, bcm_get_blparms_size());
#ifdef CONFIG_OF
    memset(reserve_mem, 0x0, sizeof(reserve_mem_t)*TOTAL_RESERVE_MEM_NUM);
    of_scan_flat_dt(bcm_early_scan_dt, NULL);

    bcm_create_reserved_memory_mapping();
#endif
    check_if_rootfs_is_set(boot_command_line);
    check_if_ikosboot(boot_command_line);

#if defined(CONFIG_BCM_GLB_COHERENCY) && defined(CONFIG_BCM963158)
    cci_coherent_enable();
#endif
#if !defined(CONFIG_BRCM_IKOS)
    pmc_init();
#else
    pmc_initmode();
#endif
    /* 68360 arch should be modified after PMC support!!! */
#if !defined(CONFIG_BRCM_IKOS)
    kerSysEarlyFlashInit();
    kerSysFlashInit();

    /* Setup external irqs */
    bcm_extirq_init();
#endif
    bcm_init_pinmux();

#if !defined(CONFIG_BCM94908)
    ubus_master_port_init();
#endif
    return 0;
}

early_initcall(bcm_arch_early_init);
