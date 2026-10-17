/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 * Copyright 2022 Linaro
 */

#ifndef __IMX8MP_RSB3720_H
#define __IMX8MP_RSB3720_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>
#include <config_distro_bootcmd.h>

#define CFG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

/* GUIDs for capsule updatable firmware images */
#define IMX8MP_RSB3720A1_4G_FIT_IMAGE_GUID \
	EFI_GUID(0xb1251e89, 0x384a, 0x4635, 0xa8, 0x06, \
		 0x3a, 0xa0, 0xb0, 0xe9, 0xf9, 0x65)

#define IMX8MP_RSB3720A1_6G_FIT_IMAGE_GUID \
	EFI_GUID(0xb5fb6f08, 0xe142, 0x4db1, 0x97, 0xea, \
		 0x5f, 0xd3, 0x6b, 0x9b, 0xe5, 0xb9)

#ifdef CONFIG_SPL_BUILD
#define CFG_MALLOC_F_ADDR		0x184000 /* malloc f used before \
						  * GD_FLG_FULL_MALLOC_INIT \
						  * set \
						  */
#endif

/* ENET Config */
/* ENET1 */
#if defined(CONFIG_CMD_NET)
#define CFG_FEC_MXC_PHYADDR          4

#define PHY_ANEG_TIMEOUT 20000

#endif

#if IS_ENABLED(CONFIG_CMD_MMC)
# define BOOT_TARGET_MMC(func) \
	func(MMC, mmc, 2)      \
	func(MMC, mmc, 1)
#else
# define BOOT_TARGET_MMC(func)
#endif

#if CONFIG_IS_ENABLED(CMD_PXE)
# define BOOT_TARGET_PXE(func) func(PXE, pxe, na)
#else
# define BOOT_TARGET_PXE(func)
#endif

#if CONFIG_IS_ENABLED(CMD_DHCP)
# define BOOT_TARGET_DHCP(func) func(DHCP, dhcp, na)
#else
# define BOOT_TARGET_DHCP(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_MMC(func) \
	BOOT_TARGET_PXE(func) \
	BOOT_TARGET_DHCP(func)

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	BOOTENV \
	"script=boot.scr\0" \
	"image=Image\0" \
	"splashimage=0x50000000\0" \
	"console=ttymxc2,115200 earlycon=ec_imx6q,0x30880000,115200\0" \
	"fdt_addr=0x43000000\0"			\
	"fdt_addr_r=0x43000000\0"			\
	"boot_fit=no\0" \
	"dfu_alt_info=mmc 2=flash-bin raw 0 0x1B00 mmcpart 1\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"initrd_addr=0x43800000\0"		\
	"bootm_size=0x10000000\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk1p2 rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs ${jh_clk} console=${console} root=${mmcroot}\0 " \
	"loadbootscript=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"kernel_addr_r=0x40480000\0" \
	"pxefile_addr_r=0x40480000\0" \
	"ramdisk_addr_r=0x43800000\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fit} = yes || test ${boot_fit} = try; then " \
			"bootm ${loadaddr}; " \
		"else " \
			"if run loadfdt; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"fi;\0" \
	"netargs=setenv bootargs ${jh_clk} console=${console} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs;  " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${loadaddr} ${image}; " \
		"if test ${boot_fit} = yes || test ${boot_fit} = try; then " \
			"bootm ${loadaddr}; " \
		"else " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"fi;\0"

/* Link Definitions */
#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	0x80000


/* Totally 6GB or 4G DDR */
#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#if defined(CONFIG_TARGET_IMX8MP_RSB3720A1_6G)
#define PHYS_SDRAM_SIZE			0xC0000000	/* 3 GB */
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		0xC0000000	/* 3 GB */
#elif defined(CONFIG_TARGET_IMX8MP_RSB3720A1_4G)
#define PHYS_SDRAM_SIZE			0x80000000	/* 2 GB */
#define PHYS_SDRAM_2			0xC0000000
#define PHYS_SDRAM_2_SIZE		0x80000000	/* 2 GB */
#endif

#define CFG_MXC_UART_BASE		UART3_BASE_ADDR

#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR	0

#ifdef CONFIG_FSL_FSPI
#define FSL_FSPI_FLASH_SIZE		SZ_32M
#define FSL_FSPI_FLASH_NUM		1
#define FSPI0_BASE_ADDR			0x30bb0000
#define FSPI0_AMBA_BASE			0x0
#endif

#ifdef CONFIG_NAND_MXS

/* NAND stuff */
#define CFG_SYS_NAND_BASE           0x20000000
#endif /* CONFIG_NAND_MXS */

#endif /* __IMX8MP_RSB3720_H */
