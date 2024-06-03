/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor
 */

#ifndef __LS1012A_COMMON_H
#define __LS1012A_COMMON_H

#define CONFIG_GICV2

#include <asm/arch/config.h>
#include <asm/arch/stream_id_lsch2.h>

#define CONFIG_SYS_CLK_FREQ		125000000

#define CONFIG_SKIP_LOWLEVEL_INIT

#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_INIT_SP_ADDR                CONFIG_SYS_TEXT_BASE
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_FSL_OCRAM_BASE + 0xfff0)
#endif
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000)

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE     0x880000000ULL

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		25000000	/* 25MHz */

/* CSU */
#define CONFIG_LAYERSCAPE_NS_ACCESS

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)

/*SPI device */
#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_TFABOOT)
#define CONFIG_SYS_FMAN_FW_ADDR		0x400d0000
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_FSL_SPI_INTERFACE
#define CONFIG_SF_DATAFLASH

#define QSPI0_AMBA_BASE		0x40000000
#define CONFIG_SPI_FLASH_SPANSION

#define FSL_QSPI_FLASH_SIZE		SZ_64M
#define FSL_QSPI_FLASH_NUM		2

/*
 * Environment
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_ENV_SIZE			0x40000          /* 256KB */
#ifdef CONFIG_TFABOOT
#define CONFIG_ENV_OFFSET		0x500000        /* 5MB */
#else
#define CONFIG_ENV_OFFSET		0x300000        /* 3MB */
#endif
#define CONFIG_ENV_SECT_SIZE		0x40000
#endif

/* SATA */
#define CONFIG_SCSI_AHCI_PLAT

#define CONFIG_SYS_SATA				AHCI_BASE_ADDR

#define CONFIG_SYS_SCSI_MAX_SCSI_ID		1
#define CONFIG_SYS_SCSI_MAX_LUN			1
#define CONFIG_SYS_SCSI_MAX_DEVICE		(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)

/* I2C */
#define CONFIG_SYS_I2C

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE     1
#define CONFIG_SYS_NS16550_CLK          (get_serial_clock())

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_SYS_HZ			1000

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"verify=no\0"				\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"fdt_high=0xffffffffffffffff\0"		\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x1000000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\

#undef CONFIG_BOOTCOMMAND
#ifdef CONFIG_TFABOOT
#define QSPI_NOR_BOOTCOMMAND	"pfe stop; sf probe 0:0; sf read $kernel_load "\
				"$kernel_start $kernel_size && "\
				"bootm $kernel_load"
#else
#define CONFIG_BOOTCOMMAND	"pfe stop; sf probe 0:0; sf read $kernel_load "\
				"$kernel_start $kernel_size && "\
				"bootm $kernel_load"
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SYS_BOOTM_LEN   (64 << 20)      /* Increase max gunzip size */

#include <asm/arch/soc.h>

#endif /* __LS1012A_COMMON_H */
