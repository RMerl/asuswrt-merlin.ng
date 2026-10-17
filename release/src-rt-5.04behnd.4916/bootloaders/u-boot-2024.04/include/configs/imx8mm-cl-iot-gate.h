/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#ifndef __IMX8MM_CL_IOT_GATE_H
#define __IMX8MM_CL_IOT_GATE_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>
#include <config_distro_bootcmd.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CFG_MALLOC_F_ADDR		0x912000
/* For RAW image gives a error info not panic */

#endif

/* GUIDs for capsule updatable firmware images */
#define IMX8MM_CL_IOT_GATE_FIT_IMAGE_GUID \
	EFI_GUID(0x7a32a939, 0xab92, 0x467b, 0x91, 0x52, \
		 0x74, 0x77, 0x1b, 0x95, 0xe6, 0x46)

#define IMX8MM_CL_IOT_GATE_OPTEE_FIT_IMAGE_GUID \
	EFI_GUID(0x0bf1165c, 0x1831, 0x4864, 0x94, 0x5e, \
		 0xac, 0x3d, 0x38, 0x48, 0xf4, 0x99)

#if IS_ENABLED(CONFIG_CMD_MMC)
# define BOOT_TARGET_MMC(func) \
	func(MMC, mmc, 2)      \
	func(MMC, mmc, 0)
#else
# define BOOT_TARGET_MMC(func)
#endif

#if IS_ENABLED(CONFIG_CMD_USB)
# define BOOT_TARGET_USB(func) func(USB, usb, 0)
#else
# define BOOT_TARGET_USB(func)
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
	BOOT_TARGET_USB(func) \
	BOOT_TARGET_MMC(func) \
	BOOT_TARGET_PXE(func) \
	BOOT_TARGET_DHCP(func)

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	BOOTENV \
	"script=boot.scr\0" \
	"image=Image\0" \
	"console=ttymxc2,115200 earlycon=ec_imx6q,0x30880000,115200\0" \
	"fdt_addr=0x43000000\0"			\
	"fdt_addr_r=0x43000000\0" \
	"boot_fit=no\0" \
	"dfu_alt_info=mmc 2=flash-bin raw 0x42 0x1D00 mmcpart 1\0" \
	"fdt_file=sb-iotgimx8.dtb\0" \
	"fdtfile=sb-iotgimx8.dtb\0" \
	"initrd_addr=0x43800000\0"		\
	"bootm_size=0x10000000\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk1p2 rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console} root=${mmcroot}\0 " \
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
	"netargs=setenv bootargs console=${console} " \
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


#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x80000000 /* 2GB DDR */

/* USDHC */

#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR	0

#define CFG_FEC_MXC_PHYADDR		0

/* USB Configs */
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)

#endif /*__IMX8MM_CL_IOT_GATE_H*/
