/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6DL aristainetos2 board.
 */
#ifndef __ARISTAINETOS2_CONFIG_H
#define __ARISTAINETOS2_CONFIG_H

#define CONFIG_SYS_BOARD_VERSION	2
#define CONFIG_HOSTNAME		"aristainetos2"
#define CONFIG_BOARDNAME	"aristainetos2"

#define CONFIG_MXC_UART_BASE	UART2_BASE
#define CONSOLE_DEV	"ttymxc1"

#define CONFIG_FEC_XCV_TYPE		RGMII

#define CONFIG_EXTRA_ENV_BOARD_SETTINGS \
	"board_type=aristainetos2_7@1\0" \
	"nor_bootdelay=-2\0" \
	"mtdids=nand0=gpmi-nand,nor0=spi3.1\0" \
	"mtdparts=mtdparts=spi3.1:832k(u-boot),64k(env),64k(env-red)," \
		"-(rescue-system);gpmi-nand:-(ubi)\0" \
	"addmisc=setenv bootargs ${bootargs} net.ifnames=0 consoleblank=0\0" \
	"ubiargs=setenv bootargs console=${console},${baudrate} " \
		"ubi.mtd=0,4096 root=ubi0:rootfs rootfstype=ubifs\0 " \
	"ubifs_load_fit=sf probe;ubi part ubi 4096;ubifsmount ubi:rootfs;" \
		"ubifsload ${fit_addr_r} /boot/system.itb; " \
		"imi ${fit_addr_r}\0 "

#define CONFIG_SYS_I2C_MXC_I2C4		/* enable I2C bus 4 */

#define ARISTAINETOS_USB_OTG_PWR	IMX_GPIO_NR(4, 15)
#define ARISTAINETOS_USB_H1_PWR	IMX_GPIO_NR(1, 0)
#define CONFIG_GPIO_ENABLE_SPI_FLASH	IMX_GPIO_NR(2, 15)

/* Framebuffer */
#define CONFIG_SYS_LDB_CLOCK 33246000
#define CONFIG_LG4573
#define CONFIG_LG4573_BUS 0
#define CONFIG_LG4573_CS 0

#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK	66000000

#include "aristainetos-common.h"

#endif                         /* __ARISTAINETOS2_CONFIG_H */
