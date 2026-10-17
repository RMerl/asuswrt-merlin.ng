/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#ifndef __IMX8MM_DATA_MODUL_EDM_SBC_H
#define __IMX8MM_DATA_MODUL_EDM_SBC_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#ifdef CONFIG_SPL_BUILD
#define CFG_MALLOC_F_ADDR		0x930000

/* For RAW image gives a error info not panic */

#endif

/* Link Definitions */
#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	0x200000

#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x40000000 /* Minimum 1 GiB DDR */

#define CFG_MXC_UART_BASE		UART3_BASE_ADDR

/* PHY needs a longer autonegotiation timeout after reset */
#define PHY_ANEG_TIMEOUT		20000

/* USDHC */
#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR	0

#define CFG_EXTRA_ENV_SETTINGS						\
	"altbootcmd=setenv devpart 2 && run bootcmd ; reset\0"		\
	"bootlimit=3\0"							\
	"devtype=mmc\0"							\
	"devpart=1\0"							\
	"dfu_alt_info="							\
		/* RAM block at DRAM offset 256..768 MiB */		\
		"ram ram0=ram ram 0x50000000 0x20000000&"		\
		/* 16 MiB SPI NOR */					\
		"mtd nor0=sf raw 0x0 0x1000000\0"			\
	"dmo_preboot="							\
		"sf probe ; " /* Scan for SPI NOR, needed by DFU */	\
		/* Attempt to start USB and Network console */		\
		"run dmo_usb_cdc_acm_start ; "				\
		"run dmo_netconsole_start\0"				\
	"dmo_update_env="						\
		"setenv dmo_update_env true ; saveenv ; saveenv\0"	\
	"dmo_usb_cdc_acm_start="					\
		"if test \"${dmo_usb_cdc_acm_enabled}\" = \"true\" ; then "\
			/* Ungate IMX8MM_CLK_USB1_CTRL_ROOT */		\
			"mw 0x303844d0 3 ; "				\
			/* Read USBNC_n_PHY_STATUS BIT(4) VBUS_VLD */	\
			"setexpr.l usbnc_n_phy_status *0x32e4023c \\\\& 0x8 ; "	\
			/* If USB OTG has valid VBUS, enable CDC ACM */	\
			"if test \"${usbnc_n_phy_status}\" -eq 8 ; then "\
				"usb start && "				\
				"setenv stderr ${stderr},usbacm && "	\
				"setenv stdout ${stdout},usbacm && "	\
				"setenv stdin ${stdin},usbacm ; "	\
			"fi ; "						\
		"fi\0"							\
	"dmo_netconsole_start="						\
		"if test \"${dmo_netconsole_enabled}\" = \"true\" ; then "\
			"setenv autoload false && "			\
			"dhcp && "					\
			"setenv autoload && "				\
			"setenv ncip ${serverip} && "			\
			"setenv stderr ${stderr},nc && "		\
			"setenv stdout ${stdout},nc && "		\
			"setenv stdin ${stdin},nc ; "			\
		"fi\0"							\
	"stdin=serial\0"						\
	"stdout=serial\0"						\
	"stderr=serial\0"						\
	/* Give slow devices beyond USB HUB chance to come up. */	\
	"usb_pgood_delay=2000\0"

#endif
