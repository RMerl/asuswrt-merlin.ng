/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_THEADORABLE_H
#define _CONFIG_THEADORABLE_H

#include <linux/sizes.h>

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

/*
 * The debugging version enables USB support via defconfig.
 * This version should also enable all other non-production
 * interfaces / features.
 */

/* I2C */
#define CFG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE
#define CFG_I2C_MVTWSI_BASE1		MVEBU_TWSI1_BASE

/* USB/EHCI configuration */

/* Environment in SPI NOR flash */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define CFG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/*
 * Bootcounter
 */
/* Max size of RAM minus BOOTCOUNT_ADDR is the bootcounter address */
#define BOOTCOUNT_ADDR			0x1000

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * Memory layout while starting into the bin_hdr via the
 * BootROM:
 *
 * 0x4000.4000 - 0x4003.4000	headers space (192KiB)
 * 0x4000.4030			bin_hdr start address
 * 0x4003.4000 - 0x4004.7c00	BootROM memory allocations (15KiB)
 * 0x4007.fffc			BootROM stack top
 *
 * The address space between 0x4007.fffc and 0x400f.fff is not locked in
 * L2 cache thus cannot be used.
 */

/* SPL */
/* Defines for SPL */

/* Enable DDR support in SPL (DDR3 training from Marvell bin_hdr) */
#define CFG_SYS_SDRAM_SIZE		SZ_2G

#endif /* _CONFIG_THEADORABLE_H */
