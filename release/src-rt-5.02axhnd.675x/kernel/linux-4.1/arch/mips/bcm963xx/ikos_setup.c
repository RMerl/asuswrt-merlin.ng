#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && defined(CONFIG_MIPS_BCM963XX) && defined(CONFIG_BCM_KF_IKOS) && defined(CONFIG_BRCM_IKOS)

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
 * Generic setup routines for Broadcom 963xx MIPS IKOS emulation environment
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/console.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/bootmem.h>

#include <asm/addrspace.h>
#include <asm/bcache.h>
#include <asm/irq.h>
#include <asm/time.h>
#include <asm/reboot.h>
//#include <asm/gdb-stub.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>

#include <bcm_map_part.h>
#include <bcm_cpu.h>
#include <bcm_intr.h>
#include <boardparms.h>

#include "shared_utils.h"

extern unsigned long memsize;
extern unsigned long getMemorySize(void);

static void __init setMemorySize(void)
{
    memsize = 16 * 1024 * 1024;
}

void __init plat_mem_setup(void)
{
    /* set up the ddr memory size first */
    setMemorySize();

    add_memory_region(0, (getMemorySize()), BOOT_MEM_RAM);
    {
        volatile unsigned long *cr;
        uint32 mipsBaseAddr = MIPS_BASE;

        cr = (void *)(mipsBaseAddr + MIPS_RAC_CR0);
    	*cr = *cr | RAC_D | RAC_PF_D;

#if defined(MIPS_RAC_CR1)
        cr = (void *)(mipsBaseAddr + MIPS_RAC_CR1);
    	*cr = *cr | RAC_D | RAC_PF_D;
#endif        
    }
}

void __init plat_time_init(void)
{
    /* hard code to 320MHz */ 
    mips_hpt_frequency = 160000000;
    // Enable cp0 counter/compare interrupt when
    // not using workaround for clock divide
    write_c0_status(IE_IRQ5 | read_c0_status());
}

extern void stop_other_cpu(void);  // in arch/mips/kernel/smp.c

/* IKOS does not need real restart. Same as halt */
static void brcm_machine_restart(char *command)
{
#if defined(CONFIG_SMP)
    stop_other_cpu();
#endif
    printk("IKOS restart --> system halted\n");
    local_irq_disable();
    while (1);
}

static void brcm_machine_halt(void)
{
    /*
     * we don't support power off yet.  This halt will cause both CPU's to
     * spin in a while(1) loop with interrupts disabled.  (Used for gathering
     * wlan debug dump via JTAG)
     */
#if defined(CONFIG_SMP)
    stop_other_cpu();
#endif
    printk("System halted\n");
    local_irq_disable();
    while (1);
}

/* this funciton implement any necessary hardware related initialization for ikos */ 
static int __init ikos_hw_init(void)
{
    return 0;
}
#define bcm63xx_specific_hw_init() ikos_hw_init()

static int __init bcm63xx_hw_init(void)
{
    return bcm63xx_specific_hw_init();
}
arch_initcall(bcm63xx_hw_init);


static int __init brcm63xx_setup(void)
{
    extern int panic_timeout;

    _machine_restart = brcm_machine_restart;
    _machine_halt = brcm_machine_halt;
    pm_power_off = brcm_machine_halt;

    panic_timeout = 1;

    return 0;
}

arch_initcall(brcm63xx_setup);

int kerSysGetSdramSize( void )
{
  return getMemorySize();
} /* kerSysGetSdramSize */


/* return the cmdline for ramdisk boot */
static void __init create_cmdline(char *cmdline)
{

}

extern struct plat_smp_ops brcm_smp_ops;

void __init prom_init(void)
{
    int argc = fw_arg0;
    u32 *argv = (u32 *)CKSEG0ADDR(fw_arg1);
    int i;

    PERF->IrqControl[0].IrqMask=0;

    arcs_cmdline[0] = '\0';

    create_cmdline(arcs_cmdline);

    strcat(arcs_cmdline, " ");

    for (i = 1; i < argc; i++) {
        strcat(arcs_cmdline, (char *)CKSEG0ADDR(argv[i]));
        if (i < (argc - 1))
            strcat(arcs_cmdline, " ");
    }

#if defined (CONFIG_SMP)
    register_smp_ops(&brcm_smp_ops);
#endif

}


void __init allocDspModBuffers(void);
/*
*****************************************************************************
*  stub functions for ikos build.
*****************************************************************************
*/
void __init allocDspModBuffers(void)
{
}

void __init prom_free_prom_memory(void)
{

}

/* ikos does not use external interrupt */
unsigned int kerSysGetExtIntInfo(unsigned int irq)
{
    return (unsigned int)(-1);
}

const char *get_system_type(void)
{
    return "ikos emulation system";
}



#endif
