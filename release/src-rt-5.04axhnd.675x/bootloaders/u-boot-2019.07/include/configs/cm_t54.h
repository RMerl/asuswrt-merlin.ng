/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Config file for Compulab CM-T54 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Dmitry Lifshitz <lifshitz@compulab.co.il>
 */

#ifndef __CONFIG_CM_T54_H
#define __CONFIG_CM_T54_H

#define CONFIG_CM_T54
#define CONFIG_DRAM_2G

#define PARTS_DEFAULT

#include <configs/ti_omap5_common.h>

/* EEPROM related defines */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_I2C_EEPROM_BUS	0

/* Enable SD/MMC CD and WP GPIOs */
#define OMAP_HSMMC_USE_GPIO

/* UART setup */
#define CONFIG_SYS_NS16550_COM4		UART4_BASE

/* MMC ENV related defines */
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE

#define CONFIG_SYS_MMC_ENV_DEV		1		/* SLOT2: eMMC(1) */
#define CONFIG_SYS_MMC_ENV_PART		0
#define CONFIG_ENV_OFFSET		0xc0000		/* (in bytes) 768 KB */
#define CONFIG_ENV_SIZE			(16 << 10)	/* 16 KB */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

/* Enhance our eMMC support / experience. */
#define CONFIG_HSMMC2_8BIT

/* SATA Boot related defines */
#define CONFIG_SPL_SATA_BOOT_DEVICE		0
#define CONFIG_SYS_SATA_FAT_BOOT_PARTITION	1

#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)
/* USB UHH support options */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

#define CONFIG_OMAP_EHCI_PHY2_RESET_GPIO	76 /* HSIC2 HUB #RESET */
#define CONFIG_OMAP_EHCI_PHY3_RESET_GPIO	83 /* HSIC3 ETH #RESET */

/* Enabled commands */

/* EEPROM */
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
#define CONFIG_SYS_EEPROM_SIZE			256

/* USB Networking options */

/*
 * Miscellaneous configurable options
 */
#undef CONFIG_SYS_AUTOLOAD
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_SYS_AUTOLOAD		"no"

#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"baudrate=115200\0" \
	"bootdelay=3\0" \
	"autoload=no\0" \
	"bootscr=bootscr.img\0" \
	"fdtfile=omap5-sbc-t54.dtb\0" \
	"kernel=zImage-cm-t54\0" \
	"ramdisk=ramdisk-cm-t54.img\0" \
	"console=ttyO3\0" \
	"ramdisksize=16384\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk1p2\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=${mmcroot} rw rootwait\0" \
	"ramroot=/dev/ram0\0" \
	"ramargs=setenv bootargs console=${console} " \
		"root=${ramroot} ramdisk_size=${ramdisksize} rw\0" \
	"mmcloadkernel=load mmc ${mmcdev} ${loadaddr} ${kernel}\0" \
	"mmcloadfdt=load mmc ${mmcdev} ${fdtaddr} ${fdtfile}\0" \
	"mmcloadramdisk=load mmc ${mmcdev} ${rdaddr} ${ramdisk}\0" \
	"mmcloadbootscript=load mmc ${mmcdev} ${loadaddr} ${bootsrc}\0" \
	"mmcbootscript=echo Running bootscript from mmc${mmcdev}...; " \
			"source ${loadaddr}\0" \
	"mmcbootlinux=echo Booting from mmc${mmcdev} ...; " \
			"bootz ${loadaddr} ${rdaddr} ${fdtaddr}\0" \
	"mmcboot=if mmc dev ${mmcdev} && mmc rescan; then " \
			"if run mmcloadbootscript; " \
				"then run mmcbootscript; " \
			"fi; " \
			"if run mmcloadkernel; then " \
				"if run mmcloadfdt; then " \
					"if run mmcloadramdisk; then " \
						"run ramargs; " \
						"run mmcbootlinux; " \
					"fi; " \
					"run mmcargs; " \
					"setenv rdaddr - ; " \
					"run mmcbootlinux; " \
				"fi; " \
			"fi; " \
		"fi;\0"

#define CONFIG_BOOTCOMMAND \
	"bootcmd=run mmcboot || setenv mmcdev 1; setenv mmcroot /dev/mmcblk0p2; run mmcboot;"

#endif /* __CONFIG_CM_T54_H */
