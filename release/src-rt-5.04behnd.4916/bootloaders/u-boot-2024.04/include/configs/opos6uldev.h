/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Armadeus Systems
 *
 * Configuration settings for the OPOS6ULDev board
 */

#ifndef __OPOS6ULDEV_CONFIG_H
#define __OPOS6ULDEV_CONFIG_H

#include "mx6_common.h"

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define CFG_SYS_SDRAM_BASE		MMDC0_ARB_BASE_ADDR
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* USB */
#ifdef CONFIG_USB_EHCI_MX6
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0
#endif

/* LCD */
#define MXS_LCDIF_BASE MX6UL_LCDIF1_BASE_ADDR

#endif /* __OPOS6ULDEV_CONFIG_H */
