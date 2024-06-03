// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2017 CS Systemes d'Information
 * Florent Trinh Thai <florent.trinh-thai@c-s.fr>
 * Christophe Leroy <christophe.leroy@c-s.fr>
 *
 * Board specific routines for the MCR3000 board
 */

#include <common.h>
#include <hwconfig.h>
#include <mpc8xx.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

#define SDRAM_MAX_SIZE			(32 * 1024 * 1024)

static const uint cs1_dram_table_66[] = {
	/* DRAM - single read. (offset 0 in upm RAM) */
	0x0F3DFC04, 0x0FEFBC04, 0x00BE7804, 0x0FFDF400,
	0x1FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* DRAM - burst read. (offset 8 in upm RAM) */
	0x0F3DFC04, 0x0FEFBC04, 0x00BF7C04, 0x00FFFC00,
	0x00FFFC00, 0x00FEF800, 0x0FFDF400, 0x1FFFFC05,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* DRAM - single write. (offset 18 in upm RAM) */
	0x0F3DFC04, 0x0FEFB800, 0x00BF7404, 0x0FFEF804,
	0x0FFDF404, 0x1FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF,

	/* DRAM - burst write. (offset 20 in upm RAM) */
	0x0F3DFC04, 0x0FEFB800, 0x00BF7400, 0x00FFFC00,
	0x00FFFC00, 0x00FFFC04,	0x0FFEF804, 0x0FFDF404,
	0x1FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* refresh  (offset 30 in upm RAM) */
	0x0FFDF404, 0x0FFEBC04, 0x0FFD7C84, 0x0FFFFC04,
	0x0FFFFC04, 0x0FFFFC04, 0x1FFFFC85, 0xFFFFFFFF,

	/* init */
	0x0FEEB874, 0x0FBD7474, 0x1FFFFC45, 0xFFFFFFFF,

	/* exception. (offset 3c in upm RAM) */
	0xFFFFFC05, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};

int ft_board_setup(void *blob, bd_t *bd)
{
	const char *sync = "receive";

	ft_cpu_setup(blob, bd);

	/* BRG */
	do_fixup_by_path_u32(blob, "/soc/cpm", "brg-frequency",
			     bd->bi_busfreq, 1);

	/* MAC addr */
	fdt_fixup_ethernet(blob);

	/* Bus Frequency for CPM */
	do_fixup_by_path_u32(blob, "/soc", "bus-frequency", bd->bi_busfreq, 1);

	/* E1 interface - Set data rate */
	do_fixup_by_path_u32(blob, "/localbus/e1-wan", "data-rate", 2, 1);

	/* E1 interface - Set channel phase to 0 */
	do_fixup_by_path_u32(blob, "/localbus/e1-wan", "channel-phase", 0, 1);

	/* E1 interface - rising edge sync pulse transmit */
	do_fixup_by_path(blob, "/localbus/e1-wan", "rising-edge-sync-pulse",
			 sync, strlen(sync), 1);

	return 0;
}

int checkboard(void)
{
	serial_puts("BOARD: MCR3000 CSSI\n");

	return 0;
}

int dram_init(void)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	memctl8xx_t __iomem *memctl = &immap->im_memctl;

	printf("UPMA init for SDRAM (CAS latency 2), ");
	printf("init address 0x%08x, size ", (int)dram_init);
	/* Configure UPMA for cs1 */
	upmconfig(UPMA, (uint *)cs1_dram_table_66,
		  sizeof(cs1_dram_table_66) / sizeof(uint));
	udelay(10);
	out_be16(&memctl->memc_mptpr, 0x0200);
	out_be32(&memctl->memc_mamr, 0x14904000);
	udelay(10);
	out_be32(&memctl->memc_or1, CONFIG_SYS_OR1_PRELIM);
	out_be32(&memctl->memc_br1, CONFIG_SYS_BR1_PRELIM);
	udelay(10);
	out_be32(&memctl->memc_mcr, 0x80002830);
	out_be32(&memctl->memc_mar, 0x00000088);
	out_be32(&memctl->memc_mcr, 0x80002038);
	udelay(200);

	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    SDRAM_MAX_SIZE);

	return 0;
}

int misc_init_r(void)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	iop8xx_t __iomem *iop = &immr->im_ioport;

	/* Set port C13 as GPIO (BTN_ACQ_AL) */
	clrbits_be16(&iop->iop_pcpar, 0x4);
	clrbits_be16(&iop->iop_pcdir, 0x4);

	/* if BTN_ACQ_AL is pressed then bootdelay is changed to 60 second */
	if ((in_be16(&iop->iop_pcdat) & 0x0004) == 0)
		env_set("bootdelay", "60");

	return 0;
}

int board_early_init_f(void)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;

	/*
	 * Erase FPGA(s) for reboot
	 */
	clrbits_be32(&immr->im_cpm.cp_pbdat, 0x00020000); /* PROGFPGA down */
	setbits_be32(&immr->im_cpm.cp_pbdir, 0x00020000); /* PROGFPGA output */
	udelay(1);				/* Wait more than 300ns */
	setbits_be32(&immr->im_cpm.cp_pbdat, 0x00020000); /* PROGFPGA up */

	return 0;
}

int board_early_init_r(void)
{
	struct udevice *watchdog_dev = NULL;

	if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
		puts("Cannot find watchdog!\n");
	} else {
		puts("Enabling watchdog.\n");
		wdt_start(watchdog_dev, 0xffff, 0);
	}

	return 0;
}
