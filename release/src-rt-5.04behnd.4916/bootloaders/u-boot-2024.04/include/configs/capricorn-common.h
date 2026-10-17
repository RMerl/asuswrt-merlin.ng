/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2018 NXP
 * Copyright 2019 Siemens AG
 */

#ifndef __IMX8X_CAPRICORN_H
#define __IMX8X_CAPRICORN_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#include "siemens-env-common.h"

/* SPL config */
#ifdef CONFIG_SPL_BUILD
#define CFG_MALLOC_F_ADDR		0x00120000

#endif /* CONFIG_SPL_BUILD */

/* ENET1 connects to base board and MUX with ESAI */
#define CFG_FEC_ENET_DEV		1
#define CFG_FEC_MXC_PHYADDR		0x0

/* EEPROM */
#define  EEPROM_I2C_BUS		0 /* I2C0 */
#define  EEPROM_I2C_ADDR	0x50
/* PCA9552 */
#define  PCA9552_1_I2C_BUS	1 /* I2C1 */
#define  PCA9552_1_I2C_ADDR	0x60

/* AHAB */
#ifdef CONFIG_AHAB_BOOT
#define AHAB_ENV "sec_boot=yes\0"
#else
#define AHAB_ENV "sec_boot=no\0"
#endif

#define MFG_ENV_SETTINGS_DEFAULT \
	"mfgtool_args=setenv bootargs console=${console},${baudrate} " \
		"rdinit=/linuxrc " \
		"clk_ignore_unused "\
		"\0" \
	"kboot=booti\0"\
	"bootcmd_mfg=run mfgtool_args;" \
	"if iminfo ${initrd_addr}; then " \
	"if test ${tee} = yes; then " \
		"bootm ${tee_addr} ${initrd_addr} ${fdt_addr}; " \
	"else " \
		"booti ${loadaddr} ${initrd_addr} ${fdt_addr}; " \
	"fi; " \
	"else " \
	    "echo \"Run fastboot ...\"; fastboot 0; "  \
	"fi;\0"

/* Boot M4 */
#define M4_BOOT_ENV \
	"m4_0_image=m4_0.bin\0" \
	"loadm4image_0=fatload mmc ${mmcdev}:${mmcpart} " \
			"${loadaddr} ${m4_0_image}\0" \
	"m4boot_0=run loadm4image_0; dcache flush; bootaux ${loadaddr} 0\0" \

#define CFG_MFG_ENV_SETTINGS \
	MFG_ENV_SETTINGS_DEFAULT \
	"initrd_addr=0x83100000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"emmc_dev=0\0"

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS \
	CFG_MFG_ENV_SETTINGS \
	M4_BOOT_ENV \
	AHAB_ENV \
	ENV_COMMON \
	"script=boot.scr\0" \
	"image=Image\0" \
	"panel=NULL\0" \
	"console=ttyLP2\0" \
	"fdt_addr=0x83000000\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"cntr_addr=0x88000000\0" \
	"cntr_file=os_cntr_signed.bin\0" \
	"initrd_addr=0x83800000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"netdev=eth0\0" \
	"nfsopts=vers=3,udp,rsize=4096,wsize=4096,nolock rw\0" \
	"hostname=capricorn\0" \
	ENV_EMMC \
	ENV_NET

/* Default location for tftp and bootm */

/* On CCP board, USDHC1 is for eMMC */

#define CFG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_2			0x880000000
/* DDR3 board total DDR is 1 GB */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1 GB */
#define PHYS_SDRAM_2_SIZE		0x00000000	/* 0 GB */

#define BOOTAUX_RESERVED_MEM_BASE	0x88000000
#define BOOTAUX_RESERVED_MEM_SIZE	SZ_128M /* Reserve from second 128MB */

#endif /* __IMX8X_CAPRICORN_H */
