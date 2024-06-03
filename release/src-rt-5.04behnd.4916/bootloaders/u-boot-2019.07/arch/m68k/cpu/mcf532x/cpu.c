// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2008, 2012 Freescale Semiconductor, Inc.
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
#ifdef CONFIG_MCF5301x
	case 0x78:
		id = 53010;
		break;
	case 0x77:
		id = 53012;
		break;
	case 0x76:
		id = 53015;
		break;
	case 0x74:
		id = 53011;
		break;
	case 0x73:
		id = 53013;
		break;
#endif
#ifdef CONFIG_MCF532x
	case 0x54:
		id = 5329;
		break;
	case 0x59:
		id = 5328;
		break;
	case 0x61:
		id = 5327;
		break;
	case 0x65:
		id = 5373;
		break;
	case 0x68:
		id = 53721;
		break;
	case 0x69:
		id = 5372;
		break;
	case 0x6B:
		id = 5372;
		break;
#endif
	}

	if (id) {
		char buf1[32], buf2[32];

		printf("Freescale MCF%d (Mask:%01x Version:%x)\n", id, msk,
		       ver);
		printf("       CPU CLK %s MHz BUS CLK %s MHz\n",
		       strmhz(buf1, gd->cpu_clk),
		       strmhz(buf2, gd->bus_clk));
	}

	return 0;
};
#endif /* CONFIG_DISPLAY_CPUINFO */

#if defined(CONFIG_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
	wdog_t *wdp = (wdog_t *) (MMAP_WDOG);

	/* Count register */
	out_be16(&wdp->sr, 0x5555);
	out_be16(&wdp->sr, 0xaaaa);
}

int watchdog_disable(void)
{
	wdog_t *wdp = (wdog_t *) (MMAP_WDOG);

	/* UserManual, once the wdog is disabled, wdog cannot be re-enabled */
	/* halted watchdog timer */
	setbits_be16(&wdp->cr, WTM_WCR_HALTED);

	puts("WATCHDOG:disabled\n");
	return (0);
}

int watchdog_init(void)
{
	wdog_t *wdp = (wdog_t *) (MMAP_WDOG);
	u32 wdog_module = 0;

	/* set timeout and enable watchdog */
	wdog_module = ((CONFIG_SYS_CLK / 1000) * CONFIG_WATCHDOG_TIMEOUT);
#ifdef CONFIG_M5329
	out_be16(&wdp->mr, wdog_module / 8192);
#else
	out_be16(&wdp->mr, wdog_module / 4096);
#endif

	out_be16(&wdp->cr, WTM_WCR_EN);
	puts("WATCHDOG:enabled\n");

	return (0);
}
#endif				/* CONFIG_WATCHDOG */

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
