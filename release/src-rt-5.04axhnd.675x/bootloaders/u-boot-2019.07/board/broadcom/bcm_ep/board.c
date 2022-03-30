// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 */

#include <common.h>
#include <asm/io.h>
#include <config.h>
#include <netdev.h>
#include <asm/system.h>
#include <asm/iproc-common/armpll.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * board_init - early hardware init
 */
int board_init(void)
{
	/*
	 * Address of boot parameters passed to kernel
	 * Use default offset 0x100
	 */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

/*
 * dram_init - sets u-boot's idea of sdram size
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

int board_early_init_f(void)
{
	uint32_t status = 0;

	/* Setup PLL if required */
#if defined(CONFIG_ARMCLK)
	armpll_config(CONFIG_ARMCLK);
#endif

	return status;
}

#ifdef CONFIG_ARMV7_NONSEC
void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
}

void smp_kick_all_cpus(void)
{
}

void smp_waitloop(unsigned previous_address)
{
}
#endif

#ifdef CONFIG_BCM_SF2_ETH
int board_eth_init(bd_t *bis)
{
	int rc = -1;
	printf("Registering BCM sf2 eth\n");
	rc = bcm_sf2_eth_register(bis, 0);
	return rc;
}
#endif
