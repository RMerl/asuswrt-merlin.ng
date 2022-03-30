// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/mipsregs.h>
#include "vct.h"

#if defined(CONFIG_VCT_PREMIUM)
#define BOARD_NAME	"PremiumD"
#elif defined(CONFIG_VCT_PLATINUM)
#define BOARD_NAME	"PlatinumD"
#elif defined(CONFIG_VCT_PLATINUMAVC)
#define BOARD_NAME	"PlatinumAVC"
#else
#error "vct: No board variant defined!"
#endif

#if defined(CONFIG_VCT_ONENAND)
#define BOARD_NAME_ADD	" OneNAND"
#else
#define BOARD_NAME_ADD	" NOR"
#endif

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * First initialize the PIN mulitplexing
	 */
	vct_pin_mux_initialize();

	/*
	 * Init the EBI very early so that FLASH can be accessed
	 */
	ebi_initialize();

	return 0;
}

void _machine_restart(void)
{
	reg_write(DCGU_EN_WDT_RESET(DCGU_BASE), DCGU_MAGIC_WDT);
	reg_write(WDT_TORR(WDT_BASE), 0x00);
	reg_write(WDT_CR(WDT_BASE), 0x1D);

	/*
	 * Now wait for the watchdog to trigger the reset
	 */
	udelay(1000000);
}

/*
 * SDRAM is already configured by the bootstrap code, only return the
 * auto-detected size here
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
			    CONFIG_SYS_MBYTES_SDRAM << 20);

	return 0;
}

int checkboard(void)
{
	char buf[64];
	int i = env_get_f("serial#", buf, sizeof(buf));
	u32 config0 = read_c0_prid();

	if ((config0 & 0xff0000) == PRID_COMP_LEGACY
	    && (config0 & 0xff00) == PRID_IMP_LX4280) {
		puts("Board: MDED \n");
		printf("CPU:   LX4280 id: 0x%02x, rev: 0x%02x\n",
		       (config0 >> 8) & 0xFF, config0 & 0xFF);
	} else if ((config0 & 0xff0000) == PRID_COMP_MIPS
		   && (config0 & 0xff00) == PRID_IMP_VGC) {
		u32 jedec_id = *((u32 *) 0xBEBC71A0);
		if ((((jedec_id) >> 12) & 0xFF) == 0x40) {
			puts("Board: VGCA \n");
		} else if ((((jedec_id) >> 12) & 0xFF) == 0x48
			   || (((jedec_id) >> 12) & 0xFF) == 0x49) {
			puts("Board: VGCB \n");
		}
		printf("CPU:   MIPS 4K id: 0x%02x, rev: 0x%02x\n",
		       (config0 >> 8) & 0xFF, config0 & 0xFF);
	} else if (config0 == 0x19378) {
		printf("CPU:   MIPS 24K id: 0x%02x, rev: 0x%02x\n",
		       (config0 >> 8) & 0xFF, config0 & 0xFF);
	} else {
		printf("Unsupported cpu %d, proc_id=0x%x\n", config0 >> 24,
		       config0);
	}

	printf("Board: Micronas VCT " BOARD_NAME BOARD_NAME_ADD);
	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
