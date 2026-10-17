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

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#ifndef CONFIG_SPL_BUILD

#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"tftp_load_addr=0x82000000\0" \
	"kernel_addr_r=0x82000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"ramdisk_addr_r=0x88080000\0" \

#define BOOT_TARGET_DEVICES(func) \
	func(UBIFS, ubifs, 0, UBI, rootfs)

#define AM335XX_BOARD_FDTFILE "fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"

#include <config_distro_bootcmd.h>

#define GUARDIAN_DEFAULT_PROD_ENV \
	"factory_assembly_status=0\0" \
	"main_pcba_part_number=0\0" \
	"main_pcba_supplier=0\0" \
	"main_pcba_timestamp=0\0" \
	"main_pcba_hardware_version=0\0" \
	"main_pcba_id=0\0" \
	"main_pcba_aux_1=0\0" \
	"main_pcba_aux_2=0\0" \
	"main_pcba_aux_3=0\0" \
	"main_pcba_aux_4=0\0" \

#define CFG_EXTRA_ENV_SETTINGS \
	AM335XX_BOARD_FDTFILE \
	MEM_LAYOUT_ENV_SETTINGS \
	BOOTENV \
	GUARDIAN_DEFAULT_PROD_ENV \
	"backlight_brightness=50\0" \
	"distro_bootcmd=" \
		"setenv rootflags \"bulk_read,chk_data_crc\"; " \
		"setenv ethact usb_ether; " \
		"if test \"${swi_status}\" -eq 1; then " \
		  "if dhcp; then " \
		    "sleep 1; " \
		    "if tftp \"${tftp_load_addr}\" \"bootscript.scr\"; then " \
		      "source \"${tftp_load_addr}\"; " \
		    "fi; " \
		  "fi; " \
		  "setenv extrabootargs $extrabootargs \"swi_attached\"; " \
		"fi;" \
		"run bootcmd_ubifs0;\0" \
	"altbootcmd=" \
		"setenv boot_syslinux_conf \"extlinux/extlinux-rollback.conf\"; " \
		"run distro_bootcmd; " \
		"setenv boot_syslinux_conf \"extlinux/extlinux.conf\"; " \
		"run bootcmd_ubifs0;\0"

#endif /* ! CONFIG_SPL_BUILD */

#define SPLASH_SCREEN_NAND_PART "nand0,10"
#define SPLASH_SCREEN_BMP_FILE_SIZE 0x26000
#define SPLASH_SCREEN_BMP_LOAD_ADDR 0x82000000
#define SPLASH_SCREEN_TEXT "U-Boot"

/* BGR 16Bit Color Definitions */
#define CONSOLE_COLOR_BLACK 0x0000
#define CONSOLE_COLOR_WHITE 0xFFFF
#define CONSOLE_COLOR_RED 0x001F

/* NS16550 Configuration */
#define CFG_SYS_NS16550_COM1		0x44e09000	/* UART0 */
#define CFG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CFG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CFG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CFG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CFG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

#ifdef CONFIG_MTD_RAW_NAND
#define CFG_SYS_NAND_ECCPOS  {   2,   3,   4,   5,   6,   7,   8,   9, \
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
#define CFG_SYS_NAND_ECCSIZE         512
#define CFG_SYS_NAND_ECCBYTES        26

#endif /* CONFIG_MTD_RAW_NAND */

#endif	/* ! __CONFIG_AM335X_GUARDIAN_H */
