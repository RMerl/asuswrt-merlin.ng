// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <netdev.h>

#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	rcm_t *rcm = (rcm_t *) (MMAP_RCM);
	udelay(1000);
	out_8(&rcm->rcr, RCM_RCR_FRCRSTOUT);
	udelay(10000);
	setbits_8(&rcm->rcr, RCM_RCR_SOFTRST);

	/* we don't return! */
	return 0;
};

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	ccm_t *ccm = (ccm_t *) MMAP_CCM;
	u16 msk;
	u16 id = 0;
	u8 ver;

	puts("CPU:   ");
	msk = (in_be16(&ccm->cir) >> 6);
	ver = (in_be16(&ccm->cir) & 0x003f);
	switch (msk) {
	case 0x48:
		id = 54455;
		break;
	case 0x49:
		id = 54454;
		break;
	case 0x4a:
		id = 54453;
		break;
	case 0x4b:
		id = 54452;
		break;
	case 0x4d:
		id = 54451;
		break;
	case 0x4f:
		id = 54450;
		break;
	case 0x9F:
		id = 54410;
		break;
	case 0xA0:
		id = 54415;
		break;
	case 0xA1:
		id = 54416;
		break;
	case 0xA2:
		id = 54417;
		break;
	case 0xA3:
		id = 54418;
		break;
	}

	if (id) {
		char buf1[32], buf2[32], buf3[32];

		printf("Freescale MCF%d (Mask:%01x Version:%x)\n", id, msk,
		       ver);
		printf("       CPU CLK %s MHz BUS CLK %s MHz FLB CLK %s MHz\n",
		       strmhz(buf1, gd->cpu_clk),
		       strmhz(buf2, gd->bus_clk),
		       strmhz(buf3, gd->arch.flb_clk));
#ifdef CONFIG_PCI
		printf("       PCI CLK %s MHz INP CLK %s MHz VCO CLK %s MHz\n",
		       strmhz(buf1, gd->pci_clk),
		       strmhz(buf2, gd->arch.inp_clk),
		       strmhz(buf3, gd->arch.vco_clk));
#else
		printf("       INP CLK %s MHz VCO CLK %s MHz\n",
		       strmhz(buf1, gd->arch.inp_clk),
		       strmhz(buf2, gd->arch.vco_clk));
#endif
	}

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

#if defined(CONFIG_MCFFEC)
/* Default initializations for MCFFEC controllers.  To override,
 * create a board-specific function called:
 * 	int board_eth_init(bd_t *bis)
 */

int cpu_eth_init(bd_t *bis)
{
	return mcffec_initialize(bis);
}
#endif
