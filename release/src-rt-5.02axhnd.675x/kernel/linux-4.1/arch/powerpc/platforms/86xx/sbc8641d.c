/*
 * SBC8641D board specific routines
 *
 * Copyright 2008 Wind River Systems Inc.
 *
 * By Paul Gortmaker (see MAINTAINERS for contact information)
 *
 * Based largely on the 8641 HPCN support by Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/of_platform.h>

#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/pci-bridge.h>
#include <asm/prom.h>
#include <mm/mmu_decl.h>
#include <asm/udbg.h>

#include <asm/mpic.h>

#include <sysdev/fsl_pci.h>
#include <sysdev/fsl_soc.h>

#include "mpc86xx.h"

static void __init
sbc8641_setup_arch(void)
{
	if (ppc_md.progress)
		ppc_md.progress("sbc8641_setup_arch()", 0);

	printk("SBC8641 board from Wind River\n");

#ifdef CONFIG_SMP
	mpc86xx_smp_init();
#endif

	fsl_pci_assign_primary();
}


static void
sbc8641_show_cpuinfo(struct seq_file *m)
{
	uint svid = mfspr(SPRN_SVR);

	seq_printf(m, "Vendor\t\t: Wind River Systems\n");

	seq_printf(m, "SVR\t\t: 0x%x\n", svid);
}


/*
 * Called very early, device-tree isn't unflattened
 */
static int __init sbc8641_probe(void)
{
	unsigned long root = of_get_flat_dt_root();

	if (of_flat_dt_is_compatible(root, "wind,sbc8641"))
		return 1;	/* Looks good */

	return 0;
}

static long __init
mpc86xx_time_init(void)
{
	unsigned int temp;

	/* Set the time base to zero */
	mtspr(SPRN_TBWL, 0);
	mtspr(SPRN_TBWU, 0);

	temp = mfspr(SPRN_HID0);
	temp |= HID0_TBEN;
	mtspr(SPRN_HID0, temp);
	asm volatile("isync");

	return 0;
}

static const struct of_device_id of_bus_ids[] __initconst = {
	{ .compatible = "simple-bus", },
	{ .compatible = "gianfar", },
	{ .compatible = "fsl,mpc8641-pcie", },
	{},
};

static int __init declare_of_platform_devices(void)
{
	of_platform_bus_probe(NULL, of_bus_ids, NULL);

	return 0;
}
machine_arch_initcall(sbc8641, declare_of_platform_devices);

define_machine(sbc8641) {
	.name			= "SBC8641D",
	.probe			= sbc8641_probe,
	.setup_arch		= sbc8641_setup_arch,
	.init_IRQ		= mpc86xx_init_irq,
	.show_cpuinfo		= sbc8641_show_cpuinfo,
	.get_irq		= mpic_get_irq,
	.restart		= fsl_rstcr_restart,
	.time_init		= mpc86xx_time_init,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= udbg_progress,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
#endif
};
