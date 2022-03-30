// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 */

#include <common.h>
#include <dm.h>
#include <netdev.h>
#include <asm/io.h>
#include <dm/platform_data/serial_pl01x.h>
#include "arm-ebi.h"
#include "integrator-sc.h"
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pl01x_serial_platdata serial_platdata = {
	.base = 0x16000000,
#ifdef CONFIG_ARCH_CINTEGRATOR
	.type = TYPE_PL011,
	.clock = 14745600,
#else
	.type = TYPE_PL010,
	.clock = 0, /* Not used for PL010 */
#endif
};

U_BOOT_DEVICE(integrator_serials) = {
	.name = "serial_pl01x",
	.platdata = &serial_platdata,
};

void peripheral_power_enable (void);

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	u32 val;

	/* arch number of Integrator Board */
#ifdef CONFIG_ARCH_CINTEGRATOR
	gd->bd->bi_arch_number = MACH_TYPE_CINTEGRATOR;
#else
	gd->bd->bi_arch_number = MACH_TYPE_INTEGRATOR;
#endif

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

#ifdef CONFIG_CM_REMAP
extern void cm_remap(void);
	cm_remap();	/* remaps writeable memory to 0x00000000 */
#endif

#ifdef CONFIG_ARCH_CINTEGRATOR
	/*
	 * Flash protection on the Integrator/CP is in a simple register
	 */
	val = readl(CP_FLASHPROG);
	val |= (CP_FLASHPROG_FLVPPEN | CP_FLASHPROG_FLWREN);
	writel(val, CP_FLASHPROG);
#else
	/*
	 * The Integrator/AP has some special protection mechanisms
	 * for the external memories, first the External Bus Interface (EBI)
	 * then the system controller (SC).
	 *
	 * The system comes up with the flash memory non-writable and
	 * configuration locked. If we want U-Boot to be used for flash
	 * access we cannot have the flash memory locked.
	 */
	writel(EBI_UNLOCK_MAGIC, EBI_BASE + EBI_LOCK_REG);
	val = readl(EBI_BASE + EBI_CSR1_REG);
	val &= EBI_CSR_WREN_MASK;
	val |= EBI_CSR_WREN_ENABLE;
	writel(val, EBI_BASE + EBI_CSR1_REG);
	writel(0, EBI_BASE + EBI_LOCK_REG);

	/*
	 * Set up the system controller to remove write protection from
	 * the flash memory and enable Vpp
	 */
	writel(SC_CTRL_FLASHVPP | SC_CTRL_FLASHWP, SC_CTRLS);
#endif

	icache_enable ();

	return 0;
}

int misc_init_r (void)
{
	env_set("verify", "n");
	return (0);
}

/*
 * The Integrator remaps the Flash memory to 0x00000000 and executes U-Boot
 * from there, which means we cannot test the RAM underneath the ROM at this
 * point. It will be unmapped later on, when we are executing from the
 * relocated in RAM U-Boot. We simply assume that this RAM is usable if the
 * RAM on higher addresses works fine.
 */
#define REMAPPED_FLASH_SZ 0x40000

int dram_init (void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
#ifdef CONFIG_CM_SPD_DETECT
	{
extern void dram_query(void);
	u32 cm_reg_sdram;
	u32 sdram_shift;

	dram_query();	/* Assembler accesses to CM registers */
			/* Queries the SPD values	      */

	/* Obtain the SDRAM size from the CM SDRAM register */

	cm_reg_sdram = readl(CM_BASE + OS_SDRAM);
	/*   Register	      SDRAM size
	 *
	 *   0xXXXXXXbbb000bb	 16 MB
	 *   0xXXXXXXbbb001bb	 32 MB
	 *   0xXXXXXXbbb010bb	 64 MB
	 *   0xXXXXXXbbb011bb	128 MB
	 *   0xXXXXXXbbb100bb	256 MB
	 *
	 */
	sdram_shift = ((cm_reg_sdram & 0x0000001C)/4)%4;
	gd->ram_size = get_ram_size((long *) CONFIG_SYS_SDRAM_BASE +
				    REMAPPED_FLASH_SZ,
				    0x01000000 << sdram_shift);
	}
#else
	gd->ram_size = get_ram_size((long *) CONFIG_SYS_SDRAM_BASE +
				    REMAPPED_FLASH_SZ,
				    PHYS_SDRAM_1_SIZE);
#endif /* CM_SPD_DETECT */
	/* We only have one bank of RAM, set it to whatever was detected */
	gd->bd->bi_dram[0].size	 = gd->ram_size;

	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
	rc += pci_eth_init(bis);
	return rc;
}
#endif
