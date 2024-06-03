/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX53-EVK Freescale board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MACH_TYPE	MACH_TYPE_MX53_EVK

#include <asm/arch/imx-regs.h>

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_SYS_FSL_CLK

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */

/* PMIC Configs */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_FSL
#define CONFIG_SYS_FSL_PMIC_I2C_ADDR    8
#define CONFIG_POWER_FSL_MC13892
#define CONFIG_RTC_MC13XXX

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	2

/* Eth Configs */

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE	FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR	0x1F

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Command definition */

#define CONFIG_ETHPRIME		"FEC0"

#define CONFIG_LOADADDR		0x70800000	/* loadaddr env var */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"uimage=uImage\0" \
	"mmcdev=0\0" \
	"mmcpart=2\0" \
	"mmcroot=/dev/mmcblk0p3 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"mmcargs=setenv bootargs console=ttymxc0,${baudrate} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loaduimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${uimage}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm\0" \
	"netargs=setenv bootargs console=ttymxc0,${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"dhcp ${uimage}; bootm\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run netboot; " \
			"fi; " \
		"fi; " \
	"else run netboot; fi"

#define CONFIG_ARP_TIMEOUT	200UL

/* Miscellaneous configurable options */

#define CONFIG_SYS_MEMTEST_START       0x70000000
#define CONFIG_SYS_MEMTEST_END         0x70010000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/* Physical Memory Map */
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(512 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */
#define CONFIG_ENV_OFFSET      (6 * 64 * 1024)
#define CONFIG_ENV_SIZE        (8 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV 0

#endif				/* __CONFIG_H */
