/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <config.h>
#include <linux/sizes.h>

/* For booting Linux, use the first 16MB of memory */
#define CFG_SYS_BOOTMAPSZ		SZ_16M

#define CFG_SYS_FLASH_BASE		0x08000000

#define CFG_SYS_HZ_CLOCK		1000000

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>
#define CFG_EXTRA_ENV_SETTINGS				\
			"kernel_addr_r=0xD0008000\0"		\
			"fdtfile=stm32h743i-eval.dtb\0"	\
			"fdt_addr_r=0xD0408000\0"		\
			"scriptaddr=0xD0418000\0"		\
			"pxefile_addr_r=0xD0428000\0" \
			"ramdisk_addr_r=0xD0438000\0"		\
			BOOTENV

#endif /* __CONFIG_H */
