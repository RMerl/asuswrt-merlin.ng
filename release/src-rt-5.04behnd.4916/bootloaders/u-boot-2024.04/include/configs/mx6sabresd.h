/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreSD board.
 */

#ifndef __MX6SABRESD_CONFIG_H
#define __MX6SABRESD_CONFIG_H

#define CFG_MXC_UART_BASE	UART1_BASE
#define CONSOLE_DEV		"ttymxc0"

#include "mx6sabre_common.h"

/* Falcon Mode */

/* Falcon Mode - MMC support: args@1MB kernel@2MB */

#define CFG_SYS_FSL_USDHC_NUM	3

#ifdef CONFIG_CMD_PCI
#define CFG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(7, 12)
#define CFG_PCIE_IMX_POWER_GPIO	IMX_GPIO_NR(3, 19)
#endif

/* USB Configs */
#ifdef CONFIG_CMD_USB
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0
#endif

#endif                         /* __MX6SABRESD_CONFIG_H */
