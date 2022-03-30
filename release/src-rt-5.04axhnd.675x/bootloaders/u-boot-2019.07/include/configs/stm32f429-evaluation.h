/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) STMicroelectronics SA 2017
 * Author(s): Patrice CHOTARD, <patrice.chotard@st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_FLASH_BASE		0x08000000

#define CONFIG_SYS_INIT_SP_ADDR		0x10010000

/*
 * Configuration of the external SDRAM memory
 */
#define CONFIG_SYS_LOAD_ADDR		0x00400000
#define CONFIG_LOADADDR			0x00400000

#define CONFIG_SYS_MAX_FLASH_SECT	12
#define CONFIG_SYS_MAX_FLASH_BANKS	2

#define CONFIG_ENV_OFFSET		(256 << 10)
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#define CONFIG_ENV_SIZE			(8 << 10)

#define CONFIG_STM32_FLASH

#define CONFIG_SYS_HZ_CLOCK		1000000	/* Timer is clocked at 1MHz */

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_SYS_CBSIZE		1024

#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS				\
			"kernel_addr_r=0x00008000\0"		\
			"fdtfile=stm32429i-eval.dtb\0"	\
			"fdt_addr_r=0x00700000\0"		\
			"scriptaddr=0x00800000\0"		\
			"pxefile_addr_r=0x00800000\0" \
			"fdt_high=0xffffffffffffffff\0"		\
			"initrd_high=0xffffffffffffffff\0"	\
			"ramdisk_addr_r=0x00900000\0"		\
			BOOTENV

/*
 * Command line configuration.
 */

#endif /* __CONFIG_H */
