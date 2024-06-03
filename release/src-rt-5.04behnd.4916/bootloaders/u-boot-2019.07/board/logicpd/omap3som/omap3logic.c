// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Logic Product Development <www.logicpd.com>
 *
 * Author :
 *	Peter Barada <peter.barada@logicpd.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 */
#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <netdev.h>
#include <flash.h>
#include <nand.h>
#include <i2c.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/omap_mmc.h>
#include <asm/mach-types.h>
#include <linux/mtd/rawnand.h>
#include <asm/omap_musb.h>
#include <linux/errno.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/musb.h>
#include "omap3logic.h"
#ifdef CONFIG_USB_EHCI_HCD
#include <usb.h>
#include <asm/ehci-omap.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG1	0x00011203
#define LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG2	0x000A1302
#define LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG3	0x000F1302
#define LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG4	0x0A021303
#define LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG5	0x00120F18
#define LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG6	0x0A030000
#define LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG7	0x00000C50

#define LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG1	0x00011203
#define LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG2	0x00091102
#define LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG3	0x000D1102
#define LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG4	0x09021103
#define LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG5	0x00100D15
#define LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG6	0x09030000
#define LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG7	0x00000C50

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	return serial_tstc() && serial_getc() == 'c';
}
#endif

#if defined(CONFIG_SPL_BUILD)
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on the first bank.  This
 * provides the timing values back to the function that configures
 * the memory.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	timings->mr = MICRON_V_MR_165;

	if (get_cpu_family() == CPU_OMAP36XX) {
		/* 200 MHz works for OMAP36/DM37 */
		/* 256MB DDR */
		timings->mcfg = MICRON_V_MCFG_200(256 << 20);
		timings->ctrla = MICRON_V_ACTIMA_200;
		timings->ctrlb = MICRON_V_ACTIMB_200;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
	} else {
		/* 165 MHz works for OMAP35 */
		timings->mcfg = MICRON_V_MCFG_165(256 << 20);
		timings->ctrla = MICRON_V_ACTIMA_165;
		timings->ctrlb = MICRON_V_ACTIMB_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	}
}

#define GPMC_NAND_COMMAND_0 (OMAP34XX_GPMC_BASE + 0x7c)
#define GPMC_NAND_DATA_0 (OMAP34XX_GPMC_BASE + 0x84)
#define GPMC_NAND_ADDRESS_0 (OMAP34XX_GPMC_BASE + 0x80)

void spl_board_prepare_for_linux(void)
{
	/* The Micron NAND starts locked which
	 * prohibits mounting the NAND as RW
	 * The following commands are what unlocks
	 * the NAND to become RW Falcon Mode does not
	 * have as many smarts as U-Boot, but Logic PD
	 * only makes NAND with 512MB so these hard coded
	 * values should work for all current models
	 */

	writeb(0x70, GPMC_NAND_COMMAND_0);
	writeb(-1, GPMC_NAND_DATA_0);
	writeb(0x7a, GPMC_NAND_COMMAND_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(-1, GPMC_NAND_COMMAND_0);

	/* Begin address 0 */
	writeb(NAND_CMD_UNLOCK1, 0x6e00007c);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(-1, GPMC_NAND_DATA_0);

	/* Ending address at the end of Flash */
	writeb(NAND_CMD_UNLOCK2, GPMC_NAND_COMMAND_0);
	writeb(0xc0, GPMC_NAND_ADDRESS_0);
	writeb(0xff, GPMC_NAND_ADDRESS_0);
	writeb(0x03, GPMC_NAND_ADDRESS_0);
	writeb(-1, GPMC_NAND_DATA_0);
	writeb(0x79, GPMC_NAND_COMMAND_0);
	writeb(-1, GPMC_NAND_DATA_0);
	writeb(-1, GPMC_NAND_DATA_0);
}
#endif

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	twl4030_power_init();
	omap_die_id_display();
	return 0;
}

#if defined(CONFIG_FLASH_CFI_DRIVER)
static const u32 gpmc_dm37_c2nor_config[] = {
	LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG1,
	LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG2,
	LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG3,
	LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG4,
	LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG5,
	LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG6,
	LOGIC_MT28_DM37_ASYNC_GPMC_CONFIG7
};

static const u32 gpmc_omap35_c2nor_config[] = {
	LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG1,
	LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG2,
	LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG3,
	LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG4,
	LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG5,
	LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG6,
	LOGIC_MT28_OMAP35_ASYNC_GPMC_CONFIG7
};
#endif

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);
#if defined(CONFIG_FLASH_CFI_DRIVER)
	if (get_cpu_family() == CPU_OMAP36XX) {
		/* Enable CS2 for NOR Flash */
		enable_gpmc_cs_config(gpmc_dm37_c2nor_config, &gpmc_cfg->cs[2],
				      0x10000000, GPMC_SIZE_64M);
	} else {
		enable_gpmc_cs_config(gpmc_omap35_c2nor_config, &gpmc_cfg->cs[2],
				      0x10000000, GPMC_SIZE_64M);
	}
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT

static void unlock_nand(void)
{
	int dev = nand_curr_device;
	struct mtd_info *mtd;

	mtd = get_nand_dev_by_index(dev);
	nand_unlock(mtd, 0, mtd->size, 0);
}

int board_late_init(void)
{
#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
	unlock_nand();
#endif
	return 0;
}
#endif

#if defined(CONFIG_MMC)
void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

#ifdef CONFIG_SMC911X
/* GPMC CS1 settings for Logic SOM LV/Torpedo LAN92xx Ethernet chip */
static const u32 gpmc_lan92xx_config[] = {
	NET_LAN92XX_GPMC_CONFIG1,
	NET_LAN92XX_GPMC_CONFIG2,
	NET_LAN92XX_GPMC_CONFIG3,
	NET_LAN92XX_GPMC_CONFIG4,
	NET_LAN92XX_GPMC_CONFIG5,
	NET_LAN92XX_GPMC_CONFIG6,
};

int board_eth_init(bd_t *bis)
{
	enable_gpmc_cs_config(gpmc_lan92xx_config, &gpmc_cfg->cs[1],
			CONFIG_SMC911X_BASE, GPMC_SIZE_16M);

	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
}
#endif
