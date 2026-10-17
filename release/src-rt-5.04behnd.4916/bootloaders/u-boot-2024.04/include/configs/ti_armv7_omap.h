/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ti_armv7_omap.h
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - https://www.ti.com/
 *
 * The various ARMv7 SoCs from TI all share a number of IP blocks when
 * implementing a given feature. This is meant to isolate the features
 * that are based on OMAP architecture.
 */
#ifndef __CONFIG_TI_ARMV7_OMAP_H__
#define __CONFIG_TI_ARMV7_OMAP_H__

/*
 * GPMC NAND block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#ifdef CONFIG_MTD_RAW_NAND
#ifndef CFG_SYS_NAND_BASE
#define CFG_SYS_NAND_BASE		0x8000000
#endif
#endif

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_TI_ARMV7_OMAP_H__ */
