// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003, Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
 */

#include <common.h>
#include <clock_legacy.h>
#include <env.h>
#include <init.h>
#include <pci.h>
#include <uuid.h>
#include <asm/global_data.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <ioports.h>
#include <flash.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <i2c.h>
#include "upm_table.h"

DECLARE_GLOBAL_DATA_PTR;

void local_bus_init (void);
ulong flash_get_size (ulong base, int banknum);

int checkboard (void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	char buf[64];
	int f;
	int i = env_get_f("serial#", buf, sizeof(buf));
#ifdef CONFIG_PCI
	char *src;
#endif

	puts("Board: Socrates");
	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

#if defined(CONFIG_PCI)
	/* Check the PCI_clk sel bit */
	if (in_be32(&gur->porpllsr) & (1<<15)) {
		src = "SYSCLK";
		f = get_board_sys_clk();
	} else {
		src = "PCI_CLK";
		/* PCI is clocked by the external source at 33 MHz */
		f = 33000000;
	}
	printf ("PCI1:  32 bit, %d MHz (%s)\n",	f/1000000, src);
#else
	printf ("PCI1:  disabled\n");
#endif

	/*
	 * Initialize local bus.
	 */
	local_bus_init ();
	return 0;
}

int misc_init_r (void)
{
	/*
	 * Adjust flash start and offset to detected values
	 */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Check if boot FLASH isn't max size
	 */
	if (gd->bd->bi_flashsize < (0 - CFG_SYS_FLASH0)) {
		set_lbc_or(0, gd->bd->bi_flashstart |
			   (CONFIG_SYS_OR0_PRELIM & 0x00007fff));
		set_lbc_br(0, gd->bd->bi_flashstart |
			   (CONFIG_SYS_BR0_PRELIM & 0x00007fff));

		/*
		 * Re-check to get correct base address
		 */
		flash_get_size(gd->bd->bi_flashstart, CONFIG_SYS_MAX_FLASH_BANKS - 1);
	}

	/*
	 * Check if only one FLASH bank is available
	 */
	if (gd->bd->bi_flashsize != CONFIG_SYS_MAX_FLASH_BANKS * (0 - CFG_SYS_FLASH0)) {
		set_lbc_or(1, 0);
		set_lbc_br(1, 0);

		/*
		 * Re-do flash protection upon new addresses
		 */
		flash_protect(FLAG_PROTECT_CLEAR,
			      gd->bd->bi_flashstart, 0xffffffff,
			      &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_SYS_MONITOR_BASE, CONFIG_SYS_MONITOR_BASE +
			      monitor_flash_len - 1,
			      &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Environment protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_ENV_ADDR,
			      CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
			      &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Redundant environment protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_ENV_ADDR_REDUND,
			      CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);
	}

	pci_init();

	return 0;
}

/*
 * Initialize Local Bus
 */
void local_bus_init (void)
{
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	volatile ccsr_local_ecm_t *ecm = (void *)(CFG_SYS_MPC85xx_ECM_ADDR);
	sys_info_t sysinfo;
	uint clkdiv;
	uint lbc_mhz;
	uint lcrr = CFG_SYS_LBC_LCRR;

	get_sys_info (&sysinfo);
	clkdiv = lbc->lcrr & LCRR_CLKDIV;
	lbc_mhz = sysinfo.freq_systembus / 1000000 / clkdiv;

	/* Disable PLL bypass for Local Bus Clock >= 66 MHz */
	if (lbc_mhz >= 66)
		lcrr &= ~LCRR_DBYP;	/* DLL Enabled */
	else
		lcrr |= LCRR_DBYP;	/* DLL Bypass */

	out_be32 (&lbc->lcrr, lcrr);
	asm ("sync;isync;msync");

	out_be32 (&lbc->ltesr, 0xffffffff);	/* Clear LBC error interrupts */
	out_be32 (&lbc->lteir, 0xffffffff);	/* Enable LBC error interrupts */
	out_be32 (&ecm->eedr, 0xffffffff);	/* Clear ecm errors */
	out_be32 (&ecm->eeer, 0xffffffff);	/* Enable ecm errors */

	/* Init UPMA for FPGA access */
	out_be32 (&lbc->mamr, 0x44440); /* Use a customer-supplied value */
	upmconfig(UPMA, (uint *)UPMTableA, sizeof(UPMTableA) / sizeof(int));

	/* Init UPMB for Lime controller access */
	out_be32 (&lbc->mbmr, 0x444440); /* Use a customer-supplied value */
	upmconfig(UPMB, (uint *)UPMTableB, sizeof(UPMTableB) / sizeof(int));
}

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r (void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);

	/* set and reset the GPIO pin 2 which will reset the W83782G chip */
	out_8((unsigned char*)&gur->gpoutdr, 0x3F );
	out_be32((unsigned int*)&gur->gpiocr, 0x200 );	/* enable GPOut */
	udelay(200);
	out_8( (unsigned char*)&gur->gpoutdr, 0x1F );

	return (0);
}
#endif /* CONFIG_BOARD_EARLY_INIT_R */

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	u32 val[12];
	int rc, i = 0;

	ft_cpu_setup(blob, bd);

	/* Fixup NOR FLASH mapping */
	val[i++] = 0;				/* chip select number */
	val[i++] = 0;				/* always 0 */
	val[i++] = gd->bd->bi_flashstart;
	val[i++] = gd->bd->bi_flashsize;

	/* Fixup FPGA mapping */
	val[i++] = 3;				/* chip select number */
	val[i++] = 0;				/* always 0 */
	val[i++] = CFG_SYS_FPGA_BASE;
	val[i++] = CFG_SYS_FPGA_SIZE;

	rc = fdt_find_and_setprop(blob, "/localbus", "ranges",
				  val, i * sizeof(u32), 1);
	if (rc)
		printf("Unable to update localbus ranges, err=%s\n",
		       fdt_strerror(rc));

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

int get_serial_clock(void)
{
	return 333333330;
}
