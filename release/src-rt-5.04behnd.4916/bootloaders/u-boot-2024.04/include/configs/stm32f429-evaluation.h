/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) STMicroelectronics SA 2017
 * Author(s): Patrice CHOTARD, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* For booting Linux, use the first 16MB of memory */
#define CFG_SYS_BOOTMAPSZ		SZ_16M

#define CFG_SYS_FLASH_BASE		0x08000000

/*
 * Configuration of the external SDRAM memory
 */

#define CFG_SYS_HZ_CLOCK		1000000	/* Timer is clocked at 1MHz */

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>
#define CFG_EXTRA_ENV_SETTINGS				\
			"kernel_addr_r=0x00008000\0"		\
			"fdtfile=stm32429i-eval.dtb\0"	\
			"fdt_addr_r=0x00408000\0"		\
			"scriptaddr=0x00418000\0"		\
			"pxefile_addr_r=0x00428000\0" \
			"ramdisk_addr_r=0x00438000\0"		\
			BOOTENV

#endif /* __CONFIG_H */
