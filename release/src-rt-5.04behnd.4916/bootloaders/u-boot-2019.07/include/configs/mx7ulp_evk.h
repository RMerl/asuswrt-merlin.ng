/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX7ULP EVK board.
 */

#ifndef __MX7ULP_EVK_CONFIG_H
#define __MX7ULP_EVK_CONFIG_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

/*Uncomment it to use secure boot*/
/*#define CONFIG_SECURE_BOOT*/

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE			0x4000
#endif
#endif

#define CONFIG_BOARD_POSTCLK_INIT
#define CONFIG_SYS_BOOTM_LEN		0x1000000

#define SRC_BASE_ADDR			CMC1_RBASE
#define IRAM_BASE_ADDR			OCRAM_0_BASE
#define IOMUXC_BASE_ADDR		IOMUXC1_RBASE

#define CONFIG_FSL_USDHC

#define CONFIG_SYS_FSL_USDHC_NUM        1

#define CONFIG_SYS_FSL_ESDHC_ADDR       0
#define CONFIG_SYS_MMC_ENV_DEV          0	/* USDHC1 */
#define CONFIG_SYS_MMC_ENV_PART         0	/* user area */
#define CONFIG_MMCROOT                  "/dev/mmcblk0p2"  /* USDHC1 */
#define CONFIG_SYS_MMC_IMG_LOAD_PART    1

#define CONFIG_ENV_OFFSET		(12 * SZ_64K)
#define CONFIG_ENV_SIZE			SZ_8K

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG1_RBASE

#define CONFIG_SYS_HZ_CLOCK		1000000 /* Fixed at 1Mhz from TSTMR */

#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
/*#define CONFIG_REVISION_TAG*/

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(8 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F

/* UART */
#define LPUART_BASE			LPUART4_RBASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_CACHELINE_SIZE      64

/* Miscellaneous configurable options */
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		512

#define CONFIG_SYS_MAXARGS		256

/* Physical Memory Map */

#define PHYS_SDRAM			0x60000000
#define PHYS_SDRAM_SIZE			SZ_1G
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CONFIG_LOADADDR             0x60800000

#define CONFIG_SYS_MEMTEST_END      0x9E000000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"console=ttyLP0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=imx7ulp-evk.dtb\0" \
	"fdt_addr=0x63000000\0" \
	"boot_fdt=try\0" \
	"earlycon=lpuart32,0x402D0010\0" \
	"ip_dyn=yes\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
		"ip=:::::eth0:dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"usb start; "\
		"${get_cmd} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \

#define CONFIG_BOOTCOMMAND \
	   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadbootscript; then " \
			   "run bootscript; " \
		   "else " \
			   "if run loadimage; then " \
				   "run mmcboot; " \
			   "fi; " \
		   "fi; " \
	   "fi"

#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	SZ_256K

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
#define CONFIG_CMD_CACHE
#endif

#endif	/* __CONFIG_H */
