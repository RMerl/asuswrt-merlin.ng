/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* ram memory-related information */
#define PHYS_SDRAM_1			0x40000000
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define PHYS_SDRAM_1_SIZE		0x3E000000

#define CFG_SYS_HZ_CLOCK		750000000	/* 750 MHz */

/* Environment */

/*
 * For booting Linux, use the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ		SZ_256M

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#define CFG_EXTRA_ENV_SETTINGS				\
			"kernel_addr_r=0x40000000\0"		\
			"fdtfile=stih410-b2260.dtb\0"		\
			"fdt_addr_r=0x47000000\0"		\
			"scriptaddr=0x50000000\0"		\
			"pxefile_addr_r=0x50100000\0"		\
			"ramdisk_addr_r=0x48000000\0"		\
			BOOTENV

/* Extra Commands */

/* USB Configs */

/* NET Configs */

#endif /* __CONFIG_H */
