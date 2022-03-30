/*
 * mmc_host_def.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef MMC_HOST_DEF_H
#define MMC_HOST_DEF_H

#include <asm/omap_mmc.h>

/*
 * OMAP HSMMC register definitions
 */
#define OMAP_HSMMC1_BASE		0x48060000
#define OMAP_HSMMC2_BASE		0x481D8000

#if defined(CONFIG_TI814X)
#undef MMC_CLOCK_REFERENCE
#define MMC_CLOCK_REFERENCE	192 /* MHz */
#elif defined(CONFIG_TI816X)
#undef MMC_CLOCK_REFERENCE
#define MMC_CLOCK_REFERENCE	48 /* MHz */
#endif

#endif /* MMC_HOST_DEF_H */
