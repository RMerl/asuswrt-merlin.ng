/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX7D SABRESD board.
 */

#ifndef __MX7D_SABRESD_CONFIG_H
#define __MX7D_SABRESD_CONFIG_H

#include "mx7_common.h"

#define PHYS_SDRAM_SIZE			SZ_1G

#define CONFIG_MXC_UART_BASE            UART1_IPS_BASE_ADDR

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(32 * SZ_1M)

/* Network */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE             RGMII
#define CONFIG_ETHPRIME                 "FEC"
#define CONFIG_FEC_MXC_PHYADDR          0

#define CONFIG_PHY_BROADCOM
/* ENET1 */
#define IMX_FEC_BASE			ENET_IPS_BASE_ADDR

/* MMC Config*/
#define CONFIG_SYS_FSL_ESDHC_ADDR       0

#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_PLAN9
#undef CONFIG_BOOTM_RTEMS

/* I2C configs */
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		100000

#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

#ifdef CONFIG_IMX_BOOTAUX
/* Set to QSPI1 A flash at default */
#define CONFIG_SYS_AUXCORE_BOOTDATA 0x60000000

#define UPDATE_M4_ENV \
	"m4image=m4_qspi.bin\0" \
	"loadm4image=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${m4image}\0" \
	"update_m4_from_sd=" \
		"if sf probe 0:0; then " \
			"if run loadm4image; then " \
				"setexpr fw_sz ${filesize} + 0xffff; " \
				"setexpr fw_sz ${fw_sz} / 0x10000; "	\
				"setexpr fw_sz ${fw_sz} * 0x10000; "	\
				"sf erase 0x0 ${fw_sz}; " \
				"sf write ${loadaddr} 0x0 ${filesize}; " \
			"fi; " \
		"fi\0" \
	"m4boot=sf probe 0:0; bootaux "__stringify(CONFIG_SYS_AUXCORE_BOOTDATA)"\0"
#else
#define UPDATE_M4_ENV ""
#endif

#define CONFIG_MFG_ENV_SETTINGS \
	"mfgtool_args=setenv bootargs console=${console},${baudrate} " \
		"rdinit=/linuxrc " \
		"g_mass_storage.stall=0 g_mass_storage.removable=1 " \
		"g_mass_storage.idVendor=0x066F g_mass_storage.idProduct=0x37FF "\
		"g_mass_storage.iSerialNumber=\"\" "\
		"clk_ignore_unused "\
		"\0" \
	"initrd_addr=0x83800000\0" \
	"initrd_high=0xffffffff\0" \
	"bootcmd_mfg=run mfgtool_args;bootz ${loadaddr} ${initrd_addr} ${fdt_addr};\0" \

#define CONFIG_DFU_ENV_SETTINGS \
	"dfu_alt_info=image raw 0 0x800000;"\
		"u-boot raw 0 0x4000;"\
		"bootimg part 0 1;"\
		"rootfs part 0 2\0" \

#define CONFIG_EXTRA_ENV_SETTINGS \
	UPDATE_M4_ENV \
	CONFIG_MFG_ENV_SETTINGS \
	CONFIG_DFU_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=imx7d-sdb.dtb\0" \
	"fdt_addr=0x83000000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"videomode=video=ctfb:x:480,y:272,depth:24,pclk:108695,le:8,ri:4,up:2,lo:4,hs:41,vs:10,sync:0,vmode:0\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
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
	   "mmc dev ${mmcdev};" \
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

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x20000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */
#define CONFIG_ENV_SIZE			SZ_8K

/*
 * If want to use nand, define CONFIG_NAND_MXS and rework board
 * to support nand, since emmc has pin conflicts with nand
 */
#ifdef CONFIG_NAND_MXS
/* NAND stuff */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */
#endif

#define CONFIG_ENV_OFFSET		(12 * SZ_64K)
#ifdef CONFIG_NAND_MXS
#define CONFIG_SYS_FSL_USDHC_NUM	1
#else
#define CONFIG_SYS_FSL_USDHC_NUM	2
#endif

#define CONFIG_SYS_MMC_ENV_DEV		0   /* USDHC1 */
#define CONFIG_SYS_MMC_ENV_PART		0	/* user area */
#define CONFIG_MMCROOT			"/dev/mmcblk0p2"  /* USDHC1 */

/* USB Configs */
#define CONFIG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)

#define CONFIG_IMX_THERMAL

#define CONFIG_USBD_HS

#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_MXS
#define CONFIG_VIDEO_LOGO
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_LOGO
#endif

#ifdef CONFIG_FSL_QSPI
#define CONFIG_SYS_FSL_QSPI_AHB
#define FSL_QSPI_FLASH_NUM		1
#define FSL_QSPI_FLASH_SIZE		SZ_64M
#define QSPI0_BASE_ADDR			QSPI1_IPS_BASE_ADDR
#define QSPI0_AMBA_BASE			QSPI0_ARB_BASE_ADDR
#endif

#endif	/* __CONFIG_H */
