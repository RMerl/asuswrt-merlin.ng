/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreAuto board.
 */

#ifndef __MX6SABREAUTO_CONFIG_H
#define __MX6SABREAUTO_CONFIG_H

#define CFG_MXC_UART_BASE	UART4_BASE
#define CONSOLE_DEV		"ttymxc3"

/* USB Configs */
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS	0

#define CFG_SYS_I2C_PCA953X_WIDTH	{ {0x30, 8}, {0x32, 8}, {0x34, 8} }

#include "mx6sabre_common.h"

/* Falcon Mode */
#ifdef CONFIG_SPL_OS_BOOT
/* Falcon Mode - MMC support: args@1MB kernel@2MB */
#endif

#ifdef CONFIG_MTD_NOR_FLASH
#define CFG_SYS_FLASH_BASE           WEIM_ARB_BASE_ADDR
#endif

#define CFG_SYS_FSL_USDHC_NUM	2

/* NAND stuff */
#define CFG_SYS_NAND_BASE           0x40000000

/* DMA stuff, needed for GPMI/MXS NAND support */

#endif                         /* __MX6SABREAUTO_CONFIG_H */
