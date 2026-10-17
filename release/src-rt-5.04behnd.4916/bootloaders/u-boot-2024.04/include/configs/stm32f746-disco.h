/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* For booting Linux, use the first 6MB of memory */
#define CFG_SYS_BOOTMAPSZ		SZ_4M + SZ_2M

#define CFG_SYS_FLASH_BASE		0x08000000

/*
 * Configuration of the external SDRAM memory
 */

#define CFG_SYS_HZ_CLOCK		1000000	/* Timer is clocked at 1MHz */

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#define STM32F746_BOARD_EXTRA_ENV \
	"splashimage=0xC0448000\0" \
	"splashpos=m,m\0"

#include <config_distro_bootcmd.h>
#define CFG_EXTRA_ENV_SETTINGS				\
			"kernel_addr_r=0xC0008000\0"		\
			"fdtfile="CONFIG_DEFAULT_DEVICE_TREE".dtb\0"	\
			"fdt_addr_r=0xC0408000\0"		\
			"scriptaddr=0xC0418000\0"		\
			"pxefile_addr_r=0xC0428000\0" \
			"ramdisk_addr_r=0xC0438000\0"		\
			BOOTENV	\
			STM32F746_BOARD_EXTRA_ENV

#define CFG_SYS_UBOOT_BASE		(CFG_SYS_FLASH_BASE + \
					 CONFIG_SPL_PAD_TO)

#endif /* __CONFIG_H */
