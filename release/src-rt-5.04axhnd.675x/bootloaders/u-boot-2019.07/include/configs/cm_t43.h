/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * cm_t43.h
 *
 * Copyright (C) 2015 Compulab, Ltd.
 */

#ifndef __CONFIG_CM_T43_H
#define __CONFIG_CM_T43_H

#define CONFIG_CM_T43
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_MAX_RAM_BANK_SIZE	(2048 << 20)	/* 2GB */
#define CONFIG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */

#include <asm/arch/omap.h>

/* Serial support */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		48000000
#define CONFIG_SYS_NS16550_COM1		0x44e09000
#if !defined(CONFIG_SPL_DM) || !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#endif

/* NAND support */
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

/* CPSW Ethernet support */
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_PHY_ATHEROS
#define CONFIG_SYS_RX_ETH_BUFFER	64

/* USB support */
#define CONFIG_USB_XHCI_OMAP
#define CONFIG_AM437X_USB2PHY2_HOST

/* Power */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_TPS65218

/* Enabling L2 Cache */
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE		0x48242000

/*
 * Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#if !defined(CONFIG_SPL_BUILD)
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

#define CONFIG_HSMMC2_8BIT

#include <configs/ti_armv7_omap.h>
#undef CONFIG_SYS_MONITOR_LEN

#define CONFIG_ENV_SIZE			(16 * 1024)

#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_OFFSET		(768 * 1024)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80200000\0" \
	"fdtaddr=0x81200000\0" \
	"bootm_size=0x8000000\0" \
	"autoload=no\0" \
	"console=ttyO0,115200n8\0" \
	"fdtfile=am437x-sb-som-t43.dtb\0" \
	"kernel=zImage-cm-t43\0" \
	"bootscr=bootscr.img\0" \
	"emmcroot=/dev/mmcblk0p2 rw\0" \
	"emmcrootfstype=ext4 rootwait\0" \
	"emmcargs=setenv bootargs console=${console} " \
		"root=${emmcroot} " \
		"rootfstype=${emmcrootfstype}\0" \
	"loadbootscript=load mmc 0 ${loadaddr} ${bootscr}\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"emmcboot=echo Booting from emmc ... && " \
		"run emmcargs && " \
		"load mmc 1 ${loadaddr} ${kernel} && " \
		"load mmc 1 ${fdtaddr} ${fdtfile} && " \
		"bootz ${loadaddr} - ${fdtaddr}\0"

#define CONFIG_BOOTCOMMAND \
	"mmc dev 0; " \
	"if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"fi; " \
	"fi; " \
	"mmc dev 1; " \
	"if mmc rescan; then " \
		"run emmcboot; " \
	"fi;"

/* SPL defines. */
#define CONFIG_SYS_SPL_ARGS_ADDR	(CONFIG_SYS_SDRAM_BASE + (128 << 20))
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(256 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

/* EEPROM */
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
#define CONFIG_SYS_EEPROM_SIZE			256

#endif	/* __CONFIG_CM_T43_H */
