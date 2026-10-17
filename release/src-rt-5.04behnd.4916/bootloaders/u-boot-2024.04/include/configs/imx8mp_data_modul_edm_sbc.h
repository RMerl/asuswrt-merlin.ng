/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#ifndef __IMX8MP_DATA_MODUL_EDM_SBC_H
#define __IMX8MP_DATA_MODUL_EDM_SBC_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

/* Link Definitions */
#define CFG_SYS_INIT_RAM_ADDR		0x40000000
#define CFG_SYS_INIT_RAM_SIZE		0x200000

#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x40000000 /* Minimum 1 GiB DDR */

#define CFG_MXC_UART_BASE		UART3_BASE_ADDR

/* PHY needs a longer autonegotiation timeout after reset */
#define PHY_ANEG_TIMEOUT		20000
#define FEC_QUIRK_ENET_MAC

#define CFG_EXTRA_ENV_SETTINGS						\
	"altbootcmd=setenv devpart 2 && run bootcmd ; reset\0"		\
	"bootlimit=3\0"							\
	"devtype=mmc\0"							\
	"devpart=1\0"							\
	"dmo_update_env="						\
		"setenv dmo_update_env true ; saveenv ; saveenv\0"	\
	"dmo_update_sf_write_data="					\
		"sf probe && sf update ${loadaddr} 0 ${filesize}\0"	\
	"dmo_update_emmc_to_sf="					\
		"load mmc 0:1 ${loadaddr} boot/flash.bin && "		\
		"run dmo_update_sf_write_data\0"			\
	"dmo_update_sd_to_sf="						\
		"load mmc 1:1 ${loadaddr} boot/flash.bin && "		\
		"run dmo_update_sf_write_data\0"			\
	"stdin=serial\0"						\
	"stdout=serial\0"						\
	"stderr=serial\0"						\
	/* Give slow devices beyond USB HUB chance to come up. */	\
	"usb_pgood_delay=2000\0"

#endif
