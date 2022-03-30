/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015  Beckhoff Automation GmbH & Co. KG
 * Patrick Bruenn <p.bruenn@beckhoff.com>
 *
 * Configuration settings for Beckhoff CX9020.
 *
 * Based on Freescale's Linux i.MX mx53loco.h file:
 * Copyright (C) 2010-2011 Freescale Semiconductor.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_SYS_FSL_CLK

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)

#define CONFIG_REVISION_TAG

#define CONFIG_MXC_UART_BASE UART2_BASE

#define CONFIG_FPGA_COUNT 1

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	2

/* bootz: zImage/initrd.img support */

/* Eth Configs */
#define IMX_FEC_BASE	FEC_BASE_ADDR
#define CONFIG_ETHPRIME		"FEC0"
#define CONFIG_FEC_MXC_PHYADDR	0x1F

/* USB Configs */
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Command definition */

#define CONFIG_LOADADDR		0x70010000	/* loadaddr env var */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_addr_r=0x71ff0000\0" \
	"pxefile_addr_r=0x73000000\0" \
	"ramdisk_addr_r=0x72000000\0" \
	"console=ttymxc1,115200\0" \
	"uenv=/boot/uEnv.txt\0" \
	"optargs=\0" \
	"cmdline=\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcrootfstype=ext4 rootwait fixrtc\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/mmcblk${mmcdev}p${mmcpart} ro " \
		"rootfstype=${mmcrootfstype} " \
		"${cmdline}\0" \
	"loadimage=load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"loadpxe=dhcp;setenv kernel_addr_r ${loadaddr};pxe get;pxe boot;\0" \
	"loadrd=load mmc ${bootpart} ${ramdisk_addr_r} ${bootdir}/${rdfile};" \
		"setenv rdsize ${filesize}\0" \
	"loadfdt=echo loading ${fdt_path} ...;" \
		"load mmc ${bootpart} ${fdt_addr_r} ${fdt_path}\0" \
	"mmcboot=mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"echo Checking for: ${uenv} ...;" \
			"setenv bootpart ${mmcdev}:${mmcpart};" \
			"if test -e mmc ${bootpart} ${uenv}; then " \
				"load mmc ${bootpart} ${loadaddr} ${uenv};" \
				"env import -t ${loadaddr} ${filesize};" \
				"echo Loaded environment from ${uenv};" \
				"if test -n ${dtb}; then " \
					"setenv fdt_file ${dtb};" \
					"echo Using: dtb=${fdt_file} ...;" \
				"fi;" \
				"echo Checking for uname_r in ${uenv}...;" \
				"if test -n ${uname_r}; then " \
					"echo Running uname_boot ...;" \
					"run uname_boot;" \
				"fi;" \
			"fi;" \
		"fi;\0" \
	"uname_boot="\
		"setenv bootdir /boot; " \
		"setenv bootfile vmlinuz-${uname_r}; " \
		"setenv ccatfile /boot/ccat.rbf; " \
		"echo loading CCAT firmware from ${ccatfile}; " \
		"load mmc ${bootpart} ${loadaddr} ${ccatfile}; " \
		"fpga load 0 ${loadaddr} ${filesize}; " \
		"if test -e mmc ${bootpart} ${bootdir}/${bootfile}; then " \
			"echo loading ${bootdir}/${bootfile} ...; " \
			"run loadimage;" \
			"setenv fdt_path /boot/dtbs/${uname_r}/${fdt_file}; " \
			"if test -e mmc ${bootpart} ${fdt_path}; then " \
				"run loadfdt;" \
			"else " \
				"echo; echo unable to find ${fdt_file} ...;" \
				"echo booting legacy ...;"\
				"run mmcargs;" \
				"echo debug: [${bootargs}] ... ;" \
				"echo debug: [bootz ${loadaddr}] ... ;" \
				"bootz ${loadaddr}; " \
			"fi;" \
			"run mmcargs;" \
			"echo debug: [${bootargs}] ... ;" \
			"echo debug: [bootz ${loadaddr} - ${fdt_addr_r}];" \
			"bootz ${loadaddr} - ${fdt_addr_r}; " \
		"else " \
			"echo loading from dhcp ...; " \
			"run loadpxe; " \
		"fi;\0"

#define CONFIG_BOOTCOMMAND \
	"run mmcboot;"

#define CONFIG_ARP_TIMEOUT	200UL

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */

#define CONFIG_SYS_MEMTEST_START       0x70000000
#define CONFIG_SYS_MEMTEST_END         0x70010000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/* Physical Memory Map */
#define PHYS_SDRAM_1			CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE		(gd->bd->bi_dram[0].size)
#define PHYS_SDRAM_2			CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE		(gd->bd->bi_dram[1].size)
#define PHYS_SDRAM_SIZE			(gd->ram_size)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */
#define CONFIG_ENV_OFFSET      (6 * 64 * 1024)
#define CONFIG_ENV_SIZE        (8 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV 0

/* Framebuffer and LCD */
#define CONFIG_IMX_VIDEO_SKIP
#define CONFIG_PREBOOT

#endif /* __CONFIG_H */
