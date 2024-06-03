/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * hardware.h
 *
 * hardware specific header
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef __OMAP_HARDWARE_H
#define __OMAP_HARDWARE_H

#include <asm/arch/omap.h>

/*
 * Common hardware definitions
 */

/* BCH Error Location Module */
#define ELM_BASE			0x48078000

/* GPMC Base address */
#define GPMC_BASE			0x50000000

/* EDMA3 Base address for DRA7XX and AM57XX */
#if defined(CONFIG_DRA7XX)
#define EDMA3_BASE			0x43300000
#endif

#endif
