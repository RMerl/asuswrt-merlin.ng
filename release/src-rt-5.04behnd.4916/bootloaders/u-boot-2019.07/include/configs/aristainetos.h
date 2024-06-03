/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreSD board.
 */
#ifndef __ARISTAINETOS_CONFIG_H
#define __ARISTAINETOS_CONFIG_H

#define CONFIG_SYS_BOARD_VERSION	1
#define CONFIG_HOSTNAME		"aristainetos"
#define CONFIG_BOARDNAME	"aristainetos"

#define CONFIG_MXC_UART_BASE	UART5_BASE
#define CONSOLE_DEV	"ttymxc4"

#define CONFIG_FEC_XCV_TYPE		RMII

#define CONFIG_EXTRA_ENV_BOARD_SETTINGS \
	"board_type=aristainetos7@1\0" \
	"mtdids=nand0=gpmi-nand,nor0=spi3.0\0" \
	"mtdparts=mtdparts=spi3.0:832k(u-boot),64k(env),64k(env-red)," \
		"-(rescue-system);gpmi-nand:-(ubi)\0" \
	"addmisc=setenv bootargs ${bootargs} consoleblank=0\0" \
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"ubiargs=setenv bootargs console=${console},${baudrate} " \
		"ubi.mtd=0,2048 root=ubi0:rootfs rootfstype=ubifs\0 " \
	"ubifs_load_fit=sf probe;ubi part ubi 2048;ubifsmount ubi:rootfs;" \
		"ubifsload ${fit_addr_r} /boot/system.itb; " \
		"imi ${fit_addr_r}\0 "

#define ARISTAINETOS_USB_OTG_PWR	IMX_GPIO_NR(4, 15)
#define ARISTAINETOS_USB_H1_PWR		IMX_GPIO_NR(3, 31)
#define CONFIG_GPIO_ENABLE_SPI_FLASH	IMX_GPIO_NR(2, 15)

#include "aristainetos-common.h"

#endif                         /* __ARISTAINETOS_CONFIG_H */
