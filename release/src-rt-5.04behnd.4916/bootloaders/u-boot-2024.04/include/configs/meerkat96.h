/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Linaro Ltd.
 * Copyright (C) 2016 NXP Semiconductors
 *
 * Configuration settings for Meerkat96 board.
 */

#ifndef __MEERKAT96_CONFIG_H
#define __MEERKAT96_CONFIG_H

#include "mx7_common.h"
#include <imximage.h>

#define PHYS_SDRAM_SIZE			SZ_512M

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Environment configs */

/* USB configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)

#endif
