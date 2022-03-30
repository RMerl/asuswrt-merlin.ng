/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 * Copyright (C) 2016 Mario Six <mario.six@gdsys.cc>
 */

#ifndef _CONFIG_CONTROLCENTERDC_H
#define _CONFIG_CONTROLCENTERDC_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_CUSTOMER_BOARD_SUPPORT

#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_BOARD_LATE_INIT

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

#define CONFIG_LOADADDR 		1000000

/*
 * SDIO/MMC Card Configuration
 */
#define CONFIG_SYS_MMC_BASE		MVEBU_SDIO_BASE

/*
 * SATA/SCSI/AHCI configuration
 */
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	2
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI

/* Environment in SPI NOR flash */
#define CONFIG_ENV_OFFSET		(1 << 20) /* 1MiB in */
#define CONFIG_ENV_SIZE			(64 << 10) /* 64KiB */
#define CONFIG_ENV_SECT_SIZE		(256 << 10) /* 256KiB sectors */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI_SCAN_SHOW
#endif

/*
 * Software (bit-bang) MII driver configuration
 */
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management */
#define CONFIG_BITBANGMII_MULTI

/* SPL */
/*
 * Select the boot device here
 *
 * Currently supported are:
 * SPL_BOOT_SPI_NOR_FLASH	- Booting via SPI NOR flash
 * SPL_BOOT_SDIO_MMC_CARD	- Booting via SDIO/MMC card (partition 1)
 */
#define SPL_BOOT_SPI_NOR_FLASH		1
#define SPL_BOOT_SDIO_MMC_CARD		2
#define CONFIG_SPL_BOOT_DEVICE		SPL_BOOT_SPI_NOR_FLASH

/* Defines for SPL */
#define CONFIG_SPL_SIZE			(160 << 10)

#if defined(CONFIG_SECURED_MODE_IMAGE)
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_SIZE - 0x2614)
#else
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_SIZE - 0x30)
#endif

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + CONFIG_SPL_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((212 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SPI_NOR_FLASH
/* SPL related SPI defines */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x30000
#define CONFIG_SYS_U_BOOT_OFFS		CONFIG_SYS_SPI_U_BOOT_OFFS
#endif

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SDIO_MMC_CARD
/* SPL related MMC defines */
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION 1
#define CONFIG_SYS_MMC_U_BOOT_OFFS		(168 << 10)
#define CONFIG_SYS_U_BOOT_OFFS			CONFIG_SYS_MMC_U_BOOT_OFFS
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	(CONFIG_SYS_U_BOOT_OFFS / 512)
#ifdef CONFIG_SPL_BUILD
#define CONFIG_FIXED_SDHCI_ALIGNED_BUFFER	0x00180000	/* in SDRAM */
#endif
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE 115200

#define CONFIG_HOSTNAME		"ccdc"
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"ccdc.img"

#define CONFIG_PREBOOT		/* enable preboot variable */

#define CONFIG_EXTRA_ENV_SETTINGS						\
	"netdev=eth1\0"						\
	"consoledev=ttyS1\0"							\
	"u-boot=u-boot.bin\0"							\
	"bootfile_addr=1000000\0"						\
	"keyprogram_addr=3000000\0"						\
	"keyprogram_file=keyprogram.img\0"						\
	"fdtfile=controlcenterdc.dtb\0"						\
	"load=tftpboot ${loadaddr} ${u-boot}\0"					\
	"mmcdev=0:2\0"								\
	"update=sf probe 1:0;"							\
		" sf erase 0 +${filesize};"					\
		" sf write ${fileaddr} 0 ${filesize}\0"				\
	"upd=run load update\0"							\
	"fdt_high=0x10000000\0"							\
	"initrd_high=0x10000000\0"						\
	"loadkeyprogram=tpm flush_keys;"					\
		" mmc rescan;"							\
		" ext4load mmc ${mmcdev} ${keyprogram_addr} ${keyprogram_file};"\
		" source ${keyprogram_addr}:script@1\0"				\
	"gpio1=gpio@22_25\0"							\
	"gpio2=A29\0"								\
	"blinkseq='0 0 0 0 2 0 2 2 3 1 3 1 0 0 2 2 3 1 3 3 2 0 2 2 3 1 1 1 "	\
		  "2 0 2 2 3 1 3 1 0 0 2 0 3 3 3 1 2 0 0 0 3 1 1 1 0 0 0 0'\0"	\
	"bootfail=for i in ${blinkseq}; do"					\
		" if test $i -eq 0; then"					\
		" gpio clear ${gpio1}; gpio set ${gpio2};"			\
		" elif test $i -eq 1; then"					\
		" gpio clear ${gpio1}; gpio clear ${gpio2};"			\
		" elif test $i -eq 2; then"					\
		" gpio set ${gpio1}; gpio set ${gpio2};"			\
		" else;"							\
		" gpio clear ${gpio1}; gpio set ${gpio2};"			\
		" fi; sleep 0.12; done\0"

#define CONFIG_NFSBOOTCOMMAND								\
	"setenv bootargs root=/dev/nfs rw "						\
	"nfsroot=${serverip}:${rootpath} "						\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}:${netdev}:off "	\
	"console=${consoledev},${baudrate} ${othbootargs}; "				\
	"tftpboot ${bootfile_addr} ${bootfile}; "						\
	"bootm ${bootfile_addr}"

#define CONFIG_MMCBOOTCOMMAND					\
	"setenv bootargs root=/dev/mmcblk0p3 rw rootwait "	\
	"console=${consoledev},${baudrate} ${othbootargs}; "	\
	"ext2load mmc 0:2 ${bootfile_addr} ${bootfile}; "	\
	"bootm ${bootfile_addr}"

#define CONFIG_BOOTCOMMAND			\
	"if env exists keyprogram; then;"	\
	" setenv keyprogram; run nfsboot;"	\
        " fi;"					\
        " run dobootfail"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#endif /* _CONFIG_CONTROLCENTERDC_H */
