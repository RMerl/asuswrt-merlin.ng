/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Marek Behún <kabel@kernel.org>
 * Copyright (C) 2016 Tomas Hlavacek <tomas.hlavacek@nic.cz>
 */

#ifndef _CONFIG_TURRIS_OMNIA_H
#define _CONFIG_TURRIS_OMNIA_H

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

/* Environment in SPI NOR flash */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define RELOCATION_LIMITS_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Include the common distro boot environment */
#ifndef CONFIG_SPL_BUILD

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(NVME, nvme, 0) \
	func(SCSI, scsi, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#define KERNEL_ADDR_R	__stringify(0x1000000)
#define FDT_ADDR_R	__stringify(0x2000000)
#define RAMDISK_ADDR_R	__stringify(0x2200000)
#define SCRIPT_ADDR_R	__stringify(0x1800000)
#define PXEFILE_ADDR_R	__stringify(0x1900000)

#define LOAD_ADDRESS_ENV_SETTINGS \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0"

#include <config_distro_bootcmd.h>

/*
 * The factory reset bootcommand on Omnia first sets all the front LEDs to green
 * and then tries to load the rescue image from SPI flash memory and boot it
 */
#define TURRIS_OMNIA_BOOTCMD_RESCUE \
	"i2c dev 2; " \
	"i2c mw 0x2a.1 0x3 0x1c 1; " \
	"i2c mw 0x2a.1 0x4 0x1c 1; " \
	"mw.l 0x01000000 0x00ff000c; " \
	"i2c write 0x01000000 0x2a.1 0x5 4 -s; " \
	"setenv bootargs \"earlyprintk console=ttyS0,115200" \
		" omniarescue=$omnia_reset rescue_mode=$omnia_reset\"; " \
	"sf probe; " \
	"sf read 0x1000000 0x100000 0x700000; " \
	"lzmadec 0x1000000 0x1700000; " \
	"if gpio input gpio@71_4; then " \
		"bootm 0x1700000#sfp; " \
	"else " \
		"bootm 0x1700000; " \
	"fi; " \
	"bootz 0x1000000"

#define CFG_EXTRA_ENV_SETTINGS \
	RELOCATION_LIMITS_ENV_SETTINGS \
	LOAD_ADDRESS_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"console=ttyS0,115200\0" \
	"bootcmd_rescue=" TURRIS_OMNIA_BOOTCMD_RESCUE "\0" \
	BOOTENV

#endif /* CONFIG_SPL_BUILD */

#endif /* _CONFIG_TURRIS_OMNIA_H */
