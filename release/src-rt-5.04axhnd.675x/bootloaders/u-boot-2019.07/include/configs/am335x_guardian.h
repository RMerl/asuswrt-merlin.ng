/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am335x_guardian_.h
 *
 * Copyright (C) 2018 Robert Bosch Power Tools GmbH
 * Copyright (C) 2018 sjoerd Simons <sjoerd.simons@collabora.co.uk>
 *
 */

#ifndef __CONFIG_AM335X_GUARDIAN_H
#define __CONFIG_AM335X_GUARDIAN_H

#include <configs/ti_am335x_common.h>

#ifndef CONFIG_SPL_BUILD
#define CONFIG_TIMESTAMP
#endif

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#ifndef CONFIG_SPL_BUILD

#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"kernel_addr_r=0x82000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"ramdisk_addr_r=0x88080000\0" \

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(UBIFS, ubifs, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#define AM335XX_BOARD_FDTFILE "fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	AM335XX_BOARD_FDTFILE \
	MEM_LAYOUT_ENV_SETTINGS \
	BOOTENV \
	"bootlimit=3\0" \
	"altbootcmd=" \
		"setenv boot_config \"extlinux-rollback.conf\"; " \
		"run distro_bootcmd\0"

#endif /* CONFIG_SPL_BUILD */

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

/* PMIC support */
#define CONFIG_POWER_TPS65217

/* Bootcount using the RTC block */
#define CONFIG_SYS_BOOTCOUNT_LE

#ifdef CONFIG_NAND
#define CONFIG_ENV_OFFSET		0x300000
#define CONFIG_ENV_OFFSET_REDUND	0x340000
#define CONFIG_ENV_SIZE			0x040000

#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT      (CONFIG_SYS_NAND_BLOCK_SIZE / \
					CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE       4096
#define CONFIG_SYS_NAND_OOBSIZE         256
#define CONFIG_SYS_NAND_BLOCK_SIZE      (256 * 1024)

#define CONFIG_SYS_NAND_ECCPOS  {   2,   3,   4,   5,   6,   7,   8,   9, \
			 10,  11,  12,  13,  14,  15,  16,  17,  18,  19, \
			 20,  21,  22,  23,  24,  25,  26,  27,  28,  29, \
			 30,  31,  32,  33,  34,  35,  36,  37,  38,  39, \
			 40,  41,  42,  43,  44,  45,  46,  47,  48,  49, \
			 50,  51,  52,  53,  54,  55,  56,  57,  58,  59, \
			 60,  61,  62,  63,  64,  65,  66,  67,  68,  69, \
			 70,  71,  72,  73,  74,  75,  76,  77,  78,  79, \
			 80,  81,  82,  83,  84,  85,  86,  87,  88,  89, \
			 90,  91,  92,  93,  94,  95,  96,  97,  98,  99, \
			100, 101, 102, 103, 104, 105, 106, 107, 108, 109, \
			110, 111, 112, 113, 114, 115, 116, 117, 118, 119, \
			120, 121, 122, 123, 124, 125, 126, 127, 128, 129, \
			130, 131, 132, 133, 134, 135, 136, 137, 138, 139, \
			140, 141, 142, 143, 144, 145, 146, 147, 148, 149, \
			150, 151, 152, 153, 154, 155, 156, 157, 158, 159, \
			160, 161, 162, 163, 164, 165, 166, 167, 168, 169, \
			170, 171, 172, 173, 174, 175, 176, 177, 178, 179, \
			180, 181, 182, 183, 184, 185, 186, 187, 188, 189, \
			190, 191, 192, 193, 194, 195, 196, 197, 198, 199, \
			200, 201, 202, 203, 204, 205, 206, 207, 208, 209, \
			}
#define CONFIG_SYS_NAND_ECCSIZE         512
#define CONFIG_SYS_NAND_ECCBYTES        26
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_OMAP_ECCSCHEME      OMAP_ECC_BCH16_CODE_HW
#define MTDIDS_DEFAULT                  "nand0=nand.0"

#define CONFIG_SYS_NAND_BAD_BLOCK_POS   NAND_LARGE_BADBLOCK_POS

#endif /* CONFIG_NAND */

#endif	/* ! __CONFIG_AM335X_GUARDIAN_H */
