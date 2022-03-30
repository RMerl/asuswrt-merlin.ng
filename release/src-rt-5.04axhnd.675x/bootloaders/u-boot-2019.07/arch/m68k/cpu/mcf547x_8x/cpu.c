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
	gptmr_t *gptmr = (gptmr_t *) (MMAP_GPTMR);

	out_be16(&gptmr->pre, 10);
	out_be16(&gptmr->cnt, 1);

	/* enable watchdog, set timeout to 0 and wait */
	out_8(&gptmr->mode, GPT_TMS_SGPIO);
	out_8(&gptmr->ctrl, GPT_CTRL_WDEN | GPT_CTRL_CE);

	/* we don't return! */
	return 1;
};

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	siu_t *siu = (siu_t *) MMAP_SIU;
	u16 id = 0;

	puts("CPU:   ");

	switch ((in_be32(&siu->jtagid) & 0x000FF000) >> 12) {
	case 0x0C:
		id = 5485;
		break;
	case 0x0D:
		id = 5484;
		break;
	case 0x0E:
		id = 5483;
		break;
	case 0x0F:
		id = 5482;
		break;
	case 0x10:
		id = 5481;
		break;
	case 0x11:
		id = 5480;
		break;
	case 0x12:
		id = 5475;
		break;
	case 0x13:
		id = 5474;
		break;
	case 0x14:
		id = 5473;
		break;
	case 0x15:
		id = 5472;
		break;
	case 0x16:
		id = 5471;
		break;
	case 0x17:
		id = 5470;
		break;
	}

	if (id) {
		char buf1[32], buf2[32];

		printf("Freescale MCF%d\n", id);
		printf("       CPU CLK %s MHz BUS CLK %s MHz\n",
		       strmhz(buf1, gd->cpu_clk),
		       strmhz(buf2, gd->bus_clk));
	}

	return 0;
};
#endif /* CONFIG_DISPLAY_CPUINFO */

#if defined(CONFIG_HW_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void hw_watchdog_reset(void)
{
	gptmr_t *gptmr = (gptmr_t *) (MMAP_GPTMR);

	out_8(&gptmr->ocpw, 0xa5);
}

int watchdog_disable(void)
{
	gptmr_t *gptmr = (gptmr_t *) (MMAP_GPTMR);

	/* UserManual, once the wdog is disabled, wdog cannot be re-enabled */
	out_8(&gptmr->mode, 0);
	out_8(&gptmr->ctrl, 0);

	puts("WATCHDOG:disabled\n");

	return (0);
}

int watchdog_init(void)
{
	gptmr_t *gptmr = (gptmr_t *) (MMAP_GPTMR);

	out_be16(&gptmr->pre, CONFIG_WATCHDOG_TIMEOUT);
	out_be16(&gptmr->cnt, CONFIG_SYS_TIMER_PRESCALER * 1000);

	out_8(&gptmr->mode, GPT_TMS_SGPIO);
	out_8(&gptmr->ctrl, GPT_CTRL_CE | GPT_CTRL_WDEN);
	puts("WATCHDOG:enabled\n");

	return (0);
}
#endif				/* CONFIG_HW_WATCHDOG */

#if defined(CONFIG_FSLDMAFEC) || defined(CONFIG_MCFFEC)
/* Default initializations for MCFFEC controllers.  To override,
 * create a board-specific function called:
 * 	int board_eth_init(bd_t *bis)
 */

int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSLDMAFEC)
	mcdmafec_initialize(bis);
#endif
#if defined(CONFIG_MCFFEC)
	mcffec_initialize(bis);
#endif
	return 0;
}
#endif
