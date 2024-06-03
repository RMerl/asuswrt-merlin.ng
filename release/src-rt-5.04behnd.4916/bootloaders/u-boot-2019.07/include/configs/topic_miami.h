/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Topic Embedded Products
 *
 * Configuration for Zynq Evaluation and Development Board - Miami
 * See zynq-common.h for Zynq common configs
 */

#ifndef __CONFIG_TOPIC_MIAMI_H
#define __CONFIG_TOPIC_MIAMI_H


/* Speed up boot time by ignoring the environment which we never used */

#include "zynq-common.h"

/* Fixup settings */
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE			0x8000
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET		0x80000

/* SPL settings */
#undef CONFIG_SPL_ETH_SUPPORT
#undef CONFIG_SYS_SPI_U_BOOT_OFFS
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#undef CONFIG_SPL_MAX_FOOTPRINT
#define CONFIG_SPL_MAX_FOOTPRINT	CONFIG_SYS_SPI_U_BOOT_OFFS
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME     "u-boot.img"

/* sspi command isn't useful */
#undef CONFIG_CMD_SPI

/* No useful gpio */
#undef CONFIG_ZYNQ_GPIO
#undef CONFIG_CMD_GPIO

/* No falcon support */
#undef CONFIG_SPL_OS_BOOT
#undef CONFIG_SPL_FPGA_SUPPORT

/* FPGA commands that we don't use */

/* Extras */
#undef CONFIG_SYS_MEMTEST_START
#define CONFIG_SYS_MEMTEST_START	0
#undef CONFIG_SYS_MEMTEST_END
#define CONFIG_SYS_MEMTEST_END	0x18000000

/* Faster flash, ours may run at 108 MHz */
#undef CONFIG_SPI_FLASH_WINBOND

/* Setup proper boot sequences for Miami boards */

#if defined(CONFIG_USB)
# define EXTRA_ENV_USB \
	"usbreset=i2c dev 1 && i2c mw 41 1 ff && i2c mw 41 3 fe && "\
		"i2c mw 41 1 fe && i2c mw 41 1 ff\0" \
	"usbboot=run usbreset && if usb start; then " \
		"echo Booting from USB... && " \
		"if load usb 0 0x1900000 ${bootscript}; then "\
		"source 0x1900000; fi; " \
		"load usb 0 ${kernel_addr} ${kernel_image} && " \
		"load usb 0 ${devicetree_addr} ${devicetree_image} && " \
		"load usb 0 ${ramdisk_load_address} ${ramdisk_image} && " \
		"bootm ${kernel_addr} ${ramdisk_load_address} "\
			"${devicetree_addr}; " \
	"fi\0"
  /* Note that addresses here should match the addresses in the env */
# undef DFU_ALT_INFO
# define DFU_ALT_INFO \
	"dfu_alt_info=" \
	"uImage ram 0x2080000 0x500000;" \
	"devicetree.dtb ram 0x2000000 0x20000;" \
	"uramdisk.image.gz ram 0x4000000 0x10000000\0" \
	"dfu_ram=run usbreset && dfu 0 ram 0\0" \
	"thor_ram=run usbreset && thordown 0 ram 0\0"
#else
# define EXTRA_ENV_USB
#endif

#undef CONFIG_PREBOOT

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"kernel_image=uImage\0"	\
	"kernel_addr=0x2080000\0" \
	"ramdisk_image=uramdisk.image.gz\0"	\
	"ramdisk_load_address=0x4000000\0"	\
	"devicetree_image=devicetree.dtb\0"	\
	"devicetree_addr=0x2000000\0"	\
	"bitstream_image=fpga.bin\0"	\
	"bootscript=autorun.scr\0" \
	"loadbit_addr=0x100000\0"	\
	"loadbootenv_addr=0x2000000\0" \
	"kernel_size=0x440000\0"	\
	"devicetree_size=0x10000\0"	\
	"boot_size=0xF00000\0"	\
	"fdt_high=0x20000000\0"	\
	"initrd_high=0x20000000\0"	\
	"mmc_loadbit=echo Loading bitstream from SD/MMC/eMMC to RAM.. && " \
		"mmcinfo && " \
		"load mmc 0 ${loadbit_addr} ${bitstream_image} && " \
		"fpga load 0 ${loadbit_addr} ${filesize}\0" \
	"qspiboot=echo Booting from QSPI flash... && " \
		"sf probe && " \
		"sf read ${devicetree_addr} 0xA0000 ${devicetree_size} && " \
		"sf read ${kernel_addr} 0xC0000 ${kernel_size} && " \
		"bootm ${kernel_addr} - ${devicetree_addr}\0" \
	"sdboot=if mmcinfo; then " \
			"setenv bootargs console=ttyPS0,115200 " \
				"root=/dev/mmcblk0p2 rw rootfstype=ext4 " \
				"rootwait quiet ; " \
			"load mmc 0 ${kernel_addr} ${kernel_image}&& " \
			"load mmc 0 ${devicetree_addr} ${devicetree_image}&& " \
			"bootm ${kernel_addr} - ${devicetree_addr}; " \
		"fi\0" \
	EXTRA_ENV_USB \
	DFU_ALT_INFO

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND	"if mmcinfo; then " \
	"if fatload mmc 0 0x1900000 ${bootscript}; then source 0x1900000; " \
	"fi; fi; run $modeboot"
#undef CONFIG_DISPLAY_BOARDINFO

#endif /* __CONFIG_TOPIC_MIAMI_H */
