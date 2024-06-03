/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

/* High Level Configuration Options */

#define CONFIG_SYS_TEXT_BASE		0x81200000
#define CONFIG_SYS_FSL_CLK

#define CONFIG_SYS_TIMER_RATE		32768
#define CONFIG_SYS_TIMER_COUNTER	\
	(&((struct gpt_regs *)IMX_GPT1_BASE)->counter)

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_MACH_TYPE	MACH_TYPE_MX25_3DS

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)

/* Physical Memory Map */

#define PHYS_SDRAM_1		0x80000000
#define PHYS_SDRAM_1_SIZE	(64 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	IMX_RAM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE	IMX_RAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Memory Test */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE/2)
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE)

/* Serial Info */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE

/* No NOR flash present */
#define CONFIG_ENV_OFFSET      (6 * 64 * 1024)
#define CONFIG_ENV_SIZE        (8 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0

/* U-Boot general configuration */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size  */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/* Ethernet */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_PHYADDR		0x1f
#define CONFIG_ENV_OVERWRITE

/* ESDHC driver */
#define CONFIG_SYS_FSL_ESDHC_ADDR	IMX_MMC_SDHC1_BASE
#define CONFIG_SYS_FSL_ESDHC_NUM	1

/* PMIC Configs */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_FSL
#define CONFIG_POWER_FSL_MC34704
#define CONFIG_SYS_FSL_PMIC_I2C_ADDR	0x54

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */

/* RTC */
#define CONFIG_RTC_IMXDI

/* Fuse API support */
#define CONFIG_FSL_IIM
#define CONFIG_CMD_FUSE

/* Ethernet Configs */


#define CONFIG_LOADADDR		0x81000000	/* loadaddr env var */
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdt_addr=0x82000000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"update_sd_firmware_filename=u-boot.imx\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x2 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=load mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
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
	"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
		"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
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
		"fi;\0"

#define CONFIG_BOOTCOMMAND \
	   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadbootscript; then " \
			   "run bootscript; " \
		   "else " \
			   "if run loadimage; then " \
				   "run mmcboot; " \
			   "else run netboot; " \
			   "fi; " \
		   "fi; " \
	   "else run netboot; fi"

/* Miscellaneous configurable options */

#endif /* __CONFIG_H */
