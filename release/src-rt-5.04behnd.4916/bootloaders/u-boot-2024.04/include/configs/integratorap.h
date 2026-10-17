/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Integrator AP board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "integrator-common.h"

/* Integrator/AP-specific configuration */
#define CFG_SYS_HZ_CLOCK		24000000	/* Timer 1 is clocked at 24Mhz */

/* Flash settings */
#define CFG_SYS_FLASH_SIZE		0x02000000 /* 32 MiB */

/*-----------------------------------------------------------------------
 * PCI definitions
 */

#endif	/* __CONFIG_H */
