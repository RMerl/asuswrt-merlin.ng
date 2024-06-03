// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Nishanth Menon <nm@ti.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 */
#include <common.h>
#include <dm.h>
#include <environment.h>
#include <ns16550.h>
#include <netdev.h>
#include <twl4030.h>
#include <linux/mtd/omap_gpmc.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include "zoom1.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * gpmc_cfg is initialized by gpmc_init and we use it here.
 * GPMC definitions for Ethenet Controller LAN9211
 */
static const u32 gpmc_lab_enet[] = {
	ZOOM1_ENET_GPMC_CONF1,
	ZOOM1_ENET_GPMC_CONF2,
	ZOOM1_ENET_GPMC_CONF3,
	ZOOM1_ENET_GPMC_CONF4,
	ZOOM1_ENET_GPMC_CONF5,
	ZOOM1_ENET_GPMC_CONF6,
	/*CONF7- computed as params */
};

static const struct ns16550_platdata zoom1_serial = {
	.base = OMAP34XX_UART3,
	.reg_shift = 2,
	.clock = V_NS16550_CLK,
	.fcr = UART_FCR_DEFVAL,
};

U_BOOT_DEVICE(zoom1_uart) = {
	"ns16550_serial",
	&zoom1_serial
};

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* CS1 is Ethernet LAN9211 */
	enable_gpmc_cs_config(gpmc_lab_enet, &gpmc_cfg->cs[1],
			      DEBUG_BASE, GPMC_SIZE_16M);
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP_LDP;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Configure zoom board specific configurations
 */
int misc_init_r(void)
{
	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);
	omap_die_id_display();

	/*
	 * Board Reset
	 * The board is reset by holding the red button on the
	 * top right front face for eight seconds.
	 */
	twl4030_power_reset_init();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	/* platform specific muxes */
	MUX_ZOOM1_MDK();
}

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}

void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_SMC911X
#define STR_ENV_ETHADDR	"ethaddr"

	struct eth_device *dev;
	uchar eth_addr[6];

	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
	if (!eth_env_get_enetaddr(STR_ENV_ETHADDR, eth_addr)) {
		dev = eth_get_dev_by_index(0);
		if (dev) {
			eth_env_set_enetaddr(STR_ENV_ETHADDR, dev->enetaddr);
		} else {
			printf("zoom1: Couldn't get eth device\n");
			rc = -1;
		}
	}
#endif

	return rc;
}
#endif
