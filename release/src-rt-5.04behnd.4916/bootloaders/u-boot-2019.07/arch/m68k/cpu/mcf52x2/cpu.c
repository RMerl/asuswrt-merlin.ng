// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * MCF5282 additionals
 * (C) Copyright 2005
 * BuS Elektronik GmbH & Co. KG <esw@bus-elektronik.de>
 *
 * MCF5275 additions
 * Copyright (C) 2008 Arthur Shipkowski (art@videon-central.com)
 *
 * Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/immap.h>
#include <asm/io.h>
#include <netdev.h>
#include "cpu.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef	CONFIG_M5208
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	rcm_t *rcm = (rcm_t *)(MMAP_RCM);

	udelay(1000);

	out_8(&rcm->rcr, RCM_RCR_SOFTRST);

	/* we don't return! */
	return 0;
};

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf1[32], buf2[32];

	printf("CPU:   Freescale Coldfire MCF5208\n"
	       "       CPU CLK %s MHz BUS CLK %s MHz\n",
	       strmhz(buf1, gd->cpu_clk),
	       strmhz(buf2, gd->bus_clk));
	return 0;
};
#endif /* CONFIG_DISPLAY_CPUINFO */

#if defined(CONFIG_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	out_be16(&wdt->sr, 0x5555);
	out_be16(&wdt->sr, 0xaaaa);
}

int watchdog_disable(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	/* reset watchdog counter */
	out_be16(&wdt->sr, 0x5555);
	out_be16(&wdt->sr, 0xaaaa);
	/* disable watchdog timer */
	out_be16(&wdt->cr, 0);

	puts("WATCHDOG:disabled\n");
	return (0);
}

int watchdog_init(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	/* disable watchdog */
	out_be16(&wdt->cr, 0);

	/* set timeout and enable watchdog */
	out_be16(&wdt->mr,
		(CONFIG_WATCHDOG_TIMEOUT * CONFIG_SYS_HZ) / (32768 * 1000) - 1);

	/* reset watchdog counter */
	out_be16(&wdt->sr, 0x5555);
	out_be16(&wdt->sr, 0xaaaa);

	puts("WATCHDOG:enabled\n");
	return (0);
}
#endif				/* #ifdef CONFIG_WATCHDOG */
#endif				/* #ifdef CONFIG_M5208 */

#ifdef  CONFIG_M5271
#if defined(CONFIG_DISPLAY_CPUINFO)
/*
 * Both MCF5270 and MCF5271 are members of the MPC5271 family. Try to
 * determine which one we are running on, based on the Chip Identification
 * Register (CIR).
 */
int print_cpuinfo(void)
{
	char buf[32];
	unsigned short cir;	/* Chip Identification Register */
	unsigned short pin;	/* Part identification number */
	unsigned char prn;	/* Part revision number */
	char *cpu_model;

	cir = mbar_readShort(MCF_CCM_CIR);
	pin = cir >> MCF_CCM_CIR_PIN_LEN;
	prn = cir & MCF_CCM_CIR_PRN_MASK;

	switch (pin) {
	case MCF_CCM_CIR_PIN_MCF5270:
		cpu_model = "5270";
		break;
	case MCF_CCM_CIR_PIN_MCF5271:
		cpu_model = "5271";
		break;
	default:
		cpu_model = NULL;
		break;
	}

	if (cpu_model)
		printf("CPU:   Freescale ColdFire MCF%s rev. %hu, at %s MHz\n",
		       cpu_model, prn, strmhz(buf, CONFIG_SYS_CLK));
	else
		printf("CPU:   Unknown - Freescale ColdFire MCF5271 family"
		       " (PIN: 0x%x) rev. %hu, at %s MHz\n",
		       pin, prn, strmhz(buf, CONFIG_SYS_CLK));

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	/* Call the board specific reset actions first. */
	if(board_reset) {
		board_reset();
	}

	mbar_writeByte(MCF_RCM_RCR,
		       MCF_RCM_RCR_SOFTRST | MCF_RCM_RCR_FRCRSTOUT);
	return 0;
};

#if defined(CONFIG_WATCHDOG)
void watchdog_reset(void)
{
	mbar_writeShort(MCF_WTM_WSR, 0x5555);
	mbar_writeShort(MCF_WTM_WSR, 0xAAAA);
}

int watchdog_disable(void)
{
	mbar_writeShort(MCF_WTM_WCR, 0);
	return (0);
}

int watchdog_init(void)
{
	mbar_writeShort(MCF_WTM_WCR, MCF_WTM_WCR_EN);
	return (0);
}
#endif				/* #ifdef CONFIG_WATCHDOG */

#endif

#ifdef	CONFIG_M5272
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	wdog_t *wdp = (wdog_t *) (MMAP_WDOG);

	out_be16(&wdp->wdog_wrrr, 0);
	udelay(1000);

	/* enable watchdog, set timeout to 0 and wait */
	out_be16(&wdp->wdog_wrrr, 1);
	while (1) ;

	/* we don't return! */
	return 0;
};

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	sysctrl_t *sysctrl = (sysctrl_t *) (MMAP_CFG);
	uchar msk;
	char *suf;

	puts("CPU:   ");
	msk = (in_be32(&sysctrl->sc_dir) > 28) & 0xf;
	switch (msk) {
	case 0x2:
		suf = "1K75N";
		break;
	case 0x4:
		suf = "3K75N";
		break;
	default:
		suf = NULL;
		printf("Freescale MCF5272 (Mask:%01x)\n", msk);
		break;
	}

	if (suf)
		printf("Freescale MCF5272 %s\n", suf);
	return 0;
};
#endif /* CONFIG_DISPLAY_CPUINFO */

#if defined(CONFIG_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	out_be16(&wdt->wdog_wcr, 0);
}

int watchdog_disable(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	/* reset watchdog counter */
	out_be16(&wdt->wdog_wcr, 0);
	/* disable watchdog interrupt */
	out_be16(&wdt->wdog_wirr, 0);
	/* disable watchdog timer */
	out_be16(&wdt->wdog_wrrr, 0);

	puts("WATCHDOG:disabled\n");
	return (0);
}

int watchdog_init(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	/* disable watchdog interrupt */
	out_be16(&wdt->wdog_wirr, 0);

	/* set timeout and enable watchdog */
	out_be16(&wdt->wdog_wrrr,
		(CONFIG_WATCHDOG_TIMEOUT * CONFIG_SYS_HZ) / (32768 * 1000) - 1);

	/* reset watchdog counter */
	out_be16(&wdt->wdog_wcr, 0);

	puts("WATCHDOG:enabled\n");
	return (0);
}
#endif				/* #ifdef CONFIG_WATCHDOG */

#endif				/* #ifdef CONFIG_M5272 */

#ifdef	CONFIG_M5275
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	rcm_t *rcm = (rcm_t *)(MMAP_RCM);

	udelay(1000);

	out_8(&rcm->rcr, RCM_RCR_SOFTRST);

	/* we don't return! */
	return 0;
};

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf[32];

	printf("CPU:   Freescale Coldfire MCF5275 at %s MHz\n",
			strmhz(buf, CONFIG_SYS_CLK));
	return 0;
};
#endif /* CONFIG_DISPLAY_CPUINFO */

#if defined(CONFIG_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	out_be16(&wdt->wsr, 0x5555);
	out_be16(&wdt->wsr, 0xaaaa);
}

int watchdog_disable(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	/* reset watchdog counter */
	out_be16(&wdt->wsr, 0x5555);
	out_be16(&wdt->wsr, 0xaaaa);

	/* disable watchdog timer */
	out_be16(&wdt->wcr, 0);

	puts("WATCHDOG:disabled\n");
	return (0);
}

int watchdog_init(void)
{
	wdog_t *wdt = (wdog_t *)(MMAP_WDOG);

	/* disable watchdog */
	out_be16(&wdt->wcr, 0);

	/* set timeout and enable watchdog */
	out_be16(&wdt->wmr,
		(CONFIG_WATCHDOG_TIMEOUT * CONFIG_SYS_HZ) / (32768 * 1000) - 1);

	/* reset watchdog counter */
	out_be16(&wdt->wsr, 0x5555);
	out_be16(&wdt->wsr, 0xaaaa);

	puts("WATCHDOG:enabled\n");
	return (0);
}
#endif				/* #ifdef CONFIG_WATCHDOG */

#endif				/* #ifdef CONFIG_M5275 */

#ifdef	CONFIG_M5282
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	unsigned char resetsource = MCFRESET_RSR;

	printf("CPU:   Freescale Coldfire MCF5282 (PIN: %2.2x REV: %2.2x)\n",
	       MCFCCM_CIR >> 8, MCFCCM_CIR & MCFCCM_CIR_PRN_MASK);
	printf("Reset:%s%s%s%s%s%s%s\n",
	       (resetsource & MCFRESET_RSR_LOL) ? " Loss of Lock" : "",
	       (resetsource & MCFRESET_RSR_LOC) ? " Loss of Clock" : "",
	       (resetsource & MCFRESET_RSR_EXT) ? " External" : "",
	       (resetsource & MCFRESET_RSR_POR) ? " Power On" : "",
	       (resetsource & MCFRESET_RSR_WDR) ? " Watchdog" : "",
	       (resetsource & MCFRESET_RSR_SOFT) ? " Software" : "",
	       (resetsource & MCFRESET_RSR_LVD) ? " Low Voltage" : "");
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	MCFRESET_RCR = MCFRESET_RCR_SOFTRST;
	return 0;
};
#endif

#ifdef CONFIG_M5249
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf[32];

	printf("CPU:   Freescale Coldfire MCF5249 at %s MHz\n",
	       strmhz(buf, CONFIG_SYS_CLK));
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	/* enable watchdog, set timeout to 0 and wait */
	mbar_writeByte(MCFSIM_SYPCR, 0xc0);
	while (1) ;

	/* we don't return! */
	return 0;
};
#endif

#ifdef CONFIG_M5253
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf[32];

	unsigned char resetsource = mbar_readLong(SIM_RSR);
	printf("CPU:   Freescale Coldfire MCF5253 at %s MHz\n",
	       strmhz(buf, CONFIG_SYS_CLK));

	if ((resetsource & SIM_RSR_HRST) || (resetsource & SIM_RSR_SWTR)) {
		printf("Reset:%s%s\n",
		       (resetsource & SIM_RSR_HRST) ? " Hardware/ System Reset"
		       : "",
		       (resetsource & SIM_RSR_SWTR) ? " Software Watchdog" :
		       "");
	}
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	/* enable watchdog, set timeout to 0 and wait */
	mbar_writeByte(SIM_SYPCR, 0xc0);
	while (1) ;

	/* we don't return! */
	return 0;
};
#endif

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
